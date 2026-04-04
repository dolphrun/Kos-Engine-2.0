#pragma once
#include "TemplateSC.h"	

class FadeTransition : public TemplateSC {
public:
	void Transition(glm::vec4 targetColor, float duration) {
		//Get current post process profile
		sc = ecsPtr->GetComponent<ecs::SpriteComponent>(entity);
		this->targetColor = targetColor;
		this->time = duration;
		//Get difference betwen the two values
		colorRate = (targetColor - sc->color) / duration;

	}

	void TransitionToTransparent(float duration) {
		Transition(glm::vec4(0.f, 0.f, 0.f, 0.f), duration);
	}

	void TransitionToBlack(float duration) {
		Transition(glm::vec4(0.f, 0.f, 0.f, 1.f), duration);
	}

	void Start() {

		sc = ecsPtr->GetComponent<ecs::SpriteComponent>(entity);
		sc->color = glm::vec4(0.f, 0.f, 0.f, 1.f);
		TransitionToTransparent(1.5f);
	}
	void Update() {
		if (time>0.f) {
			//Lerp the values 
			time -= ecsPtr->m_GetDeltaTime();;
			sc->color += colorRate * ecsPtr->m_GetDeltaTime();
			if (time <= 0.0f)sc->color = targetColor;
		}
	}
	REFLECTABLE(FadeTransition);
private:
	glm::vec4 targetColor;
	float time;
	SpriteComponent* sc;
	glm::vec4 colorRate;
};
