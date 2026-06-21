#pragma once

#include "env.h"

class CpuMonitor
{

public:
    void tick()
    {
        _updateTimer++;
        if (_updateTimer > 200)
        {
            _updateTimer = 0;
        }

        if (_updateTimer)
            return;

        std::ifstream file("/proc/self/stat");
        std::string dummy;
        unsigned long long utime, stime;
        for (int i = 0; i < 13; ++i) file >> dummy;  // Skip first 13 fields
        file >> utime >> stime;  // 14th and 15th fields
        unsigned long long t2 = utime + stime;

        auto clk_tck = sysconf(_SC_CLK_TCK);
        float cpuTime = (t2 - _t1) / static_cast<float>(clk_tck);  // seconds
        _cpuUsage = (cpuTime * 1000.0f) / 1000.0f * 100.0f; 

        file = std::ifstream("/sys/class/thermal/thermal_zone0/temp");
        int temp_millideg;
        file >> temp_millideg;
        _cpuTemperature = temp_millideg / 1000.0f;
        _t1 = t2;
    }

    std::array<float, 2> getCpuState()
    {
        return {_cpuUsage, _cpuTemperature};
    }

private:
    unsigned long long _t1 = 0;
    float _cpuUsage = 0;
    float _cpuTemperature = 0;
    int _updateTimer = 0;
};