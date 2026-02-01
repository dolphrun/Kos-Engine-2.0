// Stack of Commands
// Commands: 
// Entities Related
//		- Instead of Instantly deleting the Entity ID, move it to editor side?
//		- Place it back to the entity map, order might matter
//		- Only after the command list gets filled up then completely delete the command
//		- ALL WILL STORE ENTITY ID AS A BASE
//		- Deleting Commands will just be removed

// Component Editing
//		- Might be able to involve templates

#include "ECS/ECS.h"
#include <typeinfo>

class CommandHistory {
	ecs::ECS& m_ecs;
	scenes::SceneManager& m_sceneManager;

public:
	CommandHistory(ecs::ECS& ecs, scenes::SceneManager& sceneManager) : m_ecs(ecs), m_sceneManager(sceneManager){}

	void Init();
	void Update();
	void Clear();

	struct Command {
		Command(EntityID _id) : id{ _id } {}

		EntityID id;
		virtual void Undo(ecs::ECS& ecs, CommandHistory* hist) = 0;
		virtual void Redo(ecs::ECS& ecs, CommandHistory* hist) = 0;
	};

	class CommandWrapper {
	public: 
		CommandWrapper(std::shared_ptr<Command> obj) : command(std::move(obj)) {}
		std::shared_ptr<Command> Get() const { return command; }
	private:
		std::shared_ptr<Command> command;
	};

	// Data
	static std::stack<CommandWrapper> commandQueue;
	static std::stack<CommandWrapper> redoQueue;
	std::map<EntityID, EntityID> idRemapping;

	template<typename TCommand, typename... Args>
	void AddCommand(Args&&... args) {
		EntityID id = std::get<0>(std::forward_as_tuple(args...));
		LOGGING_INFO("Command Added: {} for Entity: [{}]", typeid(TCommand).name(), id);
		static_assert(std::is_base_of_v<Command, TCommand>, "TCommand must derive from Command");
		auto cmdPtr = std::make_shared<TCommand>(std::forward<Args>(args)...);
		commandQueue.push(CommandWrapper{ cmdPtr });

		if (redoQueue.size()) {
			//Clear Redo Stack
			std::stack<CommandWrapper> empty;
			redoQueue.swap(empty);
		}
	}

	void RegisterRemapID(EntityID original, EntityID newID);
	void DeleteRemapID(EntityID remappedID);
	EntityID GetCurrentID(EntityID originalID) const;

	struct AddGameObject : Command {
		AddGameObject(EntityID _id, std::string _scene);
		void Undo(ecs::ECS& ecs, CommandHistory* hist);
		void Redo(ecs::ECS& ecs, CommandHistory* hist);
		std::string sceneName;
	};

	//	- Delete
	//		- Undo -> Add Back or Duplicate Entity
	//		- Redo -> Call Delete Entity
	//		- Store Deleted Entity and Parent?
	//		- Destructor -> Delete Stored Entity for Command
	struct DeleteGameObject : Command {
		DeleteGameObject(EntityID _idm, std::string _scene, ecs::ECS& ecs,  CommandHistory* hist);
		void Undo(ecs::ECS& ecs, CommandHistory* hist);
		void Redo(ecs::ECS& ecs, CommandHistory* hist);
		std::string sceneName;
	};

	//	- Parenting
	//		- Undo -> Unparent
	//		- Redo -> Parent
	//		- Store ParentID
	struct SetGameObjectParent : Command {
		SetGameObjectParent(EntityID _id, std::optional<EntityID> parentID);
		void Undo(ecs::ECS& ecs, CommandHistory* hist);
		void Redo(ecs::ECS& ecs, CommandHistory* hist);
	private:
		std::optional<EntityID> prevParent;
	};

	//	- Set Actve
	//		- Store Status -> Hide or UnHide -> != currentStatus?
	struct SetGameObjectActive : Command {
		SetGameObjectActive(EntityID _id);
		void Undo(ecs::ECS& ecs, CommandHistory* hist);
		void Redo(ecs::ECS& ecs, CommandHistory* hist);
	};

	template<typename T>
	struct SetComponentData : Command {
		SetComponentData(EntityID _id, int _argCount, T& old, T& current);
		void Undo(ecs::ECS& ecs, CommandHistory* hist);
		void Redo(ecs::ECS& ecs, CommandHistory* hist);

	private:
		int argCount;
		T& oldData;
		T& curData;
	};
};