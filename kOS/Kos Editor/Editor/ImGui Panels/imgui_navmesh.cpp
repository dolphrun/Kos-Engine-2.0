#include "Editor.h"
#include "ECS/ECS.h"
#include "Resources/ResourceManager.h"
#include "Pathfinding/NavMesh.h"
#include "DetourRecast/Recast.h"
#include "../Include/DetourRecast/RecastDebugDraw.h"

std::shared_ptr<Sample_TileMesh> tm = nullptr;

void gui::ImGuiHandler::DrawNavMeshWindow() {
	if (ImGui::Begin("NavMesh")) {

		const char* sampleType[3]{ "---", "Tile Mesh", "Solo Mesh" };
		static int selected = 1;
		if (ImGui::Combo(sampleType[selected], &selected, sampleType, 3)) {
			// Only allow TileMesh for now
		}

		auto iter = m_navMeshManager.navMeshData.find(m_activeScene);
		if (iter == m_navMeshManager.navMeshData.end()) {
			if (tm == nullptr) {
				tm = std::make_shared<Sample_TileMesh>();
			}
		}
		else {
			tm = iter->second;
		}

		ImGui::SeparatorText("Rasterization");
		ImGui::SliderFloat("Cell Size", &tm->m_cellSize, 0.1f, 1.0f);
		ImGui::SliderFloat("Cell Height", &tm->m_cellHeight, 0.1f, 1.0f);
		if (tm->m_geom)
		{
			const float* bmin = tm->m_geom->getNavMeshBoundsMin();
			const float* bmax = tm->m_geom->getNavMeshBoundsMax();
			int gw = 0, gh = 0;
			rcCalcGridSize(bmin, bmax, tm->m_cellSize, &gw, &gh);
			ImGui::Text("Voxels  %d x %d", gw, gh);
		}

		ImGui::SeparatorText("Agent");
		ImGui::SliderFloat("Height", &tm->m_agentHeight, 0.1f, 8.f);
		ImGui::SliderFloat("Radius", &tm->m_agentRadius, 0.0f, 5.0f);
		ImGui::SliderFloat("Max Climb", &tm->m_agentMaxClimb, 0.1f, 5.0f);
		ImGui::SliderFloat("Max Slope", &tm->m_agentMaxSlope, 0.0f, 90.0f);

		ImGui::SeparatorText("Region");
		ImGui::SliderFloat("Min Region Size", &tm->m_regionMinSize, 0.0f, 150.0f);
		ImGui::SliderFloat("Merged Region Size", &tm->m_regionMergeSize, 0.0f, 150.0f);

		ImGui::SeparatorText("Partitioning");
		ImGui::CheckboxFlags("Watershed", &tm->m_partitionType, SAMPLE_PARTITION_WATERSHED);
		ImGui::CheckboxFlags("Monotone", &tm->m_partitionType, SAMPLE_PARTITION_MONOTONE);
		ImGui::CheckboxFlags("Layers", &tm->m_partitionType, SAMPLE_PARTITION_LAYERS);

		ImGui::SeparatorText("Filtering");
		ImGui::Checkbox("Low Hanging Obstacles", &tm->m_filterLowHangingObstacles);
		ImGui::Checkbox("Ledge Spans", &tm->m_filterLedgeSpans);
		ImGui::Checkbox("Walkable Low Height Spans", &tm->m_filterWalkableLowHeightSpans);

		ImGui::SeparatorText("Polygonization");
		ImGui::SliderFloat("Max Edge Length", &tm->m_edgeMaxLen, 0.0f, 50.0f);
		ImGui::SliderFloat("Max Edge Error", &tm->m_edgeMaxError, 0.1f, 3.0f);
		ImGui::SliderFloat("Verts Per Poly", &tm->m_vertsPerPoly, 3.0f, 12.0f);

		ImGui::SeparatorText("Detail Mesh");
		ImGui::SliderFloat("Sample Distance", &tm->m_detailSampleDist, 0.0f, 16.0f);
		ImGui::SliderFloat("Max Sample Error", &tm->m_detailSampleMaxError, 0.0f, 16.0f);

		ImGui::Checkbox("Keep Intermediate Results", &tm->m_keepInterResults);
		ImGui::Checkbox("Build All Tiles", &tm->m_buildAll);

		if (ImGui::SliderFloat("TileSize", &tm->m_tileSize, 16.0f, 1024.0f)) {
			constexpr int stepSize = 16;
			tm->m_tileSize = round(tm->m_tileSize / 16) * 16;
		}

		if (tm->m_geom) {
			int gw = 0, gh = 0;
			const float* bmin = tm->m_geom->getNavMeshBoundsMin();
			const float* bmax = tm->m_geom->getNavMeshBoundsMax();
			rcCalcGridSize(bmin, bmax, tm->m_cellSize, &gw, &gh);
			const int ts = (int)tm->m_tileSize;
			const int tw = (gw + ts - 1) / ts;
			const int th = (gh + ts - 1) / ts;
			ImGui::Text("Tiles  %d x %d", tw, th);

			// Max tiles and max polys affect how the tile IDs are calculated.
			// There are 22 bits available for identifying a tile and a polygon.
			int tileBits = rcMin((int)ilog2(nextPow2(tw * th)), 14);
			if (tileBits > 14) tileBits = 14;
			int polyBits = 22 - tileBits;
			tm->m_maxTiles = 1 << tileBits;
			tm->m_maxPolysPerTile = 1 << polyBits;
			ImGui::Text("Max Tiles  %d", tm->m_maxTiles);
			ImGui::Text("Max Polys  %d", tm->m_maxPolysPerTile);
		}

		ImGui::SeparatorText("Navmesh Generation Controls");

		//renderNavMeshStatus = (tm != nullptr);
		if (ImGui::Checkbox("Render Navmesh", &renderNavMeshStatus)) {
			m_navMeshManager.SetGraphicsRenderMesh(renderNavMeshStatus ? tm : nullptr);
		}

		ImVec2 buttonSize(ImGui::GetContentRegionAvail().x, 0.f);

		if (ImGui::Button("Build Geom", buttonSize)) {
			m_navMeshManager.Build(m_activeScene, tm);
		}

		if (ImGui::Button("Save Mesh", buttonSize)) {
			m_navMeshManager.SaveMesh(m_assetManager.GetAssetManagerDirectory() + "//NavMesh", m_activeScene);
			std::filesystem::path metaPath = m_assetManager.GetAssetManagerDirectory() + "//NavMesh" + '\\' + m_activeScene + ".navmesh.meta";
			AssetData assetData = serialization::ReadJsonFile<AssetData>(metaPath.string());
			const auto& scene = m_ecs.sceneMap.find(m_activeScene);
			scene->second.NavMeshGuid = assetData.GUID;
		}

		if (ImGui::Button("Load Mesh", buttonSize)) {
			tm = m_navMeshManager.LoadMesh(m_activeScene, m_ecs.sceneMap.at(m_activeScene).NavMeshGuid);
		}
	}


	ImGui::End();
}

void gui::ImGuiHandler::SetNavMeshRenderMesh() {
	m_navMeshManager.SetGraphicsRenderMesh(renderNavMeshStatus ? tm : nullptr);
}