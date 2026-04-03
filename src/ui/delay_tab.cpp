#include "delay_tab.h"
#include "ui_helpers.h"

static void backEventHandler(lv_event_t * e);
static void onDelayButtonPowerClicked(lv_event_t * e);
static void onDelaySliderMoved(lv_event_t * e);
static void radioEventHandler(lv_event_t * e);

void DelayTab::createLayout()
{
    const DelaySettings *delaySettings = _fxChain->delaySettings();
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

    /*Create sub pages*/
    lv_obj_t * sub_type_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(sub_type_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(sub_type_page);
    section = lv_menu_section_create(sub_type_page);
    lv_obj_set_size(section, lv_pct(100), lv_pct(75));
    create_text(section, LV_SYMBOL_SETTINGS, "Delay type: ", LV_MENU_ITEM_BUILDER_VARIANT_1);

    lv_obj_t * delayTypeSelector = lv_obj_create(section);
    static lv_style_t noFrame;
    lv_style_init(&noFrame);
    lv_style_set_border_width(&noFrame, 0);
    lv_obj_add_style(delayTypeSelector, &noFrame, 0);
    lv_obj_set_flex_flow(delayTypeSelector, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(delayTypeSelector, lv_pct(50), lv_pct(80));
    lv_obj_add_event_cb(delayTypeSelector, radioEventHandler, LV_EVENT_CLICKED, &_modeInfo);
    radiobutton_create(delayTypeSelector, "Mono");
    radiobutton_create(delayTypeSelector, "Tape");
    radiobutton_create(delayTypeSelector, "Dual Tap");
    radiobutton_create(delayTypeSelector, "Ping-Pong");
    radiobutton_create(delayTypeSelector, "Circular");
    lv_obj_add_state(lv_obj_get_child(delayTypeSelector, delaySettings->delayMode), LV_STATE_CHECKED);

    lv_obj_t * sub_params_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(sub_params_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(sub_params_page);
    section = lv_menu_section_create(sub_params_page);

    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d%%", delaySettings->mix);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Mix", 0, 100, delaySettings->mix, buf);   
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_mixInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onDelaySliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%d%%", delaySettings->feedback);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Feedback", 0, 90, delaySettings->feedback, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_feedbackInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onDelaySliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%dms", delaySettings->time);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Time", 10, 1000, delaySettings->time, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_timeInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onDelaySliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%dms", delaySettings->time2);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Time'", 10, 1000, delaySettings->time2, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_time2Info);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onDelaySliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%d%%", delaySettings->diffusion);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Diffusion", 0, 90, delaySettings->diffusion, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_diffusionInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onDelaySliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%d", delaySettings->modRate);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Mod Rate", 0, 4, delaySettings->modRate, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_modRateInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onDelaySliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t * sub_eq_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(sub_eq_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(sub_eq_page);
    section = lv_menu_section_create(sub_eq_page);
    lv_snprintf(buf, sizeof(buf), "%dHz", delaySettings->hiPass);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "High Pass", 20, 800, delaySettings->hiPass, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_hiPassInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onDelaySliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%dHz", delaySettings->loPass);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Low Pass", 100, 16000, delaySettings->loPass, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_loPassInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onDelaySliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    /*Create a root page*/
    lv_obj_t * root_page = lv_menu_page_create(menu, "Delay Settings");
    lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    section = lv_menu_section_create(root_page);
    cont = create_switch(section, LV_SYMBOL_POWER, "Enabled", delaySettings->enabled);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onDelayButtonPowerClicked, LV_EVENT_VALUE_CHANGED, this);
    cont = create_text(section, LV_SYMBOL_SETTINGS, "Type", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(menu, cont, sub_type_page);
    cont = create_text(section, LV_SYMBOL_AUDIO, "Params", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(menu, cont, sub_params_page);
    cont = create_text(section, LV_SYMBOL_SETTINGS, "Equalization", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(menu, cont, sub_eq_page);

    lv_menu_set_sidebar_page(menu, NULL);
    lv_menu_set_page(menu, root_page);
}

static void onDelayButtonPowerClicked(lv_event_t * e)
{
    auto* self = static_cast<DelayTab*>(lv_event_get_user_data(e));
    self->handleDelayPower(e);
}

static void onDelaySliderMoved(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target_obj(e);
    DelayControlInfo *data = static_cast<DelayControlInfo*>(lv_obj_get_user_data(slider));
    data->chainRef->adjustDelayParam(data->param, lv_slider_get_value(slider)); 
}

static void radioEventHandler(lv_event_t * e)
{
    DelayControlInfo * info = static_cast<DelayControlInfo*>(lv_event_get_user_data(e));
    lv_obj_t * cont = (lv_obj_t *)lv_event_get_current_target(e);
    lv_obj_t * act_cb = lv_event_get_target_obj(e);
    lv_obj_t * old_cb = lv_obj_get_child(cont, info->param);

    if(act_cb == cont) return;

    lv_obj_remove_state(old_cb, LV_STATE_CHECKED);
    lv_obj_add_state(act_cb, LV_STATE_CHECKED);

    info->param = lv_obj_get_index(act_cb);
    info->chainRef->adjustDelayParam(0, info->param);
}

static void backEventHandler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    lv_obj_t * menu = (lv_obj_t *)lv_event_get_user_data(e);

    if(lv_menu_back_button_is_root(menu, obj)) {}
}

void DelayTab::handleDelayPower(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) 
    {
        lv_obj_t * obj = lv_event_get_target_obj(e);
        _fxChain->enableDelay(lv_obj_has_state(obj, LV_STATE_CHECKED));
    }
}
