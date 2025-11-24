#include "Editor.h"
#include "Resources/R_AnimController.h"

namespace ed = ax::NodeEditor;


struct DisplayController {
    std::string name;
    AnimControllerData controller;

    REFLECTABLE(DisplayController, name);
};

DisplayController controllerData;

AnimState* FindStateFromPin(std::vector<AnimState>& states, int pinId)
{
    for (auto& state : states)
    {
        for (auto& p : state.inputs)
            if (p.id == pinId)
                return &state;

        for (auto& p : state.outputs)
            if (p.id == pinId)
                return &state;
    }
    return nullptr;
}

AnimPin* FindPin(std::vector<AnimState>& states, int pinId)
{
    for (auto& state : states)
    {
        for (auto& p : state.inputs)
            if (p.id == pinId)
                return &p;

        for (auto& p : state.outputs)
            if (p.id == pinId)
                return &p;
    }
    return nullptr;
}

void gui::ImGuiHandler::DrawAnimatorControllerWindow()
{
    ImGui::Begin("Animator Controller");
    ed::SetCurrentEditor(m_animControllerContext);

    std::string test = selectedAsset.Type;
    if (test != "R_AnimController")
    {
        ImGui::Text("No Animator Controller selected.");

        ImGui::Spacing();

        // Centered buttons
        float totalWidth = 250.0f;
        float windowWidth = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX((windowWidth - totalWidth) * 0.5f);

        ImGui::BeginGroup();
        if (ImGui::Button("Create New Controller", ImVec2(250, 30)))
        {
            ImGui::OpenPopup("NewControllerPopup");
        }
        if (ImGui::BeginPopupModal("NewControllerPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char controllerNameBuffer[128] = "";

            ImGui::Text("Enter Controller Name:");
            ImGui::InputText("##controllerName", controllerNameBuffer, IM_ARRAYSIZE(controllerNameBuffer));

            // Show ".controller" as grey text
            ImGui::SameLine();
            ImGui::TextDisabled(".controller");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            bool validName = strlen(controllerNameBuffer) > 0;

            // Create button
            if (!validName)
                ImGui::BeginDisabled();

            if (ImGui::Button("Create", ImVec2(120, 0)))
            {
                 ///Create controller here
                controllerData.name = controllerNameBuffer;
                std::string fileName = controllerData.name + ".controller";
                std::string filepath = m_assetManager.GetAssetManagerDirectory() + "/Controllers/" + fileName;
                serialization::WriteJsonFile(filepath, &controllerData.controller);
                std::cout << "TEST CREATION\n";
                LOGGING_POPUP("Animator Controller Successfully Added");
                ImGui::CloseCurrentPopup();
            }

            if (!validName)
                ImGui::EndDisabled();

            ImGui::SameLine();

            // Cancel button
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        ImGui::EndGroup();
        ImGui::End();
        return; // stop drawing node editor
    }

    if (cachedControllerGUID != selectedAsset.GUID)
    {
        cachedControllerGUID = selectedAsset.GUID;
        m_activeController = m_resourceManager.GetResource<R_AnimController>(selectedAsset.GUID).get();
    }

    if (!m_activeController)
    {
        return;
    }

    std::vector<AnimState>& states = m_activeController->m_AnimControllerData.states;
    int& nextStateId = m_activeController->m_AnimControllerData.currentStateID;
    int& nextPinId = m_activeController->m_AnimControllerData.currentPinID;
    int& nextLinkId = m_activeController->m_AnimControllerData.currentLinkID;

    ed::Begin("AnimationGraph");
    ImGui::Text("Controller: %s", m_activeController->m_AnimControllerData.name.c_str());
    ImGui::Separator();

    // --- Create Default Nodes ---
    if (states.empty())
    {
        // Entry node
        AnimState entry;
        entry.id = nextStateId++;
        entry.name = "Entry";
        entry.outputs.push_back({ nextPinId++, AnimPin::PinKind::Output, "Out"});
        entry.isDefault = true;
        states.push_back(entry);
        ed::SetNodePosition(entry.id, ImVec2(50, 150));

        // Any State node
        AnimState any;
        any.id = nextStateId++;
        any.name = "Any State";
        any.outputs.push_back({ nextPinId++, AnimPin::PinKind::Output, "Out" });
        any.isDefault = true;
        states.push_back(any);
        ed::SetNodePosition(any.id, ImVec2(50, 50));

        // Exit node
        AnimState exit;
        exit.id = nextStateId++;
        exit.name = "Exit";
        exit.inputs.push_back({ nextPinId++, AnimPin::PinKind::Input, "In" });
        exit.isDefault = true;
        states.push_back(exit);
        ed::SetNodePosition(exit.id, ImVec2(50, 100));
    }

    // --- Node creation button ---
    ImGui::BeginGroup();
    if (ImGui::Button("Add State"))
    {
        AnimState s;
        s.id = nextStateId++;
        s.name = "New State";
        s.inputs.push_back({ nextPinId++, AnimPin::PinKind::Input, "Enter" });
        s.outputs.push_back({ nextPinId++, AnimPin::PinKind::Output, "Exit" });
        states.push_back(s);
        ed::SetNodePosition(s.id, ImVec2(100 + 50.0f * states.size(), 100));
    }
    ImGui::SameLine();
    ImGui::EndGroup();

    ImGui::Separator();

    // --- Draw all nodes ---
    for (auto& node : states)
    {
        ed::BeginNode(node.id);
        ImGui::Text("%s", node.name.c_str());

        for (auto& pin : node.inputs)
        {
            ed::BeginPin(pin.id, ed::PinKind::Input);
            ImGui::Text("%s", pin.name.c_str());
            ed::EndPin();
        }

        for (auto& pin : node.outputs)
        {
            ed::BeginPin(pin.id, ed::PinKind::Output);
            ImGui::Text("%s", pin.name.c_str());
            ed::EndPin();
        }

        ed::EndNode();
    }

    // --- Handle link creation ---
    ed::PinId startPin, endPin;
    if (ed::BeginCreate())
    {
        if (ed::QueryNewLink(&startPin, &endPin))
        {
            if (ed::AcceptNewItem())
            {
                AnimTransition t;
                t.id = nextLinkId++;
                t.fromPinId = startPin.Get();
                t.toPinId = endPin.Get();
                t.condition = ""; // allow editing in inspector
                AnimState* fromState = FindStateFromPin(states, t.fromPinId);
                //AnimState* toState = FindStateFromPin(states, t.toPinId);
                fromState->outgoingTransitions.push_back(t);
            }
        }
    }
    ed::EndCreate();

    // --- Draw links ---
    for (auto& state : states)
    {
        for (auto& t : state.outgoingTransitions)
            ed::Link(t.id, t.fromPinId, t.toPinId);
    }

    // --- Detect node selection ---
    static ed::NodeId selectedNode;
    static ed::LinkId selectedLink;
    selectedLink = 0;
    if (ed::GetSelectedObjectCount() > 0)
    {
        // Check if link is selected first
        ed::LinkId links[1];
        if (ed::GetSelectedLinks(links, 1) > 0)
        {
            selectedLink = links[0];
            selectedNode = ed::NodeId(0);
        }
        else
        {
            // Otherwise check nodes
            ed::NodeId nodes[1];
            if (ed::GetSelectedNodes(nodes, 1) > 0)
            {
                selectedNode = nodes[0];
                selectedLink = ed::LinkId(0);
            }
        }
    }
    else
    {
        selectedNode = 0;
        selectedLink = 0;
    }

    ed::End();
    ed::SetCurrentEditor(nullptr);

    ImGui::End();

    // --- Always-visible property window ---
    ImGui::Begin("Selected Animator Node");

    // --- TRANSITION INSPECTOR ---
    if (selectedLink)
    {
        int linkId = selectedLink.Get();

        for (auto& state : states)
        {
            for (auto& t : state.outgoingTransitions)
            {
                if (t.id == linkId)
                {
                    ImGui::Separator();
                    ImGui::Text("Transition Properties");

                    char buf[256];
                    strcpy(buf, t.condition.c_str());
                    if (ImGui::InputText("Condition", buf, sizeof(buf)))
                        t.condition = buf;

                    AnimState* fromState = FindStateFromPin(states, t.fromPinId);
                    AnimState* toState = FindStateFromPin(states, t.toPinId);

                    AnimPin* fromPin = FindPin(states, t.fromPinId);
                    AnimPin* toPin = FindPin(states, t.toPinId);

                    ImGui::Separator();
                    ImGui::Text("Transition Endpoints");

                    if (fromState)
                    {
                        ImGui::Text("From State: %s", fromState->name.c_str());
                        if (fromPin)
                            ImGui::Text("  Pin: %s", fromPin->name.c_str());
                    }
                    else ImGui::Text("From State: <Unknown>");

                    if (toState)
                    {
                        ImGui::Text("To State: %s", toState->name.c_str());
                        if (toPin)
                            ImGui::Text("  Pin: %s", toPin->name.c_str());
                    }
                    else ImGui::Text("To State: <Unknown>");

                    ImGui::Separator();
                    ImGui::End();
                    return;
                }
            }
        }
        
    }

    // --- STATE INSPECTOR ---
    if (selectedNode && selectedNode.Get() <= states.size())
    {
        AnimState& state = states[selectedNode.Get() - 1];
        state.ApplyFunction(DrawComponents{ state.Names() });
    }
    else
    {
        ImGui::TextDisabled("No node or link selected.");
    }

    ImGui::End();
}


void gui::ImGuiHandler::ShutdownAnimatorLayout()
{
    if (m_animControllerContext)
    {
        // DestroyEditor will write settings to the configured SettingsFile.
        ax::NodeEditor::DestroyEditor(m_animControllerContext);
        m_animControllerContext = nullptr;
    }
}