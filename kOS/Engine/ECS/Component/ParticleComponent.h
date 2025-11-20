#ifndef PARTICLECOMPONENT_H
#define PARTICLECOMPONENT_H

#include "Component.h"


namespace ecs {
	//USING SOA
	struct ParticleVisual_List {
		std::vector<glm::vec4> colors;
		std::vector<glm::vec2> sizes;
		std::vector<float> rotation;
	};


	struct ParticleData {
		glm::vec4 color;
		glm::vec3 position;
		float size;
		glm::vec3 velocity;
		float lifespan;
		float lifetime;
		float rotation;
		glm::vec2 padding;

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
	
	struct AttractorModule{
		bool enabled = false;

		glm::vec3 targetPosition = glm::vec3(0);   // for absorption
		float attractionStrength = 0.0f;

		float explosionStrength = 0.0f;

		float whirlpoolStrength = 0.0f;
		float whirlpoolRadius = 1.0f;

		bool useInverseFalloff = true;  // stronger when particles are closer (black-hole style)
		REFLECTABLE(AttractorModule, enabled,
			targetPosition, attractionStrength,
			explosionStrength,
			whirlpoolStrength, whirlpoolRadius,
			useInverseFalloff);
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
		float start_Rotation = 0.f;
		float end_Rotation = 0.f;
		float rotation_Modifier = 0.f;

		REFLECTABLE(RotationOverLifetimeModule, enabled, start_Rotation, end_Rotation, rotation_Modifier);
	};

	class ParticleComponent : public Component {
	public:
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

		PlayState playback_State = PlayState::PLAY;

		//Color over lifetime
		ColorOverLifetimeModule colorModule;

		// Velocity over lifetime
		VelocityOverLifetimeModule velocityModule;
		
		//Force over Lifetime
		ForceOverLifetimeModule forceModule;
		
		//Shape module
		ShapeModule shapeModule;

		// Size over lifetime
		SizeOverLifetimeModule sizeModule;

		// Rotation over lifetime
		RotationOverLifetimeModule rotationModule;

		//Attraction MOdule
		AttractorModule attractorModule;

		//Drag Gravity Damping
		glm::vec3 gravity = glm::vec3(0.0f, -9.8f, 0.0f);
		glm::vec3 gravity_Prev = gravity;
 

		//FOR THE ALIVE PARTICLES
		std::vector<short> freeIndices;                       
		std::vector<short> alive_Particles;
		int alive_no_Of_Particles = 0;
		std::vector<ParticleData> particle_List;



		//EMISSTION RATE
		float emitterTime = 0.f;
		float durationCounter = 0.f;
		float emissionInterval = 0.1f;


		//Per particle visual data
		ParticleVisual_List visualData_List;

		REFLECTABLE(ParticleComponent, duration, looping, play_On_Awake, 
					start_Lifetime, end_Lifetime, lifetime_Random_Enable,
					start_Velocity, playback_State,
					velocityModule,forceModule, shapeModule, colorModule, sizeModule, rotationModule, attractorModule,
					gravity,
					emissionInterval);
	};
}
#endif