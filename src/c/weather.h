#include <pebble.h>

/* VARIABLES */
static GBitmap *s_weather_bitmap;
static TextLayer *s_weather_text_layer;
static Layer *s_weather_layer;
static GFont s_weather_font;

static char s_current_weather_icon[4];
static char s_temperature_buffer[8];

/* FUNCTIONS */

static void weather_icon_update_proc(Layer *layer, GContext *ctx);
static void load_persist_string(const uint32_t key, char *buffer, char *default_value);

void weather_received_temperature(int temperature);
void weather_received_icon(char *icon);
void weather_load(Layer*, int width);
void weather_unload();
void weather_init();