#include <pebble.h>
#include "math.h"

#define KEY_ANIMATION 0
#define KEY_COLOR 1
#define KEY_PRIMARY 2
#define KEY_SECONDARY 3
/*ADD NUMS OF COLOR SCHEME
 *
 * default = 0;
 * bumblebee = 1;
 * festive = 2;
 * red + black = 3;
 * vividcerulean + white = 4;
 *
 */

static Window *window;
static Layer *hands_layer;
static TextLayer *time_layer;
static Layer *bg_layer;
static AppTimer *anim_timer;

static int circle_radius = 0;

static int battery_status = 0;


static int colors[] = {0x0000FF, 0x000000, 0x00FF00, 0x000000, 0x00AAFF}; // circle and hand_background

static int secondary_colors[] = {0xFF0000, 0xFFFF00, 0xFF0000, 0xFF0000, 0xFFFFFF}; // background

static int battery_low_colors[] = {0xFFFF00, 0xFF0000, 0xFFFF00, 0xFFFF00,0xFF0000};

static int battery_charge_colors[] = {0x00FF00, 0x00FF00, 0x0000FF, 0x00FF00, 0x00FF00};

static int battery_low_color = 0xFFFF00;
static int battery_charge_color = 0x00FF00;


static void bg_create_proc(Layer *layer, GContext *ctx) {
	/*
	graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorFromHEX(colors[(persist_exists(KEY_COLOR) ? persist_read_int(KEY_COLOR) : 0)]), GColorBlack));
	*/
	if (persist_exists(KEY_PRIMARY)) {
		graphics_context_set_fill_color(ctx, GColorFromHEX((persist_read_int(KEY_COLOR) == 99) ? persist_read_int(KEY_PRIMARY) : colors[(persist_read_int(KEY_COLOR))]));
	} else {
		graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorBlue, GColorWhite));
	}

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
			//graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorFromHEX(/*secondary_colors[(persist_exists(KEY_COLOR) ? persist_read_int(KEY_COLOR) : 0)]*/persist_read_int(KEY_SECONDARY)), GColorBlack));
			
			if (persist_exists(KEY_PRIMARY)) {
				graphics_context_set_stroke_color(ctx, GColorFromHEX((persist_read_int(KEY_COLOR) == 99) ? persist_read_int(KEY_SECONDARY) : secondary_colors[(persist_read_int(KEY_COLOR))]));
			} else {
				graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorRed, GColorBlack));
			}

			break;
		case 1:
			//graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorFromHEX(/*battery_low_colors[(persist_exists(KEY_COLOR) ? persist_read_int(KEY_COLOR) : 0)]*/persist_read_int(KEY_SECONDARY)), GColorBlack));

			if (persist_exists(KEY_PRIMARY)) {
				graphics_context_set_stroke_color(ctx, GColorFromHEX((persist_read_int(KEY_COLOR) == 99) ? battery_low_color : battery_low_colors[(persist_read_int(KEY_COLOR))]));
			} else {
				graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorYellow, GColorBlack));
			}

			break;
		case 2:
			//graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorFromHEX(battery_charge_colors[(persist_exists(KEY_COLOR) ? persist_read_int(KEY_COLOR) : 0)]), GColorBlack));
			
			if (persist_exists(KEY_PRIMARY)) {
				graphics_context_set_stroke_color(ctx, GColorFromHEX((persist_read_int(KEY_COLOR) == 99) ? battery_charge_color : battery_charge_colors[(persist_read_int(KEY_COLOR))]));
			} else {
				graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorGreen, GColorBlack));
			}

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
		if (persist_exists(KEY_PRIMARY)) {
			window_set_background_color(window, PBL_IF_COLOR_ELSE(GColorFromHEX((persist_read_int(KEY_COLOR) == 99) ? battery_charge_color : battery_charge_colors[persist_read_int(KEY_COLOR)]), GColorBlack));
		} else {
			window_set_background_color(window, PBL_IF_COLOR_ELSE(GColorGreen, GColorBlack));
		}
		battery_status = 2;
	} else if (percent <= 30) {
		//window_set_background_color(window, PBL_IF_COLOR_ELSE(GColorFromHEX(battery_low_colors[(persist_exists(KEY_COLOR) ? persist_read_int(KEY_COLOR) : 0)]), GColorBlack));
		
		if (persist_exists(KEY_PRIMARY)) {
			window_set_background_color(window, PBL_IF_COLOR_ELSE(GColorFromHEX((persist_read_int(KEY_COLOR) == 99) ? battery_low_color : battery_low_colors[persist_read_int(KEY_COLOR)]), GColorBlack));
		} else {
			window_set_background_color(window, PBL_IF_COLOR_ELSE(GColorYellow, GColorBlack));
		}

		battery_status = 1;
	} else {
		//window_set_background_color(window, PBL_IF_COLOR_ELSE(GColorFromHEX(secondary_colors[(persist_exists(KEY_COLOR) ? persist_read_int(KEY_COLOR) : 0)]), GColorBlack));

		if (persist_exists(KEY_PRIMARY)) {
			window_set_background_color(window, PBL_IF_COLOR_ELSE(GColorFromHEX((persist_read_int(KEY_COLOR) == 99) ? persist_read_int(KEY_SECONDARY) : secondary_colors[persist_read_int(KEY_COLOR)]), GColorBlack));
		} else {
			window_set_background_color(window, PBL_IF_COLOR_ELSE(GColorRed, GColorBlack));
		}
		battery_status = 0;
	}

//optimise
	switch(battery_status) {
		case 0:
			//text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorFromHEX(secondary_colors[(persist_exists(KEY_COLOR) ? persist_read_int(KEY_COLOR) : 0)]), GColorBlack));

			if (persist_exists(KEY_PRIMARY)) {
				text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorFromHEX((persist_read_int(KEY_COLOR) == 99) ? persist_read_int(KEY_SECONDARY) : secondary_colors[persist_read_int(KEY_COLOR)]), GColorBlack));
			} else {
				text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorRed, GColorBlack));
			}

			break;
		case 1:
			//text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorFromHEX(battery_low_colors[(persist_exists(KEY_COLOR) ? persist_read_int(KEY_COLOR) : 0)]), GColorBlack));

			if (persist_exists(KEY_PRIMARY)) {
				text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorFromHEX((persist_read_int(KEY_COLOR) == 99) ? battery_low_color : battery_low_colors[persist_read_int(KEY_COLOR)]), GColorBlack));
			} else {
				text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorYellow, GColorBlack));
			}

			break;
		case 2:
			//text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorFromHEX(battery_charge_colors[(persist_exists(KEY_COLOR) ? persist_read_int(KEY_COLOR) : 0)]), GColorBlack));

			if (persist_exists(KEY_PRIMARY)) {
				text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorFromHEX((persist_read_int(KEY_COLOR) == 99) ? battery_charge_color : battery_charge_colors[persist_read_int(KEY_COLOR)]), GColorBlack));
			} else {
				text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorGreen, GColorBlack));
			}

			break;
		default:
			//text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorFromHEX(secondary_colors[(persist_exists(KEY_COLOR) ? persist_read_int(KEY_COLOR) : 0)]), GColorBlack));

			if (persist_exists(KEY_PRIMARY)) {
				text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorFromHEX((persist_read_int(KEY_COLOR) == 99) ? persist_read_int(KEY_SECONDARY) : secondary_colors[persist_read_int(KEY_COLOR)]), GColorBlack));
			} else {
				text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorRed, GColorBlack));
			}
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
	text_layer_set_text_color(time_layer, PBL_IF_COLOR_ELSE(GColorFromHEX(secondary_colors[(persist_exists(KEY_COLOR) ? persist_read_int(KEY_COLOR) : 0)]), GColorWhite));


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
	if (persist_read_int(KEY_ANIMATION) == 1) {
		shrink_radius();
		anim_timer = app_timer_register(1000, app_timer_callback, NULL);
	}
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	Tuple *animation_tuple = dict_find(iterator, KEY_ANIMATION);
	Tuple *color_tuple = dict_find(iterator, KEY_COLOR);
	Tuple *pri_tuple = dict_find(iterator, KEY_PRIMARY);
	Tuple *sec_tuple = dict_find(iterator, KEY_SECONDARY);

	if (animation_tuple) {
		persist_write_int(KEY_ANIMATION, (int)animation_tuple->value->int16);
	}

	if (color_tuple) {
		persist_write_int(KEY_COLOR, (int)color_tuple->value->int16);
	}

	if (pri_tuple && sec_tuple) {
		persist_write_int(KEY_PRIMARY, (int)pri_tuple->value->int32);
		persist_write_int(KEY_SECONDARY, (int)sec_tuple->value->int32);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Got pri colours: %d", (int)pri_tuple->value->int32);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Got sec colours: %d", (int)sec_tuple->value->int32);
	}
	layer_mark_dirty(window_get_root_layer(window));
	update_battery();
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
	  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}


static void init() {
	window = window_create();
	
	window_set_background_color(window, PBL_IF_COLOR_ELSE(GColorFromHEX(secondary_colors[(persist_exists(KEY_COLOR) ? persist_read_int(KEY_COLOR) : 0)]), GColorBlack));

	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
	battery_state_service_subscribe(battery_callback);
	
	accel_tap_service_subscribe(tap_handler);

	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);

	window_set_window_handlers(window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload,
	});

	window_stack_push(window, true);

	update_time();
	update_battery();
	
	app_message_open(128, 128);

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
