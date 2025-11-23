#include "Config/pch.h"
#include "R_Font.h"
#include "STB_IMAGE/stb_image.h"

void R_Font::Load()
{
    m_atlasRGBA.clear();
    m_characters.clear();
    m_atlasWidth = m_atlasHeight = 0;

    std::ifstream ifs(this->m_filePath.string().c_str(), std::ios::binary);
    if (!ifs) {
        std::cerr << "ERROR: cannot open file: " << this->m_filePath.string() << "\n";
        return;
    }

    char magic[4];
    ifs.read(magic, 4);
    if (ifs.gcount() != 4 || std::string(magic, 4) != "FNTC") {
        std::cerr << "ERROR: bad magic or not a FNTC file\n";
        return;
    }

    auto read_u32 = [&]()->uint32_t { uint32_t v; ifs.read(reinterpret_cast<char*>(&v), sizeof(v)); return v; };
    auto read_i32 = [&]()->int32_t { int32_t v; ifs.read(reinterpret_cast<char*>(&v), sizeof(v)); return v; };
    auto read_u8 = [&]()->uint8_t { uint8_t v; ifs.read(reinterpret_cast<char*>(&v), sizeof(v)); return v; };
    auto read_f = [&]()->float { float v; ifs.read(reinterpret_cast<char*>(&v), sizeof(v)); return v; };

    uint32_t version = read_u32();
    if (version != 1) {
        std::cerr << "WARNING: unknown version: " << version << " (expected 1)\n";
    }

    m_atlasWidth = static_cast<int>(read_u32());
    m_atlasHeight = static_cast<int>(read_u32());
    uint32_t pngSize = read_u32();

    std::vector<unsigned char> pngBytes;
    if (pngSize) {
        pngBytes.resize(pngSize);
        ifs.read(reinterpret_cast<char*>(pngBytes.data()), pngSize);
    }

    // decode PNG bytes into m_atlasRGBA
    if (!pngBytes.empty()) {
        int w, h, comp;
        stbi_set_flip_vertically_on_load(0); // try without flipping first
        unsigned char* img = stbi_load_from_memory(
            pngBytes.data(),
            (int)pngBytes.size(),
            &w, &h,
            &comp,
            4 // force RGBA output
        );

        if (!img) {
            std::cerr << "ERROR: failed to decode PNG in file\n";
            return;
        }

        std::cout << "Decoded PNG: " << w << "x" << h
            << " original components=" << comp
            << " forced=4\n";

        m_atlasWidth = w;
        m_atlasHeight = h;
        m_atlasRGBA.assign(img, img + (w * h * 4));
        stbi_image_free(img);

        if (m_atlasRGBA.size() != (size_t)(w * h * 4)) {
            std::cerr << "ERROR: unexpected atlas buffer size: "
                << m_atlasRGBA.size() << " expected " << (w * h * 4) << "\n";
        }
    }
    else {
        std::cerr << "WARNING: no PNG embedded in .fntc file\n";
        m_atlasRGBA.clear();
    }

    uint32_t numChars = read_u32();
    //std::cout << "Reading " << numChars << " characters...\n";

    for (uint32_t i = 0; i < numChars; ++i) {
        uint8_t ch = read_u8();
        int32_t sx = read_i32();
        int32_t sy = read_i32();
        int32_t bx = read_i32();
        int32_t by = read_i32();
        uint32_t adv = read_u32();
        float u0 = read_f();
        float v0 = read_f();
        float u1 = read_f();
        float v1 = read_f();

        CharacterData cd;
        cd.m_textureID = 0;
        cd.m_size = glm::ivec2(sx, sy);
        cd.m_bearing = glm::ivec2(bx, by);
        cd.m_advance = adv;
        cd.m_topLeftTexCoords = glm::vec2(u0, v0);
        cd.m_bottomRightTexCoords = glm::vec2(u1, v1);

        m_characters[static_cast<char>(ch)] = cd;
    }

    if (m_atlasRGBA.empty()) {
        std::cerr << "ERROR: atlas RGBA buffer is empty, nothing to upload\n";
        return;
    }

    // Upload OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        m_atlasWidth,
        m_atlasHeight,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        m_atlasRGBA.data()
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Assign texture to all characters
    for (auto& kv : m_characters) {
        kv.second.m_textureID = textureID;
    }

    //std::cout << "Loaded atlas texture (ID=" << textureID << ")\n";
}



void R_Font::Unload()
{

}
