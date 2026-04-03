#pragma once

#include "lvgl/lvgl.h"
#include "env.h"
#include "dsp/noise_gate.h"


class NGTab
{
public:
    NGTab(lv_obj_t *tabview, NoiseGate *ng) : _noiseGate(ng)
    {
        _tab = lv_tabview_add_tab(tabview, "Noise\n Gate");
        lv_obj_set_scroll_dir(_tab, LV_DIR_VER);

        createLayout();
    }

    ~NGTab()
    {
        lv_obj_delete(_tab);
    }

    void handleNGPower(lv_event_t * e);
    void handleThresholdChange(float threshold);
    void createLayout();

    private:
        lv_obj_t * _tab;
        NoiseGate *_noiseGate;
};