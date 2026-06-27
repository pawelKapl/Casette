#pragma once

#include "lvgl/lvgl.h"
#include "env.h"

typedef enum {
    LV_MENU_ITEM_BUILDER_VARIANT_1,
    LV_MENU_ITEM_BUILDER_VARIANT_2
} lv_menu_builder_variant_t;

static lv_style_t style_radio;
static lv_style_t style_radio_chk;

static int string_ends_with(const char * str, const char * suffix)
{
  int str_len = strlen(str);
  int suffix_len = strlen(suffix);

  return 
    (str_len >= suffix_len) &&
    (0 == strcmp(str + (str_len-suffix_len), suffix));
}

static void update_slider_value_percent(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target_obj(e);
    lv_obj_t * parent = lv_obj_get_parent(slider);
    lv_obj_t * label = lv_obj_get_child(parent, 3);
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
    lv_label_set_text(label, buf);
    lv_obj_align_to(label, parent, LV_ALIGN_CENTER, 0, 0);
}

static void update_slider_value_ms(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target_obj(e);
    lv_obj_t * parent = lv_obj_get_parent(slider);
    lv_obj_t * label = lv_obj_get_child(parent, 3);
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%dms", (int)lv_slider_get_value(slider));
    lv_label_set_text(label, buf);
    lv_obj_align_to(label, parent, LV_ALIGN_CENTER, 0, 0);
}

static void update_slider_value_hz(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target_obj(e);
    lv_obj_t * parent = lv_obj_get_parent(slider);
    lv_obj_t * label = lv_obj_get_child(parent, 3);
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%dHz", (int)lv_slider_get_value(slider));
    lv_label_set_text(label, buf);
    lv_obj_align_to(label, parent, LV_ALIGN_CENTER, 0, 0);
}

static void update_slider_value_mhz(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target_obj(e);
    lv_obj_t * parent = lv_obj_get_parent(slider);
    lv_obj_t * label = lv_obj_get_child(parent, 3);
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%dmHz", (int)lv_slider_get_value(slider));
    lv_label_set_text(label, buf);
    lv_obj_align_to(label, parent, LV_ALIGN_CENTER, 0, 0);
}

static void resize_back_button(lv_obj_t *menu)
{
    lv_obj_t * btn  = lv_menu_get_main_header_back_button(menu);
    lv_obj_t * icon = lv_obj_get_child_by_type(btn, 0, &lv_image_class);

    /* Use a larger font */
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_30, 0);

    lv_obj_set_width(btn, 48);
    lv_obj_set_height(btn, 48);
}

static float positionToDb(float position, float minDb = -50.0f, float maxDb = 40.0f)
{
    float dB;
    if (position < 0.5f)
        return (1.0f - position / 0.5f) * minDb;
    else
        return ((position - 0.5f) / 0.5f) * maxDb;
}

static float dbToPosition(float dB, float minDb = -50.0f, float maxDb = 40.0f)
{
    if (dB < 0.0f)
    {
        float t = (dB - minDb) / (0.0f - minDb); // t in [0, 1]
        return t * 0.5f;                        // map to [0, 0.5]
    }
    else
    {
        float t = dB / maxDb;                   // t in [0, 1]
        return 0.5f + t * 0.5f;                 // map to [0.5, 1]
    }
}

static void update_slider_value_db(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target_obj(e);
    lv_obj_t * parent = lv_obj_get_parent(slider);
    lv_obj_t * label = lv_obj_get_child(parent, 3);

    auto sliderPos = (float)lv_slider_get_value(slider) / 100.0f;
    float dB = positionToDb(sliderPos, -50.0f, 12.0f);
    float dB_rounded = std::round(dB * 10.0f) / 12.0f;

    char buf[8];
    std::snprintf(buf, sizeof(buf), "%+.1fdB", dB_rounded);
    lv_label_set_text(label, buf);
    lv_obj_align_to(label, parent, LV_ALIGN_CENTER, 0, 0);
}

static void update_knob_value(lv_event_t * e)
{
    lv_obj_t * knob = lv_event_get_target_obj(e);
    lv_obj_t * label = lv_obj_get_child(knob, 0);

    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%.1f", (float)lv_arc_get_value(knob) / 10.0f);
    lv_label_set_text(label, buf);
}

static lv_obj_t * create_text(lv_obj_t * parent, const char * icon, const char * txt,
                              lv_menu_builder_variant_t builder_variant)
{
    lv_obj_t * obj = lv_menu_cont_create(parent);
    lv_obj_set_style_pad_top(obj, 9, 0);


    lv_obj_t * img = NULL;
    lv_obj_t * label = NULL;

    if(icon) {
        img = lv_image_create(obj);
        lv_obj_set_style_text_color(img, lv_color_make(224, 26, 79), 0);
        lv_image_set_src(img, icon);
    }

    if(txt) {
        label = lv_label_create(obj);
        lv_label_set_text(label, txt);
        lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(label, 1);
    }

    if(builder_variant == LV_MENU_ITEM_BUILDER_VARIANT_2 && icon && txt) {
        lv_obj_add_flag(img, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
        lv_obj_swap(img, label);
    }

    return obj;
}

static lv_obj_t * create_text(lv_obj_t * parent, const char * txt)
{
    lv_obj_t * obj = lv_menu_cont_create(parent);
    lv_obj_set_style_pad_top(obj, 9, 0);


    lv_obj_t * label = NULL;

    if(txt) {
        label = lv_label_create(obj);
        lv_label_set_text(label, txt);
        lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(label, 1);
    }

    return obj;
}

static lv_obj_t * create_slider(lv_obj_t * parent, const char * icon, const char * txt, int32_t min, int32_t max,
                                int32_t val, const char * unit, lv_event_cb_t customHandler = nullptr)
{
    lv_obj_t * obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_2);

    lv_obj_t * slider = lv_slider_create(obj);
    lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, val, LV_ANIM_OFF);

    if(icon == NULL) {
        lv_obj_add_flag(slider, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    }

    auto slider_label = lv_label_create(obj);
    lv_label_set_text(slider_label, unit);
    lv_obj_align_to(slider_label, obj, LV_ALIGN_CENTER, 0, 0);

    lv_event_cb_t handler;
    if (customHandler != nullptr)
    {
        handler = customHandler;
    }
    else if (string_ends_with(unit, "ms"))
    {
        handler = update_slider_value_ms;
    }
    else if (string_ends_with(unit, "mHz"))
    {
        handler = update_slider_value_mhz;
    }
    else if (string_ends_with(unit, "Hz"))
    {
        handler = update_slider_value_hz;
    }
    else if (string_ends_with(unit, "dB"))
    {
        handler = update_slider_value_db;
    }
    else
    {
        handler = update_slider_value_percent;
    }

    lv_obj_add_event_cb(slider, handler, LV_EVENT_VALUE_CHANGED, NULL);
    

    return obj;
}

static lv_obj_t * create_switch(lv_obj_t * parent, const char * icon, const char * txt, bool chk)
{
    lv_obj_t * obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_1);

    lv_obj_t * sw = lv_switch_create(obj);
    lv_obj_set_height(sw, 22);
    lv_obj_set_width(sw, 45);
    lv_obj_add_state(sw, chk ? LV_STATE_CHECKED : 0);

    return obj;
}

static void radiobutton_create(lv_obj_t * parent, const char * txt)
{
    lv_style_init(&style_radio);
    lv_style_set_radius(&style_radio, LV_RADIUS_CIRCLE);
    lv_style_init(&style_radio_chk);
    lv_style_set_bg_image_src(&style_radio_chk, NULL);

    lv_obj_t * obj = lv_checkbox_create(parent);
    lv_checkbox_set_text(obj, txt);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_style(obj, &style_radio, LV_PART_INDICATOR);
    lv_obj_add_style(obj, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
}

static lv_obj_t * create_knob(lv_obj_t * parent, const char * txt, int32_t min, int32_t max,
                                int32_t val, const char * unit)
{
    static lv_style_t noFrame;
    lv_style_init(&noFrame);
    lv_style_set_border_width(&noFrame, 0);
    
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_add_style(cont, &noFrame, 0);
    lv_obj_set_size(cont, 215, 170);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * arc = lv_arc_create(cont);
    lv_obj_set_size(arc, 90, 90);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 0, 270);
    lv_arc_set_range(arc, min, max);
    lv_arc_set_value(arc, val);

    lv_obj_t * value_label = lv_label_create(arc);
    lv_label_set_text(value_label, unit);
    lv_obj_center(value_label);

    lv_obj_t * name_label = lv_label_create(cont);
    lv_label_set_text(name_label, txt);

    lv_obj_add_event_cb(arc, update_knob_value, LV_EVENT_VALUE_CHANGED, NULL);

    return cont;
}
