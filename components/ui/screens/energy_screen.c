// Persistent pointer line object and style for energy balance pointer
static lv_obj_t *pointer_line = NULL;
static lv_point_t line_points[2];
static lv_style_t style_line_blue;
static bool style_initialized = false;

void draw_pointer_for_balance(float energy_balance) {
    // Map: -4kW (FSD left) to -135°, 0kW (center) to 0°, +6kW (FSD right) to +135°
    // Use sqrt scale for both sides
    float angle = 0.0f;
    if (energy_balance < 0.0f) {
        // Negative: map [-4000, 0] W to [-135, 0] deg
        float frac = fminf(1.0f, sqrtf(fabsf(energy_balance) / 4000.0f));
        angle = -135.0f * frac;
    } else if (energy_balance > 0.0f) {
        // Positive: map [0, 6000] W to [0, +135] deg
        float frac = fminf(1.0f, sqrtf(energy_balance / 6000.0f));
        angle = 135.0f * frac;
    } else {
        angle = 0.0f;
    }

    float rad = angle * (M_PI / 180.0f);
    int r = 125;
    int x0 = r, y0 = r; // center of the object
    int x1 = r + (int)roundf(r * sinf(rad));
    int y1 = r - (int)roundf(r * cosf(rad));
    line_points[0].x = x0;
    line_points[0].y = y0;
    line_points[1].x = x1;
    line_points[1].y = y1;

    lv_obj_t *parent = lv_scr_act();
    if (!pointer_line) {
        pointer_line = lv_line_create(parent);
        lv_obj_set_pos(pointer_line, 180 - r, 180 - r);
        lv_obj_set_size(pointer_line, 2*r, 2*r);
        lv_obj_move_foreground(pointer_line);
        if (!style_initialized) {
            lv_style_init(&style_line_blue);
            lv_style_set_line_width(&style_line_blue, 5);
            lv_style_set_line_color(&style_line_blue, lv_color_hex(0x0000FF));
            lv_style_set_line_rounded(&style_line_blue, true);
            style_initialized = true;
        }
        lv_obj_add_style(pointer_line, &style_line_blue, LV_PART_MAIN);
    }
    lv_line_set_points(pointer_line, line_points, 2);
}
#include "energy_screen.h"
#include <stdio.h>

// Static pointer to the root object of the Energy screen
static lv_obj_t *energy_root = NULL;
static lv_obj_t *energy_label = NULL;

// Include the image asset declaration
#include "../ui.h"

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
    lv_obj_align(energy_label, LV_ALIGN_CENTER, 0, 0);

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
    }
}

lv_obj_t *energy_screen_get_root(void) {
    return energy_root;
}
