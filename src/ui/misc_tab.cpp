#include "misc_tab.h"
#include "ui_helpers.h"
#include "storage.h"

static void backEventHandler(lv_event_t * e);
static void ta_event_cb(lv_event_t * e);
static void onPresetPicked(lv_event_t *e);
static void onLeftFsAssigned(lv_event_t *e);
static void onMidFsAssigned(lv_event_t *e);
static void onRightFsAssigned(lv_event_t *e);
static void reloadPresetList(std::string picked, lv_obj_t *selector);
static void draw_event_cb(lv_event_t * e);


static lv_obj_t * kb;
static lv_obj_t * menu;
static lv_obj_t * root_page;
static lv_obj_t * presetSelector;
static lv_obj_t * leftFsSelector;
static lv_obj_t * midFsSelector;
static lv_obj_t * rightFsSelector;

static void add_faded_area(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    lv_area_t coords;
    lv_obj_get_coords(obj, &coords);

    lv_draw_task_t * draw_task = lv_event_get_draw_task(e);
    lv_draw_dsc_base_t * base_dsc = (lv_draw_dsc_base_t *)lv_draw_task_get_draw_dsc(draw_task);

    const lv_chart_series_t * ser = lv_chart_get_series_next(obj, NULL);
    lv_color_t ser_color = lv_chart_get_series_color(obj, ser);

    /*Draw a triangle below the line witch some opacity gradient*/
    lv_draw_line_dsc_t * draw_line_dsc = (lv_draw_line_dsc_t *)lv_draw_task_get_draw_dsc(draw_task);
    lv_draw_triangle_dsc_t tri_dsc;

    lv_draw_triangle_dsc_init(&tri_dsc);
    tri_dsc.p[0].x = draw_line_dsc->p1.x;
    tri_dsc.p[0].y = draw_line_dsc->p1.y;
    tri_dsc.p[1].x = draw_line_dsc->p2.x;
    tri_dsc.p[1].y = draw_line_dsc->p2.y;
    tri_dsc.p[2].x = draw_line_dsc->p1.y < draw_line_dsc->p2.y ? draw_line_dsc->p1.x : draw_line_dsc->p2.x;
    tri_dsc.p[2].y = LV_MAX(draw_line_dsc->p1.y, draw_line_dsc->p2.y);
    tri_dsc.grad.dir = LV_GRAD_DIR_VER;

    int32_t full_h = lv_obj_get_height(obj);
    int32_t fract_uppter = (int32_t)(LV_MIN(draw_line_dsc->p1.y, draw_line_dsc->p2.y) - coords.y1) * 255 / full_h;
    int32_t fract_lower = (int32_t)(LV_MAX(draw_line_dsc->p1.y, draw_line_dsc->p2.y) - coords.y1) * 255 / full_h;
    tri_dsc.grad.stops[0].color = ser_color;
    tri_dsc.grad.stops[0].opa = (lv_opa_t)(255 - fract_uppter);
    tri_dsc.grad.stops[0].opa = 255 - fract_uppter;
    tri_dsc.grad.stops[0].frac = 0;
    tri_dsc.grad.stops[1].color = ser_color;
    tri_dsc.grad.stops[1].opa = (lv_opa_t)(255 - fract_lower);
    tri_dsc.grad.stops[1].frac = 255;

    lv_draw_triangle(base_dsc->layer, &tri_dsc);

    /*Draw rectangle below the triangle*/
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_grad.dir = LV_GRAD_DIR_VER;
    rect_dsc.bg_grad.stops[0].color = ser_color;
    rect_dsc.bg_grad.stops[0].frac = 0;
    rect_dsc.bg_grad.stops[0].opa = (lv_opa_t)(255 - fract_lower);
    rect_dsc.bg_grad.stops[1].color = ser_color;
    rect_dsc.bg_grad.stops[1].frac = 255;
    rect_dsc.bg_grad.stops[1].opa = 0;

    lv_area_t rect_area;
    rect_area.x1 = (int32_t)draw_line_dsc->p1.x;
    rect_area.x2 = (int32_t)draw_line_dsc->p2.x - 1;
    rect_area.y1 = (int32_t)LV_MAX(draw_line_dsc->p1.y, draw_line_dsc->p2.y) - 1;
    rect_area.y2 = (int32_t)coords.y2;
    lv_draw_rect(base_dsc->layer, &rect_dsc, &rect_area);
}

void MiscTab::createLayout()
{
    menu = lv_menu_create(_tab);

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

    lv_obj_t *create_preset_sub_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(create_preset_sub_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    
    lv_obj_t * oneline_label = lv_label_create(create_preset_sub_page);
    lv_label_set_text(oneline_label, "Enter preset name:");

    /*Create the one-line mode text area*/
    lv_obj_t * text_ta = lv_textarea_create(create_preset_sub_page);
    lv_textarea_set_one_line(text_ta, true);
    lv_textarea_set_password_mode(text_ta, false);
    lv_obj_set_width(text_ta, lv_pct(40));
    lv_obj_add_event_cb(text_ta, ta_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_align(text_ta, LV_ALIGN_TOP_RIGHT, -5, 20);
    create_text(create_preset_sub_page, NULL, "", LV_MENU_ITEM_BUILDER_VARIANT_1);

    /*Create a keyboard*/
    kb = lv_keyboard_create(create_preset_sub_page);
    lv_obj_set_size(kb,  LV_HOR_RES - 90, LV_VER_RES / 2);

    lv_keyboard_set_textarea(kb, text_ta);

    lv_obj_t *assign_fs_sub_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(assign_fs_sub_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    cont = create_text(assign_fs_sub_page, LV_SYMBOL_EDIT, "Assign left switch: ", LV_MENU_ITEM_BUILDER_VARIANT_1);
    leftFsSelector = lv_dropdown_create(cont);
    lv_obj_set_style_text_color(leftFsSelector, lv_color_make(224, 26, 79), LV_PART_INDICATOR);
    lv_obj_add_flag(leftFsSelector, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    lv_obj_set_flex_grow(leftFsSelector, 1);
    lv_obj_align(leftFsSelector, LV_ALIGN_BOTTOM_LEFT, 0, 20);
    lv_obj_add_event_cb(leftFsSelector, onLeftFsAssigned, LV_EVENT_ALL, this);
    reloadPresetList(State::get().leftFsAssignement, leftFsSelector);

    cont = create_text(assign_fs_sub_page, LV_SYMBOL_EDIT, "Assign middle switch: ", LV_MENU_ITEM_BUILDER_VARIANT_1);
    midFsSelector = lv_dropdown_create(cont);
    lv_obj_set_style_text_color(midFsSelector, lv_color_make(224, 26, 79), LV_PART_INDICATOR);
    lv_obj_add_flag(midFsSelector, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    lv_obj_set_flex_grow(midFsSelector, 1);
    lv_obj_align(midFsSelector, LV_ALIGN_BOTTOM_LEFT, 0, 20);
    lv_obj_add_event_cb(midFsSelector, onMidFsAssigned, LV_EVENT_ALL, this);
    reloadPresetList(State::get().midFsAssignement, midFsSelector);

    cont = create_text(assign_fs_sub_page, LV_SYMBOL_EDIT, "Assign right switch: ", LV_MENU_ITEM_BUILDER_VARIANT_1);
    rightFsSelector = lv_dropdown_create(cont);
    lv_obj_set_style_text_color(rightFsSelector, lv_color_make(224, 26, 79), LV_PART_INDICATOR);
    lv_obj_add_flag(rightFsSelector, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    lv_obj_set_flex_grow(rightFsSelector, 1);
    lv_obj_align(rightFsSelector, LV_ALIGN_BOTTOM_LEFT, 0, 20);
    lv_obj_add_event_cb(rightFsSelector, onRightFsAssigned, LV_EVENT_ALL, this);
    reloadPresetList(State::get().rightFsAssignement, rightFsSelector);


    /*Create a root page*/
    root_page = lv_menu_page_create(menu, "Misc");
    lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    section = lv_menu_section_create(root_page);

    cont = create_text(section, LV_SYMBOL_AUDIO, "Select preset: ", LV_MENU_ITEM_BUILDER_VARIANT_1);
    presetSelector = lv_dropdown_create(cont);
    lv_obj_set_style_text_color(presetSelector, lv_color_make(224, 26, 79), LV_PART_INDICATOR);
    lv_obj_add_flag(presetSelector, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    lv_obj_set_flex_grow(presetSelector, 1);
    lv_obj_align(presetSelector, LV_ALIGN_BOTTOM_LEFT, 0, 20);
    lv_obj_add_event_cb(presetSelector, onPresetPicked, LV_EVENT_ALL, this);
    reloadPresetList(State::get().activePreset, presetSelector);

    cont = create_text(section, LV_SYMBOL_SAVE, "Create New Preset", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(menu, cont, create_preset_sub_page);

    cont = create_text(section, LV_SYMBOL_EDIT, "Assign footswitches", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(menu, cont, assign_fs_sub_page);


    section = lv_menu_section_create(root_page);
    cont = create_text(section, NULL, "\nCpu load: 0%", LV_MENU_ITEM_BUILDER_VARIANT_1);
    _cpuLabel = lv_obj_get_child(cont, 0);

    /*Create a chart*/
    _cpuChart = lv_chart_create(section);
    lv_chart_set_type(_cpuChart, LV_CHART_TYPE_LINE);
    lv_obj_set_size(_cpuChart, lv_display_get_horizontal_resolution(NULL) - 120, 120);
    lv_obj_set_style_pad_all(_cpuChart, 0, 0);
    lv_obj_set_style_border_width(_cpuChart, 0, LV_PART_MAIN);
    lv_obj_set_style_line_color(_cpuChart, lv_color_make(20, 20, 20), LV_PART_MAIN);
    lv_obj_set_style_radius(_cpuChart, 0, 0);
    lv_obj_center(_cpuChart);

    lv_chart_set_div_line_count(_cpuChart, 3, 0);
    lv_chart_set_point_count(_cpuChart, 200);
    lv_obj_set_style_size(_cpuChart, 0, 0, LV_PART_INDICATOR);

    lv_obj_add_event_cb(_cpuChart, draw_event_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
    lv_obj_add_flag(_cpuChart, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

    _cpuChartSeries = lv_chart_add_series(_cpuChart, lv_color_make(224, 26, 79), LV_CHART_AXIS_PRIMARY_Y);

    cont = create_text(section, NULL, "Memory utilization: 0MB", LV_MENU_ITEM_BUILDER_VARIANT_1);
    _memoryLabel = lv_obj_get_child(cont, 0);

    lv_menu_set_sidebar_page(menu, NULL);
    lv_menu_set_page(menu, root_page);
}

static void backEventHandler(lv_event_t * e)
{
}

static void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target_obj(e);
    if(code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        if(kb != NULL) lv_keyboard_set_textarea(kb, ta);
    }

    else if(code == LV_EVENT_READY) {
        std::string newPresetName = lv_textarea_get_text(ta);
        // todo filtering

        if (!newPresetName.empty())
            std::filesystem::create_directory("../storage/" + newPresetName);

        lv_textarea_set_text(ta, "");
        lv_menu_set_page(menu, root_page);

        reloadPresetList(newPresetName, presetSelector);
        State::get().activePreset = newPresetName;
        Storage::get().persistActivePreset();
    }
}

static void reloadPresetList(std::string picked, lv_obj_t *selector)
{
    auto presetsPath = std::string("../storage/");

    std::string presets;
    int index = 0;
    int pickedOne = 0;
    for (const auto &object : std::filesystem::directory_iterator(presetsPath))
    {
        if (object.is_directory())
        {
            std::string dirName = object.path().stem();
            if (dirName == picked)
                pickedOne = index;
            presets += dirName;
            presets += "\n";
            index++;
        }
    }
    if (!presets.empty()) presets.pop_back();

    lv_dropdown_set_options(selector, presets.c_str());
    lv_dropdown_set_selected(selector, pickedOne, NULL);
}

static void onPresetPicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target_obj(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
        State::get().changePreset(buf);
        Storage::get().persistActivePreset();
    }
}

void onLeftFsAssigned(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target_obj(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
        State::get().leftFsAssignement = buf;
        Storage::get().persistFsAssignement();
        auto *self = static_cast<MiscTab *>(lv_event_get_user_data(e));
        self->footswitches[0]->setShortPressCallback([]() {
            State::get().changePreset(State::get().leftFsAssignement);
            Storage::get().persistActivePreset();
        });
    }
}

void onMidFsAssigned(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target_obj(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
        State::get().midFsAssignement = buf;
        Storage::get().persistFsAssignement();
        auto *self = static_cast<MiscTab *>(lv_event_get_user_data(e));
        self->footswitches[1]->setShortPressCallback([]() {
            State::get().changePreset(State::get().midFsAssignement);
            Storage::get().persistActivePreset();
        });
    }
}

void onRightFsAssigned(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target_obj(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
        State::get().rightFsAssignement = buf;
        Storage::get().persistFsAssignement();
        auto *self = static_cast<MiscTab *>(lv_event_get_user_data(e));
        self->footswitches[2]->setShortPressCallback([]() {
            State::get().changePreset(State::get().rightFsAssignement);
            Storage::get().persistActivePreset();
        });
    }
}

static void draw_event_cb(lv_event_t * e)
{
    lv_draw_task_t * draw_task = lv_event_get_draw_task(e);
    lv_draw_dsc_base_t * base_dsc = (lv_draw_dsc_base_t *)lv_draw_task_get_draw_dsc(draw_task);

    if(base_dsc->part == LV_PART_ITEMS && lv_draw_task_get_type(draw_task) == LV_DRAW_TASK_TYPE_LINE) {
        add_faded_area(e);
    }
}

long get_memory_usage_mb() {
    std::ifstream file("/proc/self/status");
    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("VmRSS:", 0) == 0) {
            long mem_kb;
            sscanf(line.c_str(), "VmRSS: %ld kB", &mem_kb);
            return mem_kb / 1024.0;
        }
    }
    return -1;
}

std::vector<int32_t> MiscTab::getCpuData()
{
    std::vector<int32_t> dataY;
    uint16_t pointCount = lv_chart_get_point_count(_cpuChart);

    auto ySeries = lv_chart_get_series_y_array(_cpuChart, _cpuChartSeries);
    for (uint16_t i = 0; i < pointCount; i++) 
    {
        if (ySeries[i] < 101)
            dataY.push_back(ySeries[i]);
    }

    return dataY;
}

void MiscTab::updateCpuChart()
{
    auto cpuState = _cpuMonitor->getCpuState();
    auto cpuUsage = cpuState[0];

    std::string cpuLabel =  "\nCpu temperature: " + std::to_string(int(cpuState[1])) + "°C \nCpu load: " + std::to_string(int(cpuUsage)) + "%";

    lv_label_set_text(_cpuLabel, cpuLabel.c_str());

    if (cpuUsage > 100)
        cpuUsage = 100;
    lv_chart_set_next_value(_cpuChart, _cpuChartSeries, (int32_t)(cpuUsage));

    std::string memoryLabel = "Memory utilization: " + std::to_string(int(get_memory_usage_mb())) + "MB";
    lv_label_set_text(_memoryLabel, memoryLabel.c_str());
}