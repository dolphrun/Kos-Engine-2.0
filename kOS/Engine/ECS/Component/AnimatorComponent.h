#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "Component.h"

namespace ecs {

    class AnimatorComponent : public Component {

    public:
        utility::GUID controllerGUID{};  // Animation controller / state machine
        float m_PlaybackSpeed{ 1.0f };
        float m_CurrentTime{ 0.f };
        bool  m_IsPlaying{ true };

        void* m_currentState{};

        REFLECTABLE(AnimatorComponent, controllerGUID, m_PlaybackSpeed, m_CurrentTime, m_IsPlaying);
    };

}

#endif // ANIMATOR_H
