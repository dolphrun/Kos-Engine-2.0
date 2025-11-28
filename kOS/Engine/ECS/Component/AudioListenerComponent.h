#ifndef AUDIOLISTENERCOM_H
#define AUDIOLISTENERCOM_H

#include "Component.h"

namespace ecs {

    class AudioListenerComponent : public Component {
    public:
        bool active = true;


        REFLECTABLE(AudioListenerComponent, active)
    };
}
#endif AUDIOLISTENERCOM_H