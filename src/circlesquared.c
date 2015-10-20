#include <pebble.h>

static Window *window;
static Layer *s_layer;

static uint8_t s_hour;
static uint8_t s_min;
static uint8_t s_sec;

static int PADDING = 12;
static int HEIGHT = 25;

static GBitmap *s_ternary_seconds_bitmap;
static BitmapLayer *s_ternary_seconds_layer;

static void update_time(struct tm * tick_time) {
	s_hour = tick_time->tm_hour;
	s_min = tick_time->tm_min;
	s_sec = tick_time->tm_sec;
	layer_mark_dirty(s_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time(tick_time);
}

static void draw_watchface(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(layer);
	uint16_t width = bounds.size.w - (2 * PADDING);

	//set the colour
	graphics_context_set_fill_color(ctx, COLOR_FALLBACK(GColorRed, GColorWhite));	

	//display the seconds
	graphics_fill_rect(ctx, GRect(((s_sec * width)/60) + PADDING, 84 - 10, 2, 19), 0 ,0);
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	s_layer = layer_create(layer_get_bounds(window_get_root_layer(window)));
	layer_add_child(window_get_root_layer(window), s_layer);
	layer_set_update_proc(s_layer, draw_watchface);

	uint16_t width = bounds.size.w - (2 * PADDING);

	//create the ternary seconds image
	s_ternary_seconds_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SECONDS_TERNARY_REFLECT);
	s_ternary_seconds_layer = bitmap_layer_create(GRect(PADDING, 0, width + 1, 168));
	bitmap_layer_set_background_color(s_ternary_seconds_layer, GColorClear);
	bitmap_layer_set_bitmap(s_ternary_seconds_layer, s_ternary_seconds_bitmap);
	bitmap_layer_set_compositing_mode(s_ternary_seconds_layer, GCompOpSet);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_ternary_seconds_layer));
}

static void window_unload(Window *window) {
	gbitmap_destroy(s_ternary_seconds_bitmap);
	bitmap_layer_destroy(s_ternary_seconds_layer);

}

static void init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
			.load = window_load,
			.unload = window_unload,
			});
	const bool animated = true;
	window_stack_push(window, animated);

	//Register with TickTimerService
	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

	//display the time right away
	time_t start_time = time(NULL);
	update_time(localtime(&start_time));
}

static void deinit(void) {
	tick_timer_service_unsubscribe();
	window_destroy(window);
}

int main(void) {
	init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

	app_event_loop();
	deinit();
}
