Feature: Energy app overview and behaviour
  A simple, readable description of what the Energy app shows and how a person uses it.
  This file is intentionally written so a non-technical reader can skim it and understand
  what the dial, centre number, coloured circle, and mode cycling mean.

  # ---------------------------------------------------------------------------
  # HIGH-LEVEL PURPOSE
  # ---------------------------------------------------------------------------
  # The Energy app is a single circular gauge (dial) that helps a home user see:
  #   - Whether they are currently using more power than they generate (import) or exporting.
  #   - Instant solar generation and household usage (with markers and peaks).
  #   - Daily running energy total (kWh Today) based on pulses from a meter.
  #   - An estimated cost for today (£ Today) using a tariff.
  #   - Historical peak solar and peak usage values since the last daily reset.
  # A central display can show one of several "modes" at a time. The user rotates the encoder
  # to step through them. If the user leaves the default daily energy view, the display automatically
  # reverts back after 20 seconds so it stays useful at a glance.
  # A coloured status circle in the middle gives a quick traffic‑light style indication of balance:
  #   Green  = exporting (negative balance)
  #   Orange = modest import (0 to 2000 W)
  #   Red    = high import (> 2000 W)
  # Peak markers show where the highest solar and highest usage reached today.
  #
  # Terms in plain language:
  #   Balance: Net power flow now (negative means giving power back / exporting).
  #   Solar:   Current solar generation in watts.
  #   Using:   Current household consumption in watts.
  #   Peak Solar / Peak Used: Highest solar / usage values seen so far today.
  #   kWh Today: Energy used today (from pulses) converted to kilowatt‑hours.
  #   £ Today:  Cost estimate of today's energy usage (kWh Today * tariff rate).
  #
  # NOTE: Some scenarios are tagged @wip because supporting test endpoints (e.g. a full
  # energy view model or fast-forwarding the day / 20 seconds) are not implemented yet.
  # They still serve as living documentation of intent.

  Background:
    Given the system is ACTIVE
    And the active app is "Energy"

  # ---------------------------------------------------------------------------
  # MODES & DEFAULT STATE
  # ---------------------------------------------------------------------------
  @energy @modes
  Scenario: The Energy app provides a set of readable modes
    Then the Energy modes are in this order:
      | Balance        |
      | Solar          |
      | Using          |
      | Peak Solar     |
      | Peak Used      |
      | kWh Today      |
      | £ Today        |
    And the initial Energy mode is "kWh Today"

  @energy @modes
  Scenario: Rotating RIGHT moves to the next mode
    Given the Energy mode is "kWh Today"
    When I rotate the encoder RIGHT
    Then the Energy mode becomes the next mode

  @energy @modes
  Scenario: Rotating LEFT moves backwards
    Given the Energy mode is "kWh Today"
    When I rotate the encoder LEFT
    Then the Energy mode becomes the previous mode

  @energy @modes
  Scenario: Mode cycling wraps around at the ends
    Given the Energy mode is "£ Today"
    When I rotate the encoder RIGHT
    Then the Energy mode is "Balance"
    When I rotate the encoder LEFT
    Then the Energy mode is "£ Today"

  # ---------------------------------------------------------------------------
  # AUTO REVERT (returns user gently to daily energy view)
  # ---------------------------------------------------------------------------
  @energy @autorevert
  Scenario: Leaving the default view temporarily
    Given the Energy mode is "Balance"
    When no input occurs for 20 seconds
    Then the Energy mode is "kWh Today"
    # Rationale: After browsing other modes the display falls back to the daily energy summary.

  # ---------------------------------------------------------------------------
  # PEAKS & CURRENT MARKERS
  # ---------------------------------------------------------------------------
  @energy @peaks
  Scenario: Peaks record highest solar and usage reached today
    Given the peak solar is 0 W
    And the peak usage is 0 W
    When new solar data arrives with value 1800 W
    And new usage data arrives with value 2500 W
    Then the peak solar is 1800 W
    And the peak usage is 2500 W
    When new usage data arrives with value 2400 W
    Then the peak usage remains 2500 W

  @energy @peaks @dailyreset
  Scenario: Daily reset clears peaks and latches pulse baseline
    Given the peak solar is 3000 W
    And the peak usage is 4500 W
    And the pulse count since baseline is 500
    When a new day starts
    Then the peak solar is 0 W
    And the peak usage is 0 W
    And the pulse baseline is latched to the current raw meter value

  # ---------------------------------------------------------------------------
  # BALANCE & STATUS COLOUR
  # ---------------------------------------------------------------------------
  @energy @balance @colour
  Scenario Outline: Status circle colour reflects current balance
    Given the balance value is <balance>
    When the Energy screen updates
    Then the status circle colour state is <colour_state>

    Examples:
      | balance | colour_state |
      | -500    | Exporting    |
      | 0       | Low Import   |
      | 1500    | Low Import   |
      | 2500    | High Import  |

  # ---------------------------------------------------------------------------
  # PULSES, ENERGY, AND COST
  # ---------------------------------------------------------------------------
  @energy @pulses @kwh
  Scenario: Converting pulses to kWh Today
    Given the pulse baseline is 1000
    And the current pulse counter is 2500
    When the Energy mode is "kWh Today"
    Then the center value shows 1.50 kWh
    # (2500 - 1000) = 1500 pulses / 1000 = 1.50 kWh

  @energy @pulses @baseline @kwh
  Scenario: Pulse counter drop rebases baseline safely
    Given the pulse baseline is 5000
    And the current pulse counter is 5200
    When the pulse counter jumps down to 300
    Then the baseline is reset to 300
    And kWh Today is 0.00 kWh
    # Rationale: A meter reset or rollover should not invent huge negative usage.

  @energy @cost @tariff
  Scenario: Converting pulses to cost (£ Today)
    Given the tariff rate is 0.55 GBP per kWh
    And the pulse baseline is 2000
    And the current pulse counter is 2600
    When the Energy mode is "£ Today"
    Then the center value shows £0.33
    # Pulses delta = 600 => 0.60 kWh * £0.55 = £0.33

  # ---------------------------------------------------------------------------
  # DATA DISPLAY PER MODE
  # ---------------------------------------------------------------------------
  @energy @modes @display
  Scenario Outline: Center display meaning in each mode
    Given the last known values are:
      | Balance    | -350  |
      | Solar      | 1800  |
      | Using      | 2200  |
      | Peak Solar | 2500  |
      | Peak Used  | 4000  |
      | Pulses     | 4800  |
      | Baseline   | 4000  |
    When I set the Energy mode to <mode>
    Then the center title is <title>
    And the center number reflects the <meaning>

    Examples:
      | mode        | title        | meaning                               |
      | Balance     | Balance      | current net power (negative=export)   |
      | Solar       | Solar        | current solar generation              |
      | Using       | Using        | current household usage               |
      | Peak Solar  | Peak Solar   | highest solar so far today            |
      | Peak Used   | Peak Used    | highest usage so far today            |
      | kWh Today   | kWh Today    | pulses delta / 1000                   |
      | £ Today     | £ Today      | (pulses delta / 1000) * tariff rate   |

  # ---------------------------------------------------------------------------
  # WRAP-UP / INTENT
  # ---------------------------------------------------------------------------
  # The above scenarios collectively define what the Energy app conveys. Even where the
  # automated harness cannot yet assert (marked @wip), this feature file is the canonical
  # intent and drives future endpoint and step-definition work. As implementation matures,
  # tags like @wip can be removed to bring the scenarios into the standard regression set.
