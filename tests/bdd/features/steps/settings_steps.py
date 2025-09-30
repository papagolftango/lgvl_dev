from behave import given, when, then

# BDD-first in-memory model for Settings navigation. Editing/persistence deferred.

class SettingsModel:
    def __init__(self):
        # Flattened sequence including group headers (marked) followed by items
        self.sequence = [
            {"name": "Energy App Settings", "type": "group"},
            {"name": "Tariff Rate", "type": "item", "value": 0.00, "unit": "p/kWh"},
            {"name": "Currency Symbol", "type": "item", "value": "£"},
            {"name": "Daily Reset Time", "type": "item", "value": "00:00"},
            {"name": "Clock App Settings", "type": "group"},
            {"name": "Time Format", "type": "item", "value": "24"},
            {"name": "Default Display Type", "type": "item", "value": "Digital"},
            {"name": "Date Popup Duration", "type": "item", "value": 3},
            {"name": "Weather App Settings", "type": "group"},
            {"name": "Poll Interval", "type": "item", "value": 15, "unit": "min"},
            {"name": "Default View", "type": "item", "value": "Hourly"},
            {"name": "Units", "type": "item", "value": "C", "wip": True},
            {"name": "System Settings", "type": "group"},
            {"name": "Time Zone", "type": "item", "value": "UTC"},
            {"name": "Brightness Level", "type": "item", "value": 80},
            {"name": "Sleep Timeout", "type": "item", "value": 120},
            {"name": "Firmware Update Channel", "type": "item", "value": "Stable", "wip": True},
            {"name": "Re-Provision Device", "type": "action", "action": "reprovision"},
        ]
        self.index = 0
        self.last_index = 0
        self.app_open = False
        self.reprovision_flag = False

    def current(self):
        return self.sequence[self.index]

    def tap(self, times=1):
        for _ in range(times):
            self.index = (self.index + 1) % len(self.sequence)

    def find_index(self, name):
        for i, entry in enumerate(self.sequence):
            if entry["name"] == name:
                return i
        raise AssertionError(f"Setting '{name}' not found")

    def navigate_to(self, name):
        target = self.find_index(name)
        # naive forward stepping
        steps = 0
        while self.index != target and steps < len(self.sequence)+1:
            self.tap(1)
            steps += 1
        assert self.index == target, f"Failed to navigate to {name}"


def get_settings(context) -> SettingsModel:
    if not hasattr(context, 'settings_model'):
        context.settings_model = SettingsModel()
    return context.settings_model

# ---------------- GIVEN -----------------
@given('I open the Settings app')
def step_impl(context):
    m = get_settings(context)
    m.app_open = True
    m.index = 0

@when('I open the Settings app')
def step_impl(context):
    # alias for Given form
    m = get_settings(context)
    m.app_open = True

@given('the settings current group is "Energy App Settings"')
def step_impl(context):
    m = get_settings(context)
    # ensure index at first group
    m.index = 0

@given('I advance through all settings items')
def step_impl(context):
    m = get_settings(context)
    m.tap(len(m.sequence)-1)

@given('I am on item "{name}"')
def step_impl(context, name):
    m = get_settings(context)
    m.index = m.find_index(name)

@given('I navigate to group "{group}"')
def step_impl(context, group):
    m = get_settings(context)
    m.navigate_to(group)

@given('I navigate to item "{name}"')
def step_impl(context, name):
    m = get_settings(context)
    m.navigate_to(name)

# ---------------- WHEN ------------------
@when('I tap the screen {times:d} times')
def step_impl(context, times):
    m = get_settings(context)
    m.tap(times)

@when('I tap within settings')
def step_impl(context):
    m = get_settings(context)
    m.tap(1)

## Removed local 'I tap the screen' step to avoid ambiguity; generic step handles interaction.

@when('I long press the screen')
def step_impl(context):
    # Placeholder backwards navigation
    raise AssertionError('Backwards navigation not implemented yet')

@when('I begin editing the setting')
def step_impl(context):
    # For action entries we treat begin editing as 'arm' phase
    m = get_settings(context)
    cur = m.current()
    if cur.get('type') == 'action' and cur.get('action') == 'reprovision':
        # mark a transient armed state
        cur['armed'] = True
    else:
        raise AssertionError('Editing mode not implemented')

@when('I rotate the dial +{steps:d} steps')
def step_impl(context, steps):
    raise AssertionError('Rotation edit not implemented')

@when('I tap to confirm the setting')
def step_impl(context):
    m = get_settings(context)
    cur = m.current()
    if cur.get('type') == 'action' and cur.get('action') == 'reprovision' and cur.get('armed'):
        m.reprovision_flag = True
    else:
        raise AssertionError('Confirm edit not implemented')

@when('I close the Settings app')
def step_impl(context):
    m = get_settings(context)
    m.last_index = m.index
    m.app_open = False

@given('I close the Settings app')
def step_impl(context):
    m = get_settings(context)
    m.last_index = m.index
    m.app_open = False

# ---------------- THEN ------------------
@then('the settings current group is "{group}"')
def step_impl(context, group):
    m = get_settings(context)
    current = m.current()
    assert current['type'] == 'group'
    assert current['name'] == group

@then('the settings current item is group header')
def step_impl(context):
    m = get_settings(context)
    assert m.current()['type'] == 'group'

@then('the settings current item name is "{name}"')
def step_impl(context, name):
    m = get_settings(context)
    assert m.current()['name'] == name, f"Expected {name}, got {m.current()['name']}"

@then('the ordered settings sequence is:')
def step_impl(context):
    m = get_settings(context)
    names = [r['name'] for r in m.sequence]
    expected = [row['name'] for row in context.table]
    assert names == expected, f"Sequence mismatch.\nExpected: {expected}\nGot: {names}"

@then('the ordered settings sequence is')
def step_impl(context):
    # Accept variant without colon (if appears)
    m = get_settings(context)
    names = [r['name'] for r in m.sequence]
    expected = [row['name'] for row in context.table]
    assert names == expected, f"Sequence mismatch.\nExpected: {expected}\nGot: {names}"

@then('the settings item has no value field')
def step_impl(context):
    m = get_settings(context)
    cur = m.current()
    assert 'value' not in cur, 'Group header should not have value'

@then('the settings value is obscured')
def step_impl(context):
    m = get_settings(context)
    assert m.current().get('sensitive'), 'Expected sensitive item'

@then('the setting "{name}" new value is persisted')
def step_impl(context, name):
    raise AssertionError('Persistence not implemented')

@then('the device reprovision flag is set')
def step_impl(context):
    m = get_settings(context)
    assert m.reprovision_flag, 'Reprovision flag was not set'

## Removed specific item name assertions to avoid ambiguity with generic matcher.
