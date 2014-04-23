/**
	* Beam Up Date Pebble Watchface
	* Author: Chris Lewis
	* Date: 16th March 2014
	*/

#include <pebble.h>
#include "cl_util.h"    
#include "locale_RO.h"

#define DEBUG false
#define TIME_DEBUG 0

#define INV_LAYER_WIDTH 30
#define INV_LAYER_HEIGHT 101
#define HTX -13 
#define HUX 21
#define MTX 63
#define MUX 97
#define OFFSET 14

#define PEEK_TIMEOUT 2700
    
//Prototypes
void get_time_digits(struct tm *t);
void set_time_digits();

//Globals
static Window *window;
static TextLayer *h_t_layer, *h_u_layer, *colon_layer, *m_t_layer, *m_u_layer, *date_layer, *batt_layer, *blue_layer;
static InverterLayer *h_t_inv_layer, *h_u_inv_layer, *m_t_inv_layer, *m_u_inv_layer, *bottom_inv_layer;

static AppTimer *peek_timer;
    
static int 
    h_t_digit = 0, h_t_prev = 0, 
    h_u_digit = 0, h_u_prev = 0, 
    m_t_digit = 0, m_t_prev = 0,
    m_u_digit = 0, m_u_prev = 0;

static char 
    time_text[5],
    date_text[] = "Duminica 01",
    batt_text[] = "~100 %",
    blue_text[] = "neconectat";


struct tm *t;
static int currentQuarter = -1;
static int progressBarWeight = 5;

static void update_progress_bar(struct tm *t) {
    int quarter = t->tm_min/13;
    
    if(currentQuarter != quarter) {
        currentQuarter = quarter;
        switch(quarter) {
            case 0:
                cl_animate_layer(inverter_layer_get_layer(bottom_inv_layer), GRect(0, 105, 144, progressBarWeight), GRect(0, 105, 0, progressBarWeight), 500, 0);        
                break;
            case 1:
                cl_animate_layer(inverter_layer_get_layer(bottom_inv_layer), GRect(0, 105, 0, progressBarWeight), GRect(0, 105, 36, progressBarWeight), 500, 0);
                break;
            case 2:
                cl_animate_layer(inverter_layer_get_layer(bottom_inv_layer), GRect(0, 105, 36, progressBarWeight), GRect(0, 105, 72, progressBarWeight), 500, 0);
                break;
            case 3:
                cl_animate_layer(inverter_layer_get_layer(bottom_inv_layer), GRect(0, 105, 72, progressBarWeight), GRect(0, 105, 108, progressBarWeight), 500, 0);
                break;
            case 4:
                cl_animate_layer(inverter_layer_get_layer(bottom_inv_layer), GRect(0, 105, 108, progressBarWeight), GRect(0, 105, 144, progressBarWeight), 500, 1000);
                break;    
        }
    }   
}

/**
 * Handle tick function
 */
static void handle_tick(struct tm *t, TimeUnits units_changed) {    
    //Get the time
    get_time_digits(t);
    set_time_digits(t);
    update_progress_bar(t);
}

void on_peek_timer(void *data) {
    app_timer_cancel(peek_timer);
    layer_set_hidden(text_layer_get_layer(batt_layer), true);
    layer_set_hidden(text_layer_get_layer(blue_layer), true);
}

static void handle_tap(AccelAxisType axis, int32_t direction) {
    //get battery state
    BatteryChargeState state = battery_state_service_peek();
    if(state.is_plugged && state.is_charging) {
        snprintf(batt_text, 10, "+%d %%", state.charge_percent);
    } else {
        snprintf(batt_text, 10, "%d %%", state.charge_percent);     
    }
    text_layer_set_text(batt_layer, batt_text);
    layer_set_hidden(text_layer_get_layer(batt_layer), false);
    
    //get blutooth state
    if(bluetooth_connection_service_peek()) {
        text_layer_set_text(blue_layer, LOCALE_BT_ON);
    } else {
        text_layer_set_text(blue_layer, LOCALE_BT_OFF);
    }
    layer_set_hidden(text_layer_get_layer(blue_layer), false);    

    peek_timer = app_timer_register(PEEK_TIMEOUT, on_peek_timer, NULL);
}

    
/*
 * Load window members
 */
static void window_load(Window *window) {
    window_set_background_color(window, GColorBlack);

    //Allocate text layers
    h_t_layer = cl_init_text_layer(GRect(HTX, 53, 50, 60), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentRight);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(h_t_layer));

    h_u_layer = cl_init_text_layer(GRect(HUX, 53, 50, 60), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentRight);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(h_u_layer));

    colon_layer = cl_init_text_layer(GRect(68, 53, 50, 60), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentLeft);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(colon_layer));

    m_t_layer = cl_init_text_layer(GRect(MTX, 53, 50, 60), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentRight);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(m_t_layer));

    m_u_layer = cl_init_text_layer(GRect(MUX, 53, 50, 60), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentRight);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(m_u_layer));

    date_layer = cl_init_text_layer(GRect(15, 105, 130, 30), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_24, NULL, GTextAlignmentRight);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));

    batt_layer = cl_init_text_layer(GRect(54, 0, 90, 16), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_12, NULL, GTextAlignmentRight);
    layer_set_hidden(text_layer_get_layer(batt_layer), true);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(batt_layer));

    blue_layer = cl_init_text_layer(GRect(0, 0, 90, 16), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_12, NULL, GTextAlignmentRight);
    layer_set_hidden(text_layer_get_layer(blue_layer), true);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(blue_layer));
    
    
    //Allocate inverter layers
    h_t_inv_layer = inverter_layer_create(GRect(0, 0, INV_LAYER_WIDTH, 0));	
    h_u_inv_layer = inverter_layer_create(GRect(0, 0, INV_LAYER_WIDTH, 0));	
    bottom_inv_layer = inverter_layer_create(GRect(0, 0, 144, 0));
    m_t_inv_layer = inverter_layer_create(GRect(0, 0, INV_LAYER_WIDTH, 0));	
    m_u_inv_layer = inverter_layer_create(GRect(0, 0, INV_LAYER_WIDTH, 0));

    layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(h_t_inv_layer));	
    layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(h_u_inv_layer));
    layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(bottom_inv_layer));
    layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(m_t_inv_layer));	
    layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(m_u_inv_layer));	

    //Make sure the face is not blank
    time_t temp = time(NULL);
    t = localtime(&temp);
    get_time_digits(t);

    //Stop 'all change' on first minute
    m_u_prev = m_u_digit;
    m_t_prev = m_t_digit;
    h_u_prev = h_u_digit;
    h_t_prev = h_t_digit;
    
    set_time_digits();

    //Init progress bar
    update_progress_bar(t);
}

/*
 * Unload window members
 */
static void window_unload(Window *window) {
    //Free text layers
    text_layer_destroy(h_t_layer);
    text_layer_destroy(h_u_layer);
    text_layer_destroy(colon_layer);
    text_layer_destroy(m_t_layer);
    text_layer_destroy(m_u_layer);
    text_layer_destroy(date_layer);
    text_layer_destroy(batt_layer);
    text_layer_destroy(blue_layer);
    
    //Free inverter layers
    inverter_layer_destroy(h_t_inv_layer);
    inverter_layer_destroy(h_u_inv_layer);
    inverter_layer_destroy(bottom_inv_layer);
    inverter_layer_destroy(m_t_inv_layer);
    inverter_layer_destroy(m_u_inv_layer);

    //Unsubscribe from events
    tick_timer_service_unsubscribe();
    accel_tap_service_unsubscribe();
}


#ifdef DEBUG
#include "time_debug.h"
#endif
    

/*
 * Init app
 */
static void init(void) {
    window = window_create();
    WindowHandlers handlers = {
        .load = window_load,
        .unload = window_unload
    };
    window_set_window_handlers(window, (WindowHandlers) handlers);

    //Subscribe to events
    tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
    accel_tap_service_subscribe(handle_tap);
    
    //Finally
    window_stack_push(window, true);
    
    #ifdef TIME_DEBUG
    window_set_click_config_provider_with_context(window, click_config_provider, (void*)window);
    #endif

}

/*
 * De-init app
 */
static void deinit(void) {
    window_destroy(window);
}

/*
 * Entry point
 */
int main(void) {
    init();
    app_event_loop();
    deinit();
}

/*
 * Function definitions
 ***************************************************************************8
 */

/**
	* Function to get time digits
	*/
void get_time_digits(struct tm *t) 
{
    //Hour string
    if(clock_is_24h_style()) {
        strftime(time_text, sizeof("XX:XX"), "%H:%M", t);
    } else {
        strftime(time_text, sizeof("XX:XX"), "%I:%M", t);
    }

    //Get digits
    m_u_digit = time_text[4] - '0';  //Conv to int from char
    m_t_digit = time_text[3] - '0';
    h_u_digit = time_text[1] - '0';
    h_t_digit = time_text[0] - '0';

    snprintf(date_text, sizeof(date_text), "%s %02d", LOCALE_DAYS[t->tm_wday], t->tm_mday);
}

/**
 * Function to set the time and date digits on the TextLayers
 */
void set_time_digits() 
{  
    //Include null chars
    static char chars[4][2] = {"1", "2", "3", "4"};
    chars[0][0] = time_text[4];
    chars[1][0] = time_text[3];
    chars[2][0] = time_text[1];
    chars[3][0] = time_text[0];

    //Set digits in TextLayers
    text_layer_set_text(h_t_layer, chars[3]);
    text_layer_set_text(h_u_layer, chars[2]);
    text_layer_set_text(colon_layer, ":");
    text_layer_set_text(m_t_layer, chars[1]);
    text_layer_set_text(m_u_layer, chars[0]);

    text_layer_set_text(date_layer, date_text);
}


