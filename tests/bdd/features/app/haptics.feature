Feature: Haptic feedback on valid touch
  As a user
  I want haptics on meaningful touch interactions

  Background:
    Given haptics are enabled

  @haptics
  Scenario: Haptic pulse on app switch
    Given the active app is "Energy"
    When I tap the screen
    Then a short haptic pulse is emitted
    And the active app is "Clock"

  @haptics @wakeup
  Scenario: No haptics on wake touch (policy)
    Given the system is IDLE
    When I tap the screen
    Then the system is ACTIVE
    And no haptic pulse is emitted
