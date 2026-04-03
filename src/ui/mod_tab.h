#pragma once

#include "lvgl/lvgl.h"
#include "env.h"
#include "effects_chain.h"

struct ModControlInfo
{
    FxChain *chainRef;
    int param = 0;
};

class ModTab
{
public:
    ModTab(lv_obj_t *tabview, FxChain *fxChain) : _fxChain(fxChain)
    {
        _tab = lv_tabview_add_tab(tabview, "Mod");
        lv_obj_set_scroll_dir(_tab, LV_DIR_VER);

        createLayout();
    }

    ~ModTab()
    {
        lv_obj_delete(_tab);
    }

    void handleModPower(lv_event_t *e);
    void handleVibratoMode(lv_event_t * e);
    void createLayout();

private:
    lv_obj_t *_tab;
    FxChain *_fxChain;

    ModControlInfo _mixInfo{_fxChain, 1};
    ModControlInfo _depthInfo{_fxChain, 2};
    ModControlInfo _rateInfo{_fxChain, 3};
    ModControlInfo _widthInfo{_fxChain, 4};
    ModControlInfo _feedbackInfo{_fxChain, 5};
    ModControlInfo _vibratoMode{_fxChain, 6};

};