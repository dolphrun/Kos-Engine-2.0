#ifndef NAVMESH_H
#define NAVMESH_H

#include <cstdint>
#include "./DetourRecast/Recast.h"
#include "./DetourRecast/DetourNavMesh.h"
#include "./DetourRecast/DetourNavMeshBuilder.h"
#include "./DetourRecast/DetourNavMeshQuery.h"
#include "./DetourRecast/TileMesh.h"
#include "./ECS/ECS.h"
#include "./Resources/ResourceManager.h"
#include "Scene/SceneManager.h"

namespace ecs { class ECS; }
namespace scenes { class SceneManager; }
class ResourceManager;

inline unsigned int nextPow2(unsigned int v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

inline unsigned int ilog2(unsigned int v) {
	unsigned int r;
	unsigned int shift;
	r = (v > 0xffff) << 4; v >>= r;
	shift = (v > 0xff) << 3; v >>= shift; r |= shift;
	shift = (v > 0xf) << 2; v >>= shift; r |= shift;
	shift = (v > 0x3) << 1; v >>= shift; r |= shift;
	r |= (v >> 1);
	return r;
}

struct TileMeshConfig {
	// ----- Agent dimensions -----
	float cellSize = 0.3f;     // voxel size on XZ plane
	float cellHeight = 0.2f;   // voxel height (Y axis)
	float agentHeight = 2.0f;
	float agentRadius = 0.6f;
	float agentMaxClimb = 0.5f;
	float agentMaxSlope = 45.0f;

	// ----- Polygonization -----
	int regionMinSize = 8;     // small isolated regions removed
	int regionMergeSize = 20;
	int maxVertsPerPoly = 6;
	float detailSampleDist = 6.0f;
	float detailSampleMaxError = 1.0f;

	// ----- Tile configuration -----
	float tileSize = 32.f;    // world-space width/height of tile
	int tileCountX = 0;
	int tileCountY = 0;

	float bmin[3] = { 0,0,0 };
	float bmax[3] = { 0,0,0 };

	void Reset();
};

class NavMeshManager {
	ecs::ECS& m_ecs;
	ResourceManager& resourceManager;
	GraphicsManager& graphicsManager;
	scenes::SceneManager& sceneManager;
public:
	NavMeshManager(ecs::ECS& ecs, ResourceManager& rm, GraphicsManager& gm, scenes::SceneManager& sm) : m_ecs(ecs), resourceManager(rm), graphicsManager(gm), sceneManager(sm) {}
	void Init();
	void Update(float dt);
	void BuildRecastGeometry(std::string sceneName, std::shared_ptr<Sample_TileMesh>& tileMesh);
	void Build(std::string sceneName, std::shared_ptr<Sample_TileMesh> tileMesh);
	void SaveMesh(std::string sceneName);
	std::shared_ptr<Sample_TileMesh> LoadMesh(std::string fileName);
	void SetGraphicsRenderMesh(std::shared_ptr<Sample_TileMesh> tm);

	void AddAgent(int& agentID, EntityID entityID, glm::vec3 pos = glm::vec3(0), float radius = 0.6f, float height = 1.8f, float maxSpeed = 3.5f);
	void MoveAgent(int& agentID, const glm::vec3 targetPos);
	void RemoveAgent(int& agentID);

	TileMeshConfig cfg;
	BuildContext ctx;
	std::map<std::string, std::shared_ptr<Sample_TileMesh>> navMeshData;
	std::map<int, EntityID> agentData;

	// Tmp Sol for current active scene
	std::string currentScene;
};

// Plan: 
// Tools wise only need 
// - Prune NavMesh
// - Create Off-Mesh Links

#endif