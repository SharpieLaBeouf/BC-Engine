#include "BC_PCH.h"
#include "MeshComponents.h"

#include "Asset/AssetManagerAPI.h"

#include <yaml-cpp/yaml.h>

// TODO: Implement

namespace BC
{
    
#pragma region LODMeshComponent
    
    LODMeshComponent::LODMeshComponent(const LODMeshComponent& other)
    {

    }

    LODMeshComponent::LODMeshComponent(LODMeshComponent&& other) noexcept
    {
		m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

    }

    LODMeshComponent& LODMeshComponent::operator=(const LODMeshComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    LODMeshComponent& LODMeshComponent::operator=(LODMeshComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

		m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        return *this;
    }

    void LODMeshComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
            
        }
		out << YAML::EndMap;
    }

    bool LODMeshComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion
    
#pragma region MeshRendererComponent
    
    MeshRendererComponent::MeshRendererComponent(const MeshRendererComponent& other)
    {
        m_StaticMeshHandle = other.m_StaticMeshHandle;
        m_MaterialHandles = other.m_MaterialHandles;
        m_Active = other.m_Active;
        m_CastingShadow = other.m_CastingShadow;
        m_TransformedAABB = other.m_TransformedAABB;
        m_OctreeNeedsUpdate = other.m_OctreeNeedsUpdate;
        m_BoundingBoxNeedsUpdate = other.m_BoundingBoxNeedsUpdate;
        m_DisplayDebugAABB = other.m_DisplayDebugAABB;
    }

    MeshRendererComponent::MeshRendererComponent(MeshRendererComponent&& other) noexcept
    {
		m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        m_StaticMeshHandle = other.m_StaticMeshHandle;
        m_MaterialHandles = std::move(other.m_MaterialHandles);
        m_Active = other.m_Active;
        m_CastingShadow = other.m_CastingShadow;
        m_TransformedAABB = std::move(other.m_TransformedAABB);
        m_OctreeNeedsUpdate = other.m_OctreeNeedsUpdate;
        m_BoundingBoxNeedsUpdate = other.m_BoundingBoxNeedsUpdate;
        m_DisplayDebugAABB = other.m_DisplayDebugAABB;

        other.m_StaticMeshHandle = NULL_GUID;
        other.m_Active = false;
        other.m_CastingShadow = false;
        other.m_OctreeNeedsUpdate = true;
        other.m_BoundingBoxNeedsUpdate = true;
        other.m_DisplayDebugAABB = false;
    }

    MeshRendererComponent& MeshRendererComponent::operator=(const MeshRendererComponent& other)
    {
        if (this == &other)
            return *this;

        m_StaticMeshHandle = other.m_StaticMeshHandle;
        m_MaterialHandles = other.m_MaterialHandles;
        m_Active = other.m_Active;
        m_CastingShadow = other.m_CastingShadow;
        m_TransformedAABB = other.m_TransformedAABB;
        m_OctreeNeedsUpdate = other.m_OctreeNeedsUpdate;
        m_BoundingBoxNeedsUpdate = other.m_BoundingBoxNeedsUpdate;
        m_DisplayDebugAABB = other.m_DisplayDebugAABB;

        return *this;
    }

    MeshRendererComponent& MeshRendererComponent::operator=(MeshRendererComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

		m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        m_StaticMeshHandle = other.m_StaticMeshHandle;
        m_MaterialHandles = std::move(other.m_MaterialHandles);
        m_Active = other.m_Active;
        m_CastingShadow = other.m_CastingShadow;
        m_TransformedAABB = std::move(other.m_TransformedAABB);
        m_OctreeNeedsUpdate = other.m_OctreeNeedsUpdate;
        m_BoundingBoxNeedsUpdate = other.m_BoundingBoxNeedsUpdate;
        m_DisplayDebugAABB = other.m_DisplayDebugAABB;

        other.m_StaticMeshHandle = NULL_GUID;
        other.m_Active = false;
        other.m_CastingShadow = false;
        other.m_OctreeNeedsUpdate = true;
        other.m_BoundingBoxNeedsUpdate = true;
        other.m_DisplayDebugAABB = false;

        return *this;
    }

    void MeshRendererComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
            
        }
		out << YAML::EndMap;
    }

    bool MeshRendererComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

    void MeshRendererComponent::UpdateTransformedAABB()
    {
		Entity entity = GetEntity();
		if (!entity) 
        {
			BC_CORE_ERROR("MeshRendererComponent::UpdateTransformedAABB: Cannot UpdateTransformedAABB - Current Entity Is Invalid!");
			return;
		}

		if (m_StaticMeshHandle == NULL_GUID)
			return;

		if (auto mesh_asset = AssetManager::GetAsset<StaticMesh>(m_StaticMeshHandle); mesh_asset) 
        {
			// Define the 8 corner points of the AABB
            const auto& bounds = mesh_asset->GetMeshBounds();
			std::array<glm::vec3, 8> corners = 
            {
				mesh_asset->GetMeshBounds().BoundsMin,
				glm::vec3(bounds.BoundsMax.x, bounds.BoundsMin.y, bounds.BoundsMin.z),
				glm::vec3(bounds.BoundsMax.x, bounds.BoundsMax.y, bounds.BoundsMin.z),
				glm::vec3(bounds.BoundsMin.x, bounds.BoundsMax.y, bounds.BoundsMin.z),
				glm::vec3(bounds.BoundsMin.x, bounds.BoundsMin.y, bounds.BoundsMax.z),
				glm::vec3(bounds.BoundsMax.x, bounds.BoundsMin.y, bounds.BoundsMax.z),
				glm::vec3(bounds.BoundsMax.x, bounds.BoundsMax.y, bounds.BoundsMax.z),
				glm::vec3(bounds.BoundsMin.x, bounds.BoundsMax.y, bounds.BoundsMax.z)
			};

			// Transform the corner points
			const glm::mat4& global_transform = GetComponent<TransformComponent>().GetGlobalMatrix();
			for (int i = 0; i < 8; ++i) 
            {
				glm::vec4 transformed_corner = global_transform * glm::vec4(corners[i], 1.0f);
				corners[i] = glm::vec3(transformed_corner);
			}

			// Find the new BoundsMin and BoundsMax
			glm::vec3 new_min = corners[0];
			glm::vec3 new_max = corners[0];
			for (int i = 1; i < 8; ++i) {
				new_min = glm::min(new_min, corners[i]);
				new_max = glm::max(new_max, corners[i]);
			}
			m_TransformedAABB.BoundsMin = new_min;
			m_TransformedAABB.BoundsMax = new_max;

			m_BoundingBoxNeedsUpdate = false;
		}
    }

#pragma endregion

#pragma region SkinnedMeshRendererComponent
    
    SkinnedMeshRendererComponent::SkinnedMeshRendererComponent(const SkinnedMeshRendererComponent& other)
    {
        m_SkinnedMeshHandle = other.m_SkinnedMeshHandle;
        m_MaterialHandles = other.m_MaterialHandles;
        m_SkeletonHandle = other.m_SkeletonHandle;
        m_SkeletonBoneMapping = other.m_SkeletonBoneMapping;
        m_FinalBoneTransformations = other.m_FinalBoneTransformations;
        m_Active = other.m_Active;
        m_CastingShadow = other.m_CastingShadow;
        m_TransformedAABB = other.m_TransformedAABB;
        m_OctreeNeedsUpdate = other.m_OctreeNeedsUpdate;
        m_BoundingBoxNeedsUpdate = other.m_BoundingBoxNeedsUpdate;
        m_DisplayDebugAABB = other.m_DisplayDebugAABB;
    }

    SkinnedMeshRendererComponent::SkinnedMeshRendererComponent(SkinnedMeshRendererComponent&& other) noexcept
    {
		m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        m_SkinnedMeshHandle = other.m_SkinnedMeshHandle;
        m_MaterialHandles = std::move(other.m_MaterialHandles);
        m_SkeletonHandle = other.m_SkeletonHandle;
        m_SkeletonBoneMapping = std::move(other.m_SkeletonBoneMapping);
        m_FinalBoneTransformations = std::move(other.m_FinalBoneTransformations);
        m_Active = other.m_Active;
        m_CastingShadow = other.m_CastingShadow;
        m_TransformedAABB = std::move(other.m_TransformedAABB);
        m_OctreeNeedsUpdate = other.m_OctreeNeedsUpdate;
        m_BoundingBoxNeedsUpdate = other.m_BoundingBoxNeedsUpdate;
        m_DisplayDebugAABB = other.m_DisplayDebugAABB;

        other.m_SkinnedMeshHandle = NULL_GUID;
        other.m_SkeletonHandle = NULL_GUID;
        other.m_Active = false;
        other.m_CastingShadow = false;
        other.m_OctreeNeedsUpdate = true;
        other.m_BoundingBoxNeedsUpdate = true;
        other.m_DisplayDebugAABB = false;
    }

    SkinnedMeshRendererComponent& SkinnedMeshRendererComponent::operator=(const SkinnedMeshRendererComponent& other)
    {
        if (this == &other)
            return *this;

        m_SkinnedMeshHandle = other.m_SkinnedMeshHandle;
        m_MaterialHandles = other.m_MaterialHandles;
        m_SkeletonHandle = other.m_SkeletonHandle;
        m_SkeletonBoneMapping = other.m_SkeletonBoneMapping;
        m_FinalBoneTransformations = other.m_FinalBoneTransformations;
        m_Active = other.m_Active;
        m_CastingShadow = other.m_CastingShadow;
        m_TransformedAABB = other.m_TransformedAABB;
        m_OctreeNeedsUpdate = other.m_OctreeNeedsUpdate;
        m_BoundingBoxNeedsUpdate = other.m_BoundingBoxNeedsUpdate;
        m_DisplayDebugAABB = other.m_DisplayDebugAABB;

        return *this;
    }

    SkinnedMeshRendererComponent& SkinnedMeshRendererComponent::operator=(SkinnedMeshRendererComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

		m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;
        
        m_SkinnedMeshHandle = other.m_SkinnedMeshHandle;
        m_MaterialHandles = std::move(other.m_MaterialHandles);
        m_SkeletonHandle = other.m_SkeletonHandle;
        m_SkeletonBoneMapping = std::move(other.m_SkeletonBoneMapping);
        m_FinalBoneTransformations = std::move(other.m_FinalBoneTransformations);
        m_Active = other.m_Active;
        m_CastingShadow = other.m_CastingShadow;
        m_TransformedAABB = std::move(other.m_TransformedAABB);
        m_OctreeNeedsUpdate = other.m_OctreeNeedsUpdate;
        m_BoundingBoxNeedsUpdate = other.m_BoundingBoxNeedsUpdate;
        m_DisplayDebugAABB = other.m_DisplayDebugAABB;

        other.m_SkinnedMeshHandle = NULL_GUID;
        other.m_SkeletonHandle = NULL_GUID;
        other.m_Active = false;
        other.m_CastingShadow = false;
        other.m_OctreeNeedsUpdate = true;
        other.m_BoundingBoxNeedsUpdate = true;
        other.m_DisplayDebugAABB = false;

        return *this;
    }

    void SkinnedMeshRendererComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
            
        }
		out << YAML::EndMap;
    }

    bool SkinnedMeshRendererComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

    void SkinnedMeshRendererComponent::UpdateTransformedAABB()
    {
		Entity entity = GetEntity();
		if (!entity) 
        {
			BC_CORE_ERROR("SkinnedMeshRendererComponent::UpdateTransformedAABB: Cannot UpdateTransformedAABB - Current Entity Is Invalid!");
			return;
		}

		if (m_SkinnedMeshHandle == NULL_GUID)
			return;

		if (auto mesh_asset = AssetManager::GetAsset<StaticMesh>(m_SkinnedMeshHandle); mesh_asset) 
        {
			// Define the 8 corner points of the AABB
            const auto& bounds = mesh_asset->GetMeshBounds();
			std::array<glm::vec3, 8> corners = 
            {
				mesh_asset->GetMeshBounds().BoundsMin,
				glm::vec3(bounds.BoundsMax.x, bounds.BoundsMin.y, bounds.BoundsMin.z),
				glm::vec3(bounds.BoundsMax.x, bounds.BoundsMax.y, bounds.BoundsMin.z),
				glm::vec3(bounds.BoundsMin.x, bounds.BoundsMax.y, bounds.BoundsMin.z),
				glm::vec3(bounds.BoundsMin.x, bounds.BoundsMin.y, bounds.BoundsMax.z),
				glm::vec3(bounds.BoundsMax.x, bounds.BoundsMin.y, bounds.BoundsMax.z),
				glm::vec3(bounds.BoundsMax.x, bounds.BoundsMax.y, bounds.BoundsMax.z),
				glm::vec3(bounds.BoundsMin.x, bounds.BoundsMax.y, bounds.BoundsMax.z)
			};

			// Transform the corner points
			const glm::mat4& global_transform = GetComponent<TransformComponent>().GetGlobalMatrix();
			for (int i = 0; i < 8; ++i) 
            {
				glm::vec4 transformed_corner = global_transform * glm::vec4(corners[i], 1.0f);
				corners[i] = glm::vec3(transformed_corner);
			}

			// Find the new BoundsMin and BoundsMax
			glm::vec3 new_min = corners[0];
			glm::vec3 new_max = corners[0];
			for (int i = 1; i < 8; ++i) {
				new_min = glm::min(new_min, corners[i]);
				new_max = glm::max(new_max, corners[i]);
			}
			m_TransformedAABB.BoundsMin = new_min;
			m_TransformedAABB.BoundsMax = new_max;

			m_BoundingBoxNeedsUpdate = false;
		}
    }

#pragma endregion

}
