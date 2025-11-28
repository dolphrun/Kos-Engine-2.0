#ifndef PARTICLECOMPONENT_H
#define PARTICLECOMPONENT_H

#include "Component.h"


namespace ecs {

	struct ParticleData {
		glm::vec4 color;
		glm::vec3 position;
		glm::vec2 size;
		glm::vec3 velocity;
		float lifespan;
		float lifetime;
		glm::vec3 rotation;
		int textureID;
	};

	enum class PlayState {
		PLAY,
		PAUSE,
		STOP
	};

	enum Velocity_Mode {
		Absolute,
		Additive,
		Multiplier
	};

	enum EmissionShape {
		BOX,       
		CONE,       
		SPHERE,    
		CIRCLE,    
		EDGE        
	};

	enum ParticleFade {
		Color,
		Lifetime
	};

	struct ShapeModule {
		EmissionShape type = EmissionShape::CONE;
		glm::vec3 emission_Direction = glm::vec3(0.f, 1.f, 0.f);
		float random_Direction_Amount;
		glm::vec3 box_Size;
		
		float sphere_Radius;
		bool emit_From_Shell;

		float cone_Angle;
		glm::vec3 cone_Direction;
		float cone_Radius;

		float circle_Radius;
		float circle_Arc;
		bool emit_From_Edge;

		float edge_Length;

		REFLECTABLE(ShapeModule, type, emission_Direction,
					box_Size,
					sphere_Radius, emit_From_Shell,
					cone_Angle, cone_Direction, cone_Radius,
					circle_Radius, circle_Arc, emit_From_Edge,
					edge_Length, random_Direction_Amount);
	};

	struct VelocityOverLifetimeModule {
		bool enabled = false;
		Velocity_Mode mode = Velocity_Mode::Absolute;
		glm::vec3 velocity_Modifier = glm::vec3(0);

		REFLECTABLE(VelocityOverLifetimeModule,enabled, mode, velocity_Modifier);
	};

	struct ForceOverLifetimeModule {
		bool enabled = false;
		glm::vec3 force = glm::vec3(0);
		
		REFLECTABLE(ForceOverLifetimeModule, enabled, force);
	};

	struct GravityModule {
		bool enabled = false;
		glm::vec3 gravity = glm::vec3(0.f, -9.8f, 0.f);
		REFLECTABLE(GravityModule, enabled, gravity);
	};

	struct SizeOverLifetimeModule {
		bool enabled = false;
		float start_Size = 1.0f;
		float end_Size = 1.0f;

		REFLECTABLE(SizeOverLifetimeModule, enabled, start_Size, end_Size);
	};

	struct ColorOverLifetimeModule {
		bool enabled = false;
		glm::vec4 start_Color = glm::vec4(1.f);
		glm::vec4 end_Color = glm::vec4(0.f);
		
		REFLECTABLE(ColorOverLifetimeModule, enabled, start_Color, end_Color);
	};

	struct RotationOverLifetimeModule {
		bool enabled = false;
		glm::vec3 start_Rotation = glm::vec3(0.f);
		glm::vec3 end_Rotation = glm::vec3(0.f);
		glm::vec3 rotation_Modifier = glm::vec3(0.f);

		REFLECTABLE(RotationOverLifetimeModule, enabled, start_Rotation, end_Rotation, rotation_Modifier);
	};

	struct TrailingModule {
		bool enabled = false;

		glm::vec3 startPoint = glm::vec3(0);     // Fixed start point
		glm::vec3 endPoint = glm::vec3(20.f);    // Dynamic end point (will rotate)

		// End point rotation settings
		bool rotateEndPoint = true;              // Enable/disable end point rotation
		float rotationSpeed = 2.0f;              // Radians per second for end point orbit
		float rotationRadius = 5.0f;            // Radius of the circular orbit
		glm::vec3 rotationCenter = glm::vec3(20.f, 0.f, 0.f); // Center of rotation
		glm::vec3 rotationAxis = glm::vec3(0.f, 1.f, 0.f);    // Axis to rotate around (Y-axis by default)

		float spawnDuration = 5.f;               // How long to emit particles
		float spawnRate = 40.0f;                // Particles per second

		// Spiral/twister settings
		float spiralRadius = 5.f;                // Radius of the spiral/helix around the path
		float spiralFrequency = 3.0f;            // Number of complete rotations along the path
		float spiralIntensityCurve = 0.5f;       // 0 = constant spiral, 1 = peak at middle

		// Movement settings
		float pathSpeed = 2.0f;                  // How fast particles move along the path (0-1 per second)
		float arrivalThreshold = 0.5f;           // Distance at which particle "arrives" at end

		float timeAccum = 0.0f;                  // Time accumulated since start
		float spawnAccum = 0.0f;                 // Fractional particle spawn accumulator


		REFLECTABLE(TrailingModule, enabled, startPoint, endPoint,
			rotateEndPoint, rotationSpeed, rotationRadius, rotationCenter, rotationAxis,
			spawnDuration, spawnRate,
			spiralRadius, spiralFrequency, spiralIntensityCurve,
			pathSpeed, arrivalThreshold,
			timeAccum, spawnAccum);
	};


	struct NoiseModule {
		bool enabled = false;

		// Noise strength
		float strength = 1.0f;
		glm::vec3 strengthMultiplier = glm::vec3(1.0f);

		// Frequency controls how "zoomed in" the noise is
		float frequency = 0.5f;

		// Scrolling speed - animates the noise over time
		glm::vec3 scrollSpeed = glm::vec3(0.0f);

		// Damping - reduces particle velocity influence
		bool damping = true;

		// Octaves for fractal noise
		int octaves = 1;
		float octaveMultiplier = 0.5f;
		float octaveScale = 2.0f;

		// Quality settings
		enum NoiseQuality {
			LOW,
			MEDIUM,
			HIGH
		};
		NoiseQuality quality = MEDIUM;

		// Remap curve
		bool remapEnabled = false;
		float remapCurveStart = 1.0f;
		float remapCurveEnd = 1.0f;

		// Position offset
		glm::vec3 positionOffset = glm::vec3(0.0f);

		REFLECTABLE(NoiseModule, enabled, strength, strengthMultiplier,
			frequency, scrollSpeed, damping,
			octaves, octaveMultiplier, octaveScale,
			quality, remapEnabled, remapCurveStart, remapCurveEnd,
			positionOffset);
	};



	class ParticleComponent : public Component {
	public:

		enum ParticleType
		{
			TWO_DIMENSIONAL_ROTATION_BILLBOARD,
			THREE_DIMENSIONAL_ROTATION_BILLBOARD
		} particleType;

		//NO CHANGES
		int max_Particles = 255; //max particle size
		float duration = 5.0f;
		bool looping = true;
		bool play_On_Awake = true;
		glm::vec3 particle_Spawn_Location = glm::vec3(0.f);

		//Lifetime
		float start_Lifetime = 3.0f;
		float end_Lifetime	 = 7.0f;
		bool  lifetime_Random_Enable = true;
		float start_Velocity = 5.0f;

		utility::GUID textureGUID;

		PlayState playback_State = PlayState::PLAY;

		ParticleFade particleFade = ParticleFade::Lifetime;

		//Color over lifetime
		ColorOverLifetimeModule colorModule;

		// Velocity over lifetime
		VelocityOverLifetimeModule velocityModule;
		
		//Force over Lifetime
		ForceOverLifetimeModule forceModule;
		
		//Gravity
		GravityModule gravityModule;

		//Shape module
		ShapeModule shapeModule;

		// Size over lifetime
		SizeOverLifetimeModule sizeModule;

		// Rotation over lifetime
		RotationOverLifetimeModule rotationModule;

		//Attraction MOdule
		//AttractorModule attractorModule; 

		//Trailing Module
		TrailingModule trailingModule;

		NoiseModule noiseModule;



		//FOR THE ALIVE PARTICLES
		std::vector<ParticleData> particle_List;



		//EMISSTION RATE
		float emitterTime = 0.f;
		float durationCounter = 0.f;
		float emissionInterval = 0.1f;

		REFLECTABLE(ParticleComponent, particleType, duration, looping, play_On_Awake, 
					start_Lifetime, end_Lifetime, lifetime_Random_Enable,
					textureGUID,
					start_Velocity, playback_State, particleFade,
					velocityModule,forceModule, shapeModule, colorModule, sizeModule, rotationModule, gravityModule, trailingModule, noiseModule,
					emissionInterval);
	};
}
#endif