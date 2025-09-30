from behave import given, when, then
from collections import defaultdict
from datetime import datetime, timedelta

# NOTE: Pure BDD-first placeholders. Real logic will move to app/controller layers later.
# We simulate a simple in-memory weather model the steps can populate & assert against.

class WeatherModel:
    def __init__(self):
        self.current = {}
        self.hours = []  # list of dicts: hour(int 0-23), temp_c, icon, condition
        self.days = []   # list of dicts: day(str), hi_c, lo_c, icon, condition
        self.view = 'hourly'  # 'hourly' or 'multiday'
        self.poll_interval = 900  # seconds
        self.last_fetch_age = None  # seconds since last fetch (simulated)
        self.cached_exists = False
        self.retry_attempts = []
        self.now = None  # datetime

    def is_stale(self):
        if self.last_fetch_age is None:
            return True
        return self.last_fetch_age > self.poll_interval * 2


def get_model(context) -> WeatherModel:
    if not hasattr(context, 'weather_model'):
        context.weather_model = WeatherModel()
    return context.weather_model


# ------------------------------ GIVEN ---------------------------------

@given('the weather source returns for now:')
def step_impl(context):
    model = get_model(context)
    row = context.table[0]
    model.current = {
        'temp_c': float(row['temp_c']),
        'feels_c': float(row['feels_c']),
        'condition': row['condition'],
        'icon': row['icon']
    }
    model.cached_exists = True
    model.last_fetch_age = 0


@given('the weather source returns the next hours:')
def step_impl(context):
    model = get_model(context)
    model.hours = []
    for row in context.table:
        model.hours.append({
            'hour': int(row['hour']),
            'temp_c': float(row['temp_c']),
            'icon': row['icon'],
            'condition': row['condition']
        })
    model.cached_exists = True
    if model.last_fetch_age is None:
        model.last_fetch_age = 0


@given('the weather source returns multi-day forecast:')
def step_impl(context):
    model = get_model(context)
    model.days = []
    for row in context.table:
        model.days.append({
            'day': row['day'],
            'hi_c': float(row['hi_c']),
            'lo_c': float(row['lo_c']),
            'icon': row['icon'],
            'condition': row['condition']
        })
    model.cached_exists = True
    if model.last_fetch_age is None:
        model.last_fetch_age = 0


@given('the current time is {timestr}')
def step_impl(context, timestr):
    model = get_model(context)
    # Interpret HH:MM in an arbitrary date anchor
    model.now = datetime(2024, 1, 1, int(timestr[0:2]), int(timestr[3:5]), 0)


@given('the poll interval is {seconds:d} seconds')
def step_impl(context, seconds):
    model = get_model(context)
    model.poll_interval = seconds


@given('the last successful fetch was {age:d} seconds ago')
def step_impl(context, age):
    model = get_model(context)
    model.last_fetch_age = age


@given('there is no cached weather data')
def step_impl(context):
    model = get_model(context)
    model.cached_exists = False
    model.current = {}
    model.hours = []
    model.days = []
    model.last_fetch_age = None


@given('I am viewing the multi-day forecast')
def step_impl(context):
    model = get_model(context)
    model.view = 'multiday'


@given('cached weather data exists from {age:d} seconds ago')
def step_impl(context, age):
    model = get_model(context)
    model.cached_exists = True
    model.last_fetch_age = age
    if not model.current:
        # Provide minimal placeholder current so assertions can proceed
        model.current = {'temp_c': 0.0, 'feels_c': 0.0, 'condition': 'Unknown', 'icon': 'generic'}


@given('the last 2 fetch attempts failed with error codes')
def step_impl(context):
    model = get_model(context)
    model.retry_attempts = []
    for row in context.table:
        model.retry_attempts.append({'attempt': int(row['attempt']), 'code': int(row['code'])})

# ------------------------------ WHEN ----------------------------------

## Removed local 'I tap the screen' to avoid ambiguity; generic step in app_steps.py handles toggling.


@when('{elapsed:d} seconds elapse')
def step_impl(context, elapsed):
    model = get_model(context)
    if model.last_fetch_age is not None:
        model.last_fetch_age += elapsed
    else:
        # No data yet; treat as waiting
        pass

# ------------------------------ THEN ----------------------------------

@then('the weather current temperature is {temp:f}°C')
def step_impl(context, temp):
    model = get_model(context)
    assert model.current.get('temp_c') == temp, f"Expected temp {temp}, got {model.current.get('temp_c')}"


@then('the weather current feels-like is {temp:f}°C')
def step_impl(context, temp):
    model = get_model(context)
    assert model.current.get('feels_c') == temp


@then('the weather current condition text is "{text}"')
def step_impl(context, text):
    model = get_model(context)
    assert model.current.get('condition') == text


@then('the weather current icon is {icon}')
def step_impl(context, icon):
    model = get_model(context)
    actual = model.current.get('icon')
    expected = icon
    if icon == 'generic':
        # Accept generic fallback codes
        assert actual in ('generic', 'unknown', 'na'), f"Expected a generic icon fallback, got {actual}"
    else:
        assert actual == expected, f"Expected icon {expected}, got {actual}"


@then('the hourly ring has {count:d} segments')
def step_impl(context, count):
    model = get_model(context)
    # Display layer would pad to 8; we simulate by computing padded length
    padded = list(model.hours)
    while len(padded) < 8:
        padded.append(None)
    assert len(padded) == count, f"Expected {count} segments, got {len(padded)}"


@then('the segment at top corresponds to hour {hour:d}')
def step_impl(context, hour):
    model = get_model(context)
    assert model.hours, 'No hourly data set'
    top = model.hours[0]['hour']
    assert top == hour, f"Expected top hour {hour}, got {top}"


@then('the hours proceed clockwise in ascending order')
def step_impl(context):
    model = get_model(context)
    hours = [h['hour'] for h in model.hours]
    for i in range(1, len(hours)):
        prev = hours[i-1]
        curr = hours[i]
        # Allow wrap from 23 -> 0
        if prev == 23:
            assert curr in (0, 23) or curr > prev, f"Unexpected wrap sequence {prev}->{curr}"
        else:
            assert (curr > prev) or (prev == 23 and curr == 0), f"Hours not ascending: {prev}->{curr}"


@then('each segment shows an icon and temperature')
def step_impl(context):
    model = get_model(context)
    for h in model.hours:
        assert 'icon' in h and h['icon'], 'Missing icon'
        assert 'temp_c' in h, 'Missing temp'


@then('the sequence includes hour {hour:d} after {prev:d}')
def step_impl(context, hour, prev):
    model = get_model(context)
    hours = [h['hour'] for h in model.hours]
    for i in range(len(hours)-1):
        if hours[i] == prev:
            assert hours[i+1] == hour, f"Expected hour {hour} after {prev}, got {hours[i+1]}"
            return
    assert False, f"Did not find previous hour {prev} in sequence"


@then('the multi-day forecast is shown')
def step_impl(context):
    model = get_model(context)
    assert model.view == 'multiday'


@then('the daily forecast shows {count:d} days')
def step_impl(context, count):
    model = get_model(context)
    assert len(model.days) == count, f"Expected {count} days, got {len(model.days)}"


@then('each day shows hi/lo, icon, and condition text')
def step_impl(context):
    model = get_model(context)
    for d in model.days:
        for key in ('hi_c', 'lo_c', 'icon', 'condition'):
            assert key in d, f"Day entry missing {key}"


@then('the hourly ring is shown again')
def step_impl(context):
    model = get_model(context)
    assert model.view == 'hourly'


@then('a weather fetch is triggered')
def step_impl(context):
    model = get_model(context)
    # Placeholder: In real test we would detect a fetch call via mock/flag
    # For now, assert that time threshold passed
    assert model.last_fetch_age is not None and model.last_fetch_age >= model.poll_interval, \
        'Fetch should be triggered after interval exceeded'


@then('the weather data is marked stale')
def step_impl(context):
    model = get_model(context)
    assert model.is_stale(), 'Expected data to be stale'


@then('{count:d} segments are empty placeholders')
def step_impl(context, count):
    model = get_model(context)
    empty = 0
    padded_len = 8
    if len(model.hours) < padded_len:
        empty = padded_len - len(model.hours)
    assert empty == count, f"Expected {count} empty placeholders, got {empty}"


@then('the weather shows a loading indicator')
def step_impl(context):
    model = get_model(context)
    assert not model.cached_exists, 'Loading indicator only when no cache'


@then('no immediate fetch is performed')
def step_impl(context):
    model = get_model(context)
    # Without a real fetch tracker just ensure cache is fresh (< poll interval)
    assert model.last_fetch_age is not None and model.last_fetch_age < model.poll_interval, \
        'Cache should prevent immediate fetch'


@then('the current conditions are displayed from cache')
def step_impl(context):
    model = get_model(context)
    assert model.cached_exists and model.current, 'Expected cached current conditions'


@then('a retry is scheduled with backoff')
def step_impl(context):
    model = get_model(context)
    # Placeholder heuristic: more than 1 failure implies backoff path considered
    assert len(model.retry_attempts) >= 2, 'Need at least two failed attempts for backoff'


# Append extended spec state additions
try:
    WeatherModel  # type: ignore
except NameError:
    class WeatherModel:  # fallback if file structure changed
        pass

# Extend WeatherModel with new attributes if not already present
if not hasattr(WeatherModel, 'icon_code_under_test'):
    WeatherModel.icon_code_under_test = None  # type: ignore
if not hasattr(WeatherModel, 'precip_hours'):
    WeatherModel.precip_hours = []  # type: ignore
if not hasattr(WeatherModel, 'missing_hi_lo_days'):
    WeatherModel.missing_hi_lo_days = []  # type: ignore
if not hasattr(WeatherModel, 'last_view'):
    WeatherModel.last_view = None  # type: ignore
if not hasattr(WeatherModel, 'consecutive_failures'):
    WeatherModel.consecutive_failures = 0  # type: ignore
if not hasattr(WeatherModel, 'overnight_low'):
    WeatherModel.overnight_low = None  # type: ignore
if not hasattr(WeatherModel, 'overnight_dew'):
    WeatherModel.overnight_dew = None  # type: ignore
if not hasattr(WeatherModel, 'radiative_adjust'):
    WeatherModel.radiative_adjust = 0.0  # type: ignore
if not hasattr(WeatherModel, 'last_computed_risk'):
    WeatherModel.last_computed_risk = None  # type: ignore
if not hasattr(WeatherModel, 'adjusted_low'):
    WeatherModel.adjusted_low = None  # type: ignore

# Utility to fetch model from context (reuse existing get_model if present)
try:
    get_model  # type: ignore
except NameError:
    def get_model(context):
        if not hasattr(context, 'weather_model'):
            context.weather_model = WeatherModel()
        return context.weather_model

# ------------------------------ GIVEN (extended) ----------------------
@given('weather icon code {code}')
def step_impl(context, code):
    m = get_model(context)
    m.icon_code_under_test = code.strip('"')


@given('I last viewed the multi-day view')
def step_impl(context):
    m = get_model(context)
    m.last_view = 'multiday'

@given('3 consecutive failed fetch attempts')
def step_impl(context):
    m = get_model(context)
    m.consecutive_failures = 3

@given('overnight forecast low is {temp:d}°C')
def step_impl(context, temp):
    m = get_model(context)
    m.overnight_low = float(temp)

@given('overnight dew point is {temp:d}°C')
def step_impl(context, temp):
    m = get_model(context)
    m.overnight_dew = float(temp)

@given('radiative cooling adjustment is {adj:f}°C')
def step_impl(context, adj):
    m = get_model(context)
    m.radiative_adjust = float(adj)

# ------------------------------ WHEN (extended) -----------------------
@when('I open the Weather app')
def step_impl(context):
    # Placeholder: ensure model exists; could set active app if needed
    m = get_model(context)
    if m.last_view:
        m.view = m.last_view

@when('I request the overnight ice risk')
def step_impl(context):
    m = get_model(context)
    if m.overnight_low is None or m.overnight_dew is None:
        raise AssertionError('Overnight low/dew point not provided')
    # Apply radiative adjustment (may be zero)
    adjusted = m.overnight_low + getattr(m, 'radiative_adjust', 0.0)
    m.adjusted_low = adjusted
    # Heuristic classification
    # High: adjusted <=0 and adjusted <= dew + 1
    # Medium: 0 < adjusted <=2 and adjusted <= dew + 1.5
    # Low: else
    if adjusted <= 0 and adjusted <= m.overnight_dew + 1.0:
        risk = 'High'
    elif 0 < adjusted <= 2 and adjusted <= m.overnight_dew + 1.5:
        risk = 'Medium'
    else:
        risk = 'Low'
    m.last_computed_risk = risk

# ------------------------------ THEN (extended) -----------------------
@then('the radial segment for offset {offset:d} is at 12 o\'clock')
def step_impl(context, offset):
    # Placeholder: Not implementing geometry yet
    assert offset == 0, 'Only offset 0 validated in placeholder'

@then('the radial segment for offset {offset:d} is near 3 o\'clock')
@then('the radial segment for offset {offset:d} is near 6 o\'clock')
@then('the radial segment for offset {offset:d} is near 9 o\'clock (not shown if only 8 hours)')
@then('radial segment positioning is monotonic clockwise')
def step_impl(context, offset=None):
    # Placeholder: mark as pending until geometry implemented
    raise AssertionError('Radial positioning verification not implemented')

@then('the icon asset for {code} is used')
def step_impl(context, code):
    m = get_model(context)
    # Placeholder: simply echo; real implementation would map to asset set
    assert m.icon_code_under_test == code, f"Expected code {code}, got {m.icon_code_under_test}"

@then('the generic weather fallback icon is used')
def step_impl(context):
    m = get_model(context)
    # Accept any non-mapped code as generic
    assert m.icon_code_under_test not in ('sun','cloud','rain','showers','snow','fog'), 'Code unexpectedly mapped'

@then('hours with precip_pct show a precipitation badge')
@then('hours without precip_pct show no precipitation badge')
def step_impl(context):
    # For now just placeholder; full implementation would inspect hour dicts
    raise AssertionError('Precipitation badge logic not implemented')

@then('missing temperature values are rendered as dashes')
def step_impl(context):
    # Placeholder acceptance
    raise AssertionError('Missing hi/lo dash rendering not implemented')

@then('the multi-day view is visible')
def step_impl(context):
    m = get_model(context)
    assert m.view == 'multiday', f'Expected multiday view, got {m.view}'

@then('an exponential backoff tier is selected')
def step_impl(context):
    m = get_model(context)
    # Placeholder: verify failures sufficient to trigger tiering
    assert m.consecutive_failures >= 3, 'Not enough failures for backoff tier'

@then('the ice risk probability is {risk}')
def step_impl(context, risk):
    m = get_model(context)
    assert m.last_computed_risk == risk, f"Expected risk {risk}, got {m.last_computed_risk}"

@then('the adjusted forecast low is {val:f}°C')
def step_impl(context, val):
    m = get_model(context)
    assert m.adjusted_low is not None, 'Adjusted low not computed'
    assert abs(m.adjusted_low - val) < 0.001, f"Expected adjusted {val}, got {m.adjusted_low}"

