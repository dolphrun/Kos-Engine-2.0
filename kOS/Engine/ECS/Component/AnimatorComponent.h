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
        
        // Non-reflectable, will initialize based on controller
        int m_currentStateID{}; // This id will refer to the state ids belonging to the controller
        int m_transitioningStateID{}; // This id will refer to the state this component wants to change to


        REFLECTABLE(AnimatorComponent, controllerGUID, m_PlaybackSpeed, m_CurrentTime, m_IsPlaying, m_currentStateID, m_transitioningStateID);
    };

}

#endif // ANIMATOR_H
