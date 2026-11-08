#pragma once

#include "lvgl/lvgl.h"
#include "env.h"
#include "effects_chain.h"

struct ReverbControlInfo
{
    FxChain *chainRef;
    int param = 0;
};


class ReverbTab
{
public:
    ReverbTab(lv_obj_t *tabview, FxChain *fxChain) : _fxChain(fxChain)
    {
        _tab = lv_tabview_add_tab(tabview, "Reverb");
        lv_obj_set_scroll_dir(_tab, LV_DIR_VER);

        createLayout();
    }

    ~ReverbTab()
    {
        lv_obj_delete(_tab);
    }

    void handleReverbPower(lv_event_t *e);
    void handleTypeSelected(lv_event_t *e);
    void createLayout();

private:
    void showCharacter(int mode);

    lv_obj_t *_tab;
    FxChain *_fxChain;
    lv_obj_t *_characterSections[REVERB_MODE_COUNT] = {};
    lv_obj_t *_freezeSwitch = nullptr;

    ReverbControlInfo _modeInfo{_fxChain, 0};
    ReverbControlInfo _mixInfo{_fxChain, 1};
    ReverbControlInfo _roomSizeInfo{_fxChain, 2};
    ReverbControlInfo _dampingInfo{_fxChain, 3};
    ReverbControlInfo _preDelayInfo{_fxChain, 4};
    ReverbControlInfo _hiPassInfo{_fxChain, 5};
    ReverbControlInfo _loPassInfo{_fxChain, 6};
    ReverbControlInfo _decayInfo{_fxChain, 8};
    ReverbControlInfo _dwellInfo{_fxChain, 9};
    ReverbControlInfo _tensionInfo{_fxChain, 10};
    ReverbControlInfo _springsInfo{_fxChain, 11};
    ReverbControlInfo _modDepthInfo{_fxChain, 12};
    ReverbControlInfo _modRateInfo{_fxChain, 13};
    ReverbControlInfo _freezeInfo{_fxChain, 14};
};