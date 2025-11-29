#include <vector>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <glm/glm.hpp>

#include "NavMesh.h"
#include "ECS/ECS.h"
#include "../Include/DetourRecast/Recast.h"
#include "../Include/DetourRecast/RecastDebugDraw.h"
#include "../Include/DetourRecast/InputGeom.h"
#include "../Include/DetourRecast/DetourCrowd.h"

// NOTE: This .cpp aims to be a practical, drop-in implementation for the
// TileMesh.h header provided earlier. It uses a simplified approach for
// extracting geometry per-tile (triangle AABB test) and runs the standard
// Recast pipeline to produce a single tile's navmesh data which is then
// added to the dtNavMesh as a tile.
//
// It does not attempt advanced spatial acceleration for triangle queries
// (like BVH) ÅEfor large world meshes you should swap-in a faster structure.

// Helper macro for logging (you can replace with your engine logger)
#define TM_LOG(msg) std::cout << "[TileMesh] " << msg << std::endl

void TileMeshConfig::Reset() {
    cellSize = 0.3f;
    cellHeight = 0.2f;
    agentHeight = 2.0f;
    agentRadius = 0.6f;
    agentMaxClimb = 0.5f;
    agentMaxSlope = 45.0f;

    regionMinSize = 8;
    regionMergeSize = 20;
    maxVertsPerPoly = 6;
    detailSampleDist = 6.0f;
    detailSampleMaxError = 1.0f;

    tileSize = 32.0f;
    tileCountX = 0;
    tileCountY = 0;

    for (int i = 0; i < 3; i++) {
        bmin[i] = 0;
        bmax[i] = 0;
    }
}
//
//InputGeometry::InputGeometry(const InputGeometry& copy) {
//    nverts = copy.nverts;
//    ntris = copy.ntris;
//    if (copy.verts != nullptr && copy.nverts > 0) {
//        float* new_verts = new float[nverts * 3];
//        std::copy(copy.verts, copy.verts + nverts * 3, new_verts);
//        verts = new_verts;
//    }
//    if (copy.tris != nullptr && copy.ntris > 0) {
//        int* new_tris = new int[ntris * 3];
//        std::copy(copy.tris, copy.tris + ntris * 3, new_tris);
//        tris = new_tris;
//    }
//}
//
//InputGeometry::~InputGeometry() {
//    //if(verts)
//    //    delete[] const_cast<float*>(verts);
//    //if(tris)
//    //    delete[] const_cast<int*>(tris);
//
//    verts = nullptr;
//    tris = nullptr;
//}
//
//TileMesh::TileMesh() : m_navMesh(nullptr), m_navQuery(nullptr), m_ctx(nullptr),
//    m_solid(nullptr), m_chf(nullptr), m_cset(nullptr), m_pmesh(nullptr), m_dmesh(nullptr),
//    m_triareas(nullptr)
//{
//    m_ctx = new BuildContext;
//}
//
//TileMesh::TileMesh(InputGeometry geom) : m_navMesh(nullptr), m_navQuery(nullptr), 
//    m_ctx(nullptr) , m_solid(nullptr), m_chf(nullptr), m_cset(nullptr), m_pmesh(nullptr), m_dmesh(nullptr),
//    m_triareas(nullptr)
//{
//    m_geom = geom;
//    m_ctx = new BuildContext;
//}
//
//TileMesh::~TileMesh()
//{
//    // free detour objects
//    //if (m_navQuery)
//    //{
//    //    dtFreeNavMeshQuery(m_navQuery);
//    //    m_navQuery = nullptr;
//    //}
//    if (m_navMesh)
//    {
//        dtFreeNavMesh(m_navMesh);
//        m_navMesh = nullptr;
//    }
//
//    CleanupRecast();
//
//    //if (m_ctx)
//    //{
//    //    delete m_ctx;
//    //    m_ctx = nullptr;
//    //}
//}
//
//bool TileMesh::Init(const TileMeshConfig& cfg)
//{
//    m_cfg = cfg;
//
//    // Sanity checks
//    if (m_cfg.tileSize <= 0.0f)
//    {
//        TM_LOG("tileSize must be > 0");
//        return false;
//    }
//
//    // Determine tile counts from bounds
//    const float worldWidth = m_cfg.bmax[0] - m_cfg.bmin[0];
//    const float worldDepth = m_cfg.bmax[2] - m_cfg.bmin[2];
//    TM_LOG("WorldSize X=" << worldWidth << " Z=" << worldDepth);
//
//    m_cfg.tileCountX = std::max(1, (int)std::ceil(worldWidth / m_cfg.tileSize));
//    m_cfg.tileCountY = std::max(1, (int)std::ceil(worldDepth / m_cfg.tileSize));
//
//    TM_LOG("Init: tiles X=" << m_cfg.tileCountX << " Y=" << m_cfg.tileCountY);
//
//    // Create an empty navmesh with tile capability
//    // We need to pick a maxTiles and maxPolysPerTile. We'll use reasonable defaults
//    // based on tileSize and cell size. These are conservative and can be tuned.
//
//    const int maxTiles = m_cfg.tileCountX * m_cfg.tileCountY * 4; // buffer factor
//    const int maxPolysPerTile = 512; // typical default
//
//    // Allocate and init navmesh
//    m_navMesh = dtAllocNavMesh();
//    if (!m_navMesh)
//    {
//        TM_LOG("Failed to allocate dtNavMesh");
//        return false;
//    }
//
//    dtNavMeshParams params;
//    memset(&params, 0, sizeof(params));
//    params.orig[0] = m_cfg.bmin[0];
//    params.orig[1] = m_cfg.bmin[1];
//    params.orig[2] = m_cfg.bmin[2];
//    params.tileWidth = m_cfg.tileSize;
//    params.tileHeight = m_cfg.tileSize;
//    params.maxTiles = maxTiles;
//    params.maxPolys = maxPolysPerTile;
//
//    if (dtStatusFailed(m_navMesh->init(&params)))
//    {
//        TM_LOG("dtNavMesh::init failed");
//        dtFreeNavMesh(m_navMesh);
//        m_navMesh = nullptr;
//        return false;
//    }
//
//    // NavMeshQuery
//    m_navQuery = dtAllocNavMeshQuery();
//    if (!m_navQuery)
//    {
//        TM_LOG("Failed to allocate dtNavMeshQuery");
//        return false;
//    }
//    if (dtStatusFailed(m_navQuery->init(m_navMesh, 2048)))
//    {
//        TM_LOG("dtNavMeshQuery::init failed");
//        dtFreeNavMeshQuery(m_navQuery);
//        m_navQuery = nullptr;
//        return false;
//    }
//
//    TM_LOG("Navmesh initialized");
//    return true;
//}
//
//void TileMesh::SetGeometry(const InputGeometry& geom)
//{
//    m_geom = geom;
//}
//
//bool TileMesh::BuildAllTiles()
//{
//    if (!m_navMesh)
//    {
//        TM_LOG("NavMesh not initialized. Call init() first.");
//        return false;
//    }
//
//    bool ok = true;
//    for (int y = 0; y < m_cfg.tileCountY; ++y)
//    {
//        for (int x = 0; x < m_cfg.tileCountX; ++x)
//        {
//            if (!BuildTile(x, y))
//            {
//                TM_LOG("Failed to build tile: " << x << "," << y);
//                ok = false; // continue building other tiles
//            }
//        }
//    }
//    return ok;
//}
//
//bool TileMesh::BuildTile(int tx, int ty)
//{
//    if (!m_navMesh)
//    {
//        TM_LOG("NavMesh not initialized");
//        return false;
//    }
//
//    unsigned char* data = nullptr;
//    int dataSize = 0;
//
//    if (!BuildTileMesh(tx, ty, data, dataSize))
//    {
//        TM_LOG("buildTileMesh failed for tile " << tx << "," << ty);
//        if (data)
//            dtFree(data);
//        return false;
//    }
//
//    if (!data || dataSize <= 0)
//    {
//        TM_LOG("No navmesh data produced for tile " << tx << "," << ty);
//        return false;
//    }
//
//    // Add tile to navmesh. Use DT_TILE_FREE_DATA so navmesh owns the buffer.
//    dtStatus status = m_navMesh->addTile(data, dataSize, DT_TILE_FREE_DATA, 0, 0);
//    if (status < 0)
//    {
//        TM_LOG("dtNavMesh::addTile failed with code " << status);
//        // If addTile fails and didn't take ownership, free the data
//        // but spec says DT_TILE_FREE_DATA makes navmesh free it.
//        return false;
//    }
//
//    TM_LOG("Added tile " << tx << "," << ty << " size=" << dataSize);
//    return true;
//}
//
//bool TileMesh::RebuildTile(int tx, int ty)
//{
//    // remove tile if exists and rebuild
//    if (!m_navMesh) return false;
//
//    // compute tile reference
//    int txCount = m_cfg.tileCountX;
//    int tyCount = m_cfg.tileCountY;
//    if (tx < 0 || tx >= txCount || ty < 0 || ty >= tyCount) return false;
//
//    // Compute tile's tileRef by position ÅEdetour can find tile by location
//    float bmin[3], bmax[3];
//    GetTileBounds(tx, ty, bmin, bmax);
//
//    // remove tiles overlapping this bounds
//    m_navMesh->removeTile(m_navMesh->getTileRefAt(tx, ty, 0), nullptr, 0);
//
//    return BuildTile(tx, ty);
//}
//
//void TileMesh::WorldToTile(const float* pos, int& tx, int& ty) const
//{
//    const float relX = pos[0] - m_cfg.bmin[0];
//    const float relZ = pos[2] - m_cfg.bmin[2];
//    tx = (int)std::floor(relX / m_cfg.tileSize);
//    ty = (int)std::floor(relZ / m_cfg.tileSize);
//    tx = std::clamp(tx, 0, m_cfg.tileCountX - 1);
//    ty = std::clamp(ty, 0, m_cfg.tileCountY - 1);
//}
//
//void TileMesh::GetTileBounds(int tx, int ty, float* bmin, float* bmax) const
//{
//    const float minx = m_cfg.bmin[0] + tx * m_cfg.tileSize;
//    const float minz = m_cfg.bmin[2] + ty * m_cfg.tileSize;
//    const float maxx = minx + m_cfg.tileSize;
//    const float maxz = minz + m_cfg.tileSize;
//
//    // maintain world Y bounds
//    bmin[0] = minx;
//    bmin[1] = m_cfg.bmin[1];
//    bmin[2] = minz;
//
//    bmax[0] = maxx;
//    bmax[1] = m_cfg.bmax[1];
//    bmax[2] = maxz;
//}
//
//void TileMesh::RenderNavMesh() {
//    //if (!m_geom || !m_geom->getMesh())
//    //    return;
//
//    //const float texScale = 1.0f / (m_cfg.cellSize * 10.0f);
//
//    //// Draw mesh
//    //if (m_drawMode != DRAWMODE_NAVMESH_TRANS)
//    //{
//    //    // Draw mesh
//    //    duDebugDrawTriMeshSlope(&m_dd, m_geom->getMesh()->getVerts(), m_geom->getMesh()->getVertCount(),
//    //        m_geom->getMesh()->getTris(), m_geom->getMesh()->getNormals(), m_geom->getMesh()->getTriCount(),
//    //        m_agentMaxSlope, texScale);
//    //    m_geom->drawOffMeshConnections(&m_dd);
//    //}
//
//    //glDepthMask(GL_FALSE);
//
//    //// Draw bounds
//    //const float* bmin = m_geom->getNavMeshBoundsMin();
//    //const float* bmax = m_geom->getNavMeshBoundsMax();
//    //duDebugDrawBoxWire(&m_dd, bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2], duRGBA(255, 255, 255, 128), 1.0f);
//
//    //// Tiling grid.
//    //int gw = 0, gh = 0;
//    //rcCalcGridSize(bmin, bmax, m_cellSize, &gw, &gh);
//    //const int tw = (gw + (int)m_tileSize - 1) / (int)m_tileSize;
//    //const int th = (gh + (int)m_tileSize - 1) / (int)m_tileSize;
//    //const float s = m_tileSize * m_cellSize;
//    //duDebugDrawGridXZ(&m_dd, bmin[0], bmin[1], bmin[2], tw, th, s, duRGBA(0, 0, 0, 64), 1.0f);
//
//    //// Draw active tile
//    //duDebugDrawBoxWire(&m_dd, m_lastBuiltTileBmin[0], m_lastBuiltTileBmin[1], m_lastBuiltTileBmin[2],
//    //    m_lastBuiltTileBmax[0], m_lastBuiltTileBmax[1], m_lastBuiltTileBmax[2], m_tileCol, 1.0f);
//
//    //if (m_navMesh && m_navQuery &&
//    //    (m_drawMode == DRAWMODE_NAVMESH ||
//    //        m_drawMode == DRAWMODE_NAVMESH_TRANS ||
//    //        m_drawMode == DRAWMODE_NAVMESH_BVTREE ||
//    //        m_drawMode == DRAWMODE_NAVMESH_NODES ||
//    //        m_drawMode == DRAWMODE_NAVMESH_PORTALS ||
//    //        m_drawMode == DRAWMODE_NAVMESH_INVIS))
//    //{
//    //    if (m_drawMode != DRAWMODE_NAVMESH_INVIS)
//    //        duDebugDrawNavMeshWithClosedList(&m_dd, *m_navMesh, *m_navQuery, m_navMeshDrawFlags);
//    //    if (m_drawMode == DRAWMODE_NAVMESH_BVTREE)
//    //        duDebugDrawNavMeshBVTree(&m_dd, *m_navMesh);
//    //    if (m_drawMode == DRAWMODE_NAVMESH_PORTALS)
//    //        duDebugDrawNavMeshPortals(&m_dd, *m_navMesh);
//    //    if (m_drawMode == DRAWMODE_NAVMESH_NODES)
//    //        duDebugDrawNavMeshNodes(&m_dd, *m_navQuery);
//    //    duDebugDrawNavMeshPolysWithFlags(&m_dd, *m_navMesh, SAMPLE_POLYFLAGS_DISABLED, duRGBA(0, 0, 0, 128));
//    //}
//
//
//    //glDepthMask(GL_TRUE);
//
//    //if (m_chf && m_drawMode == DRAWMODE_COMPACT)
//    //    duDebugDrawCompactHeightfieldSolid(&m_dd, *m_chf);
//
//    //if (m_chf && m_drawMode == DRAWMODE_COMPACT_DISTANCE)
//    //    duDebugDrawCompactHeightfieldDistance(&m_dd, *m_chf);
//    //if (m_chf && m_drawMode == DRAWMODE_COMPACT_REGIONS)
//    //    duDebugDrawCompactHeightfieldRegions(&m_dd, *m_chf);
//    //if (m_solid && m_drawMode == DRAWMODE_VOXELS)
//    //{
//    //    glEnable(GL_FOG);
//    //    duDebugDrawHeightfieldSolid(&m_dd, *m_solid);
//    //    glDisable(GL_FOG);
//    //}
//    //if (m_solid && m_drawMode == DRAWMODE_VOXELS_WALKABLE)
//    //{
//    //    glEnable(GL_FOG);
//    //    duDebugDrawHeightfieldWalkable(&m_dd, *m_solid);
//    //    glDisable(GL_FOG);
//    //}
//
//    //if (m_cset && m_drawMode == DRAWMODE_RAW_CONTOURS)
//    //{
//    //    glDepthMask(GL_FALSE);
//    //    duDebugDrawRawContours(&m_dd, *m_cset);
//    //    glDepthMask(GL_TRUE);
//    //}
//
//    //if (m_cset && m_drawMode == DRAWMODE_BOTH_CONTOURS)
//    //{
//    //    glDepthMask(GL_FALSE);
//    //    duDebugDrawRawContours(&m_dd, *m_cset, 0.5f);
//    //    duDebugDrawContours(&m_dd, *m_cset);
//    //    glDepthMask(GL_TRUE);
//    //}
//    //if (m_cset && m_drawMode == DRAWMODE_CONTOURS)
//    //{
//    //    glDepthMask(GL_FALSE);
//    //    duDebugDrawContours(&m_dd, *m_cset);
//    //    glDepthMask(GL_TRUE);
//    //}
//    //if (m_chf && m_cset && m_drawMode == DRAWMODE_REGION_CONNECTIONS)
//    //{
//    //    duDebugDrawCompactHeightfieldRegions(&m_dd, *m_chf);
//
//    //    glDepthMask(GL_FALSE);
//    //    duDebugDrawRegionConnections(&m_dd, *m_cset);
//    //    glDepthMask(GL_TRUE);
//    //}
//    //if (m_pmesh && m_drawMode == DRAWMODE_POLYMESH)
//    //{
//    //    glDepthMask(GL_FALSE);
//    //    duDebugDrawPolyMesh(&m_dd, *m_pmesh);
//    //    glDepthMask(GL_TRUE);
//    //}
//    //if (m_dmesh && m_drawMode == DRAWMODE_POLYMESH_DETAIL)
//    //{
//    //    glDepthMask(GL_FALSE);
//    //    duDebugDrawPolyMeshDetail(&m_dd, *m_dmesh);
//    //    glDepthMask(GL_TRUE);
//    //}
//
//    //m_geom->drawConvexVolumes(&m_dd);
//
//    //if (m_tool)
//    //    m_tool->handleRender();
//    //renderToolStates();
//
//    //glDepthMask(GL_TRUE);
//}
//
//// ---------------------------
//// Internal helpers
//// ---------------------------
//
//// Build a tile mesh: gather geometry in tile bounds, run Recast pipeline,
//// and produce Detour navmesh data buffer (navData/navDataSize). The
//// caller is responsible for adding the tile to dtNavMesh.
//
//bool TileMesh::BuildTileMesh(int tx, int ty, unsigned char*& outData, int& outDataSize)
//{
//    outData = nullptr;
//    outDataSize = 0;
//
//    if (!m_geom.verts || !m_geom.tris || m_geom.nverts <= 0 || m_geom.ntris <= 0)
//    {
//        TM_LOG("No geometry set for building tiles");
//        return false;
//    }
//
//    // 1) compute tile bounds in world-space
//    float tbmin[3], tbmax[3];
//    GetTileBounds(tx, ty, tbmin, tbmax);
//
//    // Expand bounds slightly to avoid missing geometry on tile borders
//    const float border = m_cfg.tileSize * 0.1f; // 10% border
//    tbmin[0] -= border; tbmin[2] -= border;
//    tbmax[0] += border; tbmax[2] += border;
//
//    // 2) collect triangles whose AABB intersects the tile bounds
//    // We'll build a remapped vertex/index list for local processing.
//
//    std::vector<float> verts;
//    std::vector<int> tris;
//    verts.reserve(1024);
//    tris.reserve(1024);
//
//    // map from original vertex index to new local index
//    std::vector<int> vmap(m_geom.nverts, -1);
//
//    for (int i = 0; i < m_geom.ntris; ++i)
//    {
//        int i0 = m_geom.tris[i * 3 + 0];
//        int i1 = m_geom.tris[i * 3 + 1];
//        int i2 = m_geom.tris[i * 3 + 2];
//
//        assert(i0 >= 0 && i0 < m_geom.nverts);
//        assert(i1 >= 0 && i1 < m_geom.nverts);
//        assert(i2 >= 0 && i2 < m_geom.nverts);
//
//        const float* v0 = &m_geom.verts[i0 * 3];
//        const float* v1 = &m_geom.verts[i1 * 3];
//        const float* v2 = &m_geom.verts[i2 * 3];
//
//        // triangle AABB
//        float tminx = std::min({ v0[0], v1[0], v2[0] });
//        float tminy = std::min({ v0[1], v1[1], v2[1] });
//        float tminz = std::min({ v0[2], v1[2], v2[2] });
//        float tmaxx = std::max({ v0[0], v1[0], v2[0] });
//        float tmaxy = std::max({ v0[1], v1[1], v2[1] });
//        float tmaxz = std::max({ v0[2], v1[2], v2[2] });
//
//        // AABB vs tile bounds test
//        if (tmaxx < tbmin[0] || tminx > tbmax[0] ||
//            tmaxz < tbmin[2] || tminz > tbmax[2])
//        {
//            continue; // triangle outside
//        }
//
//        // include triangle
//        int idxs[3];
//        int vertsAdded = 0;
//        int triIdxBase = (int)verts.size() / 3;
//
//        for (int k = 0; k < 3; ++k)
//        {
//            int orig = (k == 0) ? i0 : ((k == 1) ? i1 : i2);
//            int mapped = vmap[orig];
//            if (mapped == -1)
//            {
//                mapped = (int)verts.size() / 3;
//                verts.push_back(m_geom.verts[orig * 3 + 0]);
//                verts.push_back(m_geom.verts[orig * 3 + 1]);
//                verts.push_back(m_geom.verts[orig * 3 + 2]);
//                vmap[orig] = mapped;
//            }
//            idxs[k] = mapped;
//        }
//
//        tris.push_back(idxs[0]);
//        tris.push_back(idxs[1]);
//        tris.push_back(idxs[2]);
//    }
//
//    if (verts.empty() || tris.empty())
//    {
//        // no geometry for this tile -> it's allowed. return empty (no nav) but
//        // caller expects false so it won't add an empty tile.
//        TM_LOG("Tile " << tx << "," << ty << " has no geometry. skipping.");
//        return false;
//    }
//
//    // 3) Setup Recast config for this tile
//    rcConfig cfg{};
//    cfg.cs = m_cfg.cellSize;
//    cfg.ch = m_cfg.cellHeight;
//    cfg.walkableSlopeAngle = m_cfg.agentMaxSlope;
//    cfg.walkableHeight = (int)std::ceil(m_cfg.agentHeight / cfg.ch);
//    cfg.walkableRadius = (int)std::ceil(m_cfg.agentRadius / cfg.cs);
//    cfg.walkableClimb = (int)std::floor(m_cfg.agentMaxClimb / cfg.ch);
//    cfg.maxEdgeLen = (int)(12.0f / cfg.cs);
//    cfg.maxSimplificationError = 1.3f;
//    cfg.minRegionArea = (int)rcSqr(m_cfg.regionMinSize);
//    cfg.mergeRegionArea = (int)rcSqr(m_cfg.regionMergeSize);
//    cfg.maxVertsPerPoly = m_cfg.maxVertsPerPoly;
//    cfg.detailSampleDist = m_cfg.detailSampleDist * cfg.cs;
//    cfg.detailSampleMaxError = m_cfg.detailSampleMaxError;
//
//    // bounds: use tile bounds bmin/bmax; but also ensure Y bounds are correct
//    cfg.bmin[0] = tbmin[0];
//    cfg.bmin[1] = m_cfg.bmin[1];
//    cfg.bmin[2] = tbmin[2];
//    cfg.bmax[0] = tbmax[0];
//    cfg.bmax[1] = m_cfg.bmax[1];
//    cfg.bmax[2] = tbmax[2];
//
//    // grid size
//    rcCalcGridSize(cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height);
//
//    // 4) run Recast pipeline
//    CleanupRecast();
//
//    m_solid = rcAllocHeightfield();
//    if (!m_solid)
//    {
//        TM_LOG("rcAllocHeightfield failed");
//        return false;
//    }
//    if (!rcCreateHeightfield(m_ctx, *m_solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch))
//    {
//        TM_LOG("rcCreateHeightfield failed");
//        CleanupRecast();
//        return false;
//    }
//
//    // Build triangle areas: mark walkable by slope
//    std::vector<unsigned char> triAreas((int)tris.size() / 3);
//    for (size_t i = 0; i < triAreas.size(); ++i) triAreas[i] = RC_WALKABLE_AREA;
//
//    // rcMarkWalkableTriangles expects the entire vertex array; pass local arrays
//    rcMarkWalkableTriangles(m_ctx, cfg.walkableSlopeAngle,
//        verts.data(), (int)verts.size() / 3,
//        tris.data(), (int)tris.size() / 3, triAreas.data());
//
//    if (!rcRasterizeTriangles(m_ctx, verts.data(), (int)verts.size() / 3,
//        tris.data(), triAreas.data(), (int)tris.size() / 3,
//        *m_solid, cfg.walkableClimb))
//    {
//        TM_LOG("rcRasterizeTriangles failed");
//        CleanupRecast();
//        return false;
//    }
//
//    //rcFilterLowHangingWalkableObstacles(m_ctx, cfg.walkableClimb, *m_solid);
//    //rcFilterLedgeSpans(m_ctx, cfg.walkableHeight, cfg.walkableClimb, *m_solid);
//    //rcFilterWalkableLowHeightSpans(m_ctx, cfg.walkableHeight, *m_solid);
//
//    m_chf = rcAllocCompactHeightfield();
//    if (!m_chf)
//    {
//        TM_LOG("rcAllocCompactHeightfield failed");
//        CleanupRecast();
//        return false;
//    }
//
//    if (!rcBuildCompactHeightfield(m_ctx, cfg.walkableHeight, cfg.walkableClimb, *m_solid, *m_chf))
//    {
//        TM_LOG("rcBuildCompactHeightfield failed");
//        CleanupRecast();
//        return false;
//    }
//
//    if (!rcErodeWalkableArea(m_ctx, cfg.walkableRadius, *m_chf))
//    {
//        TM_LOG("rcErodeWalkableArea failed");
//        CleanupRecast();
//        return false;
//    }
//
//    if (!rcBuildDistanceField(m_ctx, *m_chf))
//    {
//        TM_LOG("rcBuildDistanceField failed");
//        CleanupRecast();
//        return false;
//    }
//
//    if (!rcBuildRegions(m_ctx, *m_chf, 0, cfg.minRegionArea, cfg.mergeRegionArea))
//    {
//        TM_LOG("rcBuildRegions failed");
//        CleanupRecast();
//        return false;
//    }
//
//    m_cset = rcAllocContourSet();
//    if (!m_cset)
//    {
//        TM_LOG("rcAllocContourSet failed");
//        CleanupRecast();
//        return false;
//    }
//
//    if (!rcBuildContours(m_ctx, *m_chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *m_cset))
//    {
//        TM_LOG("rcBuildContours failed");
//        CleanupRecast();
//        return false;
//    }
//
//    m_pmesh = rcAllocPolyMesh();
//    if (!m_pmesh)
//    {
//        TM_LOG("rcAllocPolyMesh failed");
//        CleanupRecast();
//        return false;
//    }
//
//    if (!rcBuildPolyMesh(m_ctx, *m_cset, cfg.maxVertsPerPoly, *m_pmesh))
//    {
//        TM_LOG("rcBuildPolyMesh failed");
//        CleanupRecast();
//        return false;
//    }
//
//    m_dmesh = rcAllocPolyMeshDetail();
//    if (!m_dmesh)
//    {
//        TM_LOG("rcAllocPolyMeshDetail failed");
//        CleanupRecast();
//        return false;
//    }
//
//    if (!rcBuildPolyMeshDetail(m_ctx, *m_pmesh, *m_chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *m_dmesh))
//    {
//        TM_LOG("rcBuildPolyMeshDetail failed");
//        CleanupRecast();
//        return false;
//    }
//
//    // 5) Create Detour data
//    dtNavMeshCreateParams params{};
//    memset(&params, 0, sizeof(params));
//    params.verts = m_pmesh->verts;
//    params.vertCount = m_pmesh->nverts;
//    params.polys = m_pmesh->polys;
//    params.polyAreas = m_pmesh->areas;
//    params.polyFlags = m_pmesh->flags;
//    params.polyCount = m_pmesh->npolys;
//    params.nvp = m_pmesh->nvp;
//    params.detailMeshes = m_dmesh->meshes;
//    params.detailVerts = m_dmesh->verts;
//    params.detailVertsCount = m_dmesh->nverts;
//    params.detailTris = m_dmesh->tris;
//    params.detailTriCount = m_dmesh->ntris;
//    params.walkableHeight = m_cfg.agentHeight;
//    params.walkableRadius = m_cfg.agentRadius;
//    params.walkableClimb = m_cfg.agentMaxClimb;
//    params.bmin[0] = m_pmesh->bmin[0]; params.bmin[1] = m_pmesh->bmin[1]; params.bmin[2] = m_pmesh->bmin[2];
//    params.bmax[0] = m_pmesh->bmax[0]; params.bmax[1] = m_pmesh->bmax[1]; params.bmax[2] = m_pmesh->bmax[2];
//    params.cs = cfg.cs;
//    params.ch = cfg.ch;
//    params.buildBvTree = true;
//
//    unsigned char* navData = nullptr;
//    int navDataSize = 0;
//    if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
//    {
//        TM_LOG("dtCreateNavMeshData failed");
//        CleanupRecast();
//        return false;
//    }
//
//    // return buffer to caller (navmesh will own it when addTile is called with DT_TILE_FREE_DATA)
//    outData = navData;
//    outDataSize = navDataSize;
//
//    // Keep Recast intermediate data around only if you want to debug ÅEcleanup now
//    CleanupRecast();
//
//    return true;
//}
//
//void TileMesh::CleanupRecast()
//{
//    if (m_solid)
//    {
//        rcFreeHeightField(m_solid);
//        m_solid = nullptr;
//    }
//    if (m_chf)
//    {
//        rcFreeCompactHeightfield(m_chf);
//        m_chf = nullptr;
//    }
//    if (m_cset)
//    {
//        rcFreeContourSet(m_cset);
//        m_cset = nullptr;
//    }
//    if (m_pmesh)
//    {
//        rcFreePolyMesh(m_pmesh);
//        m_pmesh = nullptr;
//    }
//    if (m_dmesh)
//    {
//        rcFreePolyMeshDetail(m_dmesh);
//        m_dmesh = nullptr;
//    }
//}

void NavMeshManager::Init() {
    sceneManager.onSceneLoaded.Add([this](SceneData Data) {if(!Data.NavMeshGuid.Empty())LoadMesh(Data.sceneName, Data.NavMeshGuid); });
}

void NavMeshManager::Update(float dt) {
    auto tm = navMeshData.find(currentScene);
    if (tm == navMeshData.end()) return;

    if (m_ecs.GetState() == ecs::RUNNING) {
        dtCrowd* crowd = tm->second->m_crowd;
        crowd->update(dt, nullptr);
        for (auto [agentID, entityID] : agentData) {
            const dtCrowdAgent* ag = crowd->getAgent(agentID);
            if (ag && ag->active) {
                auto trans = m_ecs.GetComponent<TransformComponent>(entityID);
                if (!trans) {
                    continue;
                }
                trans->LocalTransformation.position.x = ag->npos[0];
                trans->LocalTransformation.position.y = ag->npos[1];
                trans->LocalTransformation.position.z = ag->npos[2];
            }
        }
    }
}

void NavMeshManager::Build(std::string sceneName, std::shared_ptr<Sample_TileMesh> tm) {
    // Build Geom
    auto iter = navMeshData.find(sceneName);
    if (iter == navMeshData.end()) {
        BuildRecastGeometry(sceneName, tm);
        iter = navMeshData.find(sceneName);
    }
    if (!tm->m_geom) {
        LOGGING_WARN("Nav Mesh Build Failed: Unable to build Geometry");
        return;
    }

    int gw = 0, gh = 0;
    const float* bmin = tm->m_geom->getNavMeshBoundsMin();
    const float* bmax = tm->m_geom->getNavMeshBoundsMax();
    rcCalcGridSize(bmin, bmax, tm->m_cellSize, &gw, &gh);
    const int ts = (int)tm->m_tileSize;
    const int tw = (gw + ts - 1) / ts;
    const int th = (gh + ts - 1) / ts;

    // Max tiles and max polys affect how the tile IDs are caculated.
    // There are 22 bits available for identifying a tile and a polygon.
    int tileBits = rcMin((int)ilog2(nextPow2(tw * th)), 14);
    if (tileBits > 14) tileBits = 14;
    int polyBits = 22 - tileBits;
    tm->m_maxTiles = 1 << tileBits;
    tm->m_maxPolysPerTile = 1 << polyBits;

    // Build NavMesh
    if (iter != navMeshData.end()) {
        if (iter->second->handleBuild()) {
            tm->renderNavMesh.triVerts.clear();
            tm->renderNavMesh.lineVerts.clear();
            tm->renderNavMesh.pointVerts.clear();
            // Set Debug Vertex
            tm->handleRender();

            iter->second->renderNavMesh.CreateMesh();
            SetGraphicsRenderMesh(tm);
            LOGGING_DEBUG("Nav Mesh Build Successful");
        }
    }
    else {
        LOGGING_WARN("Nav Mesh Build Failed: Unable to find Navmesh Data");
    }
}

void NavMeshManager::BuildRecastGeometry(std::string sceneName, std::shared_ptr<Sample_TileMesh>& tileMesh) {
    const auto& entities = m_ecs.GetSceneData(sceneName);
    std::vector<float> rcVerts;
    std::vector<int> rcTris;
    glm::vec3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
    for (auto obj : entities.sceneIDs) 
    {
        const auto* meshComp = m_ecs.GetComponent<MeshFilterComponent>(obj);
        if (!meshComp) continue;
        
        const auto* name = m_ecs.GetComponent<NameComponent>(obj);
        if (!name->isStatic) continue;

        auto meshData = resourceManager.GetResource<R_Model>(meshComp->meshGUID);
        if (!meshData) {
            LOGGING_WARN("Unable to find meshData of " + name->entityName);
            continue;
        }

        auto trans = m_ecs.GetComponent<TransformComponent>(obj);
        InputGeom* newGeom = new InputGeom;
        if (!newGeom->loadMesh(&ctx, meshData, trans->WorldTransformation.scale.x)){
            const auto* name = m_ecs.GetComponent<NameComponent>(obj);
            delete newGeom;
            ctx.dumpLog("Geom load log %s:", name->entityName);
            return;
        }
        LOGGING_INFO("Loading Mesh Process Completed");
        if (!tileMesh)
            tileMesh = std::make_shared<Sample_TileMesh>();

        tileMesh->setContext(&ctx);
        if (tileMesh && newGeom) {
            tileMesh->handleMeshChanged(newGeom);
        }

        if (navMeshData.find(sceneName) == navMeshData.end()) {
            auto result = navMeshData.emplace(sceneName, tileMesh);
            LOGGING_INFO(sceneName + " geometry has been added to scene: " + sceneName);
        }
    }
}

void NavMeshManager::SaveMesh(const std::filesystem::path& filePath, const std::string& sceneName) {
    std::string fileName(filePath.string() + '\\' + sceneName + ".navmesh");
    auto data = navMeshData.find(sceneName);
    if (data != navMeshData.end()) {
        data->second->saveTiles(fileName);
    }
}


//TODO remove this to R_Resource
std::shared_ptr<Sample_TileMesh> NavMeshManager::LoadMesh(const std::string& sceneName, const utility::GUID& navGUID) {
    if (sceneName.find(".json") == std::string::npos) return nullptr; // reject if loading prefabs
    if (navGUID.Empty()) return nullptr;


    std::shared_ptr<Sample_TileMesh> tm;
    auto iter = navMeshData.find(sceneName);
    if (iter == navMeshData.end()) {
        BuildRecastGeometry(sceneName, tm);
    }

    if (tm == nullptr || tm->m_geom == nullptr) {
        LOGGING_WARN("Nav Mesh Load Failed: Unable to build Geometry");
        return nullptr;
    }

    std::string fileName = resourceManager.GetResourceDirectory() + '\\' + navGUID.GetToString() + ".navmesh";

    tm->loadTiles(fileName);
    SetGraphicsRenderMesh(tm);

    currentScene = sceneName;

    auto crowd = tm->getCrowd();
    tm->getCrowd()->init(128, 0.6f, tm->m_navMesh);

    return tm;
}

void NavMeshManager::SetGraphicsRenderMesh(std::shared_ptr<Sample_TileMesh> tm) {
    graphicsManager.renderNavMesh = tm != nullptr ? &tm->renderNavMesh : nullptr;
}

void NavMeshManager::AddAgent(int& agentID, EntityID entityID, const glm::vec3 pos, float radius, float height, float maxSpeed) {
    dtCrowdAgentParams params{};
    //params.radius = 0.6f;
    params.radius = radius;
    params.height = height;
    params.maxAcceleration = 8.0f;
    params.maxSpeed = maxSpeed;
    params.collisionQueryRange = radius * 12.0f;
    params.pathOptimizationRange = radius * 30.0f;
    params.updateFlags = 0;
    params.updateFlags = DT_CROWD_ANTICIPATE_TURNS |
                            DT_CROWD_SEPARATION |
                            DT_CROWD_OPTIMIZE_VIS |
                            DT_CROWD_OPTIMIZE_TOPO;
    //params.obstacleAvoidanceType = 
    params.separationWeight = 2.0f;

    auto tm = navMeshData.find(currentScene);
    if (tm == navMeshData.end()) {
        LOGGING_WARN("Failed to Add Agent: Unable to find scene - " + currentScene);
        return;
    }

    const dtQueryFilter* filter = tm->second->m_crowd->getFilter(0);
    //const float* halfExtents = tm->second->m_crowd->getQueryExtents();
    dtPolyRef startRef = 0;
    glm::vec3 nearestPt;
    float ext[3] = { 2, 4, 2 };

    tm->second->m_navQuery->findNearestPoly(glm::value_ptr(pos), ext, filter, &startRef, glm::value_ptr(nearestPt));
    agentID = tm->second->m_crowd->addAgent(glm::value_ptr(nearestPt), &params);
    agentData.emplace(agentID, entityID);
}

void NavMeshManager::MoveAgent(int& agentID, const glm::vec3 targetPos) {
    auto tm = navMeshData.find(currentScene);
    if (tm == navMeshData.end()) {
        LOGGING_WARN("Failed to Move Agent: Unable to find scene - " + currentScene);
        return;
    }
    const dtQueryFilter* filter = tm->second->m_crowd->getFilter(0);
    //const float* halfExtents = tm->second->m_crowd->getQueryExtents();
    float ext[3] = { 2, 4, 2 };

    dtPolyRef targetRef;
    glm::vec3 targetPt;
    tm->second->m_navQuery->findNearestPoly(glm::value_ptr(targetPos), ext, filter, &targetRef, glm::value_ptr(targetPt));
    if (targetRef == 0) {
        LOGGING_WARN("MoveAgent: targetRef = 0 (point is not on navmesh)");
        return;
    }
    bool result = tm->second->m_crowd->requestMoveTarget(agentID, targetRef, glm::value_ptr(targetPt));
    if (!result) std::cout << "Request to move failed" << std::endl;
}

void NavMeshManager::RemoveAgent(int& agentID) {
    auto tm = navMeshData.find(currentScene);
    if (tm == navMeshData.end()) {
        LOGGING_WARN("Failed to Remove Agent: Unable to find scene - " + currentScene);
        return;
    }
    agentData.erase(agentID);
    tm->second->m_crowd->removeAgent(agentID);
}

#undef TM_LOG
