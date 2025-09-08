

#include <stdio.h>
#include "energy_screen.h"
#include "lvgl.h"
extern const lv_img_dsc_t ui_img_gauge_face_kw_final_png;

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
// Pointer radius (distance from pivot to pointer tip)
#define POINTER_RADIUS 125
// Peak marker geometry (short dashes near outer edge)
#define PEAK_MARKER_OUTER_INSET 6      // distance inward from full radius to outer endpoint
#define PEAK_MARKER_LENGTH     22      // length of the short peak marker line


// Persistent pointer line object and style for energy balance pointer
static lv_obj_t *pointer_line = NULL;
static lv_obj_t *peak_solar_line = NULL;
static lv_obj_t *peak_used_line = NULL;
static lv_obj_t *curr_solar_line = NULL;
static lv_obj_t *curr_used_line = NULL;
static lv_point_t line_points[2];
static lv_point_t peak_solar_points[2];
static lv_point_t peak_used_points[2];
static lv_point_t curr_solar_points[2];
static lv_point_t curr_used_points[2];
static lv_style_t style_line_blue;
static lv_style_t style_peak_solar;
static lv_style_t style_peak_used;
static lv_style_t style_curr_solar;
static lv_style_t style_curr_used;
static bool style_initialized = false;

// Static pointer to the root object of the Energy screen
static lv_obj_t *energy_root = NULL;
static lv_obj_t *energy_label = NULL;
static lv_style_t style_energy_label;
static bool energy_label_style_initialized = false;


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


// Helper to draw a marker line (peak/current, solar/used)
static void draw_marker_line(lv_obj_t **line_obj, lv_point_t *line_points, float value, bool invert, int r, lv_style_t *style, lv_obj_t *parent, int center_x, int center_y) {
    float angle = energy_value_to_angle(invert ? -value : value);
    float rad = angle * (M_PI / 180.0f);
    int r_marker = r - 20;
    int x1 = r + (int)roundf(r_marker * sinf(rad));
    int y1 = r - (int)roundf(r_marker * cosf(rad));
    line_points[0].x = r;
    line_points[0].y = r;
    line_points[1].x = x1;
    line_points[1].y = y1;
    if (!*line_obj) {
        *line_obj = lv_line_create(parent);
        lv_obj_set_pos(*line_obj, center_x - r, center_y - r);
        lv_obj_set_size(*line_obj, 2*r, 2*r);
        lv_obj_move_foreground(*line_obj);
        lv_obj_add_style(*line_obj, style, LV_PART_MAIN);
    }
    lv_line_set_points(*line_obj, line_points, 2);
}

// Draw a short peak marker near the circumference (does not originate at centre)
static void draw_peak_marker_line(lv_obj_t **line_obj, lv_point_t *line_points, float value, bool invert, int r, lv_style_t *style, lv_obj_t *parent, int center_x, int center_y) {
    float angle = energy_value_to_angle(invert ? -value : value);
    float rad = angle * (M_PI / 180.0f);
    // Outer and inner radii for the short tick
    int outer_r = r - PEAK_MARKER_OUTER_INSET;
    int inner_r = outer_r - PEAK_MARKER_LENGTH;
    if (inner_r < 0) inner_r = 0;

    int x_outer = r + (int)roundf(outer_r * sinf(rad));
    int y_outer = r - (int)roundf(outer_r * cosf(rad));
    int x_inner = r + (int)roundf(inner_r * sinf(rad));
    int y_inner = r - (int)roundf(inner_r * cosf(rad));

    line_points[0].x = x_inner;
    line_points[0].y = y_inner;
    line_points[1].x = x_outer;
    line_points[1].y = y_outer;

    if (!*line_obj) {
        *line_obj = lv_line_create(parent);
        lv_obj_set_pos(*line_obj, center_x - r, center_y - r);
        lv_obj_set_size(*line_obj, 2*r, 2*r);
        lv_obj_move_foreground(*line_obj);
        lv_obj_add_style(*line_obj, style, LV_PART_MAIN);
    }
    lv_line_set_points(*line_obj, line_points, 2);
}

void draw_pointer_and_peaks(float energy_balance, float peak_solar, float peak_used, float curr_solar, float curr_used) {
    // Main pointer
    float angle = energy_value_to_angle(energy_balance);
    float rad = angle * (M_PI / 180.0f);
    int r = POINTER_RADIUS;
    int center_x = GAUGE_BASE_CENTER_X + GAUGE_CENTER_OFFSET_X;
    int center_y = GAUGE_BASE_CENTER_Y + GAUGE_CENTER_OFFSET_Y;
    int x0 = r, y0 = r;
    int x1 = r + (int)roundf(r * sinf(rad));
    int y1 = r - (int)roundf(r * cosf(rad));
    line_points[0].x = x0;
    line_points[0].y = y0;
    line_points[1].x = x1;
    line_points[1].y = y1;

    lv_obj_t *parent = lv_scr_act();
    if (!pointer_line) {
        pointer_line = lv_line_create(parent);
        lv_obj_set_pos(pointer_line, center_x - r, center_y - r);
        lv_obj_set_size(pointer_line, 2*r, 2*r);
        lv_obj_move_foreground(pointer_line);
    }
    if (!style_initialized) {
        lv_style_init(&style_line_blue);
        lv_style_set_line_width(&style_line_blue, 5);
        lv_style_set_line_color(&style_line_blue, lv_color_hex(0x0000FF));
        lv_style_set_line_rounded(&style_line_blue, true);

        lv_style_init(&style_peak_solar);
        lv_style_set_line_width(&style_peak_solar, 3);
        lv_style_set_line_color(&style_peak_solar, lv_color_hex(0xFFD600)); // yellow
        lv_style_set_line_opa(&style_peak_solar, LV_OPA_40); // faded
        lv_style_set_line_rounded(&style_peak_solar, true);

        lv_style_init(&style_peak_used);
        lv_style_set_line_width(&style_peak_used, 3);
        lv_style_set_line_color(&style_peak_used, lv_color_hex(0xFF3B30)); // red
        lv_style_set_line_opa(&style_peak_used, LV_OPA_40); // faded
        lv_style_set_line_rounded(&style_peak_used, true);

        lv_style_init(&style_curr_solar);
        lv_style_set_line_width(&style_curr_solar, 3);
        lv_style_set_line_color(&style_curr_solar, lv_color_hex(0xFFD600)); // yellow
        lv_style_set_line_opa(&style_curr_solar, LV_OPA_COVER); // solid
        lv_style_set_line_rounded(&style_curr_solar, true);

        lv_style_init(&style_curr_used);
        lv_style_set_line_width(&style_curr_used, 3);
        lv_style_set_line_color(&style_curr_used, lv_color_hex(0xFF3B30)); // red
        lv_style_set_line_opa(&style_curr_used, LV_OPA_COVER); // solid
        lv_style_set_line_rounded(&style_curr_used, true);

        style_initialized = true;
    }
    lv_obj_add_style(pointer_line, &style_line_blue, LV_PART_MAIN);
    lv_line_set_points(pointer_line, line_points, 2);

    // Draw all marker lines using the helper
    draw_peak_marker_line(&peak_solar_line, peak_solar_points, peak_solar, true, r, &style_peak_solar, parent, center_x, center_y);
    draw_marker_line(&curr_solar_line, curr_solar_points, curr_solar, true, r, &style_curr_solar, parent, center_x, center_y);
    draw_peak_marker_line(&peak_used_line, peak_used_points, peak_used, false, r, &style_peak_used, parent, center_x, center_y);
    draw_marker_line(&curr_used_line, curr_used_points, curr_used, false, r, &style_curr_used, parent, center_x, center_y);
}

lv_obj_t *energy_screen_create(lv_obj_t *parent) {
    if (energy_root) return energy_root; // Already created
    energy_root = lv_obj_create(parent);
    lv_obj_set_size(energy_root, 360, 360); // Set to 360x360 for circular display
    lv_obj_set_style_bg_color(energy_root, lv_color_hex(0x09032E), LV_PART_MAIN);
    lv_obj_clear_flag(energy_root, LV_OBJ_FLAG_SCROLLABLE);

    // Add the gauge face image as a background
    lv_obj_t *bg_img = lv_img_create(energy_root);
    lv_img_set_src(bg_img, &ui_img_gauge_face_kw_final_png);
    lv_obj_set_size(bg_img, 360, 360);
    lv_obj_align(bg_img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_move_background(bg_img); // Ensure it's at the back

    // Add a label on top of the image
    energy_label = lv_label_create(energy_root);
    lv_label_set_text(energy_label, "Energy: 0 kWh");
    if (!energy_label_style_initialized) {
    lv_style_init(&style_energy_label);
#if LV_FONT_MONTSERRAT_20
    lv_style_set_text_font(&style_energy_label, &lv_font_montserrat_20);
#elif LV_FONT_MONTSERRAT_18
    lv_style_set_text_font(&style_energy_label, &lv_font_montserrat_18);
#elif LV_FONT_MONTSERRAT_16
    lv_style_set_text_font(&style_energy_label, &lv_font_montserrat_16);
#endif
    // Slightly dim to reduce visual weight
    lv_style_set_text_opa(&style_energy_label, LV_OPA_90);
    energy_label_style_initialized = true;
    }
    lv_obj_add_style(energy_label, &style_energy_label, LV_PART_MAIN);
    // Position label below center (positive y moves downward)
    lv_obj_align(energy_label, LV_ALIGN_CENTER, 0, 70);

    // Add more UI elements as needed
    return energy_root;
}

void energy_screen_destroy(void) {
    if (energy_root) {
        lv_obj_del(energy_root);
        energy_root = NULL;
        energy_label = NULL;
    }
}

void energy_screen_set_value(int value) {
    if (energy_label) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Energy: %d kWh", value);
        lv_label_set_text(energy_label, buf);

        // Set color: green if negative, orange if 0-1.5kW, red if >1.5kW
        // value is in watts or kWh? Assuming watts for color logic, adjust if needed
        int watts = value; // If value is kWh, change to watts as needed
        lv_color_t color;
        if (watts < 0) {
            color = lv_color_hex(0x40FF6D); // green
        } else if (watts <= 1500) {
            color = lv_color_hex(0xFFA500); // orange
        } else {
            color = lv_color_hex(0xFF3B30); // red
        }
        lv_obj_set_style_text_color(energy_label, color, LV_PART_MAIN);
    }
}

lv_obj_t *energy_screen_get_root(void) {
    return energy_root;
}
