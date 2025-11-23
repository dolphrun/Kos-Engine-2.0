#pragma once

class BasicParticleData
{
public:
    std::vector<glm::vec3> particlePositions;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec2> sizes;
    std::vector<float> rotates;
    R_Texture* texture_IDs;
};

class BasicParticleInstance
{
public:
    glm::vec3 position{ 1.f };
    glm::vec2 scale{ 1.f };
    glm::vec4 color{ 1.f };
    float rotation;
    unsigned int textureID;
};

class BasicParticleMesh
{
public:
    unsigned int vaoid{};
    unsigned int vboid{};
};