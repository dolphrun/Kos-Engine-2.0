#pragma once
#include "Config/pch.h"
#include "Resource.h"
#include "ResourceHeader.h"

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


    // Parameter storage for the state
    std::unordered_map<std::string, AnimCondition> parameterMap;


    // Setters for different condition types
    void SetBool(const std::string& name, bool v) { parameterMap[name].boolValue = v; }
    void SetTrigger(const std::string& name) { parameterMap[name].triggerValue = true; }
    void SetFloat(const std::string& name, float v) { parameterMap[name].floatValue = v; }
    void SetInt(const std::string& name, int v) { parameterMap[name].intValue = v; }


    // Evaluates conditions for transitions
    bool EvaluateCondition(const AnimCondition& c)
    {
        auto& param = parameterMap[c.name];


        auto compare = [&](auto a, auto b)
            {
                switch (c.op)
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


        switch (c.type)
        {
        case AnimConditionType::Bool:
            return compare(param.boolValue, c.boolValue);


        case AnimConditionType::Trigger:
            if (param.triggerValue)
            {
                param.triggerValue = false;
                return true;
            }
            return false;
        case AnimConditionType::Float:
            return compare(param.floatValue, c.floatValue);


        case AnimConditionType::Int:
            return compare(param.intValue, c.intValue);
        }
        return false;
    }
    // Check if a transition is valid
    bool CanTransition(const AnimTransition& t)
    {
        for (auto& cond : t.conditions)
        {
            if (!EvaluateCondition(cond))
                return false;
        }
        return true;
    }


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
    AnimState* FindStateFromPin(int pinId);
    AnimPin* FindPin(int pinId);

    REFLECTABLE(R_AnimController);
};
