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

//Prototypes
void get_time_digits(struct tm *t);
void set_time_digits();

//Globals
static Window *window;
static TextLayer *h_t_layer, *h_u_layer, *colon_layer, *m_t_layer, *m_u_layer, *date_layer;
static InverterLayer *h_t_inv_layer, *h_u_inv_layer, *m_t_inv_layer, *m_u_inv_layer, *bottom_inv_layer;
static int   h_t_digit = 0, h_t_prev = 0, 
    		 h_u_digit = 0, h_u_prev = 0, 
    		 m_t_digit = 0, m_t_prev = 0,
    		 m_u_digit = 0, m_u_prev = 0;
static char 
    time_text[5],
    date_text[] = "Duminica 01";

struct tm *t;
static int currentQuarter = -1;

static void update_progress_bar(struct tm *t) {
    int quarter = t->tm_min/13;
    
    if(currentQuarter != quarter) {
        currentQuarter = quarter;
        switch(quarter) {
            case 0:
                cl_animate_layer(inverter_layer_get_layer(bottom_inv_layer), GRect(0, 105, 144, 5), GRect(0, 105, 0, 5), 500, 0);        
                break;
            case 1:
                cl_animate_layer(inverter_layer_get_layer(bottom_inv_layer), GRect(0, 105, 0, 5), GRect(0, 105, 36, 5), 500, 0);
                break;
            case 2:
                cl_animate_layer(inverter_layer_get_layer(bottom_inv_layer), GRect(0, 105, 36, 5), GRect(0, 105, 72, 5), 500, 0);
                break;
            case 3:
                cl_animate_layer(inverter_layer_get_layer(bottom_inv_layer), GRect(0, 105, 72, 5), GRect(0, 105, 108, 5), 500, 0);
                break;
            case 4:
                cl_animate_layer(inverter_layer_get_layer(bottom_inv_layer), GRect(0, 105, 108, 5), GRect(0, 105, 144, 5), 500, 1000);
                break;    
        }
    }   
}

/**
 * Handle tick function
 */
static void handle_tick(struct tm *t, TimeUnits units_changed) 
{    
    //Get the time
    get_time_digits(t);
    set_time_digits(t);
    update_progress_bar(t);
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
	
	//Free inverter layers
	inverter_layer_destroy(h_t_inv_layer);
	inverter_layer_destroy(h_u_inv_layer);
	inverter_layer_destroy(bottom_inv_layer);
	inverter_layer_destroy(m_t_inv_layer);
	inverter_layer_destroy(m_u_inv_layer);
	
	//Unsubscribe from events
	tick_timer_service_unsubscribe();
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
	if(clock_is_24h_style())
    {
		strftime(time_text, sizeof("XX:XX"), "%H:%M", t);
    }
	else
    {
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


