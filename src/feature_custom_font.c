/*

   How to use a custom non-system font.

 */

#include "pebble.h"

#define TIMEZONEOFFSET 0
#define TIMEZONEABBR   1
#define METAR          2
#define REQUEST_METAR  3
  
static Window *window;

static TextLayer *local_time_text;
static TextLayer *local_date_text;
static TextLayer *utc_time_text;
static TextLayer *metar_text;

static struct tm zulu_tick_time;
int time_offset = 0;

static void drawLocalTime(struct tm *tick_time) {
  static char s_time_buffer[16]; 
  strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", tick_time);
  text_layer_set_text(local_time_text, s_time_buffer);
}

static void drawLocalDate(struct tm *tick_time) {
  static char s_time_buffer[16]; 
  strftime(s_time_buffer, sizeof(s_time_buffer), "%a %m/%d", tick_time);
  text_layer_set_text(local_date_text, s_time_buffer);
}

static void drawUTCTime(struct tm *tick_time) {
  time_t local = time(NULL);
  time_t utc = local + time_offset;
  zulu_tick_time = *(localtime(&utc)); 
  static char s_time_buffer[16]; 
  strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M:%S", tick_time);
  text_layer_set_text(utc_time_text, s_time_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  drawLocalTime(tick_time);
  drawLocalDate(tick_time);
  drawUTCTime(tick_time);
}

void request_metar_update(void *data) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "REQUESTING METAR");
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
 
  Tuplet value = TupletInteger(8, 8);
  dict_write_tuplet(iter, &value);
 
  app_message_outbox_send();
}

static void out_sent_handler(DictionaryIterator *sent, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "OUT SUCCESSSSSS!");
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "OUT FAILED!");
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "received callback!");
  // Get the first pair
  Tuple *t = dict_read_first(iterator);

  // Process all pairs present
  while (t != NULL) {
    // Long lived buffer
    static char s_buffer[2044];

    // Process this pair's key
    switch (t->key) {
      case TIMEZONEOFFSET:
        // Copy value and display
        snprintf(s_buffer, sizeof(s_buffer), "%i", (int)t->value->int32);
        time_offset = (int)t->value->int32;
        APP_LOG(APP_LOG_LEVEL_DEBUG, "TIMEZONEOFFSET %s", s_buffer);
      
//         text_layer_set_text(time_zone, s_buffer);
        break;
      case TIMEZONEABBR:
        snprintf(s_buffer, sizeof(s_buffer), "%s", t->value->cstring);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "TIMEZONEABBR %s", s_buffer);
        break;
      case METAR:
        snprintf(s_buffer, sizeof(s_buffer), "%s", t->value->cstring);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "METAR %s", s_buffer);
        text_layer_set_text(metar_text, s_buffer);
        app_timer_register(600000, request_metar_update, NULL);
        break;
    }

    // Get next pair, if any
    t = dict_read_next(iterator);
  }
}

static void init() {
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_outbox_failed(out_failed_handler);
  app_message_register_outbox_sent(out_sent_handler);

  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  window = window_create();
  window_stack_push(window, true /* Animated */);

  Layer *window_layer = window_get_root_layer(window);
  // GRect bounds = layer_get_bounds(window_layer);

  GFont large_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VISITOR_52));
  GFont medium_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VISITOR_28));
  GFont small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OSP_DIN_18));
  
  // Medium UTC time display
  utc_time_text = text_layer_create(GRect(0, 135, 144, 35));
  text_layer_set_font(utc_time_text, medium_font);
  text_layer_set_text_alignment(utc_time_text, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(utc_time_text));

  // Medium local date display
  local_date_text = text_layer_create(GRect(0, 25, 144, 40));
  text_layer_set_font(local_date_text, medium_font);
  text_layer_set_text_alignment(local_date_text, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(local_date_text));

  // Large local time display
  local_time_text = text_layer_create(GRect(0, -20, 144, 60));
  text_layer_set_font(local_time_text, large_font);
  text_layer_set_text_alignment(local_time_text, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(local_time_text));
  
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler); 
    // Small METAR text display
  metar_text = text_layer_create(GRect(4, 55, 140, 90));
  text_layer_set_font(metar_text, small_font);
  text_layer_set_text_alignment(metar_text, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(metar_text));
  
}

static void deinit() {
  text_layer_destroy(local_time_text);
  text_layer_destroy(local_date_text);
  text_layer_destroy(utc_time_text);

  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
