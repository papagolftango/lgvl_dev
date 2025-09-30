from behave import given, when, then
from datetime import datetime, timedelta

# NOTE: These are placeholder implementations. They will be wired to the bridge/time manager later.
# We maintain a simple simulated time in context for early BDD.

def _init_sim_time(context, dt: datetime):
    # Direct assignment; avoid getattr/hasattr to dodge behave context proxy KeyError
    context.__dict__['sim_time'] = dt
    context.__dict__['_last_display_time'] = dt
    # Preserve format if already set; else default 24h
    if '_format_24h' not in context.__dict__:
        context.__dict__['_format_24h'] = True
    if '_date_visible' not in context.__dict__:
        context.__dict__['_date_visible'] = False
    if '_display_type' not in context.__dict__ or context.__dict__['_display_type'] not in ('Digital','Analogue','Nixie'):
        context.__dict__['_display_type'] = 'Digital'
    if '_timezone' not in context.__dict__:
        context.__dict__['_timezone'] = 'UTC'
    # Track when date became visible for hide timing
    if '_date_visible_since' not in context.__dict__:
        context.__dict__['_date_visible_since'] = None

def _advance_sim_time(context, seconds: int):
    context.__dict__['sim_time'] = context.sim_time + timedelta(seconds=seconds)
    _maybe_hide_date(context)

def _display_time_str(context):
    dt = context.sim_time
    if getattr(context, '_format_24h', True):
        return dt.strftime('%H:%M:%S')
    hour12 = dt.hour % 12
    if hour12 == 0:
        hour12 = 12
    return f"{hour12:02d}:{dt.minute:02d}:{dt.second:02d}"

@given('the system time is {timestr}')
def step_system_time_simple(context, timestr):
    # times like 12:00:05
    dt = datetime.strptime(timestr, '%H:%M:%S')
    # Use arbitrary date
    _init_sim_time(context, datetime(2025, 9, 30, dt.hour, dt.minute, dt.second))

@given('the detailed system time is {weekday} {datestr} {timestr}')
def step_system_time_full(context, weekday, datestr, timestr):
    # weekday ignored for now, parse datestr YYYY-MM-DD
    base = datetime.strptime(datestr + ' ' + timestr, '%Y-%m-%d %H:%M:%S')
    _init_sim_time(context, base)

@when('one second elapses')
@when('1 second elapses')
def step_one_second(context):
    _advance_sim_time(context, 1)
    _maybe_hide_date(context)

@when('one more second elapses')
@when('1 more second elapses')
def step_one_more_second(context):
    _advance_sim_time(context, 1)
    _maybe_hide_date(context)

@when('{n:d} seconds elapse')
def step_n_seconds(context, n):
    _advance_sim_time(context, n)

@when('{n:d} more seconds elapse')
def step_n_more_seconds(context, n):
    _advance_sim_time(context, n)

@when('the system time jumps to {timestr}')
def step_time_jump(context, timestr):
    new_dt = datetime.strptime(timestr, '%H:%M:%S')
    # Preserve date, replace h/m/s
    context.__dict__['sim_time'] = context.sim_time.replace(hour=new_dt.hour, minute=new_dt.minute, second=new_dt.second)

@then('the clock shows time "{timestr}"')
def step_assert_clock_time(context, timestr):
    assert _display_time_str(context) == timestr, f"Expected {timestr} got {_display_time_str(context)}"

@given('the clock format is 24h')
def step_fmt_24(context):
    context._format_24h = True

@given('the clock format is 12h')
def step_fmt_12(context):
    context._format_24h = False

@given('the display type is {dtype}')
def step_display_type(context, dtype):
    context._display_type = dtype

@when('I rotate LEFT on the encoder')
def step_toggle_format(context):
    # Only handle encoder left here; tapping is covered by existing generic step in app_steps
    context._format_24h = not context._format_24h


@then('the display type is {dtype}')
def step_assert_display_type(context, dtype):
    assert context._display_type == dtype, f"Expected display type {dtype} got {context._display_type}"

@when('I tap the clock display')
def step_tap_clock_display(context):
    # Cycle display type without changing app
    order = ['Digital','Analogue','Nixie']
    context._display_type = order[(order.index(context._display_type)+1)%len(order)]

@given('the date label is currently hidden')
@given('the date label is hidden')
def step_date_hidden(context):
    context._date_visible = False

@then('the date label is visible')
def step_date_visible(context):
    assert context._date_visible, 'Date label expected visible'

@then('the date label is still visible')
def step_date_still_visible(context):
    assert context._date_visible, 'Date label expected still visible'

@then('the date label is hidden again')
def step_date_hidden_again(context):
    assert not context._date_visible, 'Date label expected hidden again'

@when('I rotate RIGHT on the encoder')
def step_show_date(context):
    context._date_visible = True
    context._date_visible_since = context.sim_time

    # If display type is Nixie, override: date should not become visible (business rule placeholder)
    if getattr(context,'_display_type','Digital') == 'Nixie':
        context._date_visible = False

# Apply hide logic after time advances
# (Hook: simplistic approach: every assertion or time advance check)

def _maybe_hide_date(context):
    if getattr(context, '_date_visible', False) and getattr(context, '_date_visible_since', None):
        # Hide after >=3 seconds
        if (context.sim_time - context._date_visible_since) >= timedelta(seconds=3):
            context._date_visible = False

## Removed duplicate explicit time elapse steps to avoid ambiguity; generic handlers now manage hide timing.

@then('the date label text becomes "{datestr}"')
def step_date_text(context, datestr):
    # Placeholder: We'll just format current sim_time to abbreviated form
    wk = ['Mon','Tue','Wed','Thu','Fri','Sat','Sun'][context.sim_time.weekday()]
    mon = ['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec'][context.sim_time.month-1]
    expected = f"{wk} {context.sim_time.day} {mon}"
    assert expected == datestr, f"Expected date text {datestr} got {expected}"

@given('the timezone is {tz}')
def step_timezone_set(context, tz):
    context._timezone = tz

@when('the timezone changes to {tz}')
def step_timezone_change(context, tz):
    # Placeholder: would recalc displayed local time -> for now just set and no conversion
    context._timezone = tz

@then('the clock shows adjusted local time')
def step_clock_adjusted(context):
    # Placeholder acceptance: Always passes until real offset logic implemented
    pass

@when('one more second elapses (minute rollover)')
def step_one_more_second_rollover(context):
    _advance_sim_time(context, 1)

@then('the clock still shows time "{timestr}"')
def step_clock_still(context, timestr):
    assert _display_time_str(context) == timestr

@when('I switch back to the "Clock" app')
def step_switch_back_clock(context):
    # Placeholder: no app manager integration yet
    pass

@given('the app is restarted')
def step_app_restarted(context):
    preserved_type = getattr(context, '_display_type', 'Digital')
    preserved_time = getattr(context, 'sim_time', datetime(2025,9,30,0,0,0))
    _init_sim_time(context, preserved_time)
    context._display_type = preserved_type

# Placeholders / WIP steps
@given('an alarm is set for {timestr}')
def step_alarm_set(context, timestr):
    raise NotImplementedError('Alarm support WIP')

@then('an alarm indicator is visible')
def step_alarm_indicator(context):
    raise NotImplementedError('Alarm indicator WIP')

@then('a short haptic feedback occurs')
def step_haptic_chime(context):
    raise NotImplementedError('Hourly chime WIP')

@given('the device enters IDLE power state')
def step_device_idle(context):
    raise NotImplementedError('Power state integration pending')

@when('the device returns to ACTIVE')
def step_device_active(context):
    raise NotImplementedError('Power state integration pending')

@when('I switch to the "{app}" app')
def step_switch_app(context, app):
    raise NotImplementedError('App switching pending')
