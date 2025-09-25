# power_idle_wake

Source: tests/bdd/features/app/power_idle_wake.feature

~~~gherkin
Feature: Power idle and wake transitions
  As a user
  I want smooth dimming and instant wake without side effects

  Background:
    Given the inactivity timeout is 120 seconds

  @power
  Scenario: Idle after inactivity
    Given the system is ACTIVE
    When no input occurs for 120 seconds
    Then the system is IDLE
    And the backlight level is 0

  @power @wakeup
  Scenario: Wake on touch with consumed event
    Given the system is IDLE
    When I tap the screen
    Then the system is ACTIVE
    And the active app is unchanged

~~~
