#pragma once

// Core Headers
#include "Component Base.h"

#include "Asset/Asset.h"

// C++ Standard Library Headers
#include <string>

// External Vendor Library Headers
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace YAML 
{
	class Emitter;
	class Node;
}

namespace BC
{
    using TransformComponentFlag = uint8_t;
    enum : uint8_t 
    {
        TransformFlag_None = 0,

        TransformFlag_PropertiesUpdated = 1U << 0,          // Only add this flag where there have been changes made to the transform properties, e.g., position changed, or matrix changed, clear when local matrix has been updated
        TransformFlag_GlobalTransformUpdated = 1U << 1,     // Only add this flag where there have been changes made to a parent in the hierarchy, e.g., when parent transform has updated, this will be marked dirty for update
    };

    struct TransformComponent : public ComponentBase
    {
    
    public:

        TransformComponent() = default;
        ~TransformComponent() = default;

        TransformComponent(const TransformComponent& other);
        TransformComponent(TransformComponent&& other) noexcept;
        TransformComponent& operator=(const TransformComponent& other);
        TransformComponent& operator=(TransformComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::TransformComponent; }
        
    #pragma region General Methods

        void UpdateLocalMatrix(bool scale_updated = false);
        void OnTransformUpdated(bool scale_updated = false);

    #pragma endregion

    #pragma region Setters

        void SetPosition(const glm::vec3& new_position, bool global = false);
        void SetPositionX(float new_x, bool global = false);
        void SetPositionY(float new_y, bool global = false);
        void SetPositionZ(float new_z, bool global = false);

        void SetOrientation(const glm::quat& new_orientation, bool global = false);
        void SetOrientationEuler(const glm::vec3& new_orientation_euler, bool global = false);
        void SetOrientationEulerX(float new_x, bool global = false);
        void SetOrientationEulerY(float new_y, bool global = false);
        void SetOrientationEulerZ(float new_z, bool global = false);

        void SetScale(const glm::vec3& new_scale, bool global = false);
        void SetScaleX(float new_x, bool global = false);
        void SetScaleY(float new_y, bool global = false);
        void SetScaleZ(float new_z, bool global = false);

        void Translate(const glm::vec3& position_delta, bool global = false);
        void TranslateX(float position_x_delta, bool global = false);
        void TranslateY(float position_y_delta, bool global = false);
        void TranslateZ(float position_z_delta, bool global = false);

        void Rotate(const glm::quat& orientation_delta, bool global = false);
        void RotateEuler(const glm::vec3& orientation_euler_delta, bool global = false);
        void RotateEulerX(float orientation_x_euler_delta, bool global = false);
        void RotateEulerY(float orientation_y_euler_delta, bool global = false);
        void RotateEulerZ(float orientation_z_euler_delta, bool global = false);

        void Scale(const glm::vec3& scale_delta, bool global = false);
        void ScaleX(float scale_x_delta, bool global = false);
        void ScaleY(float scale_y_delta, bool global = false);
        void ScaleZ(float scale_z_delta, bool global = false);

        void SetForwardDirection(const glm::vec3& direction);
        void SetRightDirection(const glm::vec3& direction);
        void SetUpDirection(const glm::vec3& direction);

        void LookAt(const glm::vec3& target_position);

        void SetMatrix(const glm::mat4& new_matrix, bool global = false);

    #pragma endregion
    
    #pragma region Getters

        const glm::vec3& GetLocalPosition() const;
        const glm::quat& GetLocalOrientation() const;
        const glm::vec3& GetLocalOrientationEulerHint() const;
        const glm::vec3& GetLocalScale() const;

        glm::vec3 GetGlobalPosition();
        glm::quat GetGlobalOrientation();
        glm::vec3 GetGlobalOrientationEulerHint();
        glm::vec3 GetGlobalScale();
        
        glm::vec3 GetLocalForwardDirection() const;
        glm::vec3 GetLocalRightDirection() const;
        glm::vec3 GetLocalUpDirection() const;

        glm::vec3 GetGlobalForwardDirection();
        glm::vec3 GetGlobalRightDirection();
        glm::vec3 GetGlobalUpDirection();

        const glm::mat4& GetLocalMatrix();
        const glm::mat4& GetGlobalMatrix();
    
    #pragma endregion

    #pragma region Flags

        void AddFlag(TransformComponentFlag flag);
        void RemoveFlag(TransformComponentFlag flag);
        void ClearFlags();

        bool CheckFlag(TransformComponentFlag flag) const;
        bool NoFlagsSet() const;

        TransformComponentFlag GetFlags() const;
    
    #pragma endregion
    
    #pragma region Static Helpers
    
        static glm::vec3 GetPositionFromMatrix(const glm::mat4& matrix);
        static glm::quat GetOrientationFromMatrix(const glm::mat4& matrix);
        static glm::vec3 GetScaleFromMatrix(const glm::mat4& matrix);
        
        static glm::vec3 GetForwardDirectionFromMatrix(const glm::mat4& matrix);
        static glm::vec3 GetRightDirectionFromMatrix(const glm::mat4& matrix);
        static glm::vec3 GetUpDirectionFromMatrix(const glm::mat4& matrix);
        
    #pragma endregion

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

    private:
        
        glm::vec3 m_LocalPosition       = glm::vec3(0.0f);
        glm::vec3 m_LocalEulerHint      = glm::vec3(0.0f);
        glm::quat m_LocalOrientation    = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 m_LocalScale          = glm::vec3(1.0f);

        glm::mat4 m_LocalMatrix         = glm::mat4(1.0f);
        glm::mat4 m_GlobalMatrix        = glm::mat4(1.0f);

        TransformComponentFlag m_StateFlags     = TransformFlag_PropertiesUpdated | TransformFlag_GlobalTransformUpdated;

        friend struct MetaComponent;

    };

    struct MetaComponent : public ComponentBase
    {
    
    public:

        MetaComponent() = default;
        ~MetaComponent() = default;

        MetaComponent(const MetaComponent& other);
        MetaComponent(MetaComponent&& other) noexcept;
        MetaComponent& operator=(const MetaComponent& other);
        MetaComponent& operator=(MetaComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::MetaComponent; }
        
        // --- Name ---
        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& new_name) { m_Name = new_name; }
        void SetUniqueName(const std::string& new_name);

        // --- GUID ---
        GUID GetEntityGUID() const { return m_EntityID; }
        void SetEntityGUID(GUID entity_guid) { m_EntityID = entity_guid; }
        
        // --- Hierarchy ---
        bool HasParent() const { return m_Parent != NULL_GUID; }
        bool HasChildren() const { return !m_Children.empty(); }
        
        void AttachParent(GUID new_parent_guid);
        void DetachParent();

        void DetachChildren();
        void RehomeChildren(GUID new_parent_guid);

        Entity FindChild(GUID child_guid) const;
        Entity FindChild(const std::string& child_name) const;

        GUID GetRootParentGUID() const;
        GUID GetParentGUID() const { return m_Parent; }
        const std::vector<GUID>& GetChildrenGUID() const { return m_Children; }
        
        // --- Scripts ---

        /// @brief Will add a script of type to Scripts
        void AddScript(const std::string& script_name, bool active_state = true)
        {
            m_Scripts.emplace_back(script_name, active_state);
        }

        /// @brief Will Remove First Script Found With Name
        void RemoveScript(const std::string& script_name) 
        {
            auto it = std::find_if(m_Scripts.begin(), m_Scripts.end(), [&](const std::pair<std::string, bool>& pair)
            {
                return pair.first == script_name;
            });

            if (it != m_Scripts.end())
                m_Scripts.erase(it);
        }

        /// @brief Remove script by index if in range
        void RemoveScript(size_t index)
        {
            if (index >= m_Scripts.size() || index < 0)
                return;

            m_Scripts.erase(m_Scripts.begin() + index);
        }

        /// @brief Will Update Active State of First Script Found With Name
        void UpdateScriptActiveState(const std::string& script_name, bool active_state)
        {
            auto it = std::find_if(m_Scripts.begin(), m_Scripts.end(), [&](const std::pair<std::string, bool>& pair)
            {
                return pair.first == script_name;
            });

            if (it != m_Scripts.end())
                it->second = active_state;
        }

        /// @brief Update active state of script at index if in range
        void UpdateScriptActiveState(size_t index, bool active_state)
        {
            if (index >= m_Scripts.size() || index < 0)
                return;

            m_Scripts[index].second = active_state;
        }

        /// @brief Update script name of script at index if in range
        void UpdateScriptNameAtIndex(size_t index, const std::string& script_name)
        {
            if (index >= m_Scripts.size() || index < 0)
                return;

            m_Scripts[index].first = script_name;
        }
        
        /// @brief Will Get Active State of First Script Found With Name
        bool GetScriptActive(const std::string& script_name) const
        {
            auto it = std::find_if(m_Scripts.begin(), m_Scripts.end(), [&](const std::pair<std::string, bool>& pair)
            {
                return pair.first == script_name;
            });

            if (it != m_Scripts.end())
                return it->second;
            
            return false;
        }

        /// @brief Get active state of script at index if in range
        bool GetScriptActive(size_t index) const
        {
            if (index >= m_Scripts.size() || index < 0)
                return false;

            return m_Scripts[index].second;
        }

        /// @brief Get script name at index if in range, otherwise returns empty string
        const std::string& GetScriptNameAtIndex(size_t index) const
        {
            if (index >= m_Scripts.size() || index < 0)
                return {};

            return m_Scripts[index].first;
        }

        bool HasScripts() const { return !m_Scripts.empty(); }

        const std::vector<std::pair<std::string, bool>>& GetScripts() const { return m_Scripts; }
        
        // --- Prefabs ---

        AssetHandle GetPrefabHandle() const { return m_PrefabSourceHandle; }
        void SetPrefabHandle(AssetHandle handle) { m_PrefabSourceHandle = handle; }

        // --- Serialisation ---
        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

    private:
        
        /// @brief The name of the current entity
        std::string m_Name = "";

        /// @brief The GUID of the current entity
        GUID m_EntityID = NULL_GUID;

        /// @brief The GUID of the parent entity which this entity is attached to
        GUID m_Parent = NULL_GUID;

        /// @brief A vector of children GUID's attached to this entity
        std::vector<GUID> m_Children = {};

        /// @brief A vector of scripts attached to this entity and their active status. First = script name, Second = active status
        std::vector<std::pair<std::string, bool>> m_Scripts = {};

        /// @brief This is the source asset handle of the prefab that this object is created from and linked to
        AssetHandle m_PrefabSourceHandle = NULL_GUID;

        friend class HierarchyPanel;

    };

}