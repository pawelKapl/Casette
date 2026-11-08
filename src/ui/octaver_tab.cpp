#include "octaver_tab.h"
#include "ui_helpers.h"

static void backEventHandler(lv_event_t * e);
static void onOctaverButtonPowerClicked(lv_event_t * e);
static void onOctaverSliderMoved(lv_event_t * e);
static void radioEventHandler(lv_event_t * e);

// in OctaverMode order
static const char *OCTAVER_TYPES[] = {"Polyphonic", "Analog"};
static_assert(sizeof(OCTAVER_TYPES) / sizeof(OCTAVER_TYPES[0]) == OCTAVER_MODE_COUNT);

void OctaverTab::createLayout()
{
    const OctaverSettings *settings = _preChain->octaverSettings();
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

    lv_obj_t * sub_type_page = lv_menu_page_create(menu, "Octaver Type");
    lv_obj_set_style_pad_hor(sub_type_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(sub_type_page);
    section = lv_menu_section_create(sub_type_page);
    create_text(section, LV_SYMBOL_SETTINGS, "Octaver type: ", LV_MENU_ITEM_BUILDER_VARIANT_1);

    lv_obj_t * typeSelector = lv_obj_create(section);
    static lv_style_t noFrame;
    lv_style_init(&noFrame);
    lv_style_set_border_width(&noFrame, 0);
    lv_obj_add_style(typeSelector, &noFrame, 0);
    lv_obj_set_flex_flow(typeSelector, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(typeSelector, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_remove_flag(typeSelector, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(typeSelector, radioEventHandler, LV_EVENT_CLICKED, &_modeInfo);
    for (const char *type : OCTAVER_TYPES)
        radiobutton_create(typeSelector, type);
    int mode = std::clamp(settings->mode, 0, OCTAVER_MODE_COUNT - 1);
    lv_obj_add_state(lv_obj_get_child(typeSelector, mode), LV_STATE_CHECKED);
    _modeInfo.param = mode;

    lv_obj_t * sub_voices_page = lv_menu_page_create(menu, "Octaver Voices");
    lv_obj_set_style_pad_hor(sub_voices_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(sub_voices_page);
    section = lv_menu_section_create(sub_voices_page);

    struct Voice
    {
        const char *name;
        int value;
        OctaverControlInfo *info;
    };
    Voice voices[] = {
        {"Dry", settings->dry, &_dryInfo},
        {"-2 Oct", settings->sub2, &_sub2Info},
        {"-1 Oct", settings->sub1, &_sub1Info},
        {"+1 Oct", settings->up1, &_up1Info},
        {"+2 Oct", settings->up2, &_up2Info},
        {"Tone", settings->tone, &_toneInfo},
    };

    char buf[8];
    for (const Voice &voice : voices)
    {
        lv_snprintf(buf, sizeof(buf), "%d%%", voice.value);
        cont = create_slider(section, LV_SYMBOL_SETTINGS, voice.name, 0, 100, voice.value, buf);
        lv_obj_set_user_data(lv_obj_get_child(cont, 2), voice.info);
        lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onOctaverSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);
    }

    /*Create a root page*/
    lv_obj_t * root_page = lv_menu_page_create(menu, "Octaver Settings");
    lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    section = lv_menu_section_create(root_page);
    cont = create_switch(section, LV_SYMBOL_POWER, "Enabled", settings->enabled);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onOctaverButtonPowerClicked, LV_EVENT_VALUE_CHANGED, this);
    cont = create_text(section, LV_SYMBOL_SETTINGS, "Type", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(menu, cont, sub_type_page);
    cont = create_text(section, LV_SYMBOL_AUDIO, "Voices", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(menu, cont, sub_voices_page);

    lv_menu_set_sidebar_page(menu, NULL);
    lv_menu_set_page(menu, root_page);
}

static void onOctaverButtonPowerClicked(lv_event_t * e)
{
    auto* self = static_cast<OctaverTab*>(lv_event_get_user_data(e));
    self->handleOctaverPower(e);
}

static void onOctaverSliderMoved(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target_obj(e);
    OctaverControlInfo *data = static_cast<OctaverControlInfo*>(lv_obj_get_user_data(slider));
    data->chainRef->adjustOctaverParam(data->param, lv_slider_get_value(slider));
}

static void radioEventHandler(lv_event_t * e)
{
    OctaverControlInfo * info = static_cast<OctaverControlInfo*>(lv_event_get_user_data(e));
    lv_obj_t * cont = (lv_obj_t *)lv_event_get_current_target(e);
    lv_obj_t * act_cb = lv_event_get_target_obj(e);
    lv_obj_t * old_cb = lv_obj_get_child(cont, info->param);

    if(act_cb == cont) return;

    lv_obj_remove_state(old_cb, LV_STATE_CHECKED);
    lv_obj_add_state(act_cb, LV_STATE_CHECKED);

    info->param = lv_obj_get_index(act_cb);
    info->chainRef->adjustOctaverParam(0, info->param);
}

static void backEventHandler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    lv_obj_t * menu = (lv_obj_t *)lv_event_get_user_data(e);

    if(lv_menu_back_button_is_root(menu, obj)) {}
}

void OctaverTab::handleOctaverPower(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        lv_obj_t * obj = lv_event_get_target_obj(e);
        _preChain->enableOctaver(lv_obj_has_state(obj, LV_STATE_CHECKED));
    }
}
