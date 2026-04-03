#pragma once

#include "lvgl/lvgl.h"
#include "env.h"
#include "dsp/amp.h"

struct AmpControlInfo
{
    Amp *ampRef;
    int param = 0;
};

class AmpTab
{
public:
    AmpTab(lv_obj_t *tabview, Amp *amp) : _amp(amp)
    {
        _tab = lv_tabview_add_tab(tabview, "Amp");
        lv_obj_set_scroll_dir(_tab, LV_DIR_VER);

        createLayout();
    }

    ~AmpTab()
    {
        lv_obj_delete(_tab);
    }

    void updateLevelMeters();

    void handleAmpPower(lv_event_t *e);
    void handleTonestackPower(lv_event_t *e);
    void changeAmpModel(const char *newModel);
    void createLayout();
    
    private:
    float getInputLevel();
    float getOutputLevel();
    lv_obj_t * levelMeter(lv_obj_t * parent, const char * title);

    lv_obj_t *_tab;
    lv_obj_t *_inputMeter;
    lv_obj_t *_outputMeter;
    Amp *_amp;

    AmpControlInfo _inputGainInfo{_amp, 1};
    AmpControlInfo _mvInfo{_amp, 2};
    AmpControlInfo _bassInfo{_amp, 3};
    AmpControlInfo _middleInfo{_amp, 4};
    AmpControlInfo _trebleInfo{_amp, 5};
    AmpControlInfo _masterInfo{_amp, 6};
};