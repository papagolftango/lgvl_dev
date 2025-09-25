# app_rotary

Source: tests\bdd\features\app\app_rotary.feature

~~~gherkin
Feature: Per-app rotary behavior
  As a user
  I want the rotary to control the current app
  So I can change in-app modes without affecting others

  Background:
    Given the active app is "Energy"

  @app @rotary
  Scenario: Rotary affects only the active app
    Given the Energy mode is "kWh Today"
    When I rotate the encoder RIGHT
    Then the Energy mode becomes the next mode
    And no other app receives the encoder event

  @app @rotary @wakeup
  Scenario: Rotary wakes first, acts second
    Given the system is IDLE
    When I rotate the encoder RIGHT
    Then the system is ACTIVE
    And the Energy mode remains unchanged
    When I rotate the encoder RIGHT again
    Then the Energy mode becomes the next mode

~~~
