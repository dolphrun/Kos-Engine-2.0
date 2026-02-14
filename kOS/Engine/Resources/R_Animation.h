#pragma once
#include "Config/pch.h"
#include "Resource.h"
#include "ResourceHeader.h"
class R_Animation :public Resource
{
	class Bone
	{
	public:
		int m_ID;
		glm::mat4 Interpolate(float time) const;

		const std::string& GetName() const;
		int GetID() const;

		std::string m_Name;


		std::vector<glm::vec3> m_Positions;
		std::vector<float> m_PosTimes;

		std::vector<glm::quat> m_Rotations;
		std::vector<float> m_RotTimes;

		std::vector<glm::vec3> m_Scales;
		std::vector<float> m_ScaleTimes;

		int FindIndex(const std::vector<float>& times, float animTime) const;
		float GetFactor(float start, float end, float time) const;

		glm::vec3 InterpolatePosition(float time) const;
		glm::quat InterpolateRotation(float time) const;
		glm::vec3 InterpolateScale(float time) const;
	};

	struct NodeData
	{
		std::string name;
		glm::mat4 transformation;
		std::vector<NodeData> children;
	};
public:
	using Resource::Resource;
	void Load() override;
	void Unload() override;
	void Update(float currentTime, const glm::mat4& parentTransform, const glm::mat4& globalInverse,
		const std::unordered_map<std::string, int>& boneMap,
		const std::vector<BoneInfo>& boneInfo, std::vector<glm::mat4>& outputMatrices);

	float GetCurrentTime() const { return m_CurrentTime; };
	float GetDuration() const { return m_Duration; };
	float GetTicksPerSecond() const { return m_TicksPerSecond; };
	const NodeData& GetRootNode() const { return m_RootNode; };
	//const std::vector<glm::mat4> GetBoneFinalMatrices() const { return m_FinalBoneTransforms; };

	float m_CurrentTime{};
	bool baked = false;
	REFLECTABLE(R_Animation);
private:
	struct OptimizedNode {
		glm::mat4 transformation;
		int boneIndex;          // -1 if this node is not a bone
		const Bone* bonePtr;    // Direct pointer to the animation track (to avoid FindBone)
		std::vector<OptimizedNode> children;
	};

	
	void BakeAnimationHierarchy(const NodeData& rawNode, OptimizedNode& optNode, const std::unordered_map<std::string, int>& boneMap);
	void CalculateBoneTransformOptimized(const OptimizedNode& node, const glm::mat4& parentTransform, const std::vector<BoneInfo>& boneInfo, std::vector<glm::mat4>& outputMatrices);

	const Bone* FindBone(const std::string& name) const {
		std::unordered_map<std::string, Bone>::const_iterator it = m_Bones.find(name);
		return it != m_Bones.end() ? &it->second : nullptr;
	}
	//void CalculateBoneTransform(const NodeData& node, const glm::mat4& parentTransform, const glm::mat4& globalInverse,
	//	const std::unordered_map<std::string, int>& boneMap,
	//	const std::vector<BoneInfo>& boneInfo, size_t boneCount = 0);

	template <typename T> T DecodeBinary(std::string& bin, int& offset);
	NodeData NodeDataParser(std::string& buffer, int& offset);
	float m_Duration{};
	float m_TicksPerSecond{};

	std::string m_Name{};
	std::unordered_map<std::string, Bone> m_Bones{};
	NodeData m_RootNode;
	OptimizedNode m_optNode;
};