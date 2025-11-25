#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H
#include "ECS/ECS.h"
#include "System.h"


namespace ecs {

    struct ParticleInstance {
        std::vector<glm::vec3> positions_Particle;
        std::vector<glm::vec4> colors;
        std::vector<glm::vec2> sizes;
        std::vector<glm::vec3> rotates;
    };
    struct EmissionData {
        glm::vec3 positionOffset;  // Offset from emitter position
        glm::vec3 direction;       // Normalized direction
    };
    enum STATE {
        POSITION,
        VELOCITY,
        PHASE,
        ACTIVE,
        LIFESPAN,
        counter  
    };


    class ParticleSystem : public ISystem {
    private:
    private:
        // Fast xorshift32 RNG using only 32-bit unsigned int
        struct FastRNG {
            unsigned int state;

            FastRNG(unsigned int seed = 0) {
                state = seed ? seed : 0x12345678u;
            }

            inline unsigned int next() {
                // Xorshift algorithm (Marsaglia 2003)
                unsigned int x = state;
                x ^= x << 13;
                x ^= x >> 17;
                x ^= x << 5;
                state = x;
                return x;
            }

            inline float nextFloat() {
                // Convert to [0, 1) range
                return (next() >> 8) * 5.960464477539063e-8f; // / 16777216.0f
            }
        };

        FastRNG m_rng;
    public:
        using ISystem::ISystem;
        void Init() override;
        void Update() override;
       
       

        // Spawn a new particle
        void EmitParticle(EntityID entityId, const glm::vec3& particle_position,
        const glm::vec3& velocity, float lifetime, ParticleComponent*& particle);
        // Update particle lifetimes and kill dead particles
        void UpdateParticleLifetimes(float dt, ParticleComponent*& particle);    
        // Handle particle emission from emitter components
        void UpdateEmitters(float dt, EntityID id, ParticleComponent*& particleComp,  TransformComponent* transform);
       
        void ExtractParticleDataOptimized(ParticleComponent* particle, ParticleInstance& data);

        void EmitTrailParticles(float dt, ParticleComponent* particle,const glm::vec3& start, const glm::vec3& end,EntityID id);
        
        //===========================================
        // Play state FUNCTION
        //===========================================
        void Play(ParticleComponent* particle);
        void Pause(ParticleComponent* particle);
        void Stop(ParticleComponent* particle);
        void Restart(ParticleComponent* particle);
        void Clear(ParticleComponent* particle);  // Kill all particles immediately
        bool IsPlaying(ParticleComponent* particle);
        bool IsPaused(ParticleComponent* particle);
        bool IsStopped(ParticleComponent* particle);

        //===========================================
        // SHAPES FUNCTION
        //===========================================
        EmissionData GenerateBoxEmission(ParticleComponent* particle);
        EmissionData GenerateConeEmission(ParticleComponent* particle);
        EmissionData GenerateSphereEmission(ParticleComponent* particle);
        EmissionData GenerateCircleEmission(ParticleComponent* particle);
        EmissionData GenerateEdgeEmission(ParticleComponent* particle);

        //===========================================
        // HELPER FUNCTIONS
        //===========================================

        void* getVoid(ParticleComponent* particle, STATE state);
       
        inline float RandomRange(float minValue, float maxValue) {
            return minValue + m_rng.nextFloat() * (maxValue - minValue);
        }

        inline float AbsRandomRange(float minValue, float maxValue) {
            float absMin = std::abs(minValue);
            float absMax = std::abs(maxValue);
            if (absMin > absMax) std::swap(absMin, absMax);
            return absMin + m_rng.nextFloat() * (absMax - absMin);
        }

        inline glm::vec4 RandomColourRange(const glm::vec4& color_Start, const glm::vec4& color_End) {
            return glm::vec4(
                color_Start.r + m_rng.nextFloat() * (color_End.r - color_Start.r),
                color_Start.g + m_rng.nextFloat() * (color_End.g - color_Start.g),
                color_Start.b + m_rng.nextFloat() * (color_End.b - color_Start.b),
                1.0f
            );
        }

        inline glm::vec3 RandomRange(const glm::vec3& start, const glm::vec3& end) {
            return glm::vec3(
                start.x + m_rng.nextFloat() * (end.x - start.x),
                start.y + m_rng.nextFloat() * (end.y - start.y),
                start.z + m_rng.nextFloat() * (end.z - start.z)
            );
        }

        // Apply random chaos to direction
        glm::vec3 ApplyRandomDirection(const glm::vec3& direction, float randomAmount);

        REFLECTABLE(ParticleSystem);
    };
}
#endif