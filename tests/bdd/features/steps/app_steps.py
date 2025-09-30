from behave import given, when, then


def _ctx_bridge(context):
    return context.bridge


# Background helpers
@given("the system is ACTIVE")
def step_system_active(context):
    b = _ctx_bridge(context)
    b.set_power_state("ACTIVE")


@given("the system is IDLE")
@given("the system is IDLE with backlight off")
def step_system_idle(context):
    b = _ctx_bridge(context)
    b.set_power_state("IDLE")


@given('the active app is "{name}"')
def step_set_active_app(context, name):
    b = _ctx_bridge(context)
    b.set_active_app(name)


@given("haptics are enabled")
def step_haptics_enabled(context):
    # In mock, haptics are always enabled.
    pass


@given("the inactivity timeout is 120 seconds")
def step_timeout_120(context):
    _ctx_bridge(context).set_inactivity_timeout(120)


# Actions
@when("I tap the screen")
def step_tap(context):
    # capture pre-action app for 'unchanged' checks
    context._pre_action_app = _ctx_bridge(context).get_active_app()
    _ctx_bridge(context).tap()


@when('I rotate the encoder {direction}')
def step_rotate(context, direction):
    # capture pre-action app for 'unchanged' checks
    context._pre_action_app = _ctx_bridge(context).get_active_app()
    # capture energy mode if Energy is active
    if context._pre_action_app == "Energy":
        vm = _ctx_bridge(context).get_energy_viewmodel()
        if vm:
            context._pre_energy_mode = vm.get("mode")
    _ctx_bridge(context).rotate(direction)


# Assertions
@then('the active app is "{name}"')
def step_assert_active_app(context, name):
    assert _ctx_bridge(context).get_active_app() == name


@then("the active app is unchanged")
def step_assert_active_app_unchanged(context):
    pre = getattr(context, "_pre_action_app", None)
    assert pre is not None, "No pre-action app captured—ensure an action step ran before this assertion."
    assert _ctx_bridge(context).get_active_app() == pre


@then("the system is ACTIVE")
def step_assert_system_active(context):
    assert _ctx_bridge(context).get_power_state() == "ACTIVE"


@then("the system is IDLE")
def step_assert_system_idle(context):
    assert _ctx_bridge(context).get_power_state() == "IDLE"


@then("the backlight level is 0")
def step_assert_backlight_zero(context):
    assert _ctx_bridge(context).get_backlight_level() == 0


@when('no input occurs for {seconds:d} seconds')
def step_no_input_for(context, seconds):
    _ctx_bridge(context).no_input_for(seconds)


@then("a short haptic pulse is emitted")
def step_assert_haptic_short(context):
    assert _ctx_bridge(context).get_last_haptic() == "short"


@then("no haptic pulse is emitted")
def step_assert_no_haptic(context):
    assert _ctx_bridge(context).get_last_haptic() is None




@then('no other app receives the encoder event')
def step_no_other_app_gets_encoder(context):
    b = _ctx_bridge(context)
    # In our model, only the active app should consume the rotary event, or None when waking from IDLE
    target = b.get_last_rotary_target_app()
    current = b.get_active_app()
    assert target in (None, current), f"Rotary was handled by '{target}' but active app is '{current}'"


@given("the device is reset")
def step_reset_device(context):
    _ctx_bridge(context).reset()
