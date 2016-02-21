#include <pebble.h>
#include "math.h"

static Window *window;
static Layer *hands_layer;
static TextLayer *time_layer;
static Layer *bg_layer;
static AppTimer *anim_timer;

static int circle_radius = 0;

static int battery_status = 0;

static void bg_create_proc(Layer *layer, GContext *ctx) {
	graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorBlue, GColorBlack));

	graphics_fill_rect(ctx, GRect(0, 0, 144, 12), 0, GCornerNone);
	graphics_fill_rect(ctx, GRect(0, 156, 144, 12), 0, GCornerNone);
	
	graphics_fill_circle(ctx, GPoint(72, 84), circle_radius);
	
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
	time_t temp = time(NULL);
	struct tm *t = localtime(&temp);

	int minute_pos = ceil((float)t->tm_min * 2.4);

	int	hour_pos = ceil((float)t->tm_hour * 6);

	graphics_context_set_stroke_width(ctx, 3);
	switch(battery_status) {
		case 0:
			graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorRed, GColorBlack));
			break;
		case 1:
			graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorYellow, GColorBlack));
			break;
		case 2:
			graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorGreen, GColorBlack));
			break;
		default:
			graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorRed, GColorBlack));
			break;
	}
	graphics_context_set_antialiased(ctx, false);
	
	graphics_draw_line(ctx, GPoint(hour_pos, 0), GPoint(hour_pos, 12));
	graphics_draw_line(ctx, GPoint(minute_pos, 156), GPoint(minute_pos, 168));
}

static void update_time() {
	time_t temp = time(NULL);
	struct tm *t = localtime(&temp);

	static char buffer[] = "00\n00\n00";

	strftime(buffer, sizeof(buffer), "%H\n%M\n%S", t);

	text_layer_set_text(time_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
	layer_mark_dirty(hands_layer);
}

static void update_battery() {
	BatteryChargeState state = battery_state_service_peek();
	int percent = state.charge_percent;
	bool charging = state.is_charging;
	bool plugged = state.is_plugged;

	if (charging || plugged) {
		window_set_background_color(window, PBL_IF_COLOR_ELSE(GColorGreen, GColorBlack));
		battery_status = 2;
	} else if (percent <= 30) {
		window_set_background_color(window, PBL_IF_COLOR_ELSE(GColorYellow, GColorBlack));
		battery_status = 1;
	} else {
		window_set_background_color(window, PBL_IF_COLOR_ELSE(GColorRed, GColorBlack));
		battery_status = 0;
	}

//optimise
	switch(battery_status) {
		case 0:
			text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorRed, GColorBlack));
			break;
		case 1:
			text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorYellow, GColorBlack));
			break;
		case 2:
			text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorGreen, GColorBlack));
			break;
		default:
			text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorRed, GColorBlack));
			break;
	}

}

static void battery_callback(BatteryChargeState charge_state) {
	update_battery();
}


static void main_window_load() {
	Layer *window_layer = window_get_root_layer(window);
	bg_layer = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(bg_layer, bg_create_proc);
	layer_add_child(window_layer, bg_layer);

	time_layer = text_layer_create(GRect(1, 21, 144, 140));
	text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorRed, GColorWhite));


	text_layer_set_background_color(time_layer, GColorClear);
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_LECO_38_BOLD_NUMBERS));
	text_layer_set_text(time_layer, "00\n00\n00");
	layer_add_child(window_layer, text_layer_get_layer(time_layer));

	hands_layer = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(hands_layer, hands_update_proc);
	layer_add_child(window_layer, hands_layer);

}

static void main_window_unload() {
	layer_destroy(bg_layer);
	text_layer_destroy(time_layer);
	layer_destroy(hands_layer);

}


static void circle_animation_update(Animation *animation, const AnimationProgress progress) {
	if (circle_radius < 60) {
		circle_radius += 3;
	}
/*
	if (bar_width < 144) {
		bar_width += 6;
	}
*/
	layer_mark_dirty(bg_layer);
	
}

static void circle_shrink_update(Animation *animation, const AnimationProgress progress) {
	if (circle_radius > 0) {
		circle_radius -= 3;
	}
	
	layer_mark_dirty(bg_layer);
}

static void expand_radius() {
	static AnimationImplementation impl = {
		.update = circle_animation_update
	};

	Animation *anim = animation_create();
	animation_set_duration(anim, 1000);
	animation_set_implementation(anim, &impl);
	animation_schedule(anim);
}

static void shrink_radius() {
	static AnimationImplementation impl = {
		.update = circle_shrink_update
	};

	Animation *anim = animation_create();
	animation_set_duration(anim, 1000);
	animation_set_implementation(anim, &impl);
	animation_schedule(anim);
}

static void app_timer_callback(void *data) {
	expand_radius();
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
	shrink_radius();
	anim_timer = app_timer_register(1000, app_timer_callback, NULL);
}

static void init() {
	window = window_create();
	
	window_set_background_color(window, PBL_IF_COLOR_ELSE(GColorRed, GColorBlack));

	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
	battery_state_service_subscribe(battery_callback);
	accel_tap_service_subscribe(tap_handler);

	window_set_window_handlers(window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload,
	});

	window_stack_push(window, true);

	update_time();
	update_battery();
	
	circle_radius = 0;
	layer_mark_dirty(bg_layer);

	expand_radius();
}

static void deinit() {
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
