
#include "ui.h"
#include <math.h>
// Peak marker variables
static lv_obj_t *bar1_peak_marker = NULL;
static lv_obj_t *bar2_peak_marker = NULL;

void ui_update_bar1_peak_marker(float peak_val) {
   if (!ui_Bar1 || !bar1_peak_marker) return;
   float max_in = 4000.0f;
   float max_out = sqrtf(max_in);
   float scaled = sqrtf(peak_val) * max_in / max_out;
   int bar_width = lv_obj_get_width(ui_Bar1);
   int x_offset = (int)((scaled / max_in) * bar_width);
   lv_obj_set_x(bar1_peak_marker, lv_obj_get_x(ui_Bar1) + x_offset);
}

void ui_update_bar2_peak_marker(float peak_val) {
   if (!ui_Bar2 || !bar2_peak_marker) return;
   float max_in = 6000.0f;
   float max_out = sqrtf(max_in);
   float scaled = sqrtf(peak_val) * max_in / max_out;
   int bar_width = lv_obj_get_width(ui_Bar2);
   int x_offset = (int)((scaled / max_in) * bar_width);
   lv_obj_set_x(bar2_peak_marker, lv_obj_get_x(ui_Bar2) + x_offset);

}
   // Helper to update marker position (call from controller)



lv_obj_t *uic_balance;
lv_obj_t *uic_Energy;
lv_obj_t *ui_Energy = NULL;
lv_obj_t *ui_balance = NULL;lv_obj_t *ui_Bar1 = NULL;lv_obj_t *ui_Bar2 = NULL;
// event funtions

// build funtions

void ui_Energy_screen_init(void)
{
   // Create the main screen object as a root screen if not already created
   if (!ui_Energy) {
      ui_Energy = lv_obj_create(NULL); // root screen
   }
// Helper to load the energy screen as the active screen
void ui_Energy_screen_load(void) {
   if (ui_Energy) {
      lv_scr_load(ui_Energy);
   }
}
   lv_obj_set_style_shadow_color(ui_Energy, lv_color_hex(0xA01515), LV_PART_MAIN | LV_STATE_DEFAULT );
   lv_obj_set_style_shadow_opa(ui_Energy, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

   lv_obj_set_style_bg_color(ui_Energy, lv_color_hex(0xFFFFFF), LV_PART_SCROLLBAR | LV_STATE_DEFAULT );
   lv_obj_set_style_bg_opa(ui_Energy, 182, LV_PART_SCROLLBAR| LV_STATE_DEFAULT);
   lv_obj_set_style_bg_main_stop(ui_Energy, 0, LV_PART_SCROLLBAR| LV_STATE_DEFAULT);
   lv_obj_set_style_bg_grad_stop(ui_Energy, 255, LV_PART_SCROLLBAR| LV_STATE_DEFAULT);

   ui_balance = lv_arc_create(ui_Energy);
   lv_obj_set_width( ui_balance, 225);
   lv_obj_set_height( ui_balance, 227);
   lv_obj_set_align( ui_balance, LV_ALIGN_CENTER );
   lv_arc_set_value(ui_balance, 50);
   lv_arc_set_mode(ui_balance, LV_ARC_MODE_SYMMETRICAL);
   lv_obj_set_style_radius(ui_balance, 5, LV_PART_MAIN| LV_STATE_DEFAULT);
   lv_obj_set_style_bg_grad_color(ui_balance, lv_color_hex(0x1261B3), LV_PART_MAIN | LV_STATE_DEFAULT );
   lv_obj_set_style_border_color(ui_balance, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT );
   lv_obj_set_style_border_opa(ui_balance, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
   lv_obj_set_style_outline_color(ui_balance, lv_color_hex(0xEF1616), LV_PART_MAIN | LV_STATE_DEFAULT );
   lv_obj_set_style_outline_opa(ui_balance, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
   lv_obj_set_style_arc_color(ui_balance, lv_color_hex(0x40FF6D), LV_PART_MAIN | LV_STATE_DEFAULT );
   lv_obj_set_style_arc_opa(ui_balance, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

   lv_obj_set_style_bg_color(ui_balance, lv_color_hex(0xA82121), LV_PART_INDICATOR | LV_STATE_DEFAULT );
   lv_obj_set_style_bg_opa(ui_balance, 255, LV_PART_INDICATOR| LV_STATE_DEFAULT);
   lv_obj_set_style_outline_color(ui_balance, lv_color_hex(0x000000), LV_PART_INDICATOR | LV_STATE_DEFAULT );
   lv_obj_set_style_outline_opa(ui_balance, 255, LV_PART_INDICATOR| LV_STATE_DEFAULT);
   lv_obj_set_style_outline_width(ui_balance, 5, LV_PART_INDICATOR| LV_STATE_DEFAULT);
   lv_obj_set_style_outline_pad(ui_balance, 3, LV_PART_INDICATOR| LV_STATE_DEFAULT);
   lv_obj_set_style_arc_color(ui_balance, lv_color_hex(0xFF5D40), LV_PART_INDICATOR | LV_STATE_DEFAULT );
   lv_obj_set_style_arc_opa(ui_balance, 255, LV_PART_INDICATOR| LV_STATE_DEFAULT);

   ui_Bar1 = lv_bar_create(ui_Energy);
   lv_bar_set_range(ui_Bar1, 0, 4000); // Solar range
   lv_bar_set_value(ui_Bar1,25,LV_ANIM_OFF);
   lv_bar_set_start_value(ui_Bar1, 0, LV_ANIM_OFF);
   lv_obj_set_width( ui_Bar1, 150);
   lv_obj_set_height( ui_Bar1, 10);
   lv_obj_set_align( ui_Bar1, LV_ALIGN_CENTER );

   ui_Bar2 = lv_bar_create(ui_Energy);
   lv_bar_set_range(ui_Bar2, 0, 6000); // Used range
   lv_bar_set_value(ui_Bar2,25,LV_ANIM_OFF);
   lv_bar_set_start_value(ui_Bar2, 0, LV_ANIM_OFF);
   lv_obj_set_width( ui_Bar2, 150);
   lv_obj_set_height( ui_Bar2, 10);
   lv_obj_set_x( ui_Bar2, -2 );
   lv_obj_set_y( ui_Bar2, 108 );
   lv_obj_set_align( ui_Bar2, LV_ALIGN_CENTER );

// Create the peak marker after ui_Bar2 is created
   static lv_point_t bar2_line_points[2] = { {0, 0}, {0, 10} };
   bar2_peak_marker = lv_line_create(ui_Energy);
   lv_line_set_points(bar2_peak_marker, bar2_line_points, 2);
   lv_obj_set_style_line_width(bar2_peak_marker, 2, 0);
   lv_obj_set_style_line_color(bar2_peak_marker, lv_color_hex(0xFF0000), 0); // red
   lv_obj_align_to(bar2_peak_marker, ui_Bar2, LV_ALIGN_TOP_MID, 0, 0); // level with bar

   // Create the peak marker after ui_Bar1 is created
   static lv_point_t bar1_line_points[2] = { {0, 0}, {0, 10} };
   bar1_peak_marker = lv_line_create(ui_Energy);
   lv_line_set_points(bar1_peak_marker, bar1_line_points, 2);
   lv_obj_set_style_line_width(bar1_peak_marker, 2, 0);
   lv_obj_set_style_line_color(bar1_peak_marker, lv_color_hex(0xFF0000), 0); // red
   lv_obj_align_to(bar1_peak_marker, ui_Bar1, LV_ALIGN_TOP_MID, 0, 0); // level with bar

   uic_Energy = ui_Energy;
   uic_balance = ui_balance;

}

void ui_Energy_screen_destroy(void)
{
//  if (ui_Energy) lv_obj_del(ui_Energy);

   // NULL screen variables
   uic_Energy= NULL;
   ui_Energy= NULL;
   uic_balance= NULL;
   ui_balance= NULL;
   ui_Bar1= NULL;
   ui_Bar2= NULL;
}
