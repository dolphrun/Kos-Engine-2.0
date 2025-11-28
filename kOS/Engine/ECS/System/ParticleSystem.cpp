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
            particle->particle_List.clear();
            particle->particle_List.reserve(particle->max_Particles);
          
            // Initialize emitter timers
            particle->emitterTime = 0.f;
            particle->durationCounter = 0.f;

            });

        onDeregister.Add([&](EntityID id) {
            auto* particle = m_ecs.GetComponent<ParticleComponent>(id);

            // Clear tracking data
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

            if (particle->playback_State == PlayState::PAUSE && particle->particle_List.empty()) {
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

            std::shared_ptr<R_Texture> textureResource = m_resourceManager.GetResource<R_Texture>(particle->textureGUID);
            float type = 0.f;
            if (particle->particleType == ParticleComponent::ParticleType::THREE_DIMENSIONAL_ROTATION_BILLBOARD)
                type = 1.f;
            m_graphicsManager.gm_PushBasicParticleData(BasicParticleData{ sending.positions_Particle, sending.colors, sending.sizes, sending.rotates , textureResource.get(), type });
        }
    }


    void ParticleSystem::EmitParticle(EntityID entityId, const glm::vec3& particle_position, const glm::vec3& velocity, float lifetime, ParticleComponent*& particle, TransformComponent*& transform) {
        // Check if we have any free slots
        if (particle->particle_List.size() >= particle->max_Particles) {
            LOGGING_WARN("No free particle slots for entity %d\n", entityId);
            return;
        }

        ParticleData pd;
        pd.lifespan = lifetime;
        pd.lifetime = lifetime;
        pd.color = particle->colorModule.enabled ? RandomColourRange(particle->colorModule.start_Color, particle->colorModule.end_Color) : particle->colorModule.start_Color;
        pd.size = glm::vec2(particle->sizeModule.enabled ? AbsRandomRange(particle->sizeModule.start_Size, particle->sizeModule.end_Size) : particle->sizeModule.start_Size);
        pd.velocity = velocity;
        pd.position = particle_position;
        pd.rotation = particle->rotationModule.enabled ? glm::radians(RandomRange(particle->rotationModule.start_Rotation, particle->rotationModule.end_Rotation)) : glm::radians(particle->rotationModule.start_Rotation);


        //init particle data
        particle->particle_List.emplace_back(pd);
        //particle->particle_List.push_back(std::move(pd));
    }

    void ParticleSystem::UpdateParticleLifetimes(float dt, ParticleComponent*& particle) {

        if (particle->playback_State == PlayState::PAUSE) {
            return;  // Don't update anything when paused
        }


        // === PRE-COMPUTE EVERYTHING ONCE ===
        const bool updateSize = particle->sizeModule.enabled;
        const bool updateColor = particle->colorModule.enabled;
        const bool updateRotation = particle->rotationModule.enabled;
        const bool updateVelocity = particle->velocityModule.enabled;
        const bool updateForce = particle->forceModule.enabled;
        //const bool updateAttractor = particle->attractorModule.enabled;
        const bool updateGravity = particle->gravityModule.enabled;
        const bool updateTrailing = particle->trailingModule.enabled;

        const bool updateNoise = particle->noiseModule.enabled;

        if (updateNoise) {
            m_noiseTime += dt;
        }

        // Pre-compute deltas
        const float sizeDelta = updateSize ? (particle->sizeModule.end_Size - particle->sizeModule.start_Size) : 0.0f;
        const float sizeStart = particle->sizeModule.start_Size;

        const glm::vec4 colorDelta = updateColor ? (particle->colorModule.end_Color - particle->colorModule.start_Color) : glm::vec4(0);
        const glm::vec4 colorStart = particle->colorModule.start_Color;

        const glm::vec3 rotModRad = glm::radians(particle->rotationModule.rotation_Modifier) * dt;

        // Update trailing module's rotating end point
        //if (updateTrailing && particle->trailingModule.rotateEndPoint) {
        //    auto& trail = particle->trailingModule;

        //    // Update rotation angle
        //    float angle = trail.timeAccum * trail.rotationSpeed;

        //    // Calculate new end point position on circular orbit
        //    glm::vec3 axis = glm::normalize(trail.rotationAxis);

        //    // Find two perpendicular vectors to the rotation axis
        //    glm::vec3 perpAxis1, perpAxis2;
        //    if (fabs(axis.y) > 0.9f) {
        //        perpAxis1 = glm::normalize(glm::cross(axis, glm::vec3(1, 0, 0)));
        //    }
        //    else {
        //        perpAxis1 = glm::normalize(glm::cross(axis, glm::vec3(0, 1, 0)));
        //    }
        //    perpAxis2 = glm::normalize(glm::cross(axis, perpAxis1));

        //    // Calculate rotating end point
        //    trail.endPoint = trail.rotationCenter +
        //        perpAxis1 * cos(angle) * trail.rotationRadius +
        //        perpAxis2 * sin(angle) * trail.rotationRadius;
        //}

        // === OPTIMIZED LOOP ===
        for (int i = static_cast<int>(particle->particle_List.size()) - 1; i >= 0; --i) {

            ParticleData& pd = particle->particle_List[i];
            // Fast clamp
            float t = 1.f - (pd.lifespan / pd.lifetime);
            t = (t < 0.0f) ? 0.0f : ((t > 1.0f) ? 1.0f : t);

            if (updateVelocity) {
                switch (particle->velocityModule.mode) {
                    case Absolute:
                        pd.velocity = particle->velocityModule.velocity_Modifier * t + pd.velocity * (1.0f - t);
                        break;
                    case Additive:
                        pd.velocity += particle->velocityModule.velocity_Modifier * (t * dt);
                        break;
                    case Multiplier: {
                        float factor = 1.0f + (glm::length(particle->velocityModule.velocity_Modifier) - 1.0f) * t;
                        pd.velocity *= factor;
                        break;
                    }
                }
            }

            if (updateGravity) {
                pd.velocity += particle->gravityModule.gravity * dt;
            }

            if (updateForce) {
                pd.velocity += particle->forceModule.force * dt;
            }

            // === OPTIMIZED VISUAL UPDATES ===
            if (updateSize) {
                pd.size = glm::vec2(sizeStart + sizeDelta * t);
            }

            if (updateColor) {
                pd.color = colorStart + colorDelta * t;
            }

            if (updateRotation) {
                pd.rotation += rotModRad;
            }

            if (updateNoise) {
                glm::vec3 noiseForce = CalculateNoiseForce(pd, particle->noiseModule, m_noiseTime, t);

                if (particle->noiseModule.damping) {
                    float velocityMag = glm::length(pd.velocity);
                    float dampFactor = 1.0f / (1.0f + velocityMag * 0.1f);
                    pd.velocity += noiseForce * dampFactor * dt;
                }
                else {
                    pd.velocity += noiseForce * dt;
                }
            }


            // ENHANCED TRAILING MODULE - Twister Effect
            if (updateTrailing) {
                glm::vec3 start = particle->trailingModule.startPoint;
                glm::vec3 end = particle->trailingModule.endPoint;

                // ===== STEP 1: Calculate position along the straight line path =====
                // Increment the particle's progress along the path
                float particleAge = (pd.lifetime - pd.lifespan);
                float pathProgress = glm::clamp(particleAge * particle->trailingModule.pathSpeed / pd.lifetime, 0.0f, 1.0f);

                // Smooth easing for more natural movement
                float easedProgress = pathProgress < 0.5f
                    ? 2.0f * pathProgress * pathProgress
                    : 1.0f - glm::pow(-2.0f * pathProgress + 2.0f, 2.0f) / 2.0f;

                // Calculate the base position along the straight line
                glm::vec3 basePosition = start + (end - start) * easedProgress;

                // ===== STEP 2: Create spiral/helix around the line =====
                glm::vec3 pathDirection = end - start;
                float pathLength = glm::length(pathDirection);

                if (pathLength > 0.001f) {
                    pathDirection = glm::normalize(pathDirection);

                    // Create perpendicular basis vectors for the spiral
                    glm::vec3 perpAxis = (fabs(pathDirection.y) > 0.9f) ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
                    glm::vec3 right = glm::normalize(glm::cross(pathDirection, perpAxis));
                    glm::vec3 up = glm::normalize(glm::cross(right, pathDirection));

                    // Calculate spiral angle based on progress and frequency
                    float spiralAngle = easedProgress * particle->trailingModule.spiralFrequency * glm::two_pi<float>()
                        + particle->trailingModule.timeAccum; // Add time offset for rotation

                    // Calculate spiral intensity (can peak in the middle or stay constant)
                    float spiralIntensity;
                    if (particle->trailingModule.spiralIntensityCurve > 0.01f) {
                        // Peak in the middle, fade at start and end
                        spiralIntensity = glm::sin(easedProgress * glm::pi<float>());
                    }
                    else {
                        // Constant intensity, but fade near the end
                        spiralIntensity = glm::clamp((1.0f - easedProgress) * 2.0f, 0.0f, 1.0f);
                    }

                    // Calculate offset from the center line
                    glm::vec3 spiralOffset = (right * cos(spiralAngle) + up * sin(spiralAngle))
                        * particle->trailingModule.spiralRadius
                        * spiralIntensity;

                    // ===== STEP 3: Set particle position on the helix =====
                    pd.position = basePosition + spiralOffset;

                    // ===== STEP 4: Calculate velocity for smooth spiral motion =====
                    // Velocity along the path
                    glm::vec3 forwardVel = pathDirection * particle->trailingModule.pathSpeed;

                    // Tangential velocity for the spiral (perpendicular to radius)
                    glm::vec3 tangentialDir = -right * sin(spiralAngle) + up * cos(spiralAngle);
                    float tangentialSpeed = particle->trailingModule.spiralRadius
                        * particle->trailingModule.spiralFrequency
                        * glm::two_pi<float>()
                        * particle->trailingModule.pathSpeed
                        * spiralIntensity / pd.lifetime;

                    glm::vec3 tangentialVel = tangentialDir * tangentialSpeed;

                    // Combine forward and tangential velocity
                    pd.velocity = forwardVel + tangentialVel;

                    // ===== STEP 5: Check if particle reached the end =====
                    float distToEnd = glm::length(end - pd.position);
                    if (distToEnd < particle->trailingModule.arrivalThreshold || pathProgress >= 0.99f) {
                        pd.lifespan = 0.0f; // Kill particle
                        continue;
                    }
                }
                else {
                    // No path, just stay at start
                    pd.position = start;
                    pd.velocity = glm::vec3(0);
                }
            }

            //=== LIFETIME UPDATE ===
            pd.lifespan -= dt;

            if (particle->particleFade == ParticleFade::Lifetime) {
                pd.color.a = pd.lifespan / pd.lifetime;
            }  
            if(!updateTrailing) pd.position += pd.velocity * dt;

            if (pd.lifespan <= 0.0f) {
                particle->particle_List[i] = particle->particle_List.back();
                //particle->particle_List[i] = std::move(particle->particle_List.back());
                particle->particle_List.pop_back();

            }
        } 
    }

    void ParticleSystem::UpdateEmitters(float dt,EntityID id, ParticleComponent*& particleComp,  TransformComponent*& transform) {
        //const auto& entities = m_entities.Data();
        if (particleComp->playback_State == PlayState::PAUSE) {
            return;
        }

        // Check if emitter should be active
        bool shouldEmit = false;
        bool particles_Alive = false;

        if (particleComp->playback_State == PlayState::STOP) {
            shouldEmit = false;
        } else if (particleComp->looping) {
            particleComp->durationCounter += dt;
            
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
                //glm::vec3 vel = emission.direction * particleComp->start_Velocity;        

               // Apply world rotation to emission direction
                glm::vec3 dir = emission.direction;
                glm::vec3 offset = emission.positionOffset;

                // Rotate direction by WORLD rotation (includes parent + local)
                glm::quat worldQ = glm::quat(glm::radians(transform->WorldTransformation.rotation));
                dir = worldQ * dir;

                // Rotate offset by LOCAL rotation only (entity's own rotation)
                glm::quat localQ = glm::quat(glm::radians(transform->LocalTransformation.rotation));
                offset = localQ * offset;

                // Set velocity using rotated direction
                glm::vec3 vel = dir * particleComp->start_Velocity;

                // WorldTransformation.position already includes parent position!
                glm::vec3 pos = transform->WorldTransformation.position + offset;


                particleComp->particle_Spawn_Location = transform->WorldTransformation.position;
                //random to the lifetime
                float particle_Lifetime = particleComp->start_Lifetime;
                if (particleComp->lifetime_Random_Enable) {
                    particle_Lifetime = RandomRange(particleComp->start_Lifetime, particleComp->end_Lifetime);
                }

                if (particleComp->trailingModule.enabled)
                {
                    glm::vec3 start = particleComp->trailingModule.startPoint;
                    glm::vec3 end = particleComp->trailingModule.endPoint;

                    EmitTrailParticles(dt, particleComp, start, end, id, transform);
                }
                else {
                    EmitParticle(id, pos, vel, particle_Lifetime, particleComp, transform);
                }
                particleComp->emitterTime -= emissionInterval;
            }
        }
    }


    // NEW: Optimized position extraction
    void ParticleSystem::ExtractParticleDataOptimized(ParticleComponent* particle,
        ParticleInstance& data) {

        if (!particle || particle->particle_List.empty()) {
            data.positions_Particle.clear();
            data.colors.clear();
            data.rotates.clear();
            data.sizes.clear();
            return;
        }

        size_t aliveCount = particle->particle_List.size();

        // OPTIMIZATION: Reserve once to avoid reallocations
        if (data.positions_Particle.capacity() < particle->max_Particles) {
            data.positions_Particle.reserve(particle->max_Particles);
            data.colors.reserve(particle->max_Particles);
            data.rotates.reserve(particle->max_Particles);
            data.sizes.reserve(particle->max_Particles);
        }

        data.positions_Particle.resize(aliveCount);
        data.colors.resize(aliveCount);
        data.rotates.resize(aliveCount);
        data.sizes.resize(aliveCount);

        // Extract data from particle_List
        for (size_t i = 0; i < aliveCount; ++i) {
            const ParticleData& pd = particle->particle_List[i];

            data.positions_Particle[i] = pd.position;
            data.colors[i] = pd.color;
            data.sizes[i] = pd.size;
            data.rotates[i] = pd.rotation;
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

        if (particle->playback_State == PlayState::STOP) {
            particle->emitterTime = 0.0f;
            particle->durationCounter = 0.0f;
        }
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
        particle->particle_List.clear();  // Actually clear the particles!
      
        LOGGING_INFO("All particles cleared (%zu particles killed)\n", particle->particle_List.size());
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

    void ParticleSystem::EmitTrailParticles(
        float dt,
        ParticleComponent* particle,
        const glm::vec3& start,
        const glm::vec3& end,
        EntityID id,
        TransformComponent*& transform)
    {
        auto& trail = particle->trailingModule;

        // Update time accumulator
        trail.timeAccum += dt;

        // Check if emission duration has ended
        if (trail.timeAccum > trail.spawnDuration) {
            return;
        }

        // Calculate number of particles to spawn this frame
        trail.spawnAccum += trail.spawnRate * dt;
        int count = static_cast<int>(trail.spawnAccum);
        trail.spawnAccum -= count;

        // Emit particles from the start point
        for (int i = 0; i < count; i++) {
            // Always spawn at start point
            glm::vec3 pos = start;

            // Initial velocity can be zero or small random - the update loop will handle motion
            glm::vec3 vel = glm::vec3(
                RandomRange(-0.1f, 0.1f),
                RandomRange(-0.1f, 0.1f),
                RandomRange(-0.1f, 0.1f)
            );

            // Random lifetime
            float life = particle->start_Lifetime;
            if (particle->lifetime_Random_Enable) {
                life = RandomRange(particle->start_Lifetime, particle->end_Lifetime);
            }

            EmitParticle(id, pos, vel, life, particle, transform);
        }
    }


    float ParticleSystem::PerlinNoise1D(float x) {
        float i = glm::floor(x);
        float f = glm::fract(x);

        float u = f * f * (3.0f - 2.0f * f);

        return glm::mix(Hash(i), Hash(i + 1.0f), u) * 2.0f - 1.0f;
    }

    float ParticleSystem::PerlinNoise2D(float x, float y) {
        float ix = glm::floor(x);
        float iy = glm::floor(y);
        float fx = glm::fract(x);
        float fy = glm::fract(y);

        float ux = fx * fx * (3.0f - 2.0f * fx);
        float uy = fy * fy * (3.0f - 2.0f * fy);

        float a = Hash2D(ix, iy);
        float b = Hash2D(ix + 1.0f, iy);
        float c = Hash2D(ix, iy + 1.0f);
        float d = Hash2D(ix + 1.0f, iy + 1.0f);

        float result = glm::mix(
            glm::mix(a, b, ux),
            glm::mix(c, d, ux),
            uy
        );

        return result * 2.0f - 1.0f;
    }

    float ParticleSystem::PerlinNoise3D(float x, float y, float z) {
        float ix = glm::floor(x);
        float iy = glm::floor(y);
        float iz = glm::floor(z);
        float fx = glm::fract(x);
        float fy = glm::fract(y);
        float fz = glm::fract(z);

        float ux = fx * fx * (3.0f - 2.0f * fx);
        float uy = fy * fy * (3.0f - 2.0f * fy);
        float uz = fz * fz * (3.0f - 2.0f * fz);

        float c000 = Hash3D(ix, iy, iz);
        float c100 = Hash3D(ix + 1.0f, iy, iz);
        float c010 = Hash3D(ix, iy + 1.0f, iz);
        float c110 = Hash3D(ix + 1.0f, iy + 1.0f, iz);
        float c001 = Hash3D(ix, iy, iz + 1.0f);
        float c101 = Hash3D(ix + 1.0f, iy, iz + 1.0f);
        float c011 = Hash3D(ix, iy + 1.0f, iz + 1.0f);
        float c111 = Hash3D(ix + 1.0f, iy + 1.0f, iz + 1.0f);

        float result = glm::mix(
            glm::mix(
                glm::mix(c000, c100, ux),
                glm::mix(c010, c110, ux),
                uy
            ),
            glm::mix(
                glm::mix(c001, c101, ux),
                glm::mix(c011, c111, ux),
                uy
            ),
            uz
        );

        return result * 2.0f - 1.0f;
    }

    float ParticleSystem::FractalNoise3D(const glm::vec3& pos, int octaves, float persistence, float lacunarity) {
        float total = 0.0f;
        float amplitude = 1.0f;
        float maxValue = 0.0f;
        float frequency = 1.0f;

        for (int i = 0; i < octaves; i++) {
            glm::vec3 p = pos * frequency;
            total += PerlinNoise3D(p.x, p.y, p.z) * amplitude;

            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= lacunarity;
        }

        return total / maxValue;
    }

    glm::vec3 ParticleSystem::CalculateNoiseForce(const ParticleData& pd, const NoiseModule& noise, float time, float lifeProgress) {
        glm::vec3 samplePos = pd.position * noise.frequency + noise.positionOffset;
        samplePos += noise.scrollSpeed * time;

        glm::vec3 noiseForce(0.0f);

        switch (noise.quality) {
        case NoiseModule::LOW: {
            noiseForce.x = PerlinNoise1D(samplePos.x);
            noiseForce.y = PerlinNoise1D(samplePos.y + 100.0f);
            noiseForce.z = PerlinNoise1D(samplePos.z + 200.0f);
            break;
        }
        case NoiseModule::MEDIUM: {
            noiseForce.x = PerlinNoise2D(samplePos.y, samplePos.z);
            noiseForce.y = PerlinNoise2D(samplePos.z, samplePos.x);
            noiseForce.z = PerlinNoise2D(samplePos.x, samplePos.y);
            break;
        }
        case NoiseModule::HIGH: {
            if (noise.octaves > 1) {
                noiseForce.x = FractalNoise3D(samplePos, noise.octaves, noise.octaveMultiplier, noise.octaveScale);
                noiseForce.y = FractalNoise3D(samplePos + glm::vec3(100, 0, 0), noise.octaves, noise.octaveMultiplier, noise.octaveScale);
                noiseForce.z = FractalNoise3D(samplePos + glm::vec3(0, 100, 0), noise.octaves, noise.octaveMultiplier, noise.octaveScale);
            }
            else {
                noiseForce.x = PerlinNoise3D(samplePos.x, samplePos.y, samplePos.z);
                noiseForce.y = PerlinNoise3D(samplePos.x + 100.0f, samplePos.y, samplePos.z);
                noiseForce.z = PerlinNoise3D(samplePos.x, samplePos.y + 100.0f, samplePos.z);
            }
            break;
        }
        }

        noiseForce *= noise.strength;
        noiseForce *= noise.strengthMultiplier;

        if (noise.remapEnabled) {
            float remapValue = glm::mix(noise.remapCurveStart, noise.remapCurveEnd, lifeProgress);
            noiseForce *= remapValue;
        }

        return noiseForce;
    }


    // ========================================
    // Scripting Helper function
    // ========================================
    void ParticleSystem::setTrailPoint(ParticleComponent* particle, const glm::vec3& start, const glm::vec3 end) {
        particle->trailingModule.startPoint = start;
        particle->trailingModule.endPoint = end;
    }
    void ParticleSystem::updateTrailEndPoint(ParticleComponent* particle, const glm::vec3& end) {
        particle->trailingModule.endPoint = end;
    }

    void ParticleSystem::setSize(ParticleComponent* particle, const float& start, const float& end, bool enable) {
        particle->sizeModule.enabled = enable;
        particle->sizeModule.start_Size = start;
        particle->sizeModule.end_Size = end;
    }
    void ParticleSystem::setColor(ParticleComponent* particle, const glm::vec4& start, const glm::vec4 end, bool enable) {
        particle->colorModule.enabled = enable;
        particle->colorModule.start_Color = start;
        particle->colorModule.end_Color = end;
    }
    void ParticleSystem::setRotation(ParticleComponent* particle, const glm::vec3& start, const glm::vec3 end, const glm::vec3 modifier, bool enable) {
        particle->rotationModule.enabled = enable;
        particle->rotationModule.start_Rotation = start;
        particle->rotationModule.end_Rotation = end;
        particle->rotationModule.rotation_Modifier = modifier;
    }
    void ParticleSystem::setVelocityModifier(ParticleComponent* particle, const glm::vec3& velocity_Modifier, const Velocity_Mode mode, bool enable) {
        particle->velocityModule.enabled = enable;
        particle->velocityModule.mode = mode;
        particle->velocityModule.velocity_Modifier = velocity_Modifier;

    }
    void ParticleSystem::setForce(ParticleComponent* particle, const glm::vec3& force, bool enable) {
        particle->forceModule.enabled = enable;
        particle->forceModule.force = force;
    }


}