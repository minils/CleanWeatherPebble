#include <pebble.h>

/* VARIABLES */
static Layer *s_battery_layer;

static int s_battery_level;
static bool s_battery_charging;

/* FUNCTIONS */
static void battery_update_proc(Layer *layer, GContext *ctx);
static void battery_callback(BatteryChargeState state);

void battery_init();
void battery_load(Layer*);
void battery_unload();
