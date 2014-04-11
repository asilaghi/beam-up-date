#pragma once
    
#ifdef TIME_DEBUG
static void up_click_handler(ClickRecognizerRef recognizer, void* context) {
    t->tm_min +=1;
    if (t->tm_min >= 60) {
        t->tm_min = 0;
        t->tm_hour += 1;
        if (t->tm_hour > 23) {
            t->tm_hour = 0;
        }
    }
    handle_tick(t, MINUTE_UNIT);
}

static void down_click_handler(ClickRecognizerRef recognizer, void* context) {
    t->tm_min -= 1;
    if(t->tm_min < 0) {
        t->tm_min = 59;
        t->tm_hour -= 1;
        if(t->tm_hour < 0) {
            t->tm_hour = 23;
        }
    }
    handle_tick(t, MINUTE_UNIT);
}

static void click_config_provider(void* context) {
    const uint16_t repeat_interval_ms = 100;
    window_set_click_context(BUTTON_ID_UP, context);
    window_single_repeating_click_subscribe(BUTTON_ID_UP, repeat_interval_ms, up_click_handler);

    window_set_click_context(BUTTON_ID_DOWN, context);
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, repeat_interval_ms, down_click_handler);
}
#endif
