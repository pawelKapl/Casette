#pragma once

#include "lvgl/lvgl.h"
#include "env.h"
#include "dsp/cab_simulator.h"

struct CabControlInfo
{
    CabSimulator *cabRef;
    int param = 0;
};

class CabTab
{
public:
    CabTab(lv_obj_t *tabview, CabSimulator *cab) : _cab(cab)
    {
        _tab = lv_tabview_add_tab(tabview, "Cab");
        lv_obj_set_scroll_dir(_tab, LV_DIR_VER);

        createLayout();
    }

    ~CabTab()
    {
        lv_obj_delete(_tab);
    }

    void handleCabPower(lv_event_t * e);
    void changeCabIR(const char* newCab);
    void createLayout();

    private:
        lv_obj_t * _tab;
        CabSimulator *_cab;

        CabControlInfo _hiPassInfo{_cab, 5};
};