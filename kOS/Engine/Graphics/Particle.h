#pragma once

class BasicParticleData
{
public:
    std::vector<glm::vec3> particlePositions;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec2> sizes;
    std::vector<glm::vec3> rotates;
    R_Texture* texture_IDs;
    float particleType{};
};

class BasicParticleInstance
{
public:
    glm::vec3 position{ 1.f };
    glm::vec2 scale{ 1.f };
    glm::vec4 color{ 1.f };
    glm::vec3 rotation;
    unsigned int textureID;
    float particleType;
};

class BasicParticleMesh
{
public:
    unsigned int vaoid{};
    unsigned int vboid{};
};