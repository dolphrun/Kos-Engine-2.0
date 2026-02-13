#include "Config/pch.h"
#include "DeSerialization/json_handler.h"
#include "PhysicsLayer.h"

namespace physicslayer {

	constexpr const char* configpath = "Engine/Physics/Layer.json";

    PhysicsLayer::PhysicsLayer() {
        //read collision layer file
        if (!std::filesystem::exists(configpath)) {

            // Create the directory structure if it doesn't exist
            std::filesystem::path path(configpath);
            std::filesystem::create_directories(path.parent_path());

            // add the "[]"
            serialization::JsonFileValidation(configpath);
        }
        else {
            for (int i = 0; i < size; ++i) {
                layerCollisions[i].reset(); // Initialize all bits to 0 (no collisions)
            }

        }



    }

    void PhysicsLayer::m_PrintCollisionMatrix() const {
        for (int i = 0; i < size; ++i) {
            std::ostringstream rowStream;
            for (int j = 0; j < size; ++j) {
                rowStream << layerCollisions[i][j] << " ";
            }
            LOGGING_INFO("Collision Matrix Row {}: {}", i, rowStream.str().c_str());
        }
    }

    void PhysicsLayer::m_SetCollision(int row, int col, bool value) {
        layerCollisions[row].set(col, value);
        layerCollisions[col].set(row, value); // Ensure symmetric relationship
    }
    bool PhysicsLayer::m_GetCollide(int layer1, int layer2) {
        return layerCollisions[layer1].test(layer2); // Check if layers should collide
    }

    std::bitset<size>* PhysicsLayer::m_GetMat() {
        return layerCollisions;
    }
    const std::bitset<size>* PhysicsLayer::m_GetMat() const {
        return layerCollisions;
    }


    std::string PhysicsLayer::ConvertLayerToString() const {
 
        // Concatenate all bitsets into one string with separators
        std::string concatenated;
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                if (layerCollisions[i][j]) {
                    concatenated += '1';
                }
                else {
                    concatenated += '0';
                }
          }
            concatenated += '|';
        }


        // Use the template function to write JSON
        return concatenated;
    }

    void PhysicsLayer::LoadCollisionLayer() {
        try {
            // Use the template function to read JSON
			auto collisionlayer = serialization::ReadJsonFile<LayerData>(configpath);

            // Split the concatenated string back into individual bitsets
            std::string remaining = collisionlayer.collisionData;

            for (int i = 0; i < size && !remaining.empty(); ++i) {
                size_t separatorPos = remaining.find('|');

                std::string rowStr;
                if (separatorPos != std::string::npos) {
                    rowStr = remaining.substr(0, separatorPos);
                    remaining = remaining.substr(separatorPos + 1);
                }
                else {
                    rowStr = remaining; // Last row
                    remaining.clear();
                }

                if (rowStr.length() == size) {
                    // Use m_SetCollision to set each bit individually
                    for (int j = 0; j <= i; ++j) {
                        bool shouldCollide = (rowStr[j] == '1');
                        m_SetCollision(i, j, shouldCollide);
                    }
                }
            }
            LOGGING_INFO("Successfully loaded collision matrix");
        }
        catch (const std::exception& e) {
            LOGGING_ERROR("Failed to load collision matrix: {}", e.what());
            // Initialize with default values on failure - reset all collisions
            for (int i = 0; i < size; ++i) {
                for (int j = i; j < size; ++j) {
                    m_SetCollision(i, j, false);
                }
            }
        }
    }


    void PhysicsLayer::SaveCollisionLayer() {
		physicslayer::LayerData collisionlayer;
		collisionlayer.collisionData = ConvertLayerToString();
		serialization::WriteJsonFile<LayerData>(configpath, &collisionlayer);
		LOGGING_INFO("Successfully saved collision matrix");

    }
}