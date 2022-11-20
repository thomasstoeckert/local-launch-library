#include <pebble.h>

static Window *s_splash_window;
static TextLayer *s_splash_text_layer;

static Window *s_all_launch_window;
static MenuLayer *s_all_launch_menu_layer;

static Window *s_saturation_warning_window;
/*
static Window *s_details_window;
static ScrollLayer *s_details_scrolllayer;
static TextLayer *s_detail_title_layer, s_detail_status_layer, s_detail_window_start_layer, s_detail_window_end_layer;
*/

static bool s_js_ready = false;
static int s_saturation_timeout = 0;

static char s_c_timeout_string[64];

typedef struct LaunchListData
{
  char id[37];
  char name[37];
  char net[19];
} LaunchListData;

static LaunchListData s_all_launch_data[10];

/*
typedef struct LaunchSpecificData
{
  char name[64];
  char status[64];
  char windowStart[20];
  char windowEnd[20];
  char provider[64];
  char rocket[64];
  char missionName[64];
  char missionDescription[512];
  char padName[64];
  char padLocation[128];
} LaunchSpecificData;

static LaunchSpecificData s_specific_launch_data;
*/

static void request_all_launches(int offset)
{
  // Send out a message for "get launch data"
  if (!s_js_ready)
    return;

  DictionaryIterator *out_iter;
  AppMessageResult result = app_message_outbox_begin(&out_iter);

  if (result == APP_MSG_OK)
  {
    // Prepare our payload of request
    dict_write_int(out_iter, MESSAGE_KEY_i_request_all_launches, &offset, sizeof(int), true);

    result = app_message_outbox_send();
  }
}

static void inbox_received_callback(DictionaryIterator *iter, void *context)
{
  APP_LOG(APP_LOG_LEVEL_INFO, "Message received from JS");

  // Check to see if we're receiving our ready tuple
  Tuple *ready_tuple = dict_find(iter, MESSAGE_KEY__ready);
  if (ready_tuple)
  {
    s_js_ready = true;
    APP_LOG(APP_LOG_LEVEL_INFO, "Ready signal received from JS");

    // Fetch data from JS
    request_all_launches(0);
  }

  // Check to see if we're receiving launch data
  Tuple *all_launch_tuple = dict_find(iter, MESSAGE_KEY_s_return_launches_ids);
  if (all_launch_tuple)
  {

    // Clear existing launch data
    memset(s_all_launch_data, 0, sizeof(LaunchListData) * 10);

    // Parse the data into the relevant arrays
    for (int i = 0; i < 10; i++)
    {
      Tuple *id_tuple = dict_find(iter, MESSAGE_KEY_s_return_launches_ids + i);
      Tuple *name_tuple = dict_find(iter, MESSAGE_KEY_s_return_launches_names + i);
      Tuple *time_tuple = dict_find(iter, MESSAGE_KEY_s_return_launches_times + i);

      if (!(id_tuple && name_tuple && time_tuple))
      {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to find all launch information at index %d", i);
        continue;
      }

      // Copy info into struct
      strncpy(s_all_launch_data[i].id, id_tuple->value->cstring, 37);
      strncpy(s_all_launch_data[i].name, name_tuple->value->cstring, 37);
      strncpy(s_all_launch_data[i].net, time_tuple->value->cstring, 20);
    }

    // Replace the top layer with the all launch layer
    Window *top_window = window_stack_get_top_window();
    window_stack_remove(top_window, true);
    window_stack_push(s_all_launch_window, true);
  }
  /*
  Tuple *specific_launch_tuple = dict_find(iter, MESSAGE_KEY_s_return_launch_name);
  if (specific_launch_tuple)
  {
    // We have specific launch data. Clear the old one, ingest the data, display
    // the details page

    int keys[10] = {
        MESSAGE_KEY_s_return_launch_name,
        MESSAGE_KEY_s_return_launch_status,
        MESSAGE_KEY_s_return_launch_time,
        MESSAGE_KEY_s_return_launch_time + 1,
        MESSAGE_KEY_s_return_launch_provider,
        MESSAGE_KEY_s_return_launch_rocket,
        MESSAGE_KEY_s_return_launch_mission_name,
        MESSAGE_KEY_s_return_launch_mission_description,
        MESSAGE_KEY_s_return_launch_pad_name,
        MESSAGE_KEY_s_return_launch_pad_location};

    char *destinations[10] = {
      s_specific_launch_data.name,
      s_specific_launch_data.status,
      s_specific_launch_data.windowStart,
      s_specific_launch_data.windowEnd,
      s_specific_launch_data.provider,
      s_specific_launch_data.rocket,
      s_specific_launch_data.missionName,
      s_specific_launch_data.missionDescription,
      s_specific_launch_data.padName,
      s_specific_launch_data.padLocation
    };

    int lengths[10] = {
      64,
      64,
      20,
      20,
      64,
      64,
      64,
      512,
      64,
      128
    };

    for (int i = 0; i < 10; i++)
    {
      Tuple *this_tuple = dict_find(iter, keys[i]);
      strncpy(destinations[i], this_tuple->value->cstring, lengths[i]);
      APP_LOG(APP_LOG_LEVEL_INFO, "%s", destinations[i]);
    }



    // Place the details page on top
    window_stack_push(s_details_window, true);
  }
  */

  Tuple *saturation_warning_tuple = dict_find(iter, MESSAGE_KEY_i_return_launch_error);
  if (saturation_warning_tuple)
  {
    // We've reached saturation. Get the int, display the warning page.
    s_saturation_timeout = saturation_warning_tuple->value->uint32;

    // Replace the top layer with the timeout warning layer
    Window *top_window = window_stack_get_top_window();
    window_stack_remove(top_window, true);
    window_stack_push(s_saturation_warning_window, true);
  }
}

static void select_all_list_callback(struct MenuLayer *s_menu_layer, MenuIndex *cell_index, void *callback_context)
{
  // Handle the callback
  int index = cell_index->row;
/*
  APP_LOG(APP_LOG_LEVEL_INFO, "Launch has been clicked: %d", index);

  // Send a request for a specific launch's details
  // Send out a message for "push this launch"
  if (!s_js_ready)
    return;

  DictionaryIterator *out_iter;
  AppMessageResult result = app_message_outbox_begin(&out_iter);

  if (result == APP_MSG_OK)
  {
    // Prepare our payload of request
    dict_write_int(out_iter, MESSAGE_KEY_i_request_specific_launch_data, &index, sizeof(int), true);

    result = app_message_outbox_send();
    vibes_short_pulse();
  }*/

  // All click functionality stripped out because I ran out of time :(
  // Might update later.

  // Timeline functionality to be implemented once I figure out how to properly do RWS support.
}

static uint16_t get_all_list_count(struct MenuLayer *menuLayer, uint16_t section_index, void *callback_context)
{
  return 10;
}

static void draw_all_list_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context)
{
  // Get the index from the cell index
  int index = cell_index->row;

  // We display regular content.
  LaunchListData our_data = s_all_launch_data[index];
  const char *name = our_data.name;
  const char *net = our_data.net;
  menu_cell_basic_draw(ctx, cell_layer, name, net, NULL);
}

static void all_list_window_load(Window *window)
{
  // Get the root layer / bounds for this window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Creathe the menu layer, assign handlers
  s_all_launch_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_all_launch_menu_layer, NULL, (MenuLayerCallbacks){.get_num_rows = get_all_list_count, .draw_row = draw_all_list_row, .select_click = select_all_list_callback});

  // Bind keypresses
  menu_layer_set_click_config_onto_window(s_all_launch_menu_layer, window);

  // Stylize
  menu_layer_set_highlight_colors(s_all_launch_menu_layer, GColorCyan, GColorBlack);

  layer_add_child(window_layer, menu_layer_get_layer(s_all_launch_menu_layer));
}

static void all_list_window_unload(Window *window)
{
  menu_layer_destroy(s_all_launch_menu_layer);
}

static void all_list_init(void)
{
  s_all_launch_window = window_create();
  window_set_window_handlers(s_all_launch_window, (WindowHandlers){
                                                      .load = all_list_window_load,
                                                      .unload = all_list_window_unload});
}

static void all_list_deinit(void)
{
  window_destroy(s_all_launch_window);
}

static void saturation_warning_window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorSunsetOrange);
  GRect bounds = layer_get_bounds(window_layer);

  snprintf(s_c_timeout_string, 64, "LaunchLibrary API Rate Limit Exceeded.\n\nPlease Wait %dm", (s_saturation_timeout / 60) + 1);

  s_splash_text_layer = text_layer_create(GRect(5, 50, bounds.size.w - 5, 80));
  text_layer_set_text(s_splash_text_layer, s_c_timeout_string);
  text_layer_set_text_alignment(s_splash_text_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_splash_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_background_color(s_splash_text_layer, GColorSunsetOrange);
  layer_add_child(window_layer, text_layer_get_layer(s_splash_text_layer));
}

static void saturation_warning_window_unload(Window *window)
{
  text_layer_destroy(s_splash_text_layer);
}

static void saturation_warning_init(void)
{
  s_saturation_warning_window = window_create();
  window_set_window_handlers(s_saturation_warning_window, (WindowHandlers){
                                                              .load = saturation_warning_window_load,
                                                              .unload = saturation_warning_window_unload,
                                                          });
}

static void saturation_warning_deinit(void)
{
  window_destroy(s_saturation_warning_window);
}

static void splash_window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorCyan);
  GRect bounds = layer_get_bounds(window_layer);

  s_splash_text_layer = text_layer_create(GRect(0, 72, bounds.size.w, 20));
  text_layer_set_text(s_splash_text_layer, "Loading Launch Data...");
  text_layer_set_text_alignment(s_splash_text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_splash_text_layer, GColorCyan);
  layer_add_child(window_layer, text_layer_get_layer(s_splash_text_layer));
}

static void splash_window_unload(Window *window)
{
  text_layer_destroy(s_splash_text_layer);
}

static void splash_init(void)
{
  s_splash_window = window_create();
  window_set_window_handlers(s_splash_window, (WindowHandlers){
                                                  .load = splash_window_load,
                                                  .unload = splash_window_unload,
                                              });
  const bool animated = true;
  window_stack_push(s_splash_window, animated);
}

static void splash_deinit(void)
{
  window_destroy(s_splash_window);
}
/*
static void specific_details_window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorCeleste);
  GRect bounds = layer_get_bounds(window_layer);

  s_details_scrolllayer = scroll_layer_create(bounds);

  // Load fonts

  // Create our text layers
  s_detail_title_layer = text_layer_create(GRect(0, 0, bounds.size.w, 40));
  text_layer_set_text(s_detail_title_layer, s_specific_launch_data.name);
  //APP_LOG(APP_LOG_LEVEL_INFO, " Me am print name! %s", s_specific_launch_data.name);
  text_layer_set_background_color(s_detail_title_layer, GColorClear);
  text_layer_set_overflow_mode(s_detail_title_layer, GTextOverflowModeWordWrap);
  //text_layer_set_font(s_detail_title_layer, fonts_get_system_font("FONT_KEY_GOTHIC_28_BOLD"));
  scroll_layer_add_child(s_details_scrolllayer, text_layer_get_layer(s_detail_title_layer));

  s_detail_status_layer = text_layer_create(GRect(0, 40, bounds.size.w, 40));
  text_layer_set_text(s_detail_status_layer, s_specific_launch_data.status);

  text_layer_set_background_color(s_detail_title_layer, GColorClear);
  text_layer_set_overflow_mode(s_detail_title_layer, GTextOverflowModeWordWrap);
  //text_layer_set_font(s_detail_title_layer, fonts_get_system_font("FONT_KEY_GOTHIC_28_BOLD"));
  scroll_layer_add_child(s_details_scrolllayer, text_layer_get_layer(s_detail_title_layer));


  layer_add_child(window_layer, scroll_layer_get_layer(s_details_scrolllayer));
}

static void specific_details_window_unload(Window *window)
{
  scroll_layer_destroy(s_details_scrolllayer);
}

static void specific_details_init(void)
{
  s_details_window = window_create();
  window_set_window_handlers(s_details_window, (WindowHandlers){
                                                  .load = specific_details_window_load,
                                                  .unload = specific_details_window_unload,
                                              });
}

static void specific_details_deinit(void)
{
  window_destroy(s_details_window);
}*/


int main(void)
{

  // Configure app message settings
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  app_message_register_inbox_received(inbox_received_callback);

  // Create the various windows
  splash_init();
  all_list_init();
  saturation_warning_init();
  //specific_details_init();

  app_event_loop();

  // App is closing. Clear things up.
  splash_deinit();
  all_list_deinit();
  saturation_warning_deinit();
  //specific_details_deinit();
}
