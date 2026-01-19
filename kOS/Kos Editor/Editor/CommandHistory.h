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

namespace CommandHistory{
	using namespace ecs;

	static std::stack<Command> commandQueue;

	struct Command {
		Command(EntityID _id) : id{ _id } {}

		EntityID id;
		virtual void Undo() = 0;
		virtual void Redo() = 0;
	};

	//	- Add
	//		- Undo -> Call DeleteEntity
	//		- Redo -> Create Entity
	//		- Destructor -> Nothing
	struct AddGameObject : Command{
		AddGameObject(EntityID _id);
		void Undo();
		void Redo();
	};

	//	- Dup
	//		- Undo -> Call DeleteEntity
	//		- Redo -> DuplicateEntity
	//		- Destructor -> Nothing
	struct DuplicateGameObject : Command {
		DuplicateGameObject(EntityID _id);
		void Undo();
		void Redo();
	};
	
	//	- Delete
	//		- Undo -> Add Back or Duplicate Entity
	//		- Redo -> Call Delete Entity
	//		- Store Deleted Entity and Parent?
	//		- Destructor -> Delete Stored Entity for Command
	struct DeleteGameObject : Command {
		DeleteGameObject(EntityID _id);
		void Undo();
		void Redo();
	};

	//	- Parenting
	//		- Undo -> Unparent
	//		- Redo -> Parent
	//		- Store ParentID
	struct SetGameObjectParent : Command {
		SetGameObjectParent(EntityID _id, EntityID parentId);
		void Undo();
		void Redo();
	private:
		EntityID prevParent;
	};

	//	- Unparenting
	//		- Undo -> Parent
	//		- Redo -> Unparent
	//		- Store ParentID 
	struct UnparentGameObject : Command {
		UnparentGameObject(EntityID _id);
		void Undo();
		void Redo();
	private:
		EntityID prevParent;
	};

	//	- Set Actve
	//		- Store Status -> Hide or UnHide -> != currentStatus?
	struct SetGameObjectActive : Command {
		SetGameObjectActive(EntityID _id);
		void Undo();
		void Redo();
	};
}