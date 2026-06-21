#pragma once

#include "env.h"
#include "lvgl/lvgl.h"
#include "ui/delay_tab.h"
#include "ui/cab_tab.h"
#include "ui/amp_tab.h"
#include "ui/ng_tab.h"
#include "ui/mod_tab.h"
#include "ui/reverb_tab.h"
#include "ui/misc_tab.h"
#include "effects_chain.h"
#include "dsp/cab_simulator.h"
#include "dsp/tuner.h"
#include "dsp/amp.h"
#include "platform/gpio_button.h"

class GUI
{
public:
    GUI(FxChain *fxChain, CabSimulator *cabSim, Amp *amp, NoiseGate *ng, Tuner *tuner, std::array<std::unique_ptr<GPIOButton>, 3> *footswitches, CpuMonitor* cpuMonitor)
        : _fxChain(fxChain),
          _cabSimulator(cabSim),
          _amp(amp),
          _noiseGate(ng),
          _tuner(tuner),
          _cpuMonitor(cpuMonitor),
          _footswitches(footswitches) {}

    ~GUI();

    void init();
    void update();
    void reload();
    void bypassMode(bool active);

private:
    void createTabMenu();
    void initTheme();

    lv_display_t *_disp;
    lv_obj_t *_mainScreen;
    lv_obj_t *_bypassScreen;
    lv_indev_t *_inputDevice;
    lv_obj_t *_tabview;

    std::unique_ptr<CabTab> _cabTab;
    std::unique_ptr<AmpTab> _ampTab;
    std::unique_ptr<NGTab> _ngTab;
    std::unique_ptr<ModTab> _modTab;
    std::unique_ptr<DelayTab> _delayTab;
    std::unique_ptr<ReverbTab> _reverbTab;
    std::unique_ptr<MiscTab> _miscTab;

    CpuMonitor *_cpuMonitor;
    FxChain *_fxChain;
    CabSimulator *_cabSimulator;
    Amp *_amp;
    NoiseGate *_noiseGate;
    Tuner *_tuner;
    std::array<std::unique_ptr<GPIOButton>, 3> *_footswitches;

    lv_theme_t *_mainTheme;

    bool _bypassMode = false;

    int _metersUpdateTimer = 0;
    int _cpuChartUpdateTimer = 0;

}; // GUI