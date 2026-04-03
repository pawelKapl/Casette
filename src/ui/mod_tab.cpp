#include "ui/mod_tab.h"
#include "reverb_tab.h"
#include "ui_helpers.h"

static void backEventHandler(lv_event_t * e);
static void onModButtonPowerClicked(lv_event_t * e);
static void onVibratoButtonClicked(lv_event_t * e);
static void onModSliderMoved(lv_event_t * e);

static void mod_update_slider_value_ms(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target_obj(e);
    lv_obj_t * parent = lv_obj_get_parent(slider);
    lv_obj_t * label = lv_obj_get_child(parent, 3);
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%.1fms", (int)lv_slider_get_value(slider) / 10.);
    lv_label_set_text(label, buf);
    lv_obj_align_to(label, parent, LV_ALIGN_CENTER, 0, 0);
}

static void mod_update_slider_value_hz(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target_obj(e);
    lv_obj_t * parent = lv_obj_get_parent(slider);
    lv_obj_t * label = lv_obj_get_child(parent, 3);
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%.2fHz", (int)lv_slider_get_value(slider) / 1000.);
    lv_label_set_text(label, buf);
    lv_obj_align_to(label, parent, LV_ALIGN_CENTER, 0, 0);
}

void ModTab::createLayout()
{
    const ModulatorSettings *modSettings = _fxChain->modulatorSettings();
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

    lv_obj_t * sub_params_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(sub_params_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(sub_params_page);
    section = lv_menu_section_create(sub_params_page);

    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d%%", modSettings->mix);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Mix", 0, 100, modSettings->mix, buf);   
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_mixInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onModSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%.1fms", modSettings->depth / 10.);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Depth", 0, 100, modSettings->depth, buf, mod_update_slider_value_ms);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_depthInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onModSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%.2fHz", modSettings->rate / 1000.);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Rate", 100, 10000, modSettings->rate, buf, mod_update_slider_value_hz);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_rateInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onModSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%d%%", modSettings->width);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Width", 0, 100, modSettings->width, buf);   
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_widthInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onModSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%d%%", modSettings->feedback);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Feedback", -100, 100, modSettings->feedback, buf);   
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_feedbackInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onModSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    /*Create a root page*/
    lv_obj_t * root_page = lv_menu_page_create(menu, "Modulation Settings");
    lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    section = lv_menu_section_create(root_page);
    cont = create_switch(section, LV_SYMBOL_POWER, "Enabled", modSettings->enabled);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onModButtonPowerClicked, LV_EVENT_VALUE_CHANGED, this);
    cont = create_switch(section, LV_SYMBOL_SHUFFLE, "Vibrato", modSettings->vibratoMode);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onVibratoButtonClicked, LV_EVENT_VALUE_CHANGED, this);

    cont = create_text(section, LV_SYMBOL_AUDIO, "Params", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(menu, cont, sub_params_page);

    lv_menu_set_sidebar_page(menu, NULL);
    lv_menu_set_page(menu, root_page);
}

static void onModButtonPowerClicked(lv_event_t * e)
{
    auto* self = static_cast<ModTab*>(lv_event_get_user_data(e));
    self->handleModPower(e);
}

static void onModSliderMoved(lv_event_t * e)
{
    lv_obj_t *slider = lv_event_get_target_obj(e);
    ModControlInfo *data = static_cast<ModControlInfo*>(lv_obj_get_user_data(slider));
    data->chainRef->adjustModParam(data->param, lv_slider_get_value(slider)); 
}

static void onVibratoButtonClicked(lv_event_t * e)
{
    auto* self = static_cast<ModTab*>(lv_event_get_user_data(e));
    self->handleVibratoMode(e);
}

static void backEventHandler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    lv_obj_t * menu = (lv_obj_t *)lv_event_get_user_data(e);

    if(lv_menu_back_button_is_root(menu, obj)) {}
}

void ModTab::handleModPower(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) 
    {
        lv_obj_t * obj = lv_event_get_target_obj(e);
        _fxChain->enableMod(lv_obj_has_state(obj, LV_STATE_CHECKED));
    }
}

void ModTab::handleVibratoMode(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) 
    {
        lv_obj_t * obj = lv_event_get_target_obj(e);
        _fxChain->adjustModParam(6, lv_obj_has_state(obj, LV_STATE_CHECKED));
    }
}