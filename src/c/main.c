#include <pebble.h>

/* WINDOWS */
static Window *s_main_window;

/* CANVAS LAYERS */
static Layer *s_background_layer;
static Layer *s_weather_layer;
static Layer *s_battery_layer;

/* TEXT LAYERS */
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_weather_text_layer;

/* FONTS */
static GFont s_time_font;
static GFont s_date_font;

/* MISC */
static GRect bounds;
static char *sys_locale;

/* BATTERY */
static int s_battery_level;
static bool s_battery_charging;

/* IMGS */
static GDrawCommandImage *s_weather_icons[11];
static int current = 0;
static GBitmap *s_weather_icon_default;

/* BUFFERS */
static char temperature_buffer[8];
static char current_weather_icon[4];

static void battery_update_proc(Layer *layer, GContext *ctx)
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

static void weather_icon_update_proc(Layer *layer, GContext *ctx)
{
  bool night = (current_weather_icon[2] == 'n');

  current_weather_icon[2] = '\0';
  int i = atoi(current_weather_icon);

  switch (i) {
  case 1:
    current = 1;
    if (night) current = 10;
    break;
  case 2:
    current = 2;
    break;
  case 3:
    current = 3;
    break;
  case 4:
    current = 4;
    break;
  case 9:
    current = 5;
    break;
  case 10:
    current = 6;
    break;
  case 11:
    current = 7;
  case 13:
    current = 8;
  case 50:
    current = 9;
  default:
    current = 0;
    GRect bitmap_bounds = gbitmap_get_bounds(s_weather_icon_default);
    bitmap_bounds.origin.x += 22;
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, s_weather_icon_default, bitmap_bounds);
  }

  
  if (night) {
    //current += 9;
    current_weather_icon[2] = 'n';
  } else {
    current_weather_icon[2] = 'd';
  }
  
  GPoint origin = GPoint(22, 0);
  gdraw_command_image_draw(ctx, s_weather_icons[current], origin);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *icon_tuple = dict_find(iterator, MESSAGE_KEY_ICON);

  if (temp_tuple && icon_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d C",
	     (int) temp_tuple->value->int32);
    text_layer_set_text(s_weather_text_layer, temperature_buffer);

    strcpy(current_weather_icon, icon_tuple->value->cstring);
    layer_mark_dirty(s_weather_layer);
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
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 10, GCornersAll);
}

static void main_window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);
  
  // BACKGROUND LAYER
  s_background_layer = layer_create(bounds);
  layer_set_update_proc(s_background_layer, background_update_proc);
  
  layer_add_child(window_layer, s_background_layer);
  
  layer_mark_dirty(s_background_layer);
  
  // WEATHER LAYER
  s_weather_layer = layer_create(GRect(0, 0, bounds.size.w, 100));
  layer_set_update_proc(s_weather_layer, weather_icon_update_proc);
  layer_add_child(window_layer, s_weather_layer);
  
  // TIME LAYER
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GAMPLAY_34));
  s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(106, 100), bounds.size.w, 50));
  
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // DATE LAYER
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GAMPLAY_20));
  s_date_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(146, 140), bounds.size.w, 50));
  
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  // WEATHER TEXT LAYER
  s_weather_text_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(125, 2), bounds.size.w, 25));
  text_layer_set_background_color(s_weather_text_layer, GColorClear);
  text_layer_set_text_color(s_weather_text_layer, GColorBlack);
  text_layer_set_text_alignment(s_weather_text_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_text_layer, "Loading...");
  text_layer_set_font(s_weather_text_layer, s_date_font);

  //layer_add_child(window_layer, text_layer_get_layer(s_weather_text_layer));

  // BATTERY LAYER
  s_battery_layer = layer_create(GRect(120, PBL_IF_ROUND_ELSE(0, 0), 18, 19));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_add_child(window_layer, s_battery_layer);
  layer_mark_dirty(s_battery_layer);
}

static void main_window_unload(Window *window)
{
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_weather_text_layer);

  for (unsigned int i = 0; i < sizeof(s_weather_icons)/sizeof(s_weather_icons[0]); ++i) {
    gdraw_command_image_destroy(s_weather_icons[i]);
  }
  gbitmap_destroy(s_weather_icon_default);
  
  layer_destroy(s_weather_layer);
  layer_destroy(s_background_layer);
  
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
}

static void init()
{
  sys_locale = setlocale(LC_ALL, "");
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // load draw commands
  //s_weather_icons[0] = gdraw_command_image_create_with_resource(RESOURCE_ID_VECTOR_WEATHER_DEFAULT);
  s_weather_icons[1] = gdraw_command_image_create_with_resource(RESOURCE_ID_VECTOR_WEATHER_01D);
  s_weather_icons[2] = gdraw_command_image_create_with_resource(RESOURCE_ID_VECTOR_WEATHER_02D);
  s_weather_icons[3] = gdraw_command_image_create_with_resource(RESOURCE_ID_VECTOR_WEATHER_03D);
  s_weather_icons[4] = gdraw_command_image_create_with_resource(RESOURCE_ID_VECTOR_WEATHER_04D);
  s_weather_icons[5] = gdraw_command_image_create_with_resource(RESOURCE_ID_VECTOR_WEATHER_09D);
  s_weather_icons[6] = gdraw_command_image_create_with_resource(RESOURCE_ID_VECTOR_WEATHER_10D);
  s_weather_icons[7] = gdraw_command_image_create_with_resource(RESOURCE_ID_VECTOR_WEATHER_11D);
  s_weather_icons[8] = gdraw_command_image_create_with_resource(RESOURCE_ID_VECTOR_WEATHER_13D);
  s_weather_icons[9] = gdraw_command_image_create_with_resource(RESOURCE_ID_VECTOR_WEATHER_50D);
  s_weather_icons[10] = gdraw_command_image_create_with_resource(RESOURCE_ID_VECTOR_WEATHER_01N);

  s_weather_icon_default = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WEATHER_DEFAULT);
  
  window_stack_push(s_main_window, true);
  update_time();

  battery_state_service_subscribe(battery_callback);
  battery_callback(battery_state_service_peek());
  
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
