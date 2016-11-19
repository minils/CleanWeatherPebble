#include "weather.h"
#include "storage_keys.h"

static void weather_icon_update_proc(Layer *layer, GContext *ctx)
{
  bool night = false;
  if (s_current_weather_icon[2] == 'n') {
    night = true;
  }
  char tmp[2];
  tmp[0] = s_current_weather_icon[0];
  tmp[1] = s_current_weather_icon[1];
  int i = atoi(tmp);
  switch(i) {
  case 1:
    s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WEATHER_01D);
    break;
  case 2:
    s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WEATHER_02D);
    break;
  case 3:
    s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WEATHER_03D);
    break;
  case 4:
    s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WEATHER_04D);
    break;
  case 9:
    s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WEATHER_09D);
    break;
  case 10:
    s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WEATHER_10D);
    break;
  case 11:
    s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WEATHER_11D);
    break;
  case 13:
    s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WEATHER_13D);
    break;
  case 50:
    s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WEATHER_50D);
    break;
  default:
    s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WEATHER_DEFAULT);
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Updated weather layer with: %s", s_current_weather_icon);
  GRect bitmap_bounds = gbitmap_get_bounds(s_weather_bitmap);
  bitmap_bounds.origin.x += 14;
  bitmap_bounds.origin.y += 15;
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(ctx, s_weather_bitmap, bitmap_bounds);
  text_layer_set_text(s_weather_text_layer, s_temperature_buffer);
  
  if (night) {
    night = false;
  }
}

static void weather_layer_animation_started(Animation *animation, void *context)
{
}

static void weather_layer_animation_stopped(Animation *animation, bool finished, void *context)
{
  layer_mark_dirty(s_weather_layer);
  PropertyAnimation *prop_anim = property_animation_create_layer_frame(
       s_weather_layer, &s_weather_layer_off, &s_weather_layer_in);
  Animation *anim = property_animation_get_animation(prop_anim);
  const int delay_ms = 500;
  const int duration_ms = 500;
  animation_set_curve(anim, AnimationCurveEaseOut);
  animation_set_delay(anim, delay_ms);
  animation_set_duration(anim, duration_ms);
  animation_schedule(anim);
}

static void weather_layer_animate()
{
  PropertyAnimation *prop_anim = property_animation_create_layer_frame(
       s_weather_layer, &s_weather_layer_in, &s_weather_layer_off);
  Animation *anim = property_animation_get_animation(prop_anim);
  const int delay_ms = 500;
  const int duration_ms = 500;
  animation_set_curve(anim, AnimationCurveEaseOut);
  animation_set_delay(anim, delay_ms);
  animation_set_duration(anim, duration_ms);
  animation_set_handlers(anim, (AnimationHandlers) {
	.started = weather_layer_animation_started,
	.stopped = weather_layer_animation_stopped
  }, NULL);
  animation_schedule(anim);
}

void weather_received_data(char* icon, int temperature)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received temperature: %d", temperature);
  snprintf(s_temperature_buffer, sizeof(s_temperature_buffer), "%d C", temperature);
  persist_write_string(key_weather_temperature, s_temperature_buffer);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received icon: %s", icon);
  strcpy(s_current_weather_icon, icon);
  persist_write_string(key_weather_icon, s_current_weather_icon);
  weather_layer_animate();
}

void weather_load(Layer *window_layer, int width)
{
  s_weather_layer_in = GRect(0, 0, width/2, 100);
  s_weather_layer_off = GRect(-width/2, 0, width/2, 100);
  s_weather_layer_visible = false;
  
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GAMPLAY_20));
  
  s_weather_layer = layer_create(s_weather_layer_in);
  layer_set_update_proc(s_weather_layer, weather_icon_update_proc);
  layer_add_child(window_layer, s_weather_layer);
  layer_mark_dirty(s_weather_layer);

  s_weather_text_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(68, 68), 80, 25));
  text_layer_set_background_color(s_weather_text_layer, GColorClear);
  text_layer_set_text_color(s_weather_text_layer, GColorBlack);
  text_layer_set_text_alignment(s_weather_text_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_text_layer, s_temperature_buffer);
  text_layer_set_font(s_weather_text_layer, s_weather_font);
  layer_add_child(s_weather_layer, text_layer_get_layer(s_weather_text_layer));
}

void weather_unload()
{
  gbitmap_destroy(s_weather_bitmap);
  layer_destroy(s_weather_layer);
  text_layer_destroy(s_weather_text_layer);
  fonts_unload_custom_font(s_weather_font);
}

static void load_persist_string(const uint32_t key, char *buffer, char *default_value)
{
  if (persist_read_string(key, buffer, sizeof(buffer)) == E_DOES_NOT_EXIST) {
    strcpy(buffer, default_value);
    persist_write_string(key, buffer);
  }
}

void weather_init()
{
  load_persist_string(key_weather_icon, s_current_weather_icon, "00d");
  load_persist_string(key_weather_temperature, s_temperature_buffer, "...");
}
