#include "battery.h"

void battery_update_proc(Layer *layer, GContext *ctx)
{
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 5, 18, 9), 0, GCornerNone);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_pixel(ctx, GPoint(16, 5));
  graphics_draw_pixel(ctx, GPoint(17, 5));
  graphics_draw_pixel(ctx, GPoint(17, 6));
  graphics_draw_pixel(ctx, GPoint(17, 7));
  graphics_draw_pixel(ctx, GPoint(17, 11));
  graphics_draw_pixel(ctx, GPoint(17, 12));
  graphics_draw_pixel(ctx, GPoint(17, 13));
  graphics_draw_pixel(ctx, GPoint(16, 13));
  graphics_draw_pixel(ctx, GPoint(0, 5));
  graphics_draw_pixel(ctx, GPoint(0, 13));
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(2, 7, 13, 5), 0, GCornerNone);
  if (s_battery_charging) {
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_line(ctx, GPoint(9, 4), GPoint(6, 8));
    graphics_draw_line(ctx, GPoint(6, 9), GPoint(10, 8));
    graphics_draw_line(ctx, GPoint(10, 9), GPoint(6, 15));
    
    graphics_draw_line(ctx, GPoint(10, 4), GPoint(7, 8));
    graphics_draw_line(ctx, GPoint(7, 9), GPoint(11, 8));
    graphics_draw_line(ctx, GPoint(11, 9), GPoint(7, 15));
    return;
  }
  graphics_context_set_fill_color(ctx, GColorBlack);
  if (s_battery_level > 0) {
    graphics_fill_rect(ctx, GRect(3, 8, 2, 3), 0, GCornerNone);
  }
  if (s_battery_level > 25) {
    graphics_fill_rect(ctx, GRect(6, 8, 2, 3), 0, GCornerNone);
  }
  if (s_battery_level > 50) {
    graphics_fill_rect(ctx, GRect(9, 8, 2, 3), 0, GCornerNone);
  }
  if (s_battery_level > 75) {
    graphics_fill_rect(ctx, GRect(12, 8, 2, 3), 0, GCornerNone);
  }
}

static void battery_callback(BatteryChargeState state)
{
  s_battery_level = state.charge_percent;
  s_battery_charging = state.is_charging;
  layer_mark_dirty(s_battery_layer);
}

void battery_init()
{
  battery_state_service_subscribe(battery_callback);
  battery_callback(battery_state_service_peek());
}

void battery_load(Layer *window_layer)
{
  s_battery_layer = layer_create(GRect(120, PBL_IF_ROUND_ELSE(0, 0), 18, 19));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_add_child(window_layer, s_battery_layer);
  layer_mark_dirty(s_battery_layer);
}

void battery_unload()
{
  layer_destroy(s_battery_layer);
}
