# app_navigation

Source: tests/bdd/features/app/app_navigation.feature

~~~gherkin
Feature: App navigation via touch
  As a user
  I want touch to switch apps
  So I can move between screens easily

  Background:
    Given the system is ACTIVE
    And the active app is "Energy"

  @app @nav
  Scenario: Touch switches to next app
    When I tap the screen
    Then the active app is "Clock"

  @app @nav @wakeup
  Scenario: Touch wakes device but does not switch apps
    Given the system is IDLE with backlight off
    When I tap the screen
    Then the system is ACTIVE
    And the active app is "Energy"

~~~
