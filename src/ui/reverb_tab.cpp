#include "reverb_tab.h"
#include "ui_helpers.h"

static void backEventHandler(lv_event_t * e);
static void onReverbButtonPowerClicked(lv_event_t * e);
static void onReverbSliderMoved(lv_event_t * e);

void ReverbTab::createLayout()
{
    const ReverbSettings *reverbSettings = _fxChain->reverbSettings();
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
    resize_back_button(menu);

    lv_obj_t * cont;
    lv_obj_t * section;

    lv_obj_t * sub_params_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(sub_params_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(sub_params_page);
    section = lv_menu_section_create(sub_params_page);

    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d%%", reverbSettings->mix);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Mix", 0, 100, reverbSettings->mix, buf);   
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_mixInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onReverbSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%d%%", reverbSettings->decay);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Decay", 0, 100, reverbSettings->decay, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_decayInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onReverbSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%d%%", reverbSettings->roomSize);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Room Size", 0, 100, reverbSettings->roomSize, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_roomSizeInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onReverbSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%d%%", reverbSettings->damping);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Damping", 0, 100, reverbSettings->damping, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_dampingInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onReverbSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%dms", reverbSettings->preDelay);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Pre-Delay", 0, 100, reverbSettings->preDelay, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_preDelayInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onReverbSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t * sub_eq_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(sub_eq_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(sub_eq_page);
    section = lv_menu_section_create(sub_eq_page);
    lv_snprintf(buf, sizeof(buf), "%dHz", reverbSettings->hiPass);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "High Pass", 20, 800, reverbSettings->hiPass, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_hiPassInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onReverbSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%dHz", reverbSettings->loPass);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Low Pass", 100, 16000, reverbSettings->loPass, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_loPassInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onReverbSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);


    /*Create a root page*/
    lv_obj_t * root_page = lv_menu_page_create(menu, "Reverb Settings");
    lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    section = lv_menu_section_create(root_page);
    cont = create_switch(section, LV_SYMBOL_POWER, "Enabled", reverbSettings->enabled);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onReverbButtonPowerClicked, LV_EVENT_VALUE_CHANGED, this);
    cont = create_text(section, LV_SYMBOL_AUDIO, "Params", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(menu, cont, sub_params_page);
    cont = create_text(section, LV_SYMBOL_SETTINGS, "Equalization", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(menu, cont, sub_eq_page);

    lv_menu_set_sidebar_page(menu, NULL);
    lv_menu_set_page(menu, root_page);
}

static void onReverbButtonPowerClicked(lv_event_t * e)
{
    auto* self = static_cast<ReverbTab*>(lv_event_get_user_data(e));
    self->handleReverbPower(e);
}

static void onReverbSliderMoved(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target_obj(e);
    ReverbControlInfo *data = static_cast<ReverbControlInfo*>(lv_obj_get_user_data(slider));
    data->chainRef->adjustReverbParam(data->param, lv_slider_get_value(slider)); 
}

static void backEventHandler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    lv_obj_t * menu = (lv_obj_t *)lv_event_get_user_data(e);

    if(lv_menu_back_button_is_root(menu, obj)) {}
}

void ReverbTab::handleReverbPower(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) 
    {
        lv_obj_t * obj = lv_event_get_target_obj(e);
        _fxChain->enableReverb(lv_obj_has_state(obj, LV_STATE_CHECKED));
    }
}