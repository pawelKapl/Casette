#pragma once

#include "lvgl/lvgl.h"
#include "env.h"
#include "effects_chain.h"

struct DelayControlInfo
{
    FxChain *chainRef;
    int param = 0;
};

class DelayTab
{
public:
    DelayTab(lv_obj_t *tabview, FxChain *fxChain) : _fxChain(fxChain)
    {
        _tab = lv_tabview_add_tab(tabview, "Delay");
        lv_obj_set_scroll_dir(_tab, LV_DIR_VER);

        createLayout();
    }

    ~DelayTab()
    {
        lv_obj_delete(_tab);
    }

    void handleDelayPower(lv_event_t * e);
    void createLayout();

    private:
        lv_obj_t * _tab;
        FxChain *_fxChain;
        DelayControlInfo _modeInfo{_fxChain, 0};
        DelayControlInfo _mixInfo{_fxChain, 1};
        DelayControlInfo _feedbackInfo{_fxChain, 2};
        DelayControlInfo _timeInfo{_fxChain, 3};
        DelayControlInfo _time2Info{_fxChain, 4};
        DelayControlInfo _hiPassInfo{_fxChain, 5};
        DelayControlInfo _loPassInfo{_fxChain, 6};
        DelayControlInfo _diffusionInfo{_fxChain, 7};
        DelayControlInfo _modRateInfo{_fxChain, 8};
};