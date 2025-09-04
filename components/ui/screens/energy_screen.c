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
