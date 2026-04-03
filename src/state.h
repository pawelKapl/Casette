#pragma once

#include "env.h"

struct State
{
    static State &get()
    {
        static State instance{};
        return instance;
    };

    std::string activePreset = "default";
    std::string leftFsAssignement = "default";
    std::string midFsAssignement = "default";
    std::string rightFsAssignement = "default";
    bool requireReload = false;

    inline void changePreset(const std::string &newPreset)
    {
        activePreset = newPreset;
        requireReload = true;
    }

    inline void ackReload()
    {
        requireReload = false;
    }
};