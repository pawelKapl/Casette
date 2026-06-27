#include "amp_tab.h"
#include "ui_helpers.h"
#include "storage.h"

static void backEventHandler(lv_event_t *e);
static void onAmpButtonPowerClicked(lv_event_t *e);
static void onSubAmpButtonPowerClicked(lv_event_t *e);
static void onAmpModelPicked(lv_event_t *e);
static void onSubAmpModelPicked(lv_event_t *e);
static void onAmpSliderMoved(lv_event_t *e);
static void onTonestackKnobMoved(lv_event_t *e);
static void onTonestackPowerClicked(lv_event_t *e);


lv_obj_t * AmpTab::levelMeter(lv_obj_t * parent, const char * title)
{
    static lv_style_t style_indic;

    lv_style_init(&style_indic);
    lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indic, lv_color_make(20, 20, 20));
    lv_style_set_bg_grad_color(&style_indic, lv_color_make(224, 26, 79));
    lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_HOR);

    lv_obj_t * obj = create_text(parent, title);

    lv_obj_t * inputMeter = lv_bar_create(obj);
    lv_obj_add_style(inputMeter, &style_indic, LV_PART_INDICATOR);
    lv_obj_set_height(inputMeter, 7);
    lv_obj_set_flex_grow(inputMeter, 5);
    lv_bar_set_range(inputMeter, 0, 100);
    lv_bar_set_orientation(inputMeter, LV_BAR_ORIENTATION_HORIZONTAL);
    lv_obj_align(inputMeter, LV_ALIGN_TOP_RIGHT, 0, 0);

    return inputMeter;
}

void AmpTab::createLayout()
{
    lv_obj_t *menu = lv_menu_create(_tab);
    auto ampSettings = _amp->ampSettings();

    lv_color_t bg_color = lv_obj_get_style_bg_color(menu, 0);
    if (lv_color_brightness(bg_color) > 127)
    {
        lv_obj_set_style_bg_color(menu, lv_color_darken(lv_obj_get_style_bg_color(menu, 0), 10), 0);
    }
    else
    {
        lv_obj_set_style_bg_color(menu, lv_color_darken(lv_obj_get_style_bg_color(menu, 0), 50), 0);
    }
    lv_menu_set_mode_root_back_button(menu, LV_MENU_ROOT_BACK_BUTTON_ENABLED);
    lv_obj_add_event_cb(menu, backEventHandler, LV_EVENT_CLICKED, menu);
    lv_obj_set_size(menu, lv_display_get_horizontal_resolution(NULL) - 80, lv_display_get_vertical_resolution(NULL));
    lv_obj_center(menu);
    resize_back_button(menu);

    lv_obj_t *cont;
    lv_obj_t *section;

    lv_obj_t *root_page = lv_menu_page_create(menu, "Amp Settings");
    lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    section = lv_menu_section_create(root_page);

    cont = create_switch(section, LV_SYMBOL_POWER, "Main Amp Enabled", ampSettings->ampEnabled);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onAmpButtonPowerClicked, LV_EVENT_VALUE_CHANGED, this);

    auto modelsPath = std::string("../models/");

    std::string models;
    int index = 0;
    int pickedOne = 0;
    int pickedSubOne = 0;
    for (const auto &file : std::filesystem::directory_iterator(modelsPath))
    {
        std::string filename = file.path().stem();
        if (filename == ampSettings->model)
            pickedOne = index;
        if (filename == ampSettings->subModel)
            pickedSubOne = index;
        models += filename;
        models += "\n";
        index++;
    }
    if (!models.empty())
        models.pop_back();

    cont = lv_menu_cont_create(section);
    lv_obj_t *ampSelector = lv_dropdown_create(cont);
    lv_obj_set_flex_grow(ampSelector, 1);
    lv_obj_set_style_text_color(ampSelector, lv_color_make(224, 26, 79), LV_PART_INDICATOR);

    lv_dropdown_set_options(ampSelector, models.c_str());
    lv_dropdown_set_selected(ampSelector, pickedOne, NULL);

    lv_obj_add_flag(ampSelector, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    lv_obj_set_flex_grow(ampSelector, 1);
    lv_obj_align(ampSelector, LV_ALIGN_BOTTOM_LEFT, 0, 20);
    lv_obj_add_event_cb(ampSelector, onAmpModelPicked, LV_EVENT_ALL, this);

    section = lv_menu_section_create(root_page);
    lv_obj_set_style_margin_top(section, 20, 0);

    cont = create_switch(section, LV_SYMBOL_POWER, "Front Amp Enabled", ampSettings->subAmpEnabled);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onSubAmpButtonPowerClicked, LV_EVENT_VALUE_CHANGED, this);

    lv_obj_t *subAmpSelector = lv_dropdown_create(cont);
    lv_obj_set_flex_grow(subAmpSelector, 1);
    lv_obj_set_style_text_color(subAmpSelector, lv_color_make(224, 26, 79), LV_PART_INDICATOR);

    lv_dropdown_set_options(subAmpSelector, models.c_str());
    lv_dropdown_set_selected(subAmpSelector, pickedSubOne, NULL);

    lv_obj_add_flag(subAmpSelector, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    lv_obj_set_flex_grow(subAmpSelector, 1);
    lv_obj_align(subAmpSelector, LV_ALIGN_BOTTOM_LEFT, 0, 30);
    lv_obj_add_event_cb(subAmpSelector, onSubAmpModelPicked, LV_EVENT_ALL, this);

    char buf[8];

    auto subMode = ampSettings->subMode;
    int subModePos = subMode == "PARALLEL" ? 0 : 1;
    cont = create_slider(section, LV_SYMBOL_EDIT, "Front Amp Wiring Mode", 0, 1, subModePos, subMode.c_str());
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_subModeInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onAmpSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    auto subBlend = ampSettings->subBlend;
    int subBlendPos = subBlend * 100;
    lv_snprintf(buf, sizeof(buf), "%d%%", subBlendPos);
    cont = create_slider(section, LV_SYMBOL_EDIT, "Front Amp Parallel Blend", 0, 100, subBlendPos, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_subBlendInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onAmpSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);


    section = lv_menu_section_create(root_page);
    lv_obj_set_style_margin_top(section, 30, 0);

    float inputGain = ampSettings->inputGain;
    int inputGainPos = dbToPosition(inputGain) * 100;
    inputGain = std::round(inputGain * 10.0f) / 10.0f;
    std::snprintf(buf, sizeof(buf), "%+.1fdB", inputGain);
    cont = create_slider(section, LV_SYMBOL_VOLUME_MID, "Input Gain", 0, 100, inputGainPos, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_inputGainInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onAmpSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    float mvGain = ampSettings->masterVolumeGain;
    int mvGainPos = dbToPosition(mvGain) * 100;
    mvGain = std::round(mvGain * 10.0f) / 10.0f;
    std::snprintf(buf, sizeof(buf), "%+.1fdB", mvGain);
    cont = create_slider(section, LV_SYMBOL_VOLUME_MID, "Master Volume", 0, 100, mvGainPos, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 2), &_mvInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onAmpSliderMoved, LV_EVENT_VALUE_CHANGED, NULL);

    _inputMeter = levelMeter(section, "Input");
    _outputMeter = levelMeter(section, "Output");

    section = lv_menu_section_create(root_page);
    lv_obj_set_style_margin_top(section, 20, 0);

    cont = create_switch(section, LV_SYMBOL_POWER, "Tonestack Enabled", ampSettings->tonestackEnabled);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), onTonestackPowerClicked, LV_EVENT_VALUE_CHANGED, this);

    static lv_style_t noFrame;
    lv_style_init(&noFrame);
    lv_style_set_border_width(&noFrame, 0);

    lv_obj_t *knobRow = lv_obj_create(section);
    lv_obj_add_style(knobRow, &noFrame, 0);
    lv_obj_set_size(knobRow, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(knobRow, LV_FLEX_FLOW_ROW);

    auto bass = ampSettings->getBassValue();
    std::snprintf(buf, sizeof(buf), "%.1f", bass);
    cont = create_knob(knobRow, "Bass", 0, 100, bass * 10, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 0), &_bassInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 0), onTonestackKnobMoved, LV_EVENT_VALUE_CHANGED, NULL);

    auto middle = ampSettings->getMiddleValue();
    std::snprintf(buf, sizeof(buf), "%.1f", middle);
    cont = create_knob(knobRow, "Middle", 0, 100, middle * 10, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 0), &_middleInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 0), onTonestackKnobMoved, LV_EVENT_VALUE_CHANGED, NULL);

    auto treble = ampSettings->getTrebleValue();
    std::snprintf(buf, sizeof(buf), "%.1f", treble);
    cont = create_knob(knobRow, "Treble", 0, 100, treble * 10, buf);
    lv_obj_set_user_data(lv_obj_get_child(cont, 0), &_trebleInfo);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 0), onTonestackKnobMoved, LV_EVENT_VALUE_CHANGED, NULL);

    lv_menu_set_sidebar_page(menu, NULL);
    lv_menu_set_page(menu, root_page);
}

static void onAmpButtonPowerClicked(lv_event_t *e)
{
    auto *self = static_cast<AmpTab *>(lv_event_get_user_data(e));
    self->handleAmpPower(e);
}

static void onSubAmpButtonPowerClicked(lv_event_t *e)
{
    auto *self = static_cast<AmpTab *>(lv_event_get_user_data(e));
    self->handleSubAmpPower(e);
}

static void onTonestackPowerClicked(lv_event_t *e)
{
    auto *self = static_cast<AmpTab *>(lv_event_get_user_data(e));
    self->handleTonestackPower(e);
}

static void backEventHandler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target_obj(e);
    lv_obj_t *menu = (lv_obj_t *)lv_event_get_user_data(e);

    if (lv_menu_back_button_is_root(menu, obj))
    {
    }
}

void AmpTab::handleAmpPower(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        lv_obj_t *obj = lv_event_get_target_obj(e);
        _amp->enabled(lv_obj_has_state(obj, LV_STATE_CHECKED));
    }
}

void AmpTab::handleSubAmpPower(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        lv_obj_t *obj = lv_event_get_target_obj(e);
        _amp->subEnabled(lv_obj_has_state(obj, LV_STATE_CHECKED));
    }
}

void AmpTab::handleTonestackPower(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        lv_obj_t *obj = lv_event_get_target_obj(e);
        _amp->tonestackEnabled(lv_obj_has_state(obj, LV_STATE_CHECKED));
    }
}

void AmpTab::changeAmpModel(const char *newModel)
{
    log_info << "Loading Amp: " << newModel;
    _amp->loadModel(newModel);
}

void AmpTab::changeSubAmpModel(const char *newModel)
{
    log_info << "Loading Sub Amp: " << newModel;
    _amp->loadSubModel(newModel);
}

float AmpTab::getInputLevel()
{
    auto inputRMS = _amp->getInputRMS();
    float levelDb = 20.0f * log10(inputRMS + 1e-6f);
    levelDb = std::clamp(levelDb, -60.0f, 0.0f);
    float normalized = ((levelDb + 60.0f) / 60.0f) * 100.0f;
    float shaped = std::pow(normalized, 4) / 1000000.0f;
    return shaped;
}

float AmpTab::getOutputLevel()
{
    auto outputRMS = _amp->getOutputRMS();
    float levelDb = 20.0f * log10(outputRMS + 1e-6f);
    levelDb = std::clamp(levelDb, -60.0f, 0.0f);
    float normalized = ((levelDb + 60.0f) / 60.0f) * 100.0f;
    float shaped = std::pow(normalized, 4) / 1000000.0f;
    return shaped;
}

void AmpTab::updateLevelMeters()
{
    lv_bar_set_value(_inputMeter, getInputLevel(), LV_ANIM_OFF);
    lv_bar_set_value(_outputMeter, getOutputLevel(), LV_ANIM_OFF);
}

static void onAmpModelPicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target_obj(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        char buf[64];
        lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
        auto *self = static_cast<AmpTab *>(lv_event_get_user_data(e));
        self->changeAmpModel(buf);
    }
}

static void onSubAmpModelPicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target_obj(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        char buf[64];
        lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
        auto *self = static_cast<AmpTab *>(lv_event_get_user_data(e));
        self->changeSubAmpModel(buf);
    }
}

static void onAmpSliderMoved(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target_obj(e);
    AmpControlInfo *data = static_cast<AmpControlInfo *>(lv_obj_get_user_data(slider));
    if (data->param == 1)
        data->ampRef->setSubBlend((float)lv_slider_get_value(slider) / 100.0f);
    else if (data->param == 2)
        data->ampRef->setInputGain(positionToDb((float)lv_slider_get_value(slider) / 100.0f, -50.0f, 12.0f));
    else if (data->param == 3)
        data->ampRef->setMVGain(positionToDb((float)lv_slider_get_value(slider) / 100.0f, -50.0f, 12.0f));
    else if (data->param == 7)
    {
        auto mode = lv_slider_get_value(slider) == 0 ? "PARALLEL" : "SERIES";
        data->ampRef->setSubMode(mode);
        lv_obj_t * parent = lv_obj_get_parent(slider);
        lv_obj_t * label = lv_obj_get_child(parent, 3);
        lv_label_set_text(label, mode);
        lv_obj_align_to(label, parent, LV_ALIGN_CENTER, 0, 0);
    }
}

static void onTonestackKnobMoved(lv_event_t *e)
{
    lv_obj_t *knob = lv_event_get_target_obj(e);
    AmpControlInfo *data = static_cast<AmpControlInfo *>(lv_obj_get_user_data(knob));
    float value = (float)lv_arc_get_value(knob) / 10.0f;

    if (data->param == 4)
        data->ampRef->setBass(value);
    else if (data->param == 5)
        data->ampRef->setMiddle(value);
    else if (data->param == 6)
        data->ampRef->setTreble(value);
}
