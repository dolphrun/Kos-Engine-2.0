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


    // Parameter storage for the state
    ///Change this to vector, hashing is causing weird behaviour
    //std::unordered_map<std::string, AnimCondition> parameterMap;

    


    // Setters for different condition types
    //void CheckExistence(const std::string& name) { if (!parameterMap.contains(name)) parameterMap[name] = {}; };
    //void SetBool(const std::string& name, bool v) { CheckExistence(name);  parameterMap[name].boolValue = v; }
    //void SetTrigger(const std::string& name) { CheckExistence(name); parameterMap[name].triggerValue = true; }
    //void SetFloat(const std::string& name, float v) { CheckExistence(name); parameterMap[name].floatValue = v; }
    //void SetInt(const std::string& name, int v) { CheckExistence(name); parameterMap[name].intValue = v; }
    //void SetOperator(const std::string& name, AnimCondition::CompareOp op) { CheckExistence(name); parameterMap[name].op = op; };
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
    /*bool CompareFloatCondition(const std::string& name, float value , bool& outcome)
    {
        AnimCondition* condition = nullptr;

        for (int i = 0; i < outgoingTransitions.size(); i++)
        {
            for (int j = 0; j < outgoingTransitions[i].conditions.size(); j++)
            {
                if (name == outgoingTransitions[i].conditions[j].name) condition = &outgoingTransitions[i].conditions[j];
            }
        }

        if (condition = CheckExistence(name)) return false; 
        return Compare(value, condition->floatValue, condition->op);
    }
    bool CompareBoolCondition(const std::string& name, bool value)
    {
        AnimCondition* condition = nullptr;

        for (int i = 0; i < outgoingTransitions.size(); i++)
        {
            for (int j = 0; j < outgoingTransitions[i].conditions.size(); j++)
            {
                if (name == outgoingTransitions[i].conditions[j].name) condition = &outgoingTransitions[i].conditions[j];
            }
        }

        if (condition == nullptr) return false;

        if (Compare(value, condition->boolValue, condition->op))
        {
            transiti
        }
    }
    bool CompareIntCondition(const std::string& name, int value)
    {
        for (int i = 0; i < outgoingTransitions.size(); i++)
        {
            for (int j = 0; j < outgoingTransitions[i].conditions.size(); j++)
            {
                if (name == outgoingTransitions[i].conditions[j].name) return &outgoingTransitions[i].conditions[j];
            }
        }
        AnimCondition* condition = nullptr;
        if (condition = CheckExistence(name)) return false;
        return Compare(value, condition->intValue, condition->op);

    }*/

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

    // Evaluates conditions for transitions

   /* bool EvaluateCondition(const AnimCondition& c)
    {
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
    }*/
    // Check if a transition is valid
   /* bool CanTransition(const AnimTransition& t)
    {
        if (t.conditions.empty())
        {
            return false;
        }
        for (auto& cond : t.conditions)
        {
            if (!EvaluateCondition(cond))
                return false;
        }
        return true;
    }*/


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

    REFLECTABLE(R_AnimController);
};
