#include "CommandHistory.h"
#include "imgui.h"

std::deque<CommandHistory::CommandWrapper> CommandHistory::commandQueue;
std::deque<CommandHistory::CommandWrapper> CommandHistory::redoQueue;

static bool onSceneLoadedDel = false;

void CommandHistory::Init() {
	m_ecs.AddScene(CACHEDSCENE, SceneData());
	m_ecs.sceneMap[CACHEDSCENE].isPrefab = true;
	m_sceneManager.SetSceneActive(CACHEDSCENE, false);

	if (!onSceneLoadedDel) {
		m_sceneManager.onSceneLoaded.Add([this](SceneData Data) {	Clear(); });
		onSceneLoadedDel = true;
	}
}

void CommandHistory::Update() {
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
		if (ImGui::IsKeyPressed(ImGuiKey_Z)){
			if (commandQueue.empty()) return;
			auto cmd = commandQueue.back();
			commandQueue.pop_back();
			cmd.Get()->Undo(m_ecs, this);
			redoQueue.push_back(cmd);
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_Y)) {
			if (redoQueue.empty()) return;
			auto cmd = redoQueue.back();
			redoQueue.pop_back();
			cmd.Get()->Redo(m_ecs, this);
			commandQueue.push_back(cmd);
			if (commandQueue.size() > MAX_COMMANDS) {
				commandQueue.pop_front();
			}
		}
	}
}

void CommandHistory::Clear() {
	std::deque<CommandWrapper> empty1;
	std::deque<CommandWrapper> empty2;
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
	LOGGING_INFO("Remapped [{}] to [{}]", original, newID);
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
	ecs.SetActive(newID, false);
}

void CommandHistory::AddGameObject::Redo(ecs::ECS& ecs, CommandHistory* hist) {
	EntityID currentID = hist->GetCurrentID(id);
	EntityID newID = ecs.DuplicateEntity(currentID, sceneName);
	ecs.DeleteEntity(currentID);
	hist->RegisterRemapID(id, newID);
	ecs.SetActive(newID, true);
}

CommandHistory::DeleteGameObject::DeleteGameObject(EntityID _id, std::string _scene, ecs::ECS& ecs, CommandHistory* hist)
	: Command(_id), sceneName(_scene), parent(-1) {
	if (ecs.GetParent(_id).has_value()) {
		parent = ecs.GetParent(_id).value();
	}
	active = !ecs.GetComponent<ecs::NameComponent>(_id)->hide;
	ecs.RemoveParent(_id);
	EntityID newID = ecs.DuplicateEntity(_id, CACHEDSCENE);
	hist->RegisterRemapID(id, newID);
	ecs.SetActive(newID, false);
}

void CommandHistory::DeleteGameObject::Undo(ecs::ECS& ecs, CommandHistory* hist) {
	EntityID currentID = hist->GetCurrentID(id);
	EntityID newID = ecs.DuplicateEntity(currentID, sceneName);
	if (parent >= 0) {
		ecs.SetParent(parent, newID, false);
	}
	ecs.DeleteEntity(currentID);
	hist->RegisterRemapID(id, newID);
	ecs.SetActive(newID, active);
}

void CommandHistory::DeleteGameObject::Redo(ecs::ECS& ecs, CommandHistory* hist) {
	EntityID currentID = hist->GetCurrentID(id);
	if (ecs.GetParent(currentID).has_value()) {
		parent = ecs.GetParent(currentID).value();
	}
	ecs.RemoveParent(currentID);
	EntityID newID = ecs.DuplicateEntity(currentID, CACHEDSCENE);
	ecs.DeleteEntity(currentID);
	hist->RegisterRemapID(id, newID);
	ecs.SetActive(newID, false);
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