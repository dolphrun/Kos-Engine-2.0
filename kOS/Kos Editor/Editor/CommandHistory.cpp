#include "CommandHistory.h"
#include "imgui.h"

std::stack<CommandHistory::CommandWrapper> CommandHistory::commandQueue;
std::stack<CommandHistory::CommandWrapper> CommandHistory::redoQueue;

#define CACHEDSCENE "CommandHistory"

void CommandHistory::Init() {
	m_ecs.AddScene(CACHEDSCENE, SceneData());
	m_ecs.sceneMap[CACHEDSCENE].isPrefab = true;
	m_sceneManager.SetSceneActive("CommandHistory", false);
	m_sceneManager.onSceneLoaded.Add([this](SceneData Data) {	Clear(); });
}

void CommandHistory::Update() {
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
		if (ImGui::IsKeyPressed(ImGuiKey_Z)){
			if (commandQueue.size() <= 0) return;

			redoQueue.push(commandQueue.top());
			commandQueue.top().Get()->Undo(m_ecs, this);
			commandQueue.pop();
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_Y)) {
			if (redoQueue.size() <= 0) return;
			
			commandQueue.push(redoQueue.top());
			redoQueue.top().Get()->Redo(m_ecs, this);
			redoQueue.pop();
		}
	}
}

void CommandHistory::Clear() {
	std::stack<CommandWrapper> empty1;
	std::stack<CommandWrapper> empty2;
	redoQueue.swap(empty1);
	commandQueue.swap(empty2);
}

void CommandHistory::RegisterRemapID(EntityID original, EntityID newID) {
	for (auto& [key, value] : idRemapping) {
		if (value == original) {
			value = newID;
		}
	}
	idRemapping[original] = newID;
	//std::cout << "Remapped [" << original << "] to [" << newID << "]" << std::endl;
}

void CommandHistory::DeleteRemapID(EntityID original) {
	idRemapping.erase(original);
}

EntityID CommandHistory::GetCurrentID(EntityID originalID) const {
	auto it = idRemapping.find(originalID);
	return (it != idRemapping.end()) ? it->second : originalID;
}

CommandHistory::AddGameObject::AddGameObject(EntityID _id, std::string _scene) : Command(_id), sceneName(_scene) {}

void CommandHistory::AddGameObject::Undo(ecs::ECS& ecs, CommandHistory* hist) {
	EntityID currentID = hist->GetCurrentID(id);
	EntityID newID = ecs.DuplicateEntity(currentID, CACHEDSCENE);
	ecs.DeleteEntity(currentID);
	hist->RegisterRemapID(id, newID);
	ecs.DeregisterEntity(newID);
}

void CommandHistory::AddGameObject::Redo(ecs::ECS& ecs, CommandHistory* hist) {
	EntityID currentID = hist->GetCurrentID(id);
	EntityID newID = ecs.DuplicateEntity(currentID, sceneName);
	ecs.DeleteEntity(currentID);
	hist->RegisterRemapID(id, newID);
}

CommandHistory::DeleteGameObject::DeleteGameObject(EntityID _id, std::string _scene, ecs::ECS& ecs, CommandHistory* hist) 
	: Command(_id), sceneName(_scene) {
	EntityID newID = ecs.DuplicateEntity(_id, CACHEDSCENE);
	hist->RegisterRemapID(id, newID);
	ecs.DeregisterEntity(newID);
}

void CommandHistory::DeleteGameObject::Undo(ecs::ECS& ecs, CommandHistory* hist) {
	EntityID currentID = hist->GetCurrentID(id);
	EntityID newID = ecs.DuplicateEntity(currentID, sceneName);
	ecs.DeleteEntity(currentID);
	hist->RegisterRemapID(id, newID);
}

void CommandHistory::DeleteGameObject::Redo(ecs::ECS& ecs, CommandHistory* hist) {
	EntityID currentID = hist->GetCurrentID(id);
	EntityID newID = ecs.DuplicateEntity(currentID, CACHEDSCENE);
	ecs.DeleteEntity(currentID);
	hist->RegisterRemapID(id, newID);
	ecs.DeregisterEntity(newID);
}

CommandHistory::SetGameObjectParent::SetGameObjectParent(EntityID _id, std::optional<EntityID> parentID) : Command(_id), prevParent(parentID) {}

void CommandHistory::SetGameObjectParent::Undo(ecs::ECS& ecs, CommandHistory* hist) {
	EntityID currentID = hist->GetCurrentID(id);
	auto currentParent = ecs.GetParent(currentID);
	prevParent.has_value() ? 
		ecs.SetParent(hist->GetCurrentID(prevParent.value()), currentID, true) : 
		ecs.RemoveParent(currentID, true);
	prevParent = currentParent;
}

void CommandHistory::SetGameObjectParent::Redo(ecs::ECS& ecs, CommandHistory* hist) {
	EntityID currentID = hist->GetCurrentID(id);
	auto currentParent = ecs.GetParent(currentID);
	prevParent.has_value() ?
		ecs.SetParent(hist->GetCurrentID(prevParent.value()), currentID, true) : 
		ecs.RemoveParent(currentID, true);
	prevParent = currentParent;
}

CommandHistory::SetGameObjectActive::SetGameObjectActive(EntityID _id) : Command(_id) {}

void CommandHistory::SetGameObjectActive::Undo(ecs::ECS& ecs, CommandHistory* hist) {
	EntityID currentID = hist->GetCurrentID(id);
	auto* nc = ecs.GetComponent<NameComponent>(currentID);
	if (nc == nullptr) {
		return;
	}
	ecs.SetActive(currentID, nc->hide);
}

void CommandHistory::SetGameObjectActive::Redo(ecs::ECS& ecs, CommandHistory* hist) {
	EntityID currentID = hist->GetCurrentID(id);
	auto* nc = ecs.GetComponent<NameComponent>(currentID);
	if (nc == nullptr) {
		return;
	}
	ecs.SetActive(currentID, nc->hide);
}

template<typename T>
CommandHistory::SetComponentData<T>::SetComponentData(EntityID _id, int _argCount, T& old, T& current) : Command(_id), argCount(_argCount), oldData(old), curData(current) {}

template<typename T>
void CommandHistory::SetComponentData<T>::Undo(ecs::ECS& ecs, CommandHistory* hist) {
	EntityID currentID = hist->GetCurrentID(id);
	T* comp = ecs.GetComponent<T>(currentID);
	comp->member();
}

template<typename T>
void CommandHistory::SetComponentData<T>::Redo(ecs::ECS& ecs, CommandHistory* hist) {
	EntityID currentID = hist->GetCurrentID(id);
	auto* comp = ecs.GetComponent<T>(currentID);
}