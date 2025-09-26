Feature: App navigation via touch
  As a user
  I want touch to switch apps
  So I can move between screens easily

  Background:
    Given the system is ACTIVE
    And the active app is "Energy"

  @app @nav
  Scenario: Default app is Energy
    Given the device is reset
    Then the active app is "Energy"

  @app @nav
  Scenario: Touch switches to next app
    When I tap the screen
    Then the active app is "Clock"

  @app @nav
  Scenario: Frequent taps wrap to the beginning
    Given the active app is "Weather"
    When I tap the screen
    Then the active app is "Energy"

  @app @nav @wakeup
  Scenario: Touch wakes device but does not switch apps
    Given the system is IDLE with backlight off
    When I tap the screen
    Then the system is ACTIVE
    And the active app is "Energy"
