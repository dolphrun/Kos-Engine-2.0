#pragma once
#include "Config/pch.h"
#include "Resource.h"

// ---------------- CONDITION TYPE -------------------

enum class AnimConditionType
{
    Bool,
    Trigger,
    Float,
    Int
};

struct AnimCondition
{
    std::string name;
    AnimConditionType type{ AnimConditionType::Bool };

    enum class CompareOp
    {
        Equal,
        NotEqual,
        Greater,
        Less,
        GreaterEqual,
        LessEqual
    };

    CompareOp op{ CompareOp::Equal };

    bool boolValue{ false };
    bool triggerValue{ false };
    float floatValue{ 0.0f };
    int intValue{ 0 };

    REFLECTABLE(AnimCondition, name, type, op, boolValue, triggerValue, floatValue, intValue);
};

// ---------------- PINS -------------------

struct AnimPin {
    int id;
    enum PinKind { Input, Output } kind;
    std::string name;
    int fromParentId{};
    int toParentId{};
    REFLECTABLE(AnimPin, id, kind, name, fromParentId, toParentId);
};

// ---------------- TRANSITIONS -------------------

struct AnimTransition {
    int id;
    int fromPinId;
    int toPinId;

    // A transition can now have MULTIPLE conditions
    std::vector<AnimCondition> conditions;

    REFLECTABLE(AnimTransition, id, fromPinId, toPinId, conditions);
};

// ---------------- STATES -------------------
struct AnimState {
    int id;
    std::vector<AnimPin> inputs;
    std::vector<AnimPin> outputs;


    std::string name;
    utility::GUID animationGUID;
    float playSpeed = 1.0f;
    bool isLooping = true;
    bool isDefault = false;


    std::vector<AnimTransition> outgoingTransitions;
    AnimState* anyState = nullptr;
    
    template <typename T, typename U>
    void Trigger(const std::string& name ,T entity, U controller)
    { 
        AnimCondition* condition = nullptr;

        for (int i = 0; i < outgoingTransitions.size(); i++)
        {
            for (int j = 0; j < outgoingTransitions[i].conditions.size(); j++)
            {
                if (name == outgoingTransitions[i].conditions[j].name)
                {
                    //Use to check type of condition but just ignore for now
                    //condition = &outgoingTransitions[i].conditions[j];
                    entity->m_transitioningStateID = controller->FindStateFromPin(outgoingTransitions[i].toPinId)->id;
                }
            }
        }
    }


    AnimCondition* CheckExistence(const std::string& name)
    {
        for (int i = 0; i < outgoingTransitions.size(); i++)
        {
            for (int j = 0; j < outgoingTransitions[i].conditions.size(); j++)
            {
                if (name == outgoingTransitions[i].conditions[j].name) return &outgoingTransitions[i].conditions[j];
            }
        }
        return nullptr;
    }

    bool Compare(auto a, auto b, AnimCondition::CompareOp conditionOperator)
    {
        switch (conditionOperator)
        {
            case AnimCondition::CompareOp::Equal: return a == b;
            case AnimCondition::CompareOp::NotEqual: return a != b;
            case AnimCondition::CompareOp::Greater: return a > b;
            case AnimCondition::CompareOp::Less: return a < b;
            case AnimCondition::CompareOp::GreaterEqual: return a >= b;
            case AnimCondition::CompareOp::LessEqual: return a <= b;
        }
        return false;
    };


    REFLECTABLE(AnimState, id, inputs, outputs, name, animationGUID, playSpeed, isLooping, outgoingTransitions);
};


// ---------------- CONTROLLER -------------------

struct AnimControllerData
{
    std::string name;
    std::vector<AnimState> states;
    int nextStateID{ 1 };
    int nextPinID{ 100 };
    int nextLinkID{ 200 };

    REFLECTABLE(AnimControllerData, name, states, nextStateID, nextPinID, nextLinkID);
};

class R_AnimController : public Resource
{
public:
    using Resource::Resource;
    void Load() override;
    void Unload() override;

    AnimControllerData m_AnimControllerData;

    AnimState* m_EnterState;
    AnimState* m_AnyState;
    AnimState* m_ExitState;

    AnimState* RetrieveEntryState();
    AnimState* RetrieveStateByID(int stateID);
    AnimState* FindStateFromPin(int pinId);
    AnimPin* FindPin(int pinId);
    AnimState* FindStateByName(const std::string& name);  // needed for lookup below

    template <typename T>
    void PlayOverlay(const std::string& stateName, T entity,
        float fadeIn = 0.1f, float fadeOut = 0.2f,
        std::vector<std::string> boneMask = {})
    {
        AnimState* state = FindStateByName(stateName);
        if (!state) return;

        entity->m_overlayStateID = state->id;
        entity->m_overlayTime = 0.f;
        entity->m_overlayWeight = 0.f;
        entity->m_overlayFadeIn = fadeIn;
        entity->m_overlayFadeOut = fadeOut;
        entity->m_overlayFadingOut = false;
        entity->m_overlayBoneMask = std::move(boneMask);
    }

    template <typename T>
    void SetState(const std::string& stateName, T entity)
    {
        AnimState* state = FindStateByName(stateName);
        if (!state) return;

        entity->m_transitioningStateID = state->id;
    }

    REFLECTABLE(R_AnimController);
};
