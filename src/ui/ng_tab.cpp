#include "ng_tab.h"
#include "ui_helpers.h"

static void backEventHandler(lv_event_t * e);
static void onNGButtonPowerClicked(lv_event_t * e);
static void onNGSliderMoved(lv_event_t * e);

static void update_threshold_slider_value(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target_obj(e);
    lv_obj_t * parent = lv_obj_get_parent(slider);
    lv_obj_t * label = lv_obj_get_child(parent, 3);

    float dB = (float)lv_slider_get_value(slider);

    char buf[8];
    std::snprintf(buf, sizeof(buf), "%-.1fdB", dB);
    lv_label_set_text(label, buf);
    lv_obj_align_to(label, parent, LV_ALIGN_CENTER, 0, 0);
}

void NGTab::createLayout()
{
    auto settings = _noiseGate->gateSettings();
    lv_obj_t * menu = lv_menu_create(_tab);

    lv_color_t bg_color = lv_obj_get_style_bg_color(menu, 0);
    if(lv_color_brightness(bg_color) > 127) {
        lv_obj_set_style_bg_color(menu, lv_color_darken(lv_obj_get_style_bg_color(menu, 0), 10), 0);
    }
    else {
        lv_obj_set_style_bg_color(menu, lv_color_darken(lv_obj_get_style_bg_color(menu, 0), 50), 0);
    }
    lv_menu_set_mode_root_back_button(menu, LV_MENU_ROOT_BACK_BUTTON_ENABLED);
    lv_obj_add_event_cb(menu, backEventHandler, LV_EVENT_CLICKED, menu);
    lv_obj_set_size(menu, lv_display_get_horizontal_resolution(NULL) - 80, lv_display_get_vertical_resolution(NULL));
    lv_obj_center(menu);

    lv_obj_t * cont;
    lv_obj_t * section;

    /*Create a root page*/
    lv_obj_t * root_page = lv_menu_page_create(menu, "Noise Gate Settings");
    lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    section = lv_menu_section_create(root_page);
    cont = create_switch(section, LV_SYMBOL_POWER, "Enabled", settings->enabled);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onNGButtonPowerClicked, LV_EVENT_VALUE_CHANGED, this);

    
    auto threshold = settings->threshold;
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%-.1fdB", threshold);
    cont = create_slider(section, LV_SYMBOL_CHARGE, "Threshold", -100, 0, threshold, buf, update_threshold_slider_value);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onNGSliderMoved, LV_EVENT_VALUE_CHANGED, this);

    lv_menu_set_sidebar_page(menu, NULL);
    lv_menu_set_page(menu, root_page);
}

static void onNGButtonPowerClicked(lv_event_t * e)
{
    auto* self = static_cast<NGTab*>(lv_event_get_user_data(e));
    self->handleNGPower(e);
}

static void backEventHandler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    lv_obj_t * menu = (lv_obj_t *)lv_event_get_user_data(e);

    if(lv_menu_back_button_is_root(menu, obj)) {}
}

void NGTab::handleNGPower(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) 
    {
        lv_obj_t * obj = lv_event_get_target_obj(e);
        _noiseGate->enabled(lv_obj_has_state(obj, LV_STATE_CHECKED));
    }
}

void NGTab::handleThresholdChange(float threshold)
{
    _noiseGate->setThresholdDB(threshold);
}

static void onNGSliderMoved(lv_event_t * e)
{
    lv_obj_t *slider = lv_event_get_target_obj(e);
    float threshold = (float)lv_slider_get_value(slider);
    auto* self = static_cast<NGTab*>(lv_event_get_user_data(e));
    self->handleThresholdChange(threshold);
}