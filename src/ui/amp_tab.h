#pragma once

#include "lvgl/lvgl.h"
#include "env.h"
#include "dsp/amp.h"

class AmpTab;

struct AmpControlInfo
{
    Amp *ampRef;
    AmpTab *tabRef;
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
    void handleSubAmpPower(lv_event_t *e);
    void handleTonestackPower(lv_event_t *e);
    void changeAmpModel(const char *newModel);
    void changeSubAmpModel(const char *newModel);
    void createLayout();

    lv_obj_t *_blendSlider;
    
private:
    float getInputLevel();
    float getOutputLevel();
    lv_obj_t * levelMeter(lv_obj_t * parent, const char * title);

    lv_obj_t *_tab;
    lv_obj_t *_inputMeter;
    lv_obj_t *_outputMeter;
    Amp *_amp;

    AmpControlInfo _subBlendInfo{_amp, this, 1};
    AmpControlInfo _inputGainInfo{_amp, this, 2};
    AmpControlInfo _mvInfo{_amp, this, 3};
    AmpControlInfo _bassInfo{_amp, this, 4};
    AmpControlInfo _middleInfo{_amp, this, 5};
    AmpControlInfo _trebleInfo{_amp, this, 6};
    AmpControlInfo _subModeInfo{_amp, this, 0};
};