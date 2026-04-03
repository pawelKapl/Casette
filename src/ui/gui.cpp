#include "gui.h"
#include <chrono>
#include <thread>
#include "lvgl/src/core/lv_global.h"
#include "lvgl_private.h"
#include "state.h"

GUI::~GUI()
{
    log_info << "Shutting down LVGL...";
    lv_evdev_delete(_inputDevice);
    lv_display_delete(_disp);
    lv_deinit();
}

auto last = std::chrono::steady_clock::now();

static lv_style_t general;
static lv_style_t style_slider_bg;
static lv_style_t style_slider_indicator;
static lv_style_t style_slider_knob;
static lv_style_t style_arc_bg;
static lv_style_t style_arc_indicator;
static lv_style_t style_arc_knob;
static lv_style_t style_switch_bg;
static lv_style_t style_switch_indicator;
static lv_style_t style_switch_knob;
static lv_style_t style_tab_btns;
static lv_style_t style_image;
static lv_style_t style_radiobutton_active;
static lv_style_t style_radiobutton_disabled;
static lv_style_t style_keyboard;
static lv_style_t style_dropdown;

static int32_t tuner_current_value = 0;
static int32_t tuner_target = 0;
static std::string current_note = "C";
static lv_obj_t * needle_line = NULL;
static lv_obj_t * tuner_value_label = NULL;
static lv_obj_t * scale = NULL;

typedef struct {
    lv_style_t items;
    lv_style_t indicator;
    lv_style_t main;
} section_styles_t;

static section_styles_t zone1_styles;
static section_styles_t zone2_styles;
static section_styles_t zone3_styles;
static section_styles_t zone4_styles;
static section_styles_t zone5_styles;

static lv_color_t get_hr_zone_color(int32_t reading)
{
    if (reading > -6 && reading < 6) 
        return lv_color_make(224, 26, 79);
    else 
        return lv_color_make(129, 14, 47);
}

static void hr_anim_timer_cb(lv_timer_t * timer)
{
    LV_UNUSED(timer);

    auto diff = tuner_target - tuner_current_value;
    if (diff)
    {
        if (diff > 0)
            tuner_current_value++;
        else
            tuner_current_value--;
    }

    lv_scale_set_line_needle_value(scale, needle_line, -8, tuner_current_value);
    lv_label_set_text(tuner_value_label, current_note.c_str());
    lv_color_t zone_color = get_hr_zone_color(tuner_current_value);
    lv_obj_set_style_text_color(tuner_value_label, zone_color, 0);
}

static void init_section_styles(section_styles_t * styles, lv_color_t color)
{
    lv_style_init(&styles->items);
    lv_style_set_line_color(&styles->items, color);
    lv_style_set_line_width(&styles->items, 0);

    lv_style_init(&styles->indicator);
    lv_style_set_line_color(&styles->indicator, color);
    lv_style_set_line_width(&styles->indicator, 0);

    lv_style_init(&styles->main);
    lv_style_set_arc_color(&styles->main, color);
    lv_style_set_arc_width(&styles->main, 20);
}

static void add_section(lv_obj_t * target_scale,
                        int32_t from,
                        int32_t to,
                        const section_styles_t * styles)
{
    lv_scale_section_t * sec = lv_scale_add_section(target_scale);
    lv_scale_set_section_range(target_scale, sec, from, to);
    lv_scale_set_section_style_items(target_scale, sec, &styles->items);
    lv_scale_set_section_style_indicator(target_scale, sec, &styles->indicator);
    lv_scale_set_section_style_main(target_scale, sec, &styles->main);
}

void scaleMeter(lv_obj_t * parent)
{
    scale = lv_scale_create(parent);
    lv_obj_center(scale);
    lv_obj_set_size(scale, 200, 200);

    lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_range(scale, -100, 100);
    lv_scale_set_total_tick_count(scale, 15);
    lv_scale_set_major_tick_every(scale, 3);
    lv_scale_set_angle_range(scale, 280);
    lv_scale_set_rotation(scale, 130);
    lv_scale_set_label_show(scale, false);

    lv_obj_set_style_length(scale, 6, LV_PART_ITEMS);
    lv_obj_set_style_length(scale, 10, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(scale, 0, LV_PART_MAIN);

    init_section_styles(&zone1_styles, lv_color_make(58, 6, 21));
    add_section(scale, -100, -22, &zone1_styles);

    init_section_styles(&zone2_styles, lv_color_make(129, 14, 47));
    add_section(scale, -22, -6, &zone2_styles);

    init_section_styles(&zone3_styles, lv_color_make(224, 26, 79));
    add_section(scale, -6, 6, &zone3_styles);

    init_section_styles(&zone4_styles, lv_color_make(129, 14, 47));
    add_section(scale, 6, 22, &zone4_styles);

    init_section_styles(&zone5_styles, lv_color_make(58, 6, 21));
    add_section(scale, 22, 100, &zone5_styles);

    needle_line = lv_line_create(scale);

    lv_obj_set_style_line_color(needle_line, lv_color_make(210, 210, 210), LV_PART_MAIN);
    lv_obj_set_style_line_width(needle_line, 6, LV_PART_MAIN);
    lv_obj_set_style_length(needle_line, 20, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(needle_line, false, LV_PART_MAIN);
    lv_obj_set_style_pad_right(needle_line, 50, LV_PART_MAIN);

    int32_t current_reading = 0;

    lv_scale_set_line_needle_value(scale, needle_line, 0, current_reading);

    lv_obj_t * circle = lv_obj_create(parent);
    lv_obj_set_size(circle, 130, 130);
    lv_obj_center(circle);

    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);

    lv_obj_set_style_bg_color(circle, lv_obj_get_style_bg_color(parent, LV_PART_MAIN), 0);
    lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(circle, 0, LV_PART_MAIN);

    lv_obj_t * hr_container = lv_obj_create(circle);
    lv_obj_center(hr_container);
    lv_obj_set_size(hr_container, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(hr_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(hr_container, 0, 0);
    lv_obj_set_layout(hr_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(hr_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(hr_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(hr_container, 0, 0);
    lv_obj_set_flex_align(hr_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    tuner_value_label = lv_label_create(hr_container);
    lv_label_set_text_fmt(tuner_value_label, "%d", current_reading);
    lv_obj_set_style_text_font(tuner_value_label, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_align(tuner_value_label, LV_TEXT_ALIGN_CENTER, 0);

    lv_color_t zone_color = get_hr_zone_color(current_reading);
    lv_obj_set_style_text_color(tuner_value_label, zone_color, 0);

    lv_timer_create(hr_anim_timer_cb, 10, NULL);
}

static void init_slider_styles()
{
    lv_style_init(&style_slider_bg);
    lv_style_set_bg_color(&style_slider_bg, lv_color_make(129, 14, 47)); // track background
    lv_style_set_height(&style_slider_bg, 8);

    lv_style_init(&style_slider_indicator);
    lv_style_set_bg_color(&style_slider_indicator, lv_color_make(224, 26, 79)); // filled bar
    lv_style_set_height(&style_slider_indicator, 8);

    lv_style_init(&style_slider_knob);
    lv_style_set_bg_color(&style_slider_knob, lv_color_make(224, 26, 79)); // knob color
    lv_style_set_radius(&style_slider_knob, 10);
}

static void init_arc_styles()
{
    lv_style_init(&style_arc_bg);
    lv_style_set_arc_color(&style_arc_bg, lv_color_make(129, 14, 47)); // track background
    lv_style_set_arc_width(&style_arc_bg, 8);

    lv_style_init(&style_arc_indicator);
    lv_style_set_arc_color(&style_arc_indicator, lv_color_make(224, 26, 79)); // filled bar
    lv_style_set_arc_width(&style_arc_indicator, 8);

    lv_style_init(&style_arc_knob);
    lv_style_set_bg_color(&style_arc_knob, lv_color_make(224, 26, 79)); // knob color
    lv_style_set_radius(&style_arc_knob, 10);
}

static void init_switch_styles()
{
    lv_style_init(&style_switch_bg);
    lv_style_set_bg_color(&style_switch_bg, lv_color_make(129, 14, 47)); // OFF state background

    lv_style_init(&style_switch_indicator);
    lv_style_set_bg_color(&style_switch_indicator, lv_color_make(224, 26, 79));

    lv_style_init(&style_switch_knob);
    lv_style_set_bg_color(&style_switch_knob, lv_color_make(210, 210, 210));
}

static void init_radiobutton_styles()
{
    lv_style_init(&style_radiobutton_active);
    lv_style_set_bg_color(&style_radiobutton_active, lv_color_make(224, 26, 79));
    lv_style_set_border_width(&style_radiobutton_active, 0);

    lv_style_init(&style_radiobutton_disabled);
    lv_style_set_bg_color(&style_radiobutton_disabled, lv_color_make(129, 14, 47));
    lv_style_set_border_width(&style_radiobutton_disabled, 0);
}

static void init_tab_button_styles()
{
    lv_style_init(&style_tab_btns);
    lv_style_set_bg_color(&style_tab_btns, lv_color_make(129, 14, 47));
    lv_style_set_border_color(&style_tab_btns, lv_color_make(224, 26, 79));
    lv_style_set_radius(&style_tab_btns, 5);
}

static void init_keyboard_styles()
{
    lv_style_init(&style_keyboard);
    lv_style_set_bg_color(&style_keyboard, lv_color_make(0, 0, 0));
    lv_style_set_text_color(&style_keyboard, lv_color_make(210, 210, 210));
}

static void init_dropdown_styles()
{
    lv_style_init(&style_dropdown);
    lv_style_set_bg_color(&style_dropdown, lv_color_make(129, 14, 47)); // track background
}

static void new_theme_apply_cb(lv_theme_t *th, lv_obj_t *obj)
{
    LV_UNUSED(th);

    lv_obj_add_style(obj, &general, 0);

    if (lv_obj_check_type(obj, &lv_slider_class))
    {
        lv_obj_add_style(obj, &style_slider_bg, LV_PART_MAIN);
        lv_obj_add_style(obj, &style_slider_indicator, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &style_slider_knob, LV_PART_KNOB);
    }
    else if (lv_obj_check_type(obj, &lv_arc_class))
    {
        lv_obj_add_style(obj, &style_arc_bg, LV_PART_MAIN);
        lv_obj_add_style(obj, &style_arc_indicator, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &style_arc_knob, LV_PART_KNOB);
    }
    else if (lv_obj_check_type(obj, &lv_switch_class))
    {
        lv_obj_add_style(obj, &style_switch_bg, LV_PART_MAIN);
        lv_obj_add_style(obj, &style_switch_bg, LV_PART_INDICATOR | LV_STATE_DISABLED);
        lv_obj_add_style(obj, &style_switch_indicator, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &style_switch_knob, LV_PART_KNOB);
        lv_obj_add_style(obj, &style_switch_knob, LV_PART_KNOB | LV_STATE_CHECKED);
    }
    else if (lv_obj_check_type(obj, &lv_button_class))
    {
        lv_obj_add_style(obj, &style_tab_btns, LV_PART_MAIN | LV_STATE_CHECKED);
    }
    else if (lv_obj_check_type(obj, &lv_checkbox_class))
    {
        lv_obj_add_style(obj, &style_radiobutton_active, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &style_radiobutton_disabled, LV_PART_INDICATOR);
    }
    else if (lv_obj_check_type(obj, &lv_keyboard_class))
    {
        lv_obj_add_style(obj, &style_keyboard, LV_PART_ITEMS);
        lv_obj_add_style(obj, &style_keyboard, LV_PART_ITEMS | LV_STATE_CHECKED);
    }
    else if (lv_obj_check_type(obj, &lv_dropdownlist_class))
    {
        lv_obj_add_style(obj, &style_dropdown, LV_PART_SELECTED | LV_STATE_CHECKED);
    }
}

static void new_theme_init_and_set(void)
{
    lv_style_init(&general);
    lv_style_set_bg_color(&general, lv_color_make(0, 0, 0));
    lv_style_set_text_color(&general, lv_color_make(210, 210, 210));
    lv_style_set_text_font(&general, &lv_font_montserrat_20);
    lv_style_set_border_color(&general, lv_color_make(30, 30, 30));

    init_slider_styles();
    init_arc_styles();
    init_switch_styles();
    init_tab_button_styles();
    init_radiobutton_styles();
    init_keyboard_styles();
    init_dropdown_styles();

    /*Initialize the new theme from the current theme*/
    lv_theme_t *th_act = lv_display_get_theme(NULL);
    static lv_theme_t th_new = *th_act;

    /*Set the parent theme and the style apply callback for the new theme*/
    lv_theme_set_parent(&th_new, th_act);
    lv_theme_set_apply_cb(&th_new, new_theme_apply_cb);

    /*Assign the new theme to the current display*/
    lv_display_set_theme(NULL, &th_new);
}

void GUI::init()
{
    lv_init();

    _disp = lv_linux_fbdev_create();
    lv_linux_fbdev_set_file(_disp, "/dev/fb0");

    _inputDevice = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event0");
    if (!_inputDevice)
        log_error << "Cant configure touch screen!";

    _mainScreen = lv_disp_get_scr_act(_disp);
    _bypassScreen = lv_obj_create(NULL);

    new_theme_init_and_set();

    lv_obj_set_style_bg_color(_bypassScreen, lv_color_make(0, 0, 0), LV_PART_MAIN);

    scaleMeter(_bypassScreen);

    createTabMenu();
}

void GUI::bypassMode(bool active)
{
    _bypassMode = active;
    if (active)
    {
        lv_screen_load(_bypassScreen);
    }
    else
    {
        lv_screen_load(_mainScreen);
    }
}

void GUI::update()
{
    if (!_metersUpdateTimer)
    {
        _ampTab->updateLevelMeters();
        if (_bypassMode)
        {
            auto tunerReading = _tuner->read();
            current_note = tunerReading.first;
            tuner_target = tunerReading.second;
        }
    }

    if (!_cpuChartUpdateTimer)
        _miscTab->updateCpuChart();

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();

    lv_tick_inc(elapsed);
    lv_timer_handler();
    last = now;

    _metersUpdateTimer++;
    if (_metersUpdateTimer > 3)
        _metersUpdateTimer = 0;

    _cpuChartUpdateTimer++;
    if (_cpuChartUpdateTimer > 200)
    {
        _cpuChartUpdateTimer = 0;
    }

    auto &state = State::get();
    if (state.requireReload)
    {
        state.ackReload();
        reload();
    }
}

void GUI::createTabMenu()
{
    uint32_t tab_count = 0;
    uint32_t i = 0;

    _tabview = lv_tabview_create(lv_screen_active());

    lv_tabview_set_tab_bar_position(_tabview, LV_DIR_LEFT);
    lv_tabview_set_tab_bar_size(_tabview, 80);

    lv_obj_set_style_bg_color(_tabview, lv_color_make(224, 26, 79), 0);

    lv_obj_t *tab_buttons = lv_tabview_get_tab_bar(_tabview);
    lv_obj_set_style_bg_color(tab_buttons, lv_color_make(0, 0, 0), 0);

    _ampTab = std::make_unique<AmpTab>(_tabview, _amp);
    _cabTab = std::make_unique<CabTab>(_tabview, _cabSimulator);
    _ngTab = std::make_unique<NGTab>(_tabview, _noiseGate);
    _modTab = std::make_unique<ModTab>(_tabview, _fxChain);
    _delayTab = std::make_unique<DelayTab>(_tabview, _fxChain);
    _reverbTab = std::make_unique<ReverbTab>(_tabview, _fxChain);

    std::vector<int32_t> cpuData = {};
    if (_miscTab)
        cpuData = _miscTab->getCpuData();
    _miscTab = std::make_unique<MiscTab>(_tabview, std::array<GPIOButton *, 3>{_footswitches->operator[](0).get(),
                                                                               _footswitches->operator[](1).get(),
                                                                               _footswitches->operator[](2).get()}, cpuData);

    tab_count = lv_tabview_get_tab_count(_tabview);
    for (i = 0; i < tab_count; i++)
    {
        lv_obj_t *button = lv_obj_get_child(tab_buttons, i);
        lv_obj_set_style_border_side(button, LV_BORDER_SIDE_RIGHT, LV_PART_MAIN | LV_STATE_CHECKED);
    }

    lv_obj_remove_flag(lv_tabview_get_content(_tabview), LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL);
}

void GUI::reload()
{
    _amp->reload();
    _cabSimulator->reload();
    _fxChain->reload();
    _noiseGate->reload();
    _mainScreen = lv_obj_create(NULL);
    lv_screen_load(_mainScreen);
    createTabMenu();
}