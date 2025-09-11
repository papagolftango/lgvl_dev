#include <stdio.h>
#include <math.h>
#include <string.h>
#include "energy_screen.h"
#include "lvgl.h"
extern const lv_img_dsc_t ui_img_rect1_png;

// ---- Gauge layout tuning --------------------------------------------------
// Base logical centre of the 360x360 dial
#define GAUGE_BASE_CENTER_X 180
#define GAUGE_BASE_CENTER_Y 180
// Adjustable pixel offsets (positive X moves right, positive Y moves down)
// Tweak these if the pointer pivot does not visually coincide with the dial centre
#ifndef GAUGE_CENTER_OFFSET_X
#define GAUGE_CENTER_OFFSET_X 0
#endif
#ifndef GAUGE_CENTER_OFFSET_Y
#define GAUGE_CENTER_OFFSET_Y 0
#endif
// Peak marker geometry (short dashes near arc circumference)
#define PEAK_MARKER_OUTER_INSET 6      // distance inward from arc radius to outer endpoint
#define PEAK_MARKER_LENGTH     22      // length of the short peak marker line
// Trim a bit from the indicator's visible end to soften the edge (in degrees)
#ifndef ARC_END_TRIM_DEG
#define ARC_END_TRIM_DEG 2
#endif

// Balance arc diameter (pixels). Reduce to make the arc radius smaller.
#ifndef BALANCE_ARC_DIAMETER
#define BALANCE_ARC_DIAMETER 220
#endif

// Single arc: balance only
static lv_obj_t *arc_balance = NULL;

// Peak marker lines (grey) and current value marker lines (black)
static lv_obj_t *peak_solar_line = NULL;
static lv_obj_t *peak_used_line  = NULL;
static lv_point_t peak_solar_points[2];
static lv_point_t peak_used_points[2];
static lv_obj_t *curr_solar_line = NULL;
static lv_obj_t *curr_used_line  = NULL;
static lv_point_t curr_solar_points[2];
static lv_point_t curr_used_points[2];

// Radius cache for balance arc (pixels)
static int radius_balance = 0;

// Styles for markers and label
static lv_style_t style_peak_solar;
static lv_style_t style_peak_used;
static lv_style_t style_curr_marker;
static bool styles_inited = false;

// Static pointer to the root object of the Energy screen
static lv_obj_t *energy_root = NULL;
// New labels for central balance value and its title
static lv_obj_t *balance_value_label = NULL;
static lv_obj_t *balance_title_label = NULL;
static lv_style_t style_balance_value_label;
static lv_style_t style_balance_title_label;
static bool balance_label_styles_initialized = false;
// Units label placed next to the center numeric value
// Removed small center-units label beside numeric

// Center display modes (easy to extend): order defines tap-cycling sequence
typedef enum {
    CENTER_BALANCE = 0,
    CENTER_SOLAR,
    CENTER_USED,
    CENTER_PEAK_SOLAR,
    CENTER_PEAK_USED,
    CENTER_MODE_COUNT
} center_mode_t;

static center_mode_t current_center_mode = CENTER_BALANCE;

// Last-known values from controller for instant refresh on tap
static float last_balance_val = 0.0f;
static float last_solar_val = 0.0f;
static float last_used_val = 0.0f;
static float last_peak_solar_val = 0.0f;
static float last_peak_used_val = 0.0f;

// Helper to color by power-like magnitude
static lv_color_t color_for_value(float v) {
    if (v < 0) return lv_color_hex(0x40FF6D); // green export
    if (v <= 1500) return lv_color_hex(0xFFA500); // orange
    return lv_color_hex(0xFF3B30); // red
}

// Format integer with thousands separators (e.g., -123,456)
static void format_with_thousands(int v, char *out, size_t outsz) {
    if (!out || outsz == 0) return;
    char tmp[32];
    bool neg = v < 0;
    unsigned int u = neg ? (unsigned int)(- (long)v) : (unsigned int)v;
    // raw digits
    snprintf(tmp, sizeof(tmp), "%u", u);
    size_t len = strlen(tmp);
    // compute commas: every 3 digits before end
    size_t commas = (len > 3) ? ( (len - 1) / 3 ) : 0;
    size_t needed = len + commas + (neg ? 1 : 0) + 1; // +1 for NUL
    if (outsz < needed) {
        // fall back to simple number if buffer too small
        snprintf(out, outsz, "%d", v);
        return;
    }
    char *p = out + needed - 1;
    *p-- = '\0';
    size_t d = 0; // digits placed since last comma
    for (ssize_t i = (ssize_t)len - 1; i >= 0; --i) {
        *p-- = tmp[i];
        if (++d == 3 && i > 0) { *p-- = ','; d = 0; }
    }
    if (neg) *p = '-';
}

static void refresh_center_display(void) {
    if (!balance_value_label || !balance_title_label) return;
    const char *title = "";
    float value = 0.0f;
    switch (current_center_mode) {
        case CENTER_BALANCE:    title = "Balance";    value = last_balance_val;    break;
        case CENTER_SOLAR:      title = "Solar";      value = last_solar_val;      break;
        case CENTER_USED:       title = "Used";       value = last_used_val;       break;
        case CENTER_PEAK_SOLAR: title = "Peak Solar"; value = last_peak_solar_val; break;
        case CENTER_PEAK_USED:  title = "Peak Used";  value = last_peak_used_val;  break;
        default:                title = "";          value = 0.0f;                break;
    }
    lv_label_set_text(balance_title_label, title);

    char num[24];
    // Integer display with thousands separators
    format_with_thousands((int)lroundf(value), num, sizeof(num));
    lv_label_set_text(balance_value_label, num);

    lv_color_t c = color_for_value(value);
    lv_obj_set_style_text_color(balance_value_label, c, LV_PART_MAIN);
}

static void cycle_center_mode(void) {
    current_center_mode = (center_mode_t)((((int)current_center_mode) + 1) % CENTER_MODE_COUNT);
    refresh_center_display();
}

static void energy_root_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_PRESSED || code == LV_EVENT_SHORT_CLICKED) {
        cycle_center_mode();
    }
}


// Helper to map a value to an angle (deg) for the gauge
static float energy_value_to_angle(float value) {
    if (value < 0.0f) {
        float frac = fminf(1.0f, sqrtf(fabsf(value) / 4000.0f));
        return -135.0f * frac;
    } else if (value > 0.0f) {
        float frac = fminf(1.0f, sqrtf(value / 6000.0f));
        return 135.0f * frac;
    } else {
        return 0.0f;
    }
}

// Draw a short peak marker exactly on an arc's centerline (does not originate at centre)
static void draw_peak_marker_line(lv_obj_t **line_obj, lv_point_t *line_points, float value, bool invert, int container_r, int arc_width, lv_style_t *style, lv_obj_t *parent, int center_x, int center_y) {
    float angle = energy_value_to_angle(invert ? -value : value);
    float rad = angle * (M_PI / 180.0f);
    // Place the outer endpoint on the arc centerline: container radius minus half the arc stroke width
    int outer_r = container_r - (arc_width / 2);
    int inner_r = outer_r - PEAK_MARKER_LENGTH;
    if (inner_r < 0) inner_r = 0;

    int x_outer = container_r + (int)roundf(outer_r * sinf(rad));
    int y_outer = container_r - (int)roundf(outer_r * cosf(rad));
    int x_inner = container_r + (int)roundf(inner_r * sinf(rad));
    int y_inner = container_r - (int)roundf(inner_r * cosf(rad));

    line_points[0].x = x_inner;
    line_points[0].y = y_inner;
    line_points[1].x = x_outer;
    line_points[1].y = y_outer;

    if (!*line_obj) {
        *line_obj = lv_line_create(parent);
        lv_obj_set_pos(*line_obj, center_x - container_r, center_y - container_r);
        lv_obj_set_size(*line_obj, 2*container_r, 2*container_r);
        lv_obj_move_foreground(*line_obj);
        lv_obj_add_style(*line_obj, style, LV_PART_MAIN);
    }
    lv_line_set_points(*line_obj, line_points, 2);
}

// Ensure arcs exist and are styled; sizes chosen to be concentric inside 360x360 root
static void ensure_arcs_created(lv_obj_t *parent) {
    if (arc_balance) return;

    // Colors for arcs
    lv_color_t color_balance = lv_color_hex(0x00B0FF); // light blue

    // Create balance arc (only arc)
    if (!arc_balance) {
        arc_balance = lv_arc_create(parent);
    // Use configurable diameter for the single balance arc
    lv_obj_set_size(arc_balance, BALANCE_ARC_DIAMETER, BALANCE_ARC_DIAMETER);
        lv_obj_center(arc_balance);
        lv_obj_clear_flag(arc_balance, LV_OBJ_FLAG_CLICKABLE);
        // Background span of gauge: -135..+135 around top => 135 -> 45 (wrap)
        lv_arc_set_bg_angles(arc_balance, 135, 45);
        // Thicker balance arc
    lv_obj_set_style_arc_width(arc_balance, 18, LV_PART_MAIN);       // track
    lv_obj_set_style_arc_width(arc_balance, 18, LV_PART_INDICATOR);  // indicator
    // Flat ends: remove rounded caps
    lv_obj_set_style_arc_rounded(arc_balance, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(arc_balance, 0, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_balance, lv_color_hex(0x2A2A2A), LV_PART_MAIN);
        lv_obj_set_style_arc_opa(arc_balance, LV_OPA_40, LV_PART_MAIN);
        lv_obj_set_style_arc_color(arc_balance, color_balance, LV_PART_INDICATOR);
    // Hide the default knob (circular marker at the end)
    lv_obj_set_style_bg_opa(arc_balance, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_border_opa(arc_balance, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_outline_opa(arc_balance, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_shadow_opa(arc_balance, LV_OPA_TRANSP, LV_PART_KNOB);
    radius_balance = lv_obj_get_width(arc_balance) / 2;
    }

    // Marker styles: peaks grey; current markers black
    if (!styles_inited) {
        lv_style_init(&style_peak_solar);
        lv_style_set_line_width(&style_peak_solar, 3);
        lv_style_set_line_color(&style_peak_solar, lv_color_hex(0x808080)); // grey
        lv_style_set_line_opa(&style_peak_solar, LV_OPA_80);
        lv_style_set_line_rounded(&style_peak_solar, true);

        lv_style_init(&style_peak_used);
        lv_style_set_line_width(&style_peak_used, 3);
        lv_style_set_line_color(&style_peak_used, lv_color_hex(0x808080)); // grey
        lv_style_set_line_opa(&style_peak_used, LV_OPA_80);
        lv_style_set_line_rounded(&style_peak_used, true);

        lv_style_init(&style_curr_marker);
        lv_style_set_line_width(&style_curr_marker, 3);
        lv_style_set_line_color(&style_curr_marker, lv_color_hex(0x000000)); // black
        lv_style_set_line_opa(&style_curr_marker, LV_OPA_100);
        lv_style_set_line_rounded(&style_curr_marker, true);

        styles_inited = true;
    }
}

// Update arcs based on current values and render peak tick marks on mid/inner arcs
void draw_pointer_and_peaks(float energy_balance, float peak_solar, float peak_used, float curr_solar, float curr_used) {
    (void)peak_solar; // used for peak marker only
    (void)peak_used;  // used for peak marker only

    lv_obj_t *parent = energy_root ? energy_root : lv_scr_act();
    ensure_arcs_created(parent);

    // Map balance to angles relative to top (270deg)
    float a_bal = energy_value_to_angle(energy_balance);

    const int base_top = 270; // LVGL angle for top

    int bal_start = (a_bal < 0) ? (base_top + (int)lroundf(a_bal)) : base_top;
    int bal_end   = (a_bal < 0) ? base_top : (base_top + (int)lroundf(a_bal));
    // Apply end trim: trim the variable end for positive values, or the variable start for negative values
    if (a_bal > 0) {
        bal_end -= ARC_END_TRIM_DEG;
        if (bal_end < bal_start) bal_end = bal_start;
    } else if (a_bal < 0) {
        bal_start += ARC_END_TRIM_DEG;
        if (bal_start > bal_end) bal_start = bal_end;
    }
    lv_arc_set_start_angle(arc_balance, bal_start);
    lv_arc_set_end_angle(arc_balance, bal_end);

    // Draw markers on balance arc centerline
    int center_x = GAUGE_BASE_CENTER_X + GAUGE_CENTER_OFFSET_X;
    int center_y = GAUGE_BASE_CENTER_Y + GAUGE_CENTER_OFFSET_Y;

    // Container radius and stroke width for balance arc
    int cont_r_bal = lv_obj_get_width(arc_balance) / 2;
    int width_bal  = lv_obj_get_style_arc_width(arc_balance, LV_PART_INDICATOR);
    if (width_bal <= 0) width_bal = lv_obj_get_style_arc_width(arc_balance, LV_PART_MAIN);

    // Peak markers (grey) – solar is inverted; used is normal
    draw_peak_marker_line(&peak_solar_line, peak_solar_points, peak_solar, true,  cont_r_bal, width_bal, &style_peak_solar, parent, center_x, center_y);
    draw_peak_marker_line(&peak_used_line,  peak_used_points,  peak_used,  false, cont_r_bal, width_bal, &style_peak_used,  parent, center_x, center_y);

    // Current value markers (black) – inside the same arc
    draw_peak_marker_line(&curr_solar_line, curr_solar_points, curr_solar, true,  cont_r_bal, width_bal, &style_curr_marker, parent, center_x, center_y);
    draw_peak_marker_line(&curr_used_line,  curr_used_points,  curr_used,  false, cont_r_bal, width_bal, &style_curr_marker, parent, center_x, center_y);

    // Store last-known values for instant UI refresh on taps and update center now
    last_balance_val = energy_balance;
    last_solar_val = curr_solar;
    last_used_val = curr_used;
    last_peak_solar_val = peak_solar;
    last_peak_used_val = peak_used;
    refresh_center_display();
}

lv_obj_t *energy_screen_create(lv_obj_t *parent) {
    if (energy_root) return energy_root; // Already created
    energy_root = lv_obj_create(parent);
    lv_obj_set_size(energy_root, 360, 360); // Set to 360x360 for circular display
    lv_obj_set_style_bg_color(energy_root, lv_color_hex(0x09032E), LV_PART_MAIN);
    lv_obj_clear_flag(energy_root, LV_OBJ_FLAG_SCROLLABLE);

    // Add the gauge face image as a background
    lv_obj_t *bg_img = lv_img_create(energy_root);
    lv_img_set_src(bg_img, &ui_img_rect1_png);
    lv_obj_set_size(bg_img, 360, 360);
    lv_obj_align(bg_img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_move_background(bg_img); // Ensure it's at the back

    // Create concentric arcs layered above the background
    ensure_arcs_created(energy_root);

    // Central balance title and numeric value, nearer the center
    if (!balance_label_styles_initialized) {
    lv_style_init(&style_balance_title_label);
#if LV_FONT_MONTSERRAT_16
    lv_style_set_text_font(&style_balance_title_label, &lv_font_montserrat_16);
#elif LV_FONT_MONTSERRAT_14
    lv_style_set_text_font(&style_balance_title_label, &lv_font_montserrat_14);
#elif LV_FONT_MONTSERRAT_12
    lv_style_set_text_font(&style_balance_title_label, &lv_font_montserrat_12);
#endif
    lv_style_set_text_opa(&style_balance_title_label, LV_OPA_90);

    lv_style_init(&style_balance_value_label);
#if LV_FONT_MONTSERRAT_24
    lv_style_set_text_font(&style_balance_value_label, &lv_font_montserrat_24);
#elif LV_FONT_MONTSERRAT_22
    lv_style_set_text_font(&style_balance_value_label, &lv_font_montserrat_22);
#elif LV_FONT_MONTSERRAT_20
    lv_style_set_text_font(&style_balance_value_label, &lv_font_montserrat_20);
#endif
    lv_style_set_text_opa(&style_balance_value_label, LV_OPA_100);
    balance_label_styles_initialized = true;
    }

    balance_title_label = lv_label_create(energy_root);
    lv_label_set_text(balance_title_label, "Balance");
    lv_obj_add_style(balance_title_label, &style_balance_title_label, LV_PART_MAIN);
    lv_obj_align(balance_title_label, LV_ALIGN_CENTER, 0, 20);

    balance_value_label = lv_label_create(energy_root);
    lv_label_set_text(balance_value_label, "0");
    lv_obj_add_style(balance_value_label, &style_balance_value_label, LV_PART_MAIN);
    lv_obj_align(balance_value_label, LV_ALIGN_CENTER, 0, 48);

    // Removed center-units label creation

    // Make the whole screen react to taps to cycle center display
    lv_obj_add_flag(energy_root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(energy_root, energy_root_event_cb, LV_EVENT_ALL, NULL);

    // Add more UI elements as needed
    return energy_root;
}

void energy_screen_destroy(void) {
    if (energy_root) {
        lv_obj_del(energy_root);
    energy_root = NULL;
    }
}

void energy_screen_set_value(int value) {
    // Update center numeric and units color; keep units label text constant
    if (balance_value_label) {
        char num[24];
        format_with_thousands(value, num, sizeof(num));
        lv_label_set_text(balance_value_label, num);

        // Color: green for negative, orange up to 1.5k, red above
        int watts = value; // adjust if value is other units
        lv_color_t color;
        if (watts < 0) {
            color = lv_color_hex(0x40FF6D); // green
        } else if (watts <= 1500) {
            color = lv_color_hex(0xFFA500); // orange
        } else {
            color = lv_color_hex(0xFF3B30); // red
        }
        lv_obj_set_style_text_color(balance_value_label, color, LV_PART_MAIN);
    // Removed center-units label updates
    }
    // Removed bottom units label color sync
}

lv_obj_t *energy_screen_get_root(void) {
    return energy_root;
}
