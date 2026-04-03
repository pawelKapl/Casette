#pragma once

#include "env.h"
#include "state.h"

struct Storage
{
    static Storage &get()
    {
        static Storage instance{};
        return instance;
    };

    template<typename T>
    void persist(const T& object, const std::string& key)
    {
        nlohmann::json j;
        j[key] = object;

        std::ofstream file(getPath(key));
        file << j.dump(4);
    }

    template<typename T>
    std::optional<T> read(const std::string& key)
    {
        std::ifstream file(getPath(key));
        if (!file.is_open()) return std::nullopt;

        nlohmann::json j;
        file >> j;

        return j.at(key).get<T>();
    }

    void persistActivePreset()
    {
        std::filesystem::create_directory("../storage/");
        std::ofstream file("../storage/activePreset.txt");
        file << State::get().activePreset;
        std::cout << "persisted: " <<State::get().activePreset << "\n";
    }

    std::string loadActivePreset()
    {
        std::ifstream file("../storage/activePreset.txt");
        if (!file.is_open()) return "default";
        std::string activePreset;
        std::getline(file, activePreset);

        return activePreset;
    }

    void persistFsAssignement()
    {
        std::filesystem::create_directory("../storage/");
        std::string filename = "../storage/fsAssignement.txt";
        std::ofstream file(filename);
        file << State::get().leftFsAssignement << "\n";
        file << State::get().midFsAssignement << "\n";
        file << State::get().rightFsAssignement << "\n";
    }

    std::array<std::string, 3> loadFsAssignements()
    {
        std::ifstream file("../storage/fsAssignement.txt");
        if (!file.is_open()) return {"default", "default", "default"};

        std::string left;
        std::string mid;
        std::string right;

        std::getline(file, left);
        std::getline(file, mid);
        std::getline(file, right);

        return {left, mid, right};
    }

private:
    Storage() {};
    ~Storage() {};

    std::string getPath(const std::string& key)
    {
        std::string activePreset = State::get().activePreset;
        std::filesystem::create_directory("../storage/" + activePreset);
        return "../storage/" + activePreset + "/" + key + ".json";
    }

    std::string getPresetPath(const std::string& key)
    {
        std::string activePreset = State::get().activePreset;
        std::filesystem::create_directory("../storage/" + activePreset);
        return "../storage/" + activePreset + "/" + key + ".json";
    }

};