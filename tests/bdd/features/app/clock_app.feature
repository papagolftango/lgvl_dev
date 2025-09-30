Feature: Clock App Core Behaviour
  The Clock app shows the current local time (timezone-adjusted via the time manager) and the date
  where applicable. It updates once per second and updates the date when a new minute begins.
  The user can: (a) cycle between three display types (Digital, Analogue, Nixie simulation) by tapping,
  (b) temporarily toggle 12/24-hour format (future: persist this), and (c) briefly reveal the date.
  No alarm/chime functionality exists (removed). Persistence of format and display type is future work.

  # ---------------------------------------------------------------------------
  # TIME TICKING
  # ---------------------------------------------------------------------------
  @clock @time @tick
  Scenario: Time label updates every second
    Given the system time is 12:00:05
    And the active app is "Clock"
    When one second elapses
    Then the clock shows time "12:00:06"
    When one more second elapses
    Then the clock shows time "12:00:07"

  # ---------------------------------------------------------------------------
  # DATE UPDATES PER MINUTE
  # ---------------------------------------------------------------------------
  @clock @date @minute
  Scenario: Date label updates when minute changes
    Given the detailed system time is Monday 2025-09-29 12:34:55
    And the active app is "Clock"
    And the date label is currently hidden
    When 10 seconds elapse
    And one more second elapses (minute rollover)
    Then the date label text becomes "Mon 29 Sep"

  # ---------------------------------------------------------------------------
  # 12/24-HOUR TOGGLE (encoder left only now; tap reserved for display cycling)
  # ---------------------------------------------------------------------------
  # (Tap format toggle removed in new design Option 3)

  @clock @format @encoder
  Scenario: Rotating encoder LEFT toggles format
    Given the system time is 21:09:59
    And the active app is "Clock"
    And the clock format is 24h
    When I rotate LEFT on the encoder
    Then the clock shows time "09:09:59"

  # ---------------------------------------------------------------------------
  # SHOW DATE BRIEFLY (encoder right or long press)
  # ---------------------------------------------------------------------------
  @clock @date @popup
  Scenario: Encoder RIGHT shows date for 3 seconds then hides
    Given the detailed system time is Tuesday 2025-09-30 08:15:10
    And the active app is "Clock"
    And the date label is hidden
    When I rotate RIGHT on the encoder
    Then the date label is visible
    When 2 seconds elapse
    Then the date label is still visible
    When 1 more second elapses
    Then the date label is hidden again

  # ---------------------------------------------------------------------------
  # POWER / IDLE INTERACTION (placeholder)
  # ---------------------------------------------------------------------------
  @clock @power @wip
  Scenario: Clock stops updating when device is IDLE
    Given the system time is 10:00:00
    And the active app is "Clock"
    And the device enters IDLE power state
    When 5 seconds elapse
    Then the clock still shows time "10:00:00"
    When the device returns to ACTIVE
    And one second elapses
    Then the clock shows time "10:00:01"

  # ---------------------------------------------------------------------------
  # TIME SYNC (placeholder for external sync tick)
  # ---------------------------------------------------------------------------
  @clock @sync @wip
  Scenario: Time jump causes immediate display update
    Given the system time is 11:59:58
    And the active app is "Clock"
    When the system time jumps to 12:10:00
    Then the clock shows time "12:10:00"

  # ---------------------------------------------------------------------------
  # ---------------------------------------------------------------------------
  # DISPLAY TYPE CYCLING (tap cycles Digital -> Analogue -> Nixie -> Digital)
  # ---------------------------------------------------------------------------
  @clock @display @cycle
  Scenario: Tap cycles display type forward
    Given the system time is 12:00:00
    And the active app is "Clock"
    And the display type is Digital
  When I tap the clock display
    Then the display type is Analogue
  When I tap the clock display
    Then the display type is Nixie
  When I tap the clock display
    Then the display type is Digital

  @clock @display @wip
  Scenario: Display type persists across restart (future)
    Given the display type is Nixie
    And the app is restarted
    Then the display type is Nixie

  # ---------------------------------------------------------------------------
  # DATE VISIBILITY PER DISPLAY TYPE
  # Digital and Analogue show date (toggle/brief); Nixie TBD (currently no date shown)
  # ---------------------------------------------------------------------------
  @clock @date @display
  Scenario: Date available on Digital display
    Given the system time is 13:10:05
    And the active app is "Clock"
    And the display type is Digital
    When I rotate RIGHT on the encoder
    Then the date label is visible

  @clock @date @display
  Scenario: Date available on Analogue display
    Given the system time is 13:10:05
    And the active app is "Clock"
    And the display type is Analogue
    When I rotate RIGHT on the encoder
    Then the date label is visible

  @clock @date @display
  Scenario: Date hidden on Nixie display
    Given the system time is 13:10:05
    And the active app is "Clock"
    And the display type is Nixie
    When I rotate RIGHT on the encoder
    Then the date label is hidden again

  # ---------------------------------------------------------------------------
  # TIMEZONE & TIME MANAGER SYNC
  # ---------------------------------------------------------------------------
  @clock @sync
  Scenario: Time jump from time manager updates immediately
    Given the system time is 11:59:58
    And the active app is "Clock"
    When the system time jumps to 12:10:00
    Then the clock shows time "12:10:00"

  @clock @timezone @wip
  Scenario: Timezone change triggers redisplay
    Given the system time is 08:00:00
    And the active app is "Clock"
    And the timezone is UTC
    When the timezone changes to Europe/London
    Then the clock shows adjusted local time

  # ---------------------------------------------------------------------------
  # MODE SWITCHING BACK (placeholder bridging with app manager)
  # ---------------------------------------------------------------------------
  @clock @navigation @wip
  Scenario: Leaving and returning preserves display type and format
    Given the system time is 16:00:05
    And the active app is "Clock"
    And the clock format is 12h
    And the display type is Analogue
    When I switch to the "Energy" app
    And 5 seconds elapse
    And I switch back to the "Clock" app
    Then the clock shows time "04:00:10"
    And the display type is Analogue

  # ---------------------------------------------------------------------------
  # NIXIE DISPLAY BEHAVIOUR (visual simulation requirements) @wip
  # ---------------------------------------------------------------------------
  # The Nixie simulation aims to mimic physical tubes:
  #  * Always shows HH:MM:SS (6 digits) with colons separating HH, MM, SS
  #  * No date ever shown (even when other display types would show it)
  #  * When a digit changes, the outgoing numeral fades down (after-glow) while the new one fades up
  #  * Slight colour bleed/halo around active digit segments
  #  * Background subtly hints at tube internals (wiring/filament cage)
  #  * Leading zeros are displayed (e.g., 04:05:09)
  #  * All transitions complete within 150ms, overlapping old fade-out and new fade-in
  # The following scenarios specify intent only; visual assertions are placeholders pending a visual test harness.

  @clock @display @nixie @wip
  Scenario: Nixie shows six digits with colons and no date
    Given the system time is 04:05:09
    And the active app is "Clock"
    And the display type is Nixie
    Then the nixie time layout is "04:05:09"
    And the date label is hidden again

  @clock @display @nixie @transition @wip
  Scenario: Digit change triggers overlapping fade-old and fade-new
    Given the system time is 12:00:09
    And the active app is "Clock"
    And the display type is Nixie
    When one second elapses
    Then the previous ones-digit begins fading out while the new ones-digit fades in
    And the transition completes within 150 ms

  @clock @display @nixie @afterglow @wip
  Scenario: After-glow lingers briefly on changed digit
    Given the system time is 12:34:59
    And the active app is "Clock"
    And the display type is Nixie
    When one second elapses
    Then the digit that changed shows a brief after-glow
    And the after-glow ends within 200 ms

  @clock @display @nixie @halo @wip
  Scenario: Active digits show subtle colour halo
    Given the system time is 09:08:07
    And the active app is "Clock"
    And the display type is Nixie
    Then each lit digit has a halo intensity between the configured min and max

  @clock @display @nixie @leadingzeros @wip
  Scenario: Leading zeros are always rendered
    Given the system time is 02:03:04
    And the active app is "Clock"
    And the display type is Nixie
    Then the nixie time layout is "02:03:04"

  @clock @display @nixie @no-date @wip
  Scenario: Date request ignored on Nixie display
    Given the system time is 11:22:33
    And the active app is "Clock"
    And the display type is Nixie
    When I rotate RIGHT on the encoder
    Then the date label is hidden again

  @clock @display @nixie @background @wip
  Scenario: Background hints at tube internals
    Given the display type is Nixie
    And the active app is "Clock"
    Then the nixie background style matches the tube intent
