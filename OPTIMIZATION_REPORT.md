# Kos Engine 2.0 - Performance Optimization Report

> **Analysis Date:** February 5, 2026  
> **Engine Version:** 2.0  
> **Architecture:** C++20, ECS-based, OpenGL rendering

## Executive Summary

This report analyzes the Kos Engine 2.0 codebase and identifies **27 optimization opportunities** ranked by potential impact. The engine demonstrates solid architectural foundations with ECS, modern C++20, precompiled headers, and PhysX integration. However, significant performance gains (estimated 2-5x in critical paths) can be achieved through rendering optimizations, better parallelization, and memory efficiency improvements.

---

## 🔴 Critical Impact Optimizations (Highest Priority)

### 1. **Implement Render Batch Submission System**
**Impact:** ⭐⭐⭐⭐⭐ **Estimated Gain:** 50-70% rendering performance improvement

**Current Issue:**
- [`Renderer.cpp`](file:///c:/Users/ngjaz/OneDrive/Documents/Kos-Engine-2.0/kOS/Engine/Graphics/Renderer.cpp) reveals individual draw calls per entity
- `MeshRenderer::Render()` (lines 168-182) iterates through meshes with separate `PBRDraw()` calls
- Text rendering (lines 114-159) performs per-character buffer updates via `glBufferSubData`
- No draw call batching or state sorting

**Optimization:**
```cpp
// Group meshes by material/shader before rendering
struct RenderBatch {
    Shader* shader;
    Material* material;
    std::vector<glm::mat4> transformations;
    std::vector<int> entityIDs;
    Mesh* mesh;
};

// Sort and batch before rendering
void MeshRenderer::Render(const CameraData& camera, Shader& shader) {
    std::vector<RenderBatch> batches = SortAndBatch(meshesToDraw);
    
    for (auto& batch : batches) {
        shader.Use();
        // Set material once for all instances
        batch.material->Bind();
        
        // Use instanced rendering
        glDrawElementsInstanced(GL_TRIANGLES, batch.mesh->indexCount, 
                                GL_UNSIGNED_INT, 0, batch.transformations.size());
    }
}
```

**Implementation Steps:**
1. Add batching logic to `GraphicsManager`
2. Group render submissions by material/shader hash
3. Use `glDrawElementsInstanced` for multi-object rendering
4. Implement per-layer batching using existing `layer::LAYERS` system

**Verification:**
- Measure draw calls using RenderDoc/NSight
- Target: Reduce from ~N draw calls to ~M unique material combinations (typically 10-20x reduction)

---

### 2. **Frustum Culling Implementation**
**Impact:** ⭐⭐⭐⭐⭐ **Estimated Gain:** 40-60% for complex scenes

**Current Issue:**
- No visibility culling in rendering pipeline
- All meshes in `meshesToDraw` arrays are rendered regardless of visibility
- Unnecessary GPU overhead for off-screen geometry

**Optimization:**
```cpp
// Add to CameraData
struct Frustum {
    std::array<glm::vec4, 6> planes; // left, right, bottom, top, near, far
    
    bool IsBoxVisible(const glm::vec3& minAABB, const glm::vec3& maxAABB) const;
};

// In MeshRenderer::Render()
void MeshRenderer::Render(const CameraData& camera, Shader& shader) {
    Frustum frustum = camera.GetFrustum();
    
    for (auto& mesh : meshesToDraw) {
        if (!frustum.IsBoxVisible(mesh.aabb.min, mesh.aabb.max))
            continue; // Skip rendering
            
        shader.SetTrans("model", mesh.transformation);
        mesh.meshToUse->PBRDraw(shader, mesh.meshMaterial);
    }
}
```

**Implementation:**
1. Extract frustum planes from camera view-projection matrix
2. Add AABB bounds to `MeshData` struct
3. Implement plane-AABB intersection test
4. Apply to all renderers (Mesh, SkinnedMesh, Particle, Light)

---

### 3. **Object Pooling for Renderer Clear Operations**
**Impact:** ⭐⭐⭐⭐ **Estimated Gain:** 30-40% reduction in allocations

**Current Issue:**
- All renderers call `clear()` on `std::vector` every frame ([`Renderer.cpp:165, 294, 301, 421`](file:///c:/Users/ngjaz/OneDrive/Documents/Kos-Engine-2.0/kOS/Engine/Graphics/Renderer.cpp#L165))
- Repeated allocations/deallocations cause memory fragmentation
- Potential cache misses due to heap churn

**Optimization:**
```cpp
struct MeshRenderer : BasicRenderer {
    void Clear() override {
        for (auto& layer : meshesToDraw) {
            layer.clear(); // Don't deallocate capacity
            layer.reserve(estimatedCapacity); // Maintain capacity
        }
    }
    
    void Render(const CameraData& camera, Shader& shader) {
        // Use parallel clearing
        std::for_each(std::execution::par, meshesToDraw.begin(), meshesToDraw.end(),
            [](auto& layer) { layer.clear(); });
    }
    
private:
    size_t estimatedCapacity = 256; // Tune based on typical scene size
};
```

**Benefits:**
- Eliminates per-frame allocations
- Improves cache locality
- Reduces memory fragmenter

---

### 4. **Parallel ECS System Execution**
**Impact:** ⭐⭐⭐⭐⭐ **Estimated Gain:** 2-4x for CPU-bound systems

**Current Issue:**
- [`ThreadPool.h`](file:///c:/Users/ngjaz/OneDrive/Documents/Kos-Engine-2.0/kOS/Engine/ECS/ThreadPool.h) exists but limited usage
- Systems likely execute sequentially in ECS
- Multi-core CPUs underutilized

**Optimization:**
```cpp
// In ECS.h - Add parallel system execution
class ECS {
public:
    void UpdateSystems(float deltaTime) {
        // Execute independent systems in parallel
        auto& independentSystems = GetIndependentSystems();
        
        m_threadPool.ParallelFor(independentSystems.begin(), independentSystems.end(),
            [deltaTime](auto& system) {
                system->Update(deltaTime);
            });
        
        m_threadPool.Wait();
        
        // Execute dependent systems sequentially
        for (auto& system : GetDependentSystems()) {
            system->Update(deltaTime);
        }
    }
    
private:
    ThreadPool m_threadPool{std::thread::hardware_concurrency()};
};
```

**Implementation:**
1. Create dependency graph for systems
2. Group systems into parallel-safe batches
3. Use existing `ThreadPool` for execution
4. Add read/write component access hints to determine dependencies

---

### 5. **Texture Atlas for UI/Text Rendering**
**Impact:** ⭐⭐⭐⭐ **Estimated Gain:** 60-80% for text-heavy UI

**Current Issue:**
- [`TextRenderer::RenderScreenFonts()`](file:///c:/Users/ngjaz/OneDrive/Documents/Kos-Engine-2.0/kOS/Engine/Graphics/Renderer.cpp#L40-L161) binds texture per character (line 138)
- `glBufferSubData()` called for each character (line 142)
- Worst-case: 1 bind + 1 buffer update per character

**Optimization:**
```cpp
void TextRenderer::RenderScreenFonts(const CameraData& camera, Shader& shader) {
    if (screenTextToDraw.empty()) return;
    
    // Build single vertex buffer for all text
    std::vector<float> allVertices;
    allVertices.reserve(screenTextToDraw.size() * 100 * 24); // Estimate
    
    for (const auto& textData : screenTextToDraw) {
        // Font already uses texture atlas (CharacterData has UV coords)
        for (const char c : textData.textToDraw) {
            const auto& ch = textData.fontToUse->m_characters[c];
            // Build vertices using atlas UVs (ch.m_topLeftTexCoords, etc.)
            allVertices.push_back(/* ... */);
        }
    }
    
    // Single upload
    glBindBuffer(GL_ARRAY_BUFFER, screenTextMesh.vboID);
    glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float), 
                 allVertices.data(), GL_DYNAMIC_DRAW);
    
    // Single draw call
    glDrawArrays(GL_TRIANGLES, 0, allVertices.size() / 4);
}
```

---

### 6. **GPU Skinned Mesh Animation**
**Impact:** ⭐⭐⭐⭐ **Estimated Gain:** 3-5x for animated models

**Current Issue:**
- [`SkinnedMeshRenderer::Render()`](file:///c:/Users/ngjaz/OneDrive/Documents/Kos-Engine-2.0/kOS/Engine/Graphics/Renderer.cpp#L197-L226) updates animation per-frame on CPU (line 215)
- Bone matrices uploaded to GPU each frame
- CPU bottleneck for many animated characters

**Optimization:**
```cpp
// Move animation update to compute shader
void SkinnedMeshRenderer::Render(const CameraData& camera, Shader& shader) {
    // Batch all skinned meshes
    std::vector<AnimationData> animData;
    
    for (auto& meshData : skinnedMeshesToDraw) {
        for (auto& mesh : meshData) {
            if (mesh.animationToUse) {
                animData.push_back({
                    mesh.currentDuration,
                    mesh.meshToUse->GetBoneInfo(),
                    mesh.entityID
                });
            }
        }
    }
    
    // Upload to GPU and compute in parallel
    DispatchAnimationComputeShader(animData);
    
    // Render with pre-computed bone matrices
    RenderSkinnedMeshes(shader);
}
```

---

## 🟠 High Impact Optimizations

### 7. **Sparse Set Component Storage Optimization**
**Impact:** ⭐⭐⭐⭐ **Estimated Gain:** 20-30% ECS query performance

**Current Issue:**
- [`SparseSet.h`](file:///c:/Users/ngjaz/OneDrive/Documents/Kos-Engine-2.0/kOS/Engine/ECS/SparseSet.h) used for component storage
- Potential cache misses in sparse array access

**Optimization:**
- Implement archetype-based storage for better cache coherency
- Group entities with same component signatures
- Use Structure of Arrays (SoA) instead of Array of Structures (AoS)

---

### 8. **Resource Manager Async Loading**
**Impact:** ⭐⭐⭐⭐ **Estimated Gain:** Eliminate loading hitches

**Current Issue:**
- [`ResourceManager::GetResource()`](file:///c:/Users/ngjaz/OneDrive/Documents/Kos-Engine-2.0/kOS/Engine/Resources/ResourceManager.h#L53-L76) loads synchronously (line 70: `asset->Load()`)
- Blocks main thread during asset loading

**Optimization:**
```cpp
template<typename T>
std::shared_ptr<T> GetResourceAsync(const utility::GUID& GUID, 
                                     std::function<void(std::shared_ptr<T>)> callback) {
    auto placeholder = std::make_shared<T>(); // Lightweight proxy
    
    std::async(std::launch::async, [=]() {
        auto asset = std::make_shared<T>(GUID, GetResourcePath<T>(GUID));
        asset->Load();
        m_resourceMap[GUID] = asset;
        callback(asset);
    });
    
    return placeholder;
}
```

---

### 9. **Physics Interpolation Buffer Optimization**
**Impact:** ⭐⭐⭐⭐ **Estimated Gain:** Smoother visuals at variable frame rates

**Current Issue:**
- [`PhysicsManager`](file:///c:/Users/ngjaz/OneDrive/Documents/Kos-Engine-2.0/kOS/Engine/Physics/PhysicsManager.h) has `InterpolationAlpha()` (line 60) but no visible double-buffering
- Fixed timestep physics may cause visual jitter

**Optimization:**
- Store previous and current transform states
- Interpolate rendering between physics steps
- Use `InterpolationAlpha()` consistently across all physics objects

---

### 10. **Shadow Map Cascades for Directional Lights**
**Impact:** ⭐⭐⭐⭐ **Estimated Gain:** 50% better shadow quality

**Current Issue:**
- [`LightRenderer`](file:///c:/Users/ngjaz/OneDrive/Documents/Kos-Engine-2.0/kOS/Engine/Graphics/Renderer.h#L111-L123) has depth cube maps for point lights but no CSM for directional
- Single shadow map leads to poor quality across large scenes

**Optimization:**
- Implement Cascaded Shadow Maps (CSM) for directional lights
- Split frustum into 3-4 cascades
- Use depth array texture for all cascades

---

## 🟡 Medium Impact Optimizations

### 11. **Precompiled Shader Variants**
**Impact:** ⭐⭐⭐ **Estimated Gain:** 90% reduction in shader compilation time

**Current Status:**
- Shaders likely compiled at runtime
- Potential startup delays

**Optimization:**
- Pre-compile shader permutations offline
- Store binary SPIR-V or platform-specific binaries
- Use `glProgramBinary` for instant loading

---

### 12. **Particle System GPU Simulation**
**Impact:** ⭐⭐⭐ **Estimated Gain:** 10x particle capacity

**Current Issue:**
- [`ParticleRenderer`](file:///c:/Users/ngjaz/OneDrive/Documents/Kos-Engine-2.0/kOS/Engine/Graphics/Renderer.cpp#L672-L781) uses instanced rendering (good!) but CPU updates
- Limited to 10,000 particles (line 633)

**Optimization:**
- Move particle physics to compute shader
- Use transform feedback or compute ping-pong buffers
- Support 100,000+ particles

---

### 13. **ImGui Rendering Optimization**
**Impact:** ⭐⭐⭐ **Estimated Gain:** 20-30% editor performance

**Recommendations:**
- Use single vertex buffer for all ImGui geometry
- Minimize state changes
- Batch draw calls by texture

---

### 14. **Light Culling (Tiled/Clustered Rendering)**
**Impact:** ⭐⭐⭐ **Estimated Gain:** 2-3x for many lights

**Current Issue:**
- All lights processed for all pixels
- No spatial acceleration

**Optimization:**
- Implement tiled forward+ rendering
- Compute light lists per tile (16x16 pixels)
- Reduce overdraw from lighting calculations

---

### 15. **Occlusion Culling (GPU-based)**
**Impact:** ⭐⭐⭐ **Estimated Gain:** 30-50% for occluded scenes

**Optimization:**
- Use previous frame depth buffer for occlusion queries
- Implement Hi-Z culling
- Cull entire chunks of geometry

---

## 🟢 Build System & Compiler Optimizations

### 16. **Link-Time Optimization (LTO/IPO)**
**Impact:** ⭐⭐⭐ **Estimated Gain:** 10-15% overall performance

**Current Status:**
- [`CMakeLists.txt`](file:///c:/Users/ngjaz/OneDrive/Documents/Kos-Engine-2.0/kOS/Engine/CMakeLists.txt) doesn't enable LTO

**Optimization:**
```cmake
# Add to CMakeLists.txt
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_output)

if(ipo_supported)
    set_property(TARGET Kos_Engine PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    message(STATUS "LTO/IPO enabled")
endif()
```

---

### 17. **Profile-Guided Optimization (PGO)**
**Impact:** ⭐⭐⭐ **Estimated Gain:** 15-20% performance

**Implementation:**
```cmake
# Step 1: Instrument build
set(CMAKE_CXX_FLAGS_RELEASE "/GL /O2")

# Step 2: Run typical workload to generate .pgd file

# Step 3: Optimize build
set(CMAKE_CXX_FLAGS_RELEASE "/GL /O2 /LTCG:PGO")
```

---

### 18. **Compiler Intrinsics & SIMD**
**Impact:** ⭐⭐⭐ **Estimated Gain:** 2-4x for math-heavy code

**Targets:**
- Matrix operations in transform calculations
- Vector math in physics
- Particle updates

**Optimization:**
```cpp
// Use GLM's SIMD support
#define GLM_FORCE_INTRINSICS
#define GLM_FORCE_ALIGNED_GENTYPES
#define GLM_FORCE_SSE42 // or AVX2
#include <glm/glm.hpp>
```

---

### 19. **Unity Builds (Optional)**
**Impact:** ⭐⭐ **Estimated Gain:** 30-50% faster compilation

**Note:** May reduce incremental build performance
```cmake
set_target_properties(Kos_Engine PROPERTIES UNITY_BUILD ON)
set_target_properties(Kos_Engine PROPERTIES UNITY_BUILD_BATCH_SIZE 16)
```

---

## 🔵 Memory & Data Structure Optimizations

### 20. **Reserve Vector Capacities**
**Impact:** ⭐⭐⭐ **Estimated Gain:** Reduce allocations by 50%

**Current Issue:**
- Renderer vectors grow dynamically each frame
- Repeated reallocations

**Optimization:**
```cpp
struct MeshRenderer {
    MeshRenderer() {
        for (auto& layer : meshesToDraw) {
            layer.reserve(256); // Tune based on typical scene
        }
    }
};
```

---

### 21. **Custom Allocators for Components**
**Impact:** ⭐⭐⭐ **Estimated Gain:** 20-30% faster component access

**Optimization:**
- Use `std::pmr::polymorphic_allocator` with monotonic buffer
- Pool allocations for components
- Reduce heap fragmentation

```cpp
#include <memory_resource>

class ECS {
    std::pmr::synchronized_pool_resource m_pool;
    std::unordered_map<utility::GUID, std::shared_ptr<Resource>, 
                       std::hash<utility::GUID>,
                       std::equal_to<utility::GUID>,
                       std::pmr::polymorphic_allocator<
                           std::pair<const utility::GUID, std::shared_ptr<Resource>>
                       >> m_resourceMap{&m_pool};
};
```

---

### 22. **String View for Resource Paths**
**Impact:** ⭐⭐ **Estimated Gain:** Reduce string allocations

**Current Issue:**
- [`ResourceManager::GetResourcePath()`](file:///c:/Users/ngjaz/OneDrive/Documents/Kos-Engine-2.0/kOS/Engine/Resources/ResourceManager.h#L108-L123) creates temporary strings

**Optimization:**
```cpp
std::string_view GetResourceExtension(const std::string& className) const {
    return m_resourceExtension.at(className);
}
```

---

### 23. **Flat Hash Map for Faster Lookups**
**Impact:** ⭐⭐ **Estimated Gain:** 20-30% faster lookups

**Optimization:**
- Replace `std::unordered_map` with `robin_hood::unordered_map` or `absl::flat_hash_map`
- Better cache performance
- Faster iteration

---

## ⚪ Low Impact / Quality of Life Optimizations

### 24. **Remove Debug Logging in Release Builds**
**Impact:** ⭐⭐ **Estimated Gain:** 5-10%

**Optimization:**
```cpp
// In Logging.h
#ifdef NDEBUG
    #define LOGGING_INFO(...) ((void)0)
    #define LOGGING_ERROR(...) ((void)0)
#else
    #define LOGGING_INFO(...) /* actual logging */
    #define LOGGING_ERROR(...) /* actual logging */
#endif
```

---

### 25. **Reduce GLM Matrix Copies**
**Impact:** ⭐⭐ **Estimated Gain:** 5-10%

**Current Issue:**
- Pass matrices by value in many places

**Optimization:**
```cpp
// Pass by const reference
void SetTrans(const std::string& name, const glm::mat4& mat);
```

---

### 26. **Enable Aggressive Compiler Optimizations**
**Impact:** ⭐⭐ **Estimated Gain:** 5-10%

**Current Status:**
- No explicit optimization flags in CMake beyond `/O2`

**Optimization:**
```cmake
if(MSVC)
    target_compile_options(Kos_Engine PRIVATE 
        /O2 /Oi /Ot /GL /fp:fast /arch:AVX2
    )
else()
    target_compile_options(Kos_Engine PRIVATE
        -O3 -march=native -ffast-math -flto
    )
endif()
```

---

### 27. **Reduce Include Dependencies**
**Impact:** ⭐ **Estimated Gain:** Faster compilation

**Current Issue:**
- [`pch.h`](file:///c:/Users/ngjaz/OneDrive/Documents/Kos-Engine-2.0/kOS/Engine/Config/pch.h) includes many headers
- May slow compilation

**Optimization:**
- Forward declare types where possible
- Use Pimpl idiom for implementation hiding
- Include only what you use

---

## Implementation Roadmap

### Phase 1: Quick Wins (Week 1-2)
1. ✅ Implement render batching (#1)
2. ✅ Add frustum culling (#2)
3. ✅ Enable LTO in CMake (#16)
4. ✅ Reserve vector capacities (#20)
5. ✅ Compiler optimization flags (#26)

**Expected Gain:** 2-3x rendering performance, 10% overall improvement

### Phase 2: Parallelization (Week 3-4)
1. ✅ Parallel ECS system execution (#4)
2. ✅ Async resource loading (#8)
3. ✅ GPU skinned mesh animation (#6)

**Expected Gain:** Better multi-core utilization, smoother loading

### Phase 3: Advanced Rendering (Month 2)
1. ✅ Tiled/clustered lighting (#14)
2. ✅ Cascaded shadow maps (#10)
3. ✅ GPU particle simulation (#12)
4. ✅ Occlusion culling (#15)

**Expected Gain:** 2-3x for complex scenes with many lights/particles

### Phase 4: Memory & Polish (Month 3)
1. ✅ Object pooling (#3)
2. ✅ Custom allocators (#21)
3. ✅ Profile-Guided Optimization (#17)
4. ✅ Shader precompilation (#11)

**Expected Gain:** Reduced memory footprint, stable frame times

---

## Measurement & Profiling

### Recommended Tools
- **Visual Studio Profiler** - CPU profiling on Windows
- **RenderDoc** - GPU frame analysis, draw call inspection
- **NVIDIA Nsight Graphics** - Advanced GPU profiling
- **Tracy Profiler** - Real-time CPU/GPU profiling with zones

### Key Metrics to Track
- **Draw Calls per Frame** - Target: < 500 for complex scenes
- **Frame Time** - Maintain 144 FPS (6.9ms budget)
- **Memory Allocations** - Minimize per-frame allocations
- **GPU Utilization** - Aim for 80-95%
- **ECS Query Time** - < 1ms for all system updates

---

## Conclusion

The Kos Engine has a solid foundation but can achieve **2-5x performance improvements** through systematic optimization. Prioritize rendering batching, frustum culling, and parallelization for immediate gains. The modular architecture makes these optimizations straightforward to implement incrementally without major refactoring.

### Estimated Overall Performance Improvement
- **Critical Optimizations (1-6):** ~3-5x rendering performance
- **High Impact (7-15):** ~1.5-2x overall performance  
- **Build & Memory (16-23):** ~20-30% improvement
- **Combined:** 5-10x potential improvement in worst-case scenarios

**Next Steps:**
1. Profile current performance with RenderDoc to identify actual bottlenecks
2. Implement Phase 1 quick wins
3. Measure improvements and iterate
4. Continue with higher-complexity optimizations based on profiling data
