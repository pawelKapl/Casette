#pragma once

#include "lvgl/lvgl.h"
#include "env.h"
#include "pre_chain.h"

struct OctaverControlInfo
{
    PreChain *chainRef;
    int param = 0;
};

class OctaverTab
{
public:
    OctaverTab(lv_obj_t *tabview, PreChain *preChain) : _preChain(preChain)
    {
        _tab = lv_tabview_add_tab(tabview, "Octaver");
        lv_obj_set_scroll_dir(_tab, LV_DIR_VER);

        createLayout();
    }

    ~OctaverTab()
    {
        lv_obj_delete(_tab);
    }

    void handleOctaverPower(lv_event_t *e);
    void createLayout();

private:
    lv_obj_t *_tab;
    PreChain *_preChain;

    OctaverControlInfo _modeInfo{_preChain, 0};
    OctaverControlInfo _dryInfo{_preChain, 1};
    OctaverControlInfo _sub2Info{_preChain, 2};
    OctaverControlInfo _sub1Info{_preChain, 3};
    OctaverControlInfo _up1Info{_preChain, 4};
    OctaverControlInfo _up2Info{_preChain, 5};
    OctaverControlInfo _toneInfo{_preChain, 6};
};
