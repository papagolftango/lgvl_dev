Feature: Settings App Navigation and Editing
  The Settings app allows the user to cycle through grouped settings (per-app and system)
  using touch (tap to advance, long-press to go back) and eventually to edit values via
  rotary + touch confirm. All settings are persisted. BDD now focuses on listing and
  navigation; editing and persistence behaviours are @wip until implemented.

  Groups (initial):
    Energy App Settings:
      - Tariff Rate (p/kWh)
      - Currency Symbol
      - Daily Reset Time (HH:MM)
    Clock App Settings:
      - Time Format (12/24)
      - Default Display Type (Digital/Analogue/Nixie)
      - Date Popup Duration (seconds)
    Weather App Settings:
      - Poll Interval (minutes)
      - Default View (Hourly/Multi-day)
      - Units (Celsius/Fahrenheit) @wip
    System Settings:
      - Time Zone
      - Brightness Level
      - Sleep Timeout (seconds)
      - Firmware Update Channel (Stable/Beta) @wip
      - Re-Provision Device (action)

  # Core navigation semantics:
  # - First entering Settings shows the first group header (Energy App Settings)
  # - Tap advances to next item within group, after last item of group shows next group header
  # - After last group item cycles back to first group header
  # - Long press (or double tap placeholder) goes backwards (@wip)
  # - Active item displays name and current value (value may be blank/placeholder for sensitive items)

  @settings @list @core
  Scenario: Entering settings shows first group header
    Given I open the Settings app
    Then the settings current group is "Energy App Settings"
    And the settings current item is group header

  @settings @cycle @core
  Scenario: Tapping cycles through first group items then next group header
    Given I open the Settings app
    When I tap within settings
    Then the settings current item name is "Tariff Rate"
    When I tap within settings
    Then the settings current item name is "Currency Symbol"
    When I tap within settings
    Then the settings current item name is "Daily Reset Time"
    When I tap within settings
    Then the settings current group is "Clock App Settings"
    And the settings current item is group header

  @settings @wrap @wip
  Scenario: Cycling wraps from last system item to first group header
    Given I open the Settings app
    And I advance through all settings items
    When I tap the screen
    Then the settings current group is "Energy App Settings"
    And the settings current item is group header

  @settings @back @wip
  Scenario: Long press navigates backwards
    Given I open the Settings app
    And I am on item "Daily Reset Time"
    When I long press the screen
    Then the settings current item name is "Currency Symbol"

  @settings @reprovision @wip
  Scenario: Re-Provision Device action arms a reprovision flag
    Given I open the Settings app
    And I navigate to group "System Settings"
    And I navigate to item "Re-Provision Device"
    When I begin editing the setting
    And I tap to confirm the setting
    Then the device reprovision flag is set

  @settings @edit @wip
  Scenario: Editing a numeric value via rotary and confirm
    Given I open the Settings app
    And I navigate to item "Tariff Rate"
    When I begin editing the setting
    And I rotate the dial +3 steps
    And I tap to confirm the setting
    Then the setting "Tariff Rate" new value is persisted

  @settings @persist @wip
  Scenario: Returning to Settings restores last visited item
    Given I open the Settings app
    And I navigate to item "Time Format"
    And I close the Settings app
    When I open the Settings app
    Then the settings current item name is "Time Format"

  @settings @order @core
  Scenario: Full ordered list of setting display names
    Given I open the Settings app
    Then the ordered settings sequence is:
      | name                       |
      | Energy App Settings        |
      | Tariff Rate                |
      | Currency Symbol            |
      | Daily Reset Time           |
      | Clock App Settings         |
      | Time Format                |
      | Default Display Type       |
      | Date Popup Duration        |
      | Weather App Settings       |
      | Poll Interval              |
      | Default View               |
      | Units                      |
  | System Settings            |
      | Time Zone                  |
      | Brightness Level           |
      | Sleep Timeout              |
      | Firmware Update Channel    |
  | Re-Provision Device        |

  @settings @integrity @core
  Scenario: Group headers have no value editable state
    Given I open the Settings app
    And the settings current group is "Energy App Settings"
    Then the settings current item is group header
    And the settings item has no value field

  @settings @advance @core
  Scenario: Advancing exactly N taps reaches expected item
    Given I open the Settings app
    When I tap the screen 5 times
    Then the settings current item name is "Time Format"

  @settings @multi @wip
  Scenario: Multi-step navigation using navigate helper
    Given I open the Settings app
    And I navigate to item "Brightness Level"
    Then the settings current item name is "Brightness Level"

  # END OF SPEC
