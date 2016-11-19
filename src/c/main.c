#include <pebble.h>

#include "modules/weather.h"
#include "modules/battery.h"

/* WINDOWS */
static Window *s_main_window;

/* CANVAS LAYERS */
static Layer *s_background_layer;

/* TEXT LAYERS */
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

/* FONTS */
static GFont s_time_font;
static GFont s_date_font;

/* MISC */
static GRect s_bounds;
static char *s_sys_locale;

static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *icon_tuple = dict_find(iterator, MESSAGE_KEY_ICON);

  if (temp_tuple && icon_tuple) {
    weather_received_data(icon_tuple->value->cstring, (int) temp_tuple->value->int32);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void update_time()
{
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // assign time to time_layer
  static char s_buffer_time[8];
  strftime(s_buffer_time, sizeof(s_buffer_time), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_buffer_time);
  
  // assign date to date_layer
  static char s_buffer_date[15];
  strftime(s_buffer_date, sizeof(s_buffer_date), "%a %d.%m.", tick_time);
  text_layer_set_text(s_date_layer, s_buffer_date);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Updated time");
}

static void tick_handler(struct tm *ticktime, TimeUnits  units_changed)
{
  update_time();
  if (ticktime->tm_min % 30 == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, 0, 0);
    app_message_outbox_send();
  }
}

static void background_update_proc(Layer *layer, GContext *ctx)
{
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, s_bounds, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, s_bounds, 10, GCornersAll);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, GPoint(5, 94), GPoint(139, 94));
}

static void main_window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  s_bounds = layer_get_bounds(window_layer);
  
  // BACKGROUND LAYER
  s_background_layer = layer_create(s_bounds);
  layer_set_update_proc(s_background_layer, background_update_proc);
  layer_add_child(window_layer, s_background_layer);
  layer_mark_dirty(s_background_layer);
  
  // WEATHER LAYER
  weather_load(window_layer, s_bounds.size.w);
  
  // TIME LAYER
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GAMPLAY_34));
  s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(106, 100), s_bounds.size.w, 50));
  
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // DATE LAYER
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GAMPLAY_20));
  s_date_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(146, 140), s_bounds.size.w, 50));
  
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  //layer_add_child(window_layer, text_layer_get_layer(s_weather_text_layer));

  // BATTERY LAYER
  battery_load(window_layer);
}

static void main_window_unload(Window *window)
{
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);  
  layer_destroy(s_background_layer);

  weather_unload();
  battery_unload();
  
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
}

static void init()
{
  s_sys_locale = setlocale(LC_ALL, "");
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(s_main_window, true);
  update_time();

  weather_init();
  battery_init();
  
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
}

static void deinit()
{
  window_destroy(s_main_window);
}

int main(void)
{
  init();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_main_window);
  app_event_loop();
  deinit();
}
