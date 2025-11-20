#include "Config/pch.h"
#include "ParticleSystem.h"
#include "glm/glm.hpp"
#include "Graphics/GraphicsManager.h"
#include "Inputs/Input.h"

namespace ecs {

    void ParticleSystem::Init() {

        onRegister.Add([&](EntityID id) {
            ParticleComponent* particle = m_ecs.GetComponent<ParticleComponent>(id);

            //init the particle velocity
            particle->freeIndices.clear();
            particle->freeIndices.reserve(particle->max_Particles);
            particle->alive_Particles.clear();
            particle->alive_Particles.reserve(particle->max_Particles);
            particle->visualData_List.colors.clear();
            particle->visualData_List.sizes.clear();
            particle->visualData_List.rotation.clear();
            particle->visualData_List.colors.resize(particle->max_Particles);
            particle->visualData_List.sizes.resize(particle->max_Particles);
            particle->visualData_List.rotation.resize(particle->max_Particles);
            particle->particle_List.clear();
            particle->particle_List.resize(particle->max_Particles);
            particle->alive_no_Of_Particles = 0;

            for (short i = particle->max_Particles - 1; i >= 0; --i) {
                particle->freeIndices.push_back(i);
            }

          
            // Initialize emitter timers
            particle->emitterTime = 0.f;
            particle->durationCounter = 0.f;

            });

        onDeregister.Add([&](EntityID id) {
            auto* particle = m_ecs.GetComponent<ParticleComponent>(id);

            // Clear tracking data
            particle->freeIndices.clear();
            particle->alive_Particles.clear();
            particle->visualData_List.colors.clear();
            particle->visualData_List.sizes.clear();
            particle->visualData_List.rotation.clear();
            particle->particle_List.clear();

            });
    }

    void ParticleSystem::Update() {
        const auto& entities = m_entities.Data();
        float dt = m_ecs.m_GetDeltaTime();
       
        for (EntityID id : entities) {
            ParticleComponent* particle = m_ecs.GetComponent<ParticleComponent>(id);
            TransformComponent* transform = m_ecs.GetComponent<TransformComponent>(id);
            if (!particle || !transform) {
                continue;
            }

            //===========================================
            // Update emitter
            //===========================================
            UpdateEmitters(dt, id, particle, transform);

            
            // ==========================================
            // Update Lifetimes (modifies alive_Particles)
            // ==========================================
            UpdateParticleLifetimes(dt, particle);
           
            // ==========================================
            // Extract Positions for Rendering (OPTIMIZED)
            // ==========================================

            ParticleInstance sending;
            ExtractParticleDataOptimized(particle, sending);

            // ==========================================
            // Sending over to the Graphic Pipeline to render
            // ==========================================
            m_graphicsManager.gm_PushBasicParticleData(BasicParticleData{ sending.positions_Particle, sending.colors, sending.sizes, sending.rotates });
        }
    }


    void ParticleSystem::EmitParticle(EntityID entityId, const glm::vec3& particle_position, const glm::vec3& velocity, float lifetime, ParticleComponent*& particle) {
        // Check if we have any free slots
        if (particle->freeIndices.empty()) {
            LOGGING_WARN("No free particle slots for entity %d\n", entityId);
            return;
        }

        // FIX: Get a free particle index (O(1) operation)
        int particleIdx = particle->freeIndices.back();
        particle->freeIndices.pop_back();

        // FIX: Set particle tracking data
        particle->alive_Particles.push_back(particleIdx);

        ParticleData pd;
        pd.lifespan = lifetime;
        pd.lifetime = lifetime;
        pd.color = particle->colorModule.enabled ? RandomColourRange(particle->colorModule.start_Color, particle->colorModule.end_Color) : particle->colorModule.start_Color;
        pd.size = particle->sizeModule.enabled ? AbsRandomRange(particle->sizeModule.start_Size, particle->sizeModule.end_Size) : particle->sizeModule.start_Size;
        pd.rotation = particle->rotationModule.enabled ? glm::radians(RandomRange(particle->rotationModule.start_Rotation, particle->rotationModule.end_Rotation)) : glm::radians(particle->rotationModule.start_Rotation);
        pd.velocity = velocity;
        pd.position = particle_position;

        //init particle data
        particle->visualData_List.colors[particleIdx] = particle->colorModule.enabled ? RandomColourRange(particle->colorModule.start_Color, particle->colorModule.end_Color) : particle->colorModule.start_Color;
        particle->visualData_List.sizes[particleIdx] = glm::vec2(particle->sizeModule.enabled ? AbsRandomRange(particle->sizeModule.start_Size, particle->sizeModule.end_Size) : particle->sizeModule.start_Size);
        particle->visualData_List.rotation[particleIdx] = particle->rotationModule.enabled ? glm::radians(RandomRange(particle->rotationModule.start_Rotation, particle->rotationModule.end_Rotation)) : glm::radians(particle->rotationModule.start_Rotation);
   
        particle->particle_List[particleIdx] = pd;
    }

    void ParticleSystem::UpdateParticleLifetimes(float dt, ParticleComponent*& particle) {

        // === PRE-COMPUTE EVERYTHING ONCE ===
        const bool updateSize = particle->sizeModule.enabled;
        const bool updateColor = particle->colorModule.enabled;
        const bool updateRotation = particle->rotationModule.enabled;
        const bool updateVelocity = particle->velocityModule.enabled;
        const bool updateForce = particle->forceModule.enabled;
        const bool updateAttractor = particle->attractorModule.enabled;

        // Pre-compute deltas
        const float sizeDelta = updateSize ? (particle->sizeModule.end_Size - particle->sizeModule.start_Size) : 0.0f;
        const float sizeStart = particle->sizeModule.start_Size;

        const glm::vec4 colorDelta = updateColor ? (particle->colorModule.end_Color - particle->colorModule.start_Color) : glm::vec4(0);
        const glm::vec4 colorStart = particle->colorModule.start_Color;

        const float rotModRad = glm::radians(particle->rotationModule.rotation_Modifier) * dt;

        // Pre-compute attractor values
        const glm::vec3 attractorTarget = particle->attractorModule.targetPosition;
        const float attractStrength = particle->attractorModule.attractionStrength;
        const float explosionStrength = particle->attractorModule.explosionStrength;
        const float whirlpoolStrength = particle->attractorModule.whirlpoolStrength;
        const bool useInverseFalloff = particle->attractorModule.useInverseFalloff;

        // === OPTIMIZED LOOP ===
        for (int i = static_cast<int>(particle->alive_Particles.size()) - 1; i >= 0; --i) {
            short particleIdx = particle->alive_Particles[i];

            // Fast clamp
            float t = 1.f - (particle->particle_List[particleIdx].lifespan / particle->particle_List[particleIdx].lifetime);
            t = (t < 0.0f) ? 0.0f : ((t > 1.0f) ? 1.0f : t);

            // === VELOCITY UPDATES ===

            if (updateVelocity) {
                switch (particle->velocityModule.mode) {
                case Absolute:
                    particle->particle_List[particleIdx].velocity = particle->velocityModule.velocity_Modifier * t +
                        particle->particle_List[particleIdx].velocity * (1.0f - t);
                    break;
                case Additive:
                    particle->particle_List[particleIdx].velocity += particle->velocityModule.velocity_Modifier * (t * dt);
                    break;
                case Multiplier: {
                    float factor = 1.0f + (glm::length(particle->velocityModule.velocity_Modifier) - 1.0f) * t;
                    particle->particle_List[particleIdx].velocity *= factor;
                    break;
                }
            }
                

                if (updateForce) {
                    particle->particle_List[particleIdx].velocity += particle->forceModule.force * dt;
                }

                // === OPTIMIZED ATTRACTORS ===
                if (updateAttractor) {
                    glm::vec3 particlePos = particle->particle_List[particleIdx].position;
                    glm::vec3 dir = attractorTarget - particlePos;
                    float distSq = glm::dot(dir, dir);

                    if (distSq > 0.000001f) {
                        float dist = glm::sqrt(distSq);
                        glm::vec3 dirNorm = dir / dist;

                        // Attraction
                        if (attractStrength > 0.0f) {
                            float strength = attractStrength;
                            if (useInverseFalloff) strength /= (distSq + 0.01f);
                            particle->particle_List[particleIdx].velocity += dirNorm * strength * dt;
                        }

                        // Explosion
                        if (explosionStrength != 0.0f) {
                            particle->particle_List[particleIdx].velocity += -dirNorm * explosionStrength * dt;
                        }

                        // Whirlpool
                        if (whirlpoolStrength > 0.0f) {
                            glm::vec3 up = (glm::abs(dirNorm.y) > 0.9f) ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
                            glm::vec3 tangential = glm::normalize(glm::cross(dirNorm, up));

                            float strength = whirlpoolStrength;
                            if (useInverseFalloff) strength /= (dist + 0.1f);

                            glm::vec3 force = dirNorm * (strength * 0.25f) + tangential * strength;
                            //velocities[particleIdx] += force * dt;
                        }
                    }
                }
            }

            // === OPTIMIZED VISUAL UPDATES ===
            if (updateSize) {
                particle->visualData_List.sizes[particleIdx] = glm::vec2(sizeStart + sizeDelta * t);
            }

            if (updateColor) {
                particle->visualData_List.colors[particleIdx] = colorStart + colorDelta * t;
            }

            if (updateRotation) {
                particle->visualData_List.rotation[particleIdx] += rotModRad;
            }

            // === LIFETIME UPDATE ===
            //lifetime_list[particleIdx] -= dt;

            //if (lifetime_list[particleIdx] <= 0.0f) {
            //    positions[particleIdx].w = 0.0f;
            //    particle->freeIndices.push_back(particleIdx);

            //    // O(1) swap-and-pop
            //    particle->alive_Particles[i] = particle->alive_Particles.back();
            //    particle->alive_Particles.pop_back();
            //}
        }
    }

    void ParticleSystem::UpdateEmitters(float dt,EntityID id, ParticleComponent*& particleComp,  TransformComponent* transform) {
        const auto& entities = m_entities.Data();

        // Check if emitter should be active
        bool shouldEmit = false;
        bool particles_Alive = false;
        if (particleComp->looping) {
            if (particleComp->durationCounter >= particleComp->duration) {
                particleComp->durationCounter = 0.f;
            }
            shouldEmit = particleComp->play_On_Awake;
        }
        else {
            if (particleComp->play_On_Awake) {
                particleComp->durationCounter += dt;
                shouldEmit = (particleComp->durationCounter < particleComp->duration);
            }
        }
        if (shouldEmit) {
            particleComp->emitterTime += dt;
            float emissionInterval = particleComp->emissionInterval;

            if (emissionInterval <= 0.0f) {
                return;  // Simply don't emit particles
            }

            while (particleComp->emitterTime >= emissionInterval) {
               

                // ==== = GENERATE DIRECTION BASED ON EMISSION SHAPE ==== =
                EmissionData emission; 

                switch (particleComp->shapeModule.type) {
                case EmissionShape::BOX:
                    emission = GenerateBoxEmission(particleComp);
                    break;
                case EmissionShape::CONE:
                    emission = GenerateConeEmission(particleComp);
                    break;
                case EmissionShape::SPHERE:
                    emission = GenerateSphereEmission(particleComp);
                    break;
                case EmissionShape::CIRCLE:
                    emission = GenerateCircleEmission(particleComp);
                    break;
                case EmissionShape::EDGE:
                    emission = GenerateEdgeEmission(particleComp);
                    break;
                default:
                    emission = GenerateConeEmission(particleComp);
                    break;
                }


                // ===== APPLY SPEED TO DIRECTION =====
                // direction is already normalized, multiply by speed
                glm::vec3 vel = emission.direction * particleComp->start_Velocity;              
                glm::vec3 pos = transform->WorldTransformation.position + emission.positionOffset;
                particleComp->particle_Spawn_Location = transform->WorldTransformation.position;
                //random to the lifetime
                float particle_Lifetime = particleComp->start_Lifetime;
                if (particleComp->lifetime_Random_Enable) {
                    particle_Lifetime = RandomRange(particleComp->start_Lifetime, particleComp->end_Lifetime);
                }

                EmitParticle(id, pos, vel, particle_Lifetime, particleComp);
                particleComp->emitterTime -= emissionInterval;
            }
        }
    }


    // NEW: Optimized position extraction
    void ParticleSystem::ExtractParticleDataOptimized(ParticleComponent* particle,
        ParticleInstance& data) {
        if (!particle || particle->alive_Particles.empty()) {
            data.positions_Particle.clear();
            data.colors.clear();
            data.rotates.clear();
            data.sizes.clear();
            return;
        }

        // OPTIMIZATION: Only extract active particles
        data.positions_Particle.clear();
        data.colors.clear();
        data.rotates.clear();
        data.sizes.clear();
        data.positions_Particle.resize(particle->alive_Particles.size());
        data.colors.resize(particle->alive_Particles.size());
        data.rotates.resize(particle->alive_Particles.size());
        data.sizes.resize(particle->alive_Particles.size());


        for (int i = 0; i < particle->alive_Particles.size(); ++i) {
            short idx = particle->alive_Particles[i];
            data.colors[i] = particle->visualData_List.colors[idx];
            data.sizes[i] = particle->visualData_List.sizes[idx];
            data.rotates[i] = particle->visualData_List.rotation[idx];
        }

    }
    // ========================================
    // BOX EMISSION (with position offset)
    // ========================================
    EmissionData ParticleSystem::GenerateBoxEmission(ParticleComponent* particle) {
        EmissionData data;

        // Random position within box
        float randX = RandomRange(-particle->shapeModule.box_Size.x * 0.5f, particle->shapeModule.box_Size.x * 0.5f);
        float randY = RandomRange(-particle->shapeModule.box_Size.y * 0.5f, particle->shapeModule.box_Size.y * 0.5f);
        float randZ = RandomRange(-particle->shapeModule.box_Size.z * 0.5f, particle->shapeModule.box_Size.z * 0.5f);

        data.positionOffset = glm::vec3(randX, randY, randZ);

        // Random direction
        glm::vec3 direction = glm::vec3(
            RandomRange(-1.0f, 1.0f),
            RandomRange(-1.0f, 1.0f),
            RandomRange(-1.0f, 1.0f)
        );

        if (glm::length(direction) > 0.001f) {
            direction = glm::normalize(direction);
        }
        else {
            direction = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        data.direction = ApplyRandomDirection(direction, particle->shapeModule.random_Direction_Amount);
        return data;
    }

    // ========================================
    // CONE EMISSION (no position offset needed)
    // ========================================
    EmissionData ParticleSystem::GenerateConeEmission(ParticleComponent* particle) {
        EmissionData data;

        // Optional: Random position at cone base
        if (particle->shapeModule.cone_Radius > 0.0f) {
            float randAngle = RandomRange(0.0f, 2.0f * glm::pi<float>());
            float randRadius = glm::sqrt(RandomRange(0.0f, 1.0f)) * particle->shapeModule.cone_Radius;

            data.positionOffset = glm::vec3(
                cos(randAngle) * randRadius,
                0.0f,
                sin(randAngle) * randRadius
            );
        }
        else {
            data.positionOffset = glm::vec3(0.0f);
        }

        // Generate direction within cone
        float angleRad = glm::radians(particle->shapeModule.cone_Angle);
        float theta = RandomRange(0.0f, 2.0f * glm::pi<float>());
        float phi = RandomRange(0.0f, angleRad);

        float sinPhi = sin(phi);
        glm::vec3 localDir = glm::vec3(
            sinPhi * cos(theta),
            cos(phi),
            sinPhi * sin(theta)
        );

        // Rotate to match emission direction
        glm::vec3 baseDir = glm::normalize(particle->shapeModule.emission_Direction);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        if (abs(glm::dot(baseDir, up)) > 0.999f) {
            up = glm::vec3(1.0f, 0.0f, 0.0f);
        }

        glm::vec3 right = glm::normalize(glm::cross(up, baseDir));
        glm::vec3 forward = glm::normalize(glm::cross(baseDir, right));

        glm::mat3 rotationMatrix = glm::mat3(right, baseDir, forward);
        glm::vec3 worldDir = rotationMatrix * localDir;

        data.direction = ApplyRandomDirection(glm::normalize(worldDir), particle->shapeModule.random_Direction_Amount);
        return data;
    }

    // ========================================
    // SPHERE EMISSION
    // ========================================
    EmissionData ParticleSystem::GenerateSphereEmission(ParticleComponent* particle) {
        EmissionData data;

        // Generate random point on/in sphere
        float theta = RandomRange(0.0f, 2.0f * glm::pi<float>());
        float phi = RandomRange(0.0f, glm::pi<float>());

        float radiusMultiplier = 1.0f;
        if (!particle->shapeModule.emit_From_Shell) {
            // Emit from volume
            radiusMultiplier = glm::pow(RandomRange(0.0f, 1.0f), 1.0f / 3.0f);
        }

        // Spherical to Cartesian
        float sinPhi = sin(phi);
        glm::vec3 direction = glm::vec3(
            sinPhi * cos(theta),
            cos(phi),
            sinPhi * sin(theta)
        );

        // Position offset at sphere surface/volume
        data.positionOffset = direction * particle->shapeModule.sphere_Radius * radiusMultiplier;

        // Direction is outward from center
        data.direction = ApplyRandomDirection(glm::normalize(direction), particle->shapeModule.random_Direction_Amount);
        return data;
    }

    // ========================================
    // CIRCLE EMISSION 
    // ========================================
    EmissionData ParticleSystem::GenerateCircleEmission(ParticleComponent* particle) {
        EmissionData data;

        float arcRad = glm::radians(particle->shapeModule.circle_Arc);
        float angle = RandomRange(0.0f, arcRad);

        float radiusMultiplier = 1.0f;
        if (!particle->shapeModule.emit_From_Edge) {
            // Emit from disc area (sqrt for uniform distribution)
            radiusMultiplier = glm::sqrt(RandomRange(0.0f, 1.0f));
        }

        // Position on circle in XZ plane
        float actualRadius = particle->shapeModule.circle_Radius * radiusMultiplier;
        data.positionOffset = glm::vec3(
            cos(angle) * actualRadius,
            0.0f,
            sin(angle) * actualRadius
        );

        //// Direction: outward from circle + upward component
        //glm::vec3 outward = glm::normalize(glm::vec3(cos(angle), 0.0f, sin(angle)));
        //glm::vec3 baseDir = glm::normalize(particle->emission_Direction);

        //// Mix outward and upward direction
        //data.direction = glm::normalize(outward * 0.3f + baseDir * 0.7f);
        //data.direction = ApplyRandomDirection(data.direction, particle->random_Direction_Amount);
        // 
        //horizontal only
        data.direction = glm::vec3(
            cos(angle),  // Same angle as position
            0.0f,        // NO vertical component!
            sin(angle)
        );
        data.direction = ApplyRandomDirection(data.direction, particle->shapeModule.random_Direction_Amount);
        return data;
    }

    // ========================================
    // EDGE (LINE) EMISSION (FIXED - Now uses length!)
    // ========================================
    EmissionData ParticleSystem::GenerateEdgeEmission(ParticleComponent* particle) {
        EmissionData data;

        // Random position along line
        float t = RandomRange(-1.0f, 1.0f);

        // Position offset along the line (perpendicular to emission direction)
        glm::vec3 lineDir = glm::vec3(1.0f, 0.0f, 0.0f);  // Default: along X axis

        // If emission direction is along X, use Z instead
        glm::vec3 emitDir = glm::normalize(particle->shapeModule.emission_Direction);
        if (abs(glm::dot(emitDir, lineDir)) > 0.9f) {
            lineDir = glm::vec3(0.0f, 0.0f, 1.0f);
        }

        data.positionOffset = lineDir * t * particle->shapeModule.edge_Length * 0.5f;

        // Direction is the emission direction
        data.direction = ApplyRandomDirection(emitDir, particle->shapeModule.random_Direction_Amount);

        return data;
    }

    glm::vec3 ParticleSystem::ApplyRandomDirection(const glm::vec3& direction, float randomAmount) {
        if (randomAmount <= 0.0f) {
            return direction;  // No randomness
        }

        // Generate a completely random direction
        float theta = RandomRange(0.0f, 2.0f * glm::pi<float>());
        float phi = RandomRange(0.0f, glm::pi<float>());

        float sinPhi = sin(phi);
        glm::vec3 randomDir = glm::vec3(
            sinPhi * cos(theta),
            cos(phi),
            sinPhi * sin(theta)
        );
        glm::vec3 result = glm::normalize(
            glm::mix(direction, randomDir, randomAmount)
        );

        return result;
    }
    // ========================================
// PLAY - Start/resume particle emission and simulation
// ========================================
    void ParticleSystem::Play(ParticleComponent* particle) {
        if (!particle) return;

        particle->playback_State = PlayState::PLAY;

        LOGGING_INFO("Particle system PLAY\n");
    }

    // ========================================
    // PAUSE - Freeze everything (emission stops, simulation stops)
    // ========================================
    void ParticleSystem::Pause(ParticleComponent* particle) {
        if (!particle) return;

        particle->playback_State = PlayState::PAUSE;

        LOGGING_INFO("Particle system PAUSED\n");
    }

    // ========================================
    // STOP - Stop emission, let existing particles die naturally
    // ========================================
    void ParticleSystem::Stop(ParticleComponent* particle) {
        if (!particle) return;

        particle->playback_State = PlayState::STOP;

        // Reset timers
        particle->emitterTime = 0.0f;
        particle->durationCounter = 0.0f;

        LOGGING_INFO("Particle system STOPPED (particles will die naturally)\n");
    }

    // ========================================
    // RESTART - Stop and immediately play again from beginning
    // ========================================
    void ParticleSystem::Restart(ParticleComponent* particle) {
        if (!particle) return;

        // Clear all particles
        Clear(particle);

        // Reset timers
        particle->emitterTime = 0.0f;
        particle->durationCounter = 0.0f;

        // Start playing
        particle->playback_State = PlayState::PLAY;

        LOGGING_INFO("Particle system RESTARTED\n");
    }

    // ========================================
    // CLEAR - Kill all particles immediately
    // ========================================
    void ParticleSystem::Clear(ParticleComponent* particle) {
        if (!particle) return;

        // Map buffers
    
       
        

        LOGGING_INFO("All particles cleared (%zu particles killed)\n", particle->alive_Particles.size());
    }

    // ========================================
    // STATE QUERIES
    // ========================================
    bool ParticleSystem::IsPlaying(ParticleComponent* particle) {
        return particle && particle->playback_State == PlayState::PLAY;
    }

    bool ParticleSystem::IsPaused(ParticleComponent* particle) {
        return particle && particle->playback_State == PlayState::PAUSE;
    }

    bool ParticleSystem::IsStopped(ParticleComponent* particle) {
        return particle && particle->playback_State == PlayState::STOP;
    }
}