/******************************************************************/
/*!
\file      Peformance.h
\author    Jaz Winn Ng
\par       jazwinn.ng@digipen.edu
\date      Sept 28, 2025
\brief     Singleton class to track and manage performance metrics such as FPS, delta time,
		   and performance data for system and script components.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************/
#pragma once
#include "Config/pch.h"

    #define PROFILE_SYSTEM(performance, category, name, code)              \
    {                                                            \
        auto __start = std::chrono::steady_clock::now();        \
        code;                                                    \
        auto __end = std::chrono::steady_clock::now();          \
        std::chrono::duration<float> __dur = __end - __start;   \
        performance.SetSystemValue(category, name, __dur.count());         \
    }


class Peformance
{
public:
    using systemPeformance = std::unordered_map<std::string, float>;


    const std::unordered_map<std::string, systemPeformance>& GetSystemPerformance() const
    {
        return m_SystemPerformance;
    }


    float GetFPS() const
    {
        return m_fps;

    }

    void SetDeltaTime(float delta)
    {
        m_detaTime = delta;
        m_fps = 1 / delta;
    }

    float GetDeltaTime() const
    {
        return m_detaTime;
    }


    // Optional: for convenience, add/modify a single system performance entry
    void SetSystemValue(const std::string& category, const std::string& key, float value)
    {
        m_SystemPerformance[category][key] = value;
    }

private:
    
    std::unordered_map<std::string, systemPeformance> m_SystemPerformance;

    float m_fps{};
    float m_detaTime{};
    
};

