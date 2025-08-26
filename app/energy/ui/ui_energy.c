

#include <stddef.h>
#include <lvgl.h>
#include "../../managed_components/lvgl__lvgl/src/extra/widgets/meter/lv_meter.h"
#include <stdio.h>
#include <math.h>
#include <inttypes.h>

// UI handles
static lv_obj_t *energy_meter = NULL;
static lv_meter_scale_t *meter_scale = NULL;
static lv_meter_indicator_t *needle_balance = NULL;
static lv_meter_indicator_t *needle_peak = NULL;
static lv_obj_t *units_label = NULL;
static lv_obj_t *debug_label = NULL;

void ui_energy_destroy(void) {
    energy_meter = NULL;
    meter_scale = NULL;
    needle_balance = NULL;
    needle_peak = NULL;
    units_label = NULL;
}

// Expose handles for update
lv_obj_t *ui_energy_meter(void) { return energy_meter; }
lv_meter_indicator_t *ui_energy_needle_balance(void) { return needle_balance; }
lv_meter_indicator_t *ui_energy_needle_peak(void) { return needle_peak; }

void ui_energy_create(lv_obj_t *parent, float initial_balance, float initial_peak) {
    energy_meter = lv_meter_create(parent);
    lv_obj_set_size(energy_meter, 360, 360);
    lv_obj_align(energy_meter, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(energy_meter, lv_color_black(), 0); // dark background
    meter_scale = lv_meter_add_scale(energy_meter);
    // 270 degree arc, -6000 to +6000 (watts), 0 at the top (270 deg), arc from 135 to 45 deg
    lv_meter_set_scale_range(energy_meter, meter_scale, -6000, 6000, 270, 135);
    // 25 minor ticks for -6kW to +6kW (every 0.5kW/500W), so each major tick (every 2nd) is 1kW/1000W
    lv_meter_set_scale_ticks(energy_meter, meter_scale, 25, 2, 10, lv_palette_main(LV_PALETTE_GREY));
    // Major ticks every 2nd minor tick (i.e., every 1000W/1kW), hide default numeric labels
    lv_meter_set_scale_major_ticks(energy_meter, meter_scale, 2, 4, 20, lv_palette_main(LV_PALETTE_GREY), 1000);
    // Place static kW labels at every 2nd minor tick (i.e., -6, -5, ..., 0, ..., +5, +6)
    static const int major_tick_count = 13;
    static const int min_watt = -6000;
    static const int max_watt = 6000;
    static const int radius = 130; // distance from center, adjust as needed
    static const int center_x = 180; // half of 360 (meter size)
    static const int center_y = 180;
    for (int i = 0; i < major_tick_count; ++i) {
        int32_t watt = min_watt + i * (max_watt - min_watt) / (major_tick_count - 1);
        int32_t kw = watt / 1000;
        char buf[8];
        snprintf(buf, sizeof(buf), "%" PRId32, kw);
        // Calculate angle for this tick
        float angle = 135 + (270.0f * i) / (major_tick_count - 1);
    float rad = angle * (M_PI / 180.0f);
        int x = center_x + (int)(radius * cosf(rad));
        int y = center_y + (int)(radius * sinf(rad));
        lv_obj_t *lbl = lv_label_create(energy_meter);
        lv_label_set_text(lbl, buf);
        lv_obj_set_style_text_color(lbl, lv_palette_main(LV_PALETTE_RED), 0);
        extern const lv_font_t lv_font_montserrat_16;
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, 0);
        lv_obj_set_style_bg_opa(lbl, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_opa(lbl, LV_OPA_TRANSP, 0);
        lv_obj_set_style_pad_all(lbl, 0, 0);
        lv_obj_align(lbl, LV_ALIGN_CENTER, x - center_x, y - center_y);
    }
    // Debug label in the center, use default font for compatibility
    debug_label = lv_label_create(energy_meter);
    lv_obj_set_style_text_color(debug_label, lv_color_white(), 0);
    extern const lv_font_t lv_font_montserrat_12;
    lv_obj_set_style_text_font(debug_label, &lv_font_montserrat_12, 0);
    lv_obj_align(debug_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(debug_label, "bal: 0\npeak: 0");

    // Set gauge scale label color to red
    lv_obj_set_style_text_color(energy_meter, lv_palette_main(LV_PALETTE_RED), LV_PART_TICKS);
    needle_balance = lv_meter_add_needle_line(energy_meter, meter_scale, 4, lv_palette_main(LV_PALETTE_BLUE), 0);
    needle_peak = lv_meter_add_needle_line(energy_meter, meter_scale, 2, lv_color_make(255, 255, 64), 0);
    lv_meter_set_indicator_value(energy_meter, needle_balance, (int32_t)initial_balance);
    lv_meter_set_indicator_value(energy_meter, needle_peak, (int32_t)initial_peak);
    // Add units label
    units_label = lv_label_create(parent);
    lv_label_set_text(units_label, "kW");
    lv_obj_set_style_text_color(units_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(units_label, LV_FONT_DEFAULT, 0);
    lv_obj_align(units_label, LV_ALIGN_CENTER, 0, 80);
}

void ui_energy_update(float balance, float peak) {
    if (!energy_meter || !needle_balance || !needle_peak) return;
    lv_meter_set_indicator_value(energy_meter, needle_balance, (int32_t)balance);
    lv_meter_set_indicator_value(energy_meter, needle_peak, (int32_t)peak);
    // Color logic for main pointer: green (<0), orange (0-2), red (>2), gradient
    lv_color_t color;
    if (balance < 0) {
        color = lv_palette_main(LV_PALETTE_GREEN);
    } else if (balance <= 2) {
        uint8_t r = (uint8_t)(255 * (balance / 2.0f));
        uint8_t g = (uint8_t)(128 + 127 * (1 - (balance / 2.0f)));
        color = lv_color_make(r, g, 0);
    } else {
        color = lv_palette_main(LV_PALETTE_RED);
    }
    // Optionally set color for balance needle if supported
    // lv_obj_set_style_line_color(needle_balance, color, 0);

    // Update debug label with all five variables
    extern float energy_solar, energy_used, energy_balance, energy_peak_solar, energy_peak_used;
    if (debug_label) {
        char buf[128];
        snprintf(buf, sizeof(buf),
            "solar: %.2f\nused: %.2f\nbal: %.2f\npeak_s: %.2f\npeak_u: %.2f",
            energy_solar / 1000.0f,
            energy_used / 1000.0f,
            energy_balance / 1000.0f,
            energy_peak_solar / 1000.0f,
            energy_peak_used / 1000.0f);
        lv_label_set_text(debug_label, buf);
    }
}
