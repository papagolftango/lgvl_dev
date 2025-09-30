Feature: Weather App Forecast Display
  The Weather app presents current conditions plus a near-term hourly ring (6-8 hours)
  and allows toggling to a multi-day forecast. Data is fetched periodically by the
  controller every 15 minutes (configurable) and cached so UI is responsive.

  Glossary:
    Current Conditions: Temperature, feels like, condition text, primary icon (e.g. sunny, rain)
    Hourly Ring: Radial layout of upcoming hours (default 8) around a clock face; top at current hour
    Multi-Day View: N upcoming days (default 5) with day icon + hi/lo + brief text
    Stale Data: Older than poll interval * 2 (e.g. >30 min if interval is 15)

  Assumptions / TBD:
    - External API provider TBD; placeholder weather source id is "mock-weather"
    - Temperature units default Celsius; future toggle (@wip)
    - Poll interval default 900 seconds

  # ---------------------------------------------------------------------------
  # CURRENT CONDITIONS
  # ---------------------------------------------------------------------------
  @weather @current
  Scenario: Current conditions show icon, temperature, feels-like and text
    Given the weather source returns for now:
      | temp_c | feels_c | condition | icon   |
      | 13.4   | 12.0    | Light Rain | rain  |
    And the active app is "Weather"
    Then the weather current temperature is 13.4°C
    And the weather current feels-like is 12.0°C
    And the weather current condition text is "Light Rain"
    And the weather current icon is rain

  # ---------------------------------------------------------------------------
  # HOURLY RING (8 hours default) - radial layout semantics
  # ---------------------------------------------------------------------------
  @weather @hourly
  Scenario: Hourly ring shows next 8 hours with current hour at top
    Given the current time is 09:10
    And the weather source returns the next hours:
      | hour | temp_c | icon  | condition       |
      | 09   | 13     | rain  | Light Rain      |
      | 10   | 14     | cloud | Cloudy          |
      | 11   | 15     | sun   | Sunny Breaks    |
      | 12   | 16     | sun   | Sunny           |
      | 13   | 17     | sun   | Sunny           |
      | 14   | 17     | sun   | Sunny           |
      | 15   | 16     | cloud | Partly Cloudy   |
      | 16   | 15     | rain  | Showers         |
    And the active app is "Weather"
    Then the hourly ring has 8 segments
    And the segment at top corresponds to hour 09
    And the hours proceed clockwise in ascending order
    And each segment shows an icon and temperature

  @weather @hourly @wrap
  Scenario: Hourly ring wraps past midnight
    Given the current time is 22:30
    And the weather source returns the next hours:
      | hour | temp_c | icon  | condition |
      | 22   | 11     | cloud | Cloudy    |
      | 23   | 10     | rain  | Showers   |
      | 00   | 9      | rain  | Showers   |
      | 01   | 8      | rain  | Showers   |
      | 02   | 8      | sun   | Clear     |
      | 03   | 7      | sun   | Clear     |
      | 04   | 7      | sun   | Clear     |
      | 05   | 6      | sun   | Clear     |
    And the active app is "Weather"
    Then the hourly ring has 8 segments
    And the segment at top corresponds to hour 22
    And the sequence includes hour 00 after 23

  # ---------------------------------------------------------------------------
  # TOGGLING TO MULTI-DAY VIEW (touch interaction)
  # ---------------------------------------------------------------------------
  @weather @multiday
  Scenario: Touch toggles from hourly ring to multi-day forecast
    Given the active app is "Weather"
    And the weather source returns multi-day forecast:
      | day | hi_c | lo_c | icon  | condition      |
      | Tue | 17   | 11   | sun   | Sunny          |
      | Wed | 18   | 10   | sun   | Sunny          |
      | Thu | 16   | 9    | cloud | Cloudy         |
      | Fri | 14   | 8    | rain  | Showers        |
      | Sat | 15   | 9    | rain  | Light Showers  |
    When I tap the screen
    Then the multi-day forecast is shown
    And the daily forecast shows 5 days
    And each day shows hi/lo, icon, and condition text

  @weather @multiday @back
  Scenario: Touch again returns to hourly ring
    Given I am viewing the multi-day forecast
    When I tap the screen
    Then the hourly ring is shown again

  # ---------------------------------------------------------------------------
  # POLLING & STALENESS
  # ---------------------------------------------------------------------------
  @weather @polling
  Scenario: Weather data is refreshed every 15 minutes
    Given the poll interval is 900 seconds
    And the last successful fetch was 0 seconds ago
    When 901 seconds elapse
    Then a weather fetch is triggered

  @weather @stale
  Scenario: Stale data is indicated after twice the poll interval
    Given the poll interval is 900 seconds
    And the last successful fetch was 2000 seconds ago
    Then the weather data is marked stale

  # ---------------------------------------------------------------------------
  # ICON MAPPING & FALLBACKS
  # ---------------------------------------------------------------------------
  @weather @icons
  Scenario: Unknown condition code falls back to generic icon
    Given the weather source returns for now:
      | temp_c | feels_c | condition | icon        |
      | 10.0   | 8.5     | Haze      | UNKNOWN_RAW |
    And the active app is "Weather"
    Then the weather current icon is generic

  # ---------------------------------------------------------------------------
  # EDGE CASES
  # ---------------------------------------------------------------------------
  @weather @edge
  Scenario: Partial hourly data (less than 8 rows) fills remaining slots as empty
    Given the current time is 05:10
    And the weather source returns the next hours:
      | hour | temp_c | icon | condition   |
      | 05   | 9      | sun  | Clear       |
      | 06   | 11     | sun  | Clear       |
      | 07   | 12     | sun  | Clear       |
    And the active app is "Weather"
    Then the hourly ring has 8 segments
    And 5 segments are empty placeholders

  @weather @edge @stale
  Scenario: No current data available (first fetch failed) shows loading state
    Given there is no cached weather data
    And the active app is "Weather"
    Then the weather shows a loading indicator

  # ---------------------------------------------------------------------------
  # RADIAL POSITIONAL SEMANTICS (extended spec)
  # ---------------------------------------------------------------------------
  @weather @hourly @radial
  Scenario: Radial hourly ring places offsets at expected clock positions
    Given the current time is 10:05
    And the weather source returns the next hours:
      | hour | temp_c | icon  | condition |
      | 10   | 14     | sun   | Clear     |
      | 11   | 15     | sun   | Clear     |
      | 12   | 16     | sun   | Clear     |
      | 13   | 17     | sun   | Clear     |
      | 14   | 17     | sun   | Clear     |
      | 15   | 16     | cloud | Cloudy    |
      | 16   | 15     | rain  | Showers   |
      | 17   | 14     | rain  | Showers   |
    And the active app is "Weather"
    Then the radial segment for offset 0 is at 12 o'clock
    And the radial segment for offset 3 is near 3 o'clock
    And the radial segment for offset 6 is near 6 o'clock
    And the radial segment for offset 9 is near 9 o'clock (not shown if only 8 hours)
    And radial segment positioning is monotonic clockwise

  # ---------------------------------------------------------------------------
  # ICON MAPPING OUTLINE (extended spec)
  # ---------------------------------------------------------------------------
  @weather @icons @mapping
  Scenario Outline: Known icon codes map to themed assets
    Given weather icon code <code>
    When I open the Weather app
    Then the icon asset for <code> is used
    Examples:
      | code          |
      | sun           |
      | cloud         |
      | rain          |
      | showers       |
      | snow          |
      | fog           |

  @weather @icons @fallback
  Scenario: Unknown icon code maps to generic fallback (extended)
    Given weather icon code "volcanic_ash" (unknown)
    When I open the Weather app
    Then the generic weather fallback icon is used

  # ---------------------------------------------------------------------------
  # MISSING HI/LO & PRECIP (extended resilience)
  # ---------------------------------------------------------------------------
  @weather @resilience @precip
  Scenario: Hourly ring omits precipitation badge when data missing
    Given the current time is 07:00
    And the weather source returns the next hours:
      | hour | temp_c | icon | condition   | precip_pct |
      | 07   | 11     | sun  | Clear       |            |
      | 08   | 12     | sun  | Clear       | 10         |
      | 09   | 13     | sun  | Clear       |            |
      | 10   | 14     | sun  | Clear       | 5          |
    And the active app is "Weather"
    Then hours with precip_pct show a precipitation badge
    And hours without precip_pct show no precipitation badge

  @weather @resilience @hilomissing
  Scenario: Multi-day view renders dashes for missing hi or lo
    Given the weather source returns multi-day forecast:
      | day | hi_c | lo_c | icon  | condition |
      | Mon |     | 7    | sun   | Clear     |
      | Tue | 14  |      | cloud | Cloudy    |
    And the active app is "Weather"
    When I tap the screen
    Then missing temperature values are rendered as dashes

  # ---------------------------------------------------------------------------
  # MODE RESTORE (extended persistence)
  # ---------------------------------------------------------------------------
  @weather @mode @wip
  Scenario: Returning to Weather app restores previously selected view
    Given I last viewed the multi-day view
    And cached weather data exists from 120 seconds ago
    When I open the Weather app
    Then the multi-day view is visible

  # ---------------------------------------------------------------------------
  # FAILURE ESCALATION (extended backoff tiers)
  # ---------------------------------------------------------------------------
  @weather @network @escalation @wip
  Scenario: Consecutive failures escalate retry backoff tiers
    Given 3 consecutive failed fetch attempts
    And the poll interval is 900 seconds
    When the next fetch window arrives
    Then an exponential backoff tier is selected

  # ---------------------------------------------------------------------------
  # OVERNIGHT ICE RISK PREDICTION
  # Predict if ice may form on car windscreen using forecast low vs dew point.
  # Simple heuristic: if forecast_low <= (dew_point + 1°C) and forecast_low <= 2°C
  # probability buckets: High (<=0°C & within 1°C of dew), Medium (0–2°C & within 1.5°C), Low otherwise.
  # ---------------------------------------------------------------------------
  @weather @ice
  Scenario: High ice risk when forecast below freezing and near dew point
    Given overnight forecast low is -2°C
    And overnight dew point is -3°C
    When I request the overnight ice risk
    Then the ice risk probability is High

  @weather @ice
  Scenario: Medium ice risk when near dew point but slightly above freezing
    Given overnight forecast low is 1°C
    And overnight dew point is 0°C
    When I request the overnight ice risk
    Then the ice risk probability is Medium

  @weather @ice
  Scenario: Low ice risk when temperature well above dew point
    Given overnight forecast low is 4°C
    And overnight dew point is 0°C
    When I request the overnight ice risk
    Then the ice risk probability is Low

  @weather @ice @edge
  Scenario Outline: Boundary conditions around classification
    Given overnight forecast low is <low>°C
    And overnight dew point is <dew>°C
    When I request the overnight ice risk
    Then the ice risk probability is <risk>
    Examples:
      | low | dew | risk   |
      | 0   | -1  | High   |
      | 2   | 1   | Medium |
      | 3   | 2   | Low    |

  @weather @ice @wip
  Scenario: Ice risk accounts for expected radiative cooling adjustment
    Given overnight forecast low is 2°C
    And overnight dew point is 1°C
    And radiative cooling adjustment is -1.5°C
    When I request the overnight ice risk
    Then the adjusted forecast low is 0.5°C
    And the ice risk probability is Medium

  # ---------------------------------------------------------------------------
  # INTEGRATION PLACEHOLDERS (@wip for external API, serialization, error handling)
  # ---------------------------------------------------------------------------
  @weather @integration @wip
  Scenario: Network failure retries with exponential backoff
    Given the poll interval is 900 seconds
    And the last 2 fetch attempts failed with error codes
      | attempt | code |
      | 1       | 500  |
      | 2       | 500  |
    When the next fetch window arrives
    Then a retry is scheduled with backoff

  @weather @integration @wip
  Scenario: Weather data persists across app restart
    Given cached weather data exists from 120 seconds ago
    And the active app is "Weather"
    Then no immediate fetch is performed
    And the current conditions are displayed from cache
