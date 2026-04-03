#pragma once

#include "lvgl/lvgl.h"
#include "env.h"
#include "platform/gpio_button.h"

class MiscTab
{
public:
    MiscTab(lv_obj_t *tabview, std::array<GPIOButton*,3> fs, std::vector<int32_t> cpuData = {}) : footswitches(fs)
    {
        _tab = lv_tabview_add_tab(tabview, "Misc");
        lv_obj_set_scroll_dir(_tab, LV_DIR_VER);

        createLayout();
        
        if (!cpuData.empty())
        {
            for (int i = 0; i < cpuData.size(); i++)
                lv_chart_set_next_value(_cpuChart, _cpuChartSeries, cpuData[i]);
        }
    }

    ~MiscTab()
    {
        lv_obj_delete(_tab);
    }

    void createLayout();
    void updateCpuChart();

    std::vector<int32_t> getCpuData();


    std::array<GPIOButton*,3> footswitches;

private:
    lv_obj_t *_tab;
    lv_obj_t *_cpuChart;
    lv_obj_t *_cpuLabel;
    lv_chart_series_t *_cpuChartSeries;
    lv_obj_t *_memoryLabel;

    lv_obj_t * _leftFsSelector;
    lv_obj_t * _midFsSelector;
    lv_obj_t * _rightFsSelector;


    int pickedPreset = 0;
};