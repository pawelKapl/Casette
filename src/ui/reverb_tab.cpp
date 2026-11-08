#include "reverb_tab.h"
#include "ui_helpers.h"

static void backEventHandler(lv_event_t * e);
static void onReverbButtonPowerClicked(lv_event_t * e);
static void onReverbSliderMoved(lv_event_t * e);
static void radioEventHandler(lv_event_t * e);
static void onReverbSwitchToggled(lv_event_t * e);

// in ReverbMode order
static const char *REVERB_TYPES[] = {"Room", "Spring", "Hall"};
static_assert(sizeof(REVERB_TYPES) / sizeof(REVERB_TYPES[0]) == REVERB_MODE_COUNT);

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
    lv_obj_set_size(menu, lv_pct(100), lv_pct(100));
    lv_obj_center(menu);
    resize_back_button(menu);

    lv_obj_t * cont;
    lv_obj_t * section;

    lv_obj_t * sub_type_page = lv_menu_page_create(menu, "Reverb Type");
    lv_obj_set_style_pad_hor(sub_type_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(sub_type_page);
    section = lv_menu_section_create(sub_type_page);
    create_text(section, LV_SYMBOL_SETTINGS, "Reverb type: ", LV_MENU_ITEM_BUILDER_VARIANT_1);

    lv_obj_t * reverbTypeSelector = lv_obj_create(section);
    static lv_style_t noFrame;
    lv_style_init(&noFrame);
    lv_style_set_border_width(&noFrame, 0);
    lv_obj_add_style(reverbTypeSelector, &noFrame, 0);
    lv_obj_set_flex_flow(reverbTypeSelector, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(reverbTypeSelector, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_remove_flag(reverbTypeSelector, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(reverbTypeSelector, radioEventHandler, LV_EVENT_CLICKED, this);
    for (const char *type : REVERB_TYPES)
        radiobutton_create(reverbTypeSelector, type);
    int reverbMode = std::clamp(reverbSettings->reverbMode, 0, REVERB_MODE_COUNT - 1);
    lv_obj_add_state(lv_obj_get_child(reverbTypeSelector, reverbMode), LV_STATE_CHECKED);
    _modeInfo.param = reverbMode;

    lv_obj_t * sub_params_page = lv_menu_page_create(menu, "Reverb Params");
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
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Pre-Delay", 0, 500, reverbSettings->preDelay, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_preDelayInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onReverbSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t * sub_eq_page = lv_menu_page_create(menu, "Reverb Equalization");
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

    lv_obj_t * sub_character_page = lv_menu_page_create(menu, "Reverb Character");
    lv_obj_set_style_pad_hor(sub_character_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(sub_character_page);

    _characterSections[ROOM] = lv_menu_section_create(sub_character_page);
    create_text(_characterSections[ROOM], LV_SYMBOL_SETTINGS, "Room has no extra controls", LV_MENU_ITEM_BUILDER_VARIANT_1);

    section = _characterSections[SPRING] = lv_menu_section_create(sub_character_page);
    lv_snprintf(buf, sizeof(buf), "%d%%", reverbSettings->dwell);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Dwell", 0, 100, reverbSettings->dwell, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_dwellInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onReverbSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%d%%", reverbSettings->tension);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Tension", 0, 100, reverbSettings->tension, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_tensionInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onReverbSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%d", reverbSettings->springs);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Springs", 1, SPRING_COUNT, reverbSettings->springs, buf, update_slider_value_plain);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_springsInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onReverbSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    section = _characterSections[HALL] = lv_menu_section_create(sub_character_page);
    lv_snprintf(buf, sizeof(buf), "%.1fms", reverbSettings->modDepth / 10.);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Mod Depth", 0, 20, reverbSettings->modDepth, buf, update_slider_value_tenth_ms);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_modDepthInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onReverbSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_snprintf(buf, sizeof(buf), "%.2fHz", reverbSettings->modRate / 1000.);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "Mod Rate", 50, 2000, reverbSettings->modRate, buf, update_slider_value_mhz_as_hz);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_modRateInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onReverbSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    cont = create_switch(section, LV_SYMBOL_PAUSE, "Freeze", reverbSettings->freeze);
    _freezeSwitch = lv_obj_get_child(cont, 2);
    lv_obj_add_event_cb(_freezeSwitch, onReverbSwitchToggled, LV_EVENT_VALUE_CHANGED, &_freezeInfo);

    showCharacter(reverbMode);

    /*Create a root page*/
    lv_obj_t * root_page = lv_menu_page_create(menu, "Reverb Settings");
    lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    section = lv_menu_section_create(root_page);
    cont = create_switch(section, LV_SYMBOL_POWER, "Enabled", reverbSettings->enabled);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onReverbButtonPowerClicked, LV_EVENT_VALUE_CHANGED, this);
    cont = create_text(section, LV_SYMBOL_SETTINGS, "Type", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(menu, cont, sub_type_page);
    cont = create_text(section, LV_SYMBOL_AUDIO, "Params", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(menu, cont, sub_params_page);
    cont = create_text(section, LV_SYMBOL_AUDIO, "Character", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(menu, cont, sub_character_page);
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

static void onReverbSwitchToggled(lv_event_t * e)
{
    lv_obj_t * sw = lv_event_get_target_obj(e);
    ReverbControlInfo *data = static_cast<ReverbControlInfo*>(lv_event_get_user_data(e));
    data->chainRef->adjustReverbParam(data->param, lv_obj_has_state(sw, LV_STATE_CHECKED));
}

static void radioEventHandler(lv_event_t * e)
{
    auto* self = static_cast<ReverbTab*>(lv_event_get_user_data(e));
    self->handleTypeSelected(e);
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

void ReverbTab::handleTypeSelected(lv_event_t * e)
{
    lv_obj_t * cont = (lv_obj_t *)lv_event_get_current_target(e);
    lv_obj_t * act_cb = lv_event_get_target_obj(e);
    lv_obj_t * old_cb = lv_obj_get_child(cont, _modeInfo.param);

    if(act_cb == cont) return;

    lv_obj_remove_state(old_cb, LV_STATE_CHECKED);
    lv_obj_add_state(act_cb, LV_STATE_CHECKED);

    // a frozen tank only makes sense for the sound that was playing
    if (lv_obj_has_state(_freezeSwitch, LV_STATE_CHECKED))
    {
        lv_obj_remove_state(_freezeSwitch, LV_STATE_CHECKED);
        _fxChain->adjustReverbParam(14, false);
    }

    _modeInfo.param = lv_obj_get_index(act_cb);
    _fxChain->adjustReverbParam(0, _modeInfo.param);
    showCharacter(_modeInfo.param);
}

void ReverbTab::showCharacter(int mode)
{
    for (int i = 0; i < REVERB_MODE_COUNT; i++)
    {
        if (!_characterSections[i])
            continue;
        if (i == mode)
            lv_obj_remove_flag(_characterSections[i], LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(_characterSections[i], LV_OBJ_FLAG_HIDDEN);
    }
}