#pragma once

class Deathzone : public TemplateSC {
public:

    void Start() override {
		physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {
			if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Player") {

				// Audio implementation for player sfx from hurtbox
				if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
					std::vector<ecs::AudioFile*> playerHurtSfxPool;

					for (auto& af : ac->audioFiles) {
						if (af.isSFX) {
							playerHurtSfxPool.push_back(&af);
						}
					}

					if (!playerHurtSfxPool.empty()) {
						int idx = rand() % static_cast<int>(playerHurtSfxPool.size());
						std::cout << "[Deathzone] Random SFX index chosen = " << idx << std::endl;

						playerHurtSfxPool[idx]->requestPlay = true;
					}
				}

				// Deal damage
				//ecsPtr->GetComponent<PlayerManagerScript>(col.otherEntityID)->currPlayerHitPoints -= bulletDamage;
				ecsPtr->GetComponent<PlayerManagerScript>(col.otherEntityID)->TakeDamage(100);
			}

		});
    }

    void Update() override {

    }

    REFLECTABLE(Deathzone)
};
