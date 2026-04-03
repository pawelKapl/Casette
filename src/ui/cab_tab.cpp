#include "cab_tab.h"
#include "ui_helpers.h"

static void backEventHandler(lv_event_t * e);
static void onCabButtonPowerClicked(lv_event_t * e);
static void onCabModelPicked(lv_event_t * e);
static void onCabSliderMoved(lv_event_t * e);

void CabTab::createLayout()
{
    auto cabSettings = _cab->cabSettings();
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
    lv_obj_t * root_page = lv_menu_page_create(menu, "Cab Simulator Settings");
    lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    section = lv_menu_section_create(root_page);
    cont = create_switch(section, LV_SYMBOL_POWER, "Enabled", cabSettings->enabled);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onCabButtonPowerClicked, LV_EVENT_VALUE_CHANGED, this);

    cont = lv_menu_cont_create(section);
    lv_obj_t * cabSelector = lv_dropdown_create(cont);
    lv_obj_set_style_text_color(cabSelector, lv_color_make(224, 26, 79), LV_PART_INDICATOR);

    auto irPath = std::string("../irs/");

    std::string irs;
    int index = 0;
    int pickedOne = 0;
    for (const auto &file : std::filesystem::directory_iterator(irPath))
    {
        std::string filename = file.path().stem();
        if (filename == cabSettings->model)
            pickedOne = index;
        irs += filename;
        irs += "\n";
        index++;
    }
    if (!irs.empty()) irs.pop_back();
    
    lv_dropdown_set_options(cabSelector, irs.c_str());
    lv_dropdown_set_selected(cabSelector, pickedOne, NULL);

    lv_obj_add_flag(cabSelector, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    lv_obj_set_flex_grow(cabSelector, 1);
    lv_obj_align(cabSelector, LV_ALIGN_BOTTOM_LEFT, 0, 20);
    lv_obj_add_event_cb(cabSelector, onCabModelPicked, LV_EVENT_ALL, this);

    char buf[10];
    lv_snprintf(buf, sizeof(buf), "%dHz", cabSettings->hiPass);
    cont = create_slider(section, LV_SYMBOL_SETTINGS, "High Pass", 1, 500, cabSettings->hiPass, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_hiPassInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onCabSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_menu_set_sidebar_page(menu, NULL);
    lv_menu_set_page(menu, root_page);
}

static void onCabButtonPowerClicked(lv_event_t * e)
{
    auto* self = static_cast<CabTab*>(lv_event_get_user_data(e));
    self->handleCabPower(e);
}

static void backEventHandler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    lv_obj_t * menu = (lv_obj_t *)lv_event_get_user_data(e);

    if(lv_menu_back_button_is_root(menu, obj)) {}
}

void CabTab::handleCabPower(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) 
    {
        lv_obj_t * obj = lv_event_get_target_obj(e);
        _cab->enabled(lv_obj_has_state(obj, LV_STATE_CHECKED));
    }
}

void CabTab::changeCabIR(const char* newCab)
{
    log_info << "Loading IR: " << newCab;
    _cab->loadIr(newCab);
}

static void onCabModelPicked(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target_obj(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
        auto* self = static_cast<CabTab*>(lv_event_get_user_data(e));
        self->changeCabIR(buf);
    }
}

static void onCabSliderMoved(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target_obj(e);
    CabControlInfo *data = static_cast<CabControlInfo*>(lv_obj_get_user_data(slider));
    data->cabRef->setHighPassFrequency(lv_slider_get_value(slider)); 
}
