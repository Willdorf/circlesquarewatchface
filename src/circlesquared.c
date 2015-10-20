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

static GBitmap *s_unary_hours_bitmap;
static RotBitmapLayer *s_unary_hours_layer_left;
static RotBitmapLayer *s_unary_hours_layer_right;

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

	//display the hours

	//left hours------------------------------ 
	int8_t cur_hours = s_hour % 12;
	int8_t left_hours = cur_hours;
	if (cur_hours > 6) {
		left_hours = 6;
	}
	GPathInfo HOURS_LEFT_PATH_INFO = {
		.num_points = 4,
		.points = (GPoint []) {{0,0},{left_hours*width/12,0},{left_hours*width/12,27},{0,27}}
	};

	GPath *s_hours_path_left = gpath_create(&HOURS_LEFT_PATH_INFO);
	gpath_rotate_to(s_hours_path_left, DEG_TO_TRIGANGLE(-45));
	gpath_move_to(s_hours_path_left, GPoint(3,32 + PADDING));
	gpath_draw_filled(ctx, s_hours_path_left);

	//right hours------------------------------ 
	
	int8_t right_hours = cur_hours;
	if (cur_hours < 6) {
		right_hours = 0;
	} else {
		right_hours = cur_hours - 6;
	}

	GPathInfo HOURS_RIGHT_PATH_INFO = {
		.num_points = 4,
		.points = (GPoint []) {{0,0},{right_hours*width/12,0},{right_hours*width/12,27},{0,27}}
	};

	GPath *s_hours_path_right = gpath_create(&HOURS_RIGHT_PATH_INFO);
	gpath_rotate_to(s_hours_path_right, DEG_TO_TRIGANGLE(45));
	gpath_move_to(s_hours_path_right, GPoint(width - 32 + 8, 1));
	gpath_draw_filled(ctx, s_hours_path_right);

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

	//create the left hours image
	s_unary_hours_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_6_SINGLE_BLOCKS);
	s_unary_hours_layer_left = rot_bitmap_layer_create(s_unary_hours_bitmap);
	rot_bitmap_layer_set_corner_clip_color(s_unary_hours_layer_left, GColorClear);
	rot_bitmap_set_compositing_mode(s_unary_hours_layer_left, GCompOpSet);
	rot_bitmap_layer_set_angle(s_unary_hours_layer_left, DEG_TO_TRIGANGLE(-45));
	//position the frame
	GRect r = layer_get_frame((Layer *) s_unary_hours_layer_left);
	r.origin.x = 2;
	layer_set_frame((Layer *) s_unary_hours_layer_left, r);
	layer_add_child(window_get_root_layer(window), (Layer *) s_unary_hours_layer_left);

	//create the right hours image
	s_unary_hours_layer_right = rot_bitmap_layer_create(s_unary_hours_bitmap);
	rot_bitmap_layer_set_corner_clip_color(s_unary_hours_layer_right, GColorClear);
	rot_bitmap_set_compositing_mode(s_unary_hours_layer_right, GCompOpSet);
	rot_bitmap_layer_set_angle(s_unary_hours_layer_right, DEG_TO_TRIGANGLE(45));
	//position the frame
	r = layer_get_frame((Layer *) s_unary_hours_layer_right);
	r.origin.x = width - 32 - PADDING; 
	layer_set_frame((Layer *) s_unary_hours_layer_right, r);
	layer_add_child(window_get_root_layer(window), (Layer *) s_unary_hours_layer_right);
	

}

static void window_unload(Window *window) {
	gbitmap_destroy(s_ternary_seconds_bitmap);
	bitmap_layer_destroy(s_ternary_seconds_layer);

	gbitmap_destroy(s_unary_hours_bitmap);
	rot_bitmap_layer_destroy(s_unary_hours_layer_left);
	rot_bitmap_layer_destroy(s_unary_hours_layer_right);

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
