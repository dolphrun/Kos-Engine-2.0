#pragma once

#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"


class MenuPt2Script : public TemplateSC {
public:

	void Start() override;
	void Update() override;

	REFLECTABLE(MenuPt2Script);
};


inline void MenuPt2Script::Start() {
	Input->HideCursor(false);  // show cursor as soon as this scene starts
}

inline void MenuPt2Script::Update() {
	// nothing needed — cursor stays visible until another scene takes over
	Input->HideCursor(false);
}
