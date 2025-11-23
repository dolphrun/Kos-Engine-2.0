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


/// <summary> To do list
/// I need to be able to label which controller im currently working with
/// which means
/// if there is no controller selected, i need an option to create a new controller or load an exisiting controller, the controller will not be displayed
/// 
/// 
/// 
/// I need to link the current controller whenever, do this by adding R_Controller inside the component window and checking if its open
/// the controller will always be related to the animator component of the currently selected object
/// 
/// add animation guid for each node
/// 
/// draw outline on the node for animation playing
/// 
/// in animator system, any outputs will change the animation to the corresponding input node, unless node is looping
/// 
/// 
/// </summary>
void gui::ImGuiHandler::DrawAnimatorControllerWindow()
{
    ImGui::Begin("Animator Controller");

    constexpr int startingStateId = 1;
    constexpr int startingPinId = 100;
    constexpr int startingLink = 200;

    static int nextStateId = startingStateId;
    static int nextPinId = startingPinId;
    static int nextLinkId = startingLink;

    static std::vector<AnimState> states;
    //static std::vector<AnimTransition> transitions;

    // --- Context & Node Editor ---
   // static ed::EditorContext* context = nullptr;
    //if (!context)
    //    context = ed::CreateEditor();
    ed::SetCurrentEditor(m_animControllerContext);

    if (!m_activeController)
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
                // Build final file name
                //std::string finalName = std::string(controllerNameBuffer) + ".controller";
                //std::string filepath = m_assetManager.GetAssetManagerDirectory() + "/Animation Controller/" + finalName;

                // Save it
                //serialization::WriteJsonFile(filepath, &m_activeController->controller);

                 ///Create controller here
                std::string fileName = controllerData.name + ".controller";
                std::string filepath = m_assetManager.GetAssetManagerDirectory() + "/Animation Controller/" + fileName;
                serialization::WriteJsonFile(filepath, &controllerData.controller);
                std::cout << "TEST CREATION\n";
                LOGGING_POPUP("Animator Controller Successfully Added");

                LOGGING_POPUP("Animator Controller Created");
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

        if (ImGui::Button("Load Existing Controller", ImVec2(250, 30)))
        {
            // Call your file dialog here
            //LoadControllerFromFile("Assets/Controllers/Example.controller");
            ///Load controller here
        }
        ImGui::EndGroup();
        ImGui::End();
        return; // stop drawing node editor
    }

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

    if (ImGui::Button("Create New", ImVec2(120, 30)))
    {
        ///Create new
    }

    ImGui::SameLine();

    if (ImGui::Button("Load", ImVec2(80, 30)))
    {
       /// Load existing
    }
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
    if (selectedNode && selectedNode.Get() < states.size())
    {
        AnimState& state = states[selectedNode.Get()];
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