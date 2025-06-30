#include "BC_PCH.h"
#include "CoreComponents.h"

#include "MeshComponents.h"
#include "PhysicsComponents.h"

#include <yaml-cpp/yaml.h>

namespace BC
{

#pragma region Transform Component

#pragma region Contructors and Operators

    TransformComponent::TransformComponent(const TransformComponent& other)
    {
        m_LocalPosition = other.m_LocalPosition;
        m_LocalEulerHint = other.m_LocalEulerHint;
        m_LocalOrientation = other.m_LocalOrientation;
        m_LocalScale = other.m_LocalScale;
        m_LocalMatrix = other.m_LocalMatrix;
        m_GlobalMatrix = other.m_GlobalMatrix;

        m_StateFlags = TransformFlag_PropertiesUpdated | TransformFlag_GlobalTransformUpdated;
    }

    TransformComponent::TransformComponent(TransformComponent&& other) noexcept
    {
        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        m_LocalPosition = other.m_LocalPosition;        other.m_LocalPosition = glm::vec3(0.0f);
        m_LocalEulerHint = other.m_LocalEulerHint;      other.m_LocalEulerHint = glm::vec3(0.0f);
        m_LocalOrientation = other.m_LocalOrientation;  other.m_LocalOrientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        m_LocalScale = other.m_LocalScale;              other.m_LocalScale = glm::vec3(1.0f);
        m_LocalMatrix = other.m_LocalMatrix;            other.m_LocalMatrix = glm::mat4(1.0f);
        m_GlobalMatrix = other.m_GlobalMatrix;          other.m_GlobalMatrix = glm::mat4(1.0f);

        m_StateFlags = TransformFlag_PropertiesUpdated | TransformFlag_GlobalTransformUpdated;
        other.m_StateFlags = TransformFlag_PropertiesUpdated | TransformFlag_GlobalTransformUpdated;
    }

    TransformComponent& TransformComponent::operator=(const TransformComponent& other)
    {
        if (this == &other)
            return *this;

        m_LocalPosition = other.m_LocalPosition;
        m_LocalEulerHint = other.m_LocalEulerHint;
        m_LocalOrientation = other.m_LocalOrientation;
        m_LocalScale = other.m_LocalScale;
        m_LocalMatrix = other.m_LocalMatrix;
        m_GlobalMatrix = other.m_GlobalMatrix;

        m_StateFlags = TransformFlag_PropertiesUpdated | TransformFlag_GlobalTransformUpdated;

        return *this;
    }

    TransformComponent& TransformComponent::operator=(TransformComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        m_LocalPosition = other.m_LocalPosition;        other.m_LocalPosition = glm::vec3(0.0f);
        m_LocalEulerHint = other.m_LocalEulerHint;      other.m_LocalEulerHint = glm::vec3(0.0f);
        m_LocalOrientation = other.m_LocalOrientation;  other.m_LocalOrientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        m_LocalScale = other.m_LocalScale;              other.m_LocalScale = glm::vec3(1.0f);
        m_LocalMatrix = other.m_LocalMatrix;            other.m_LocalMatrix = glm::mat4(1.0f);
        m_GlobalMatrix = other.m_GlobalMatrix;          other.m_GlobalMatrix = glm::mat4(1.0f);

        m_StateFlags = TransformFlag_PropertiesUpdated | TransformFlag_GlobalTransformUpdated;
        other.m_StateFlags = TransformFlag_PropertiesUpdated | TransformFlag_GlobalTransformUpdated;

        return *this;
    }

#pragma endregion

#pragma region General Methods

    void TransformComponent::UpdateLocalMatrix(bool scale_updated)
    {
        if (CheckFlag(TransformFlag_PropertiesUpdated))
        {
            m_LocalMatrix = 
                glm::translate(glm::mat4(1.0f), m_LocalPosition) *
                glm::mat4_cast(m_LocalOrientation) *
                glm::scale(glm::mat4(1.0f), m_LocalScale);

            RemoveFlag(TransformFlag_PropertiesUpdated);

            auto entity = GetEntity();
            if (!entity)
            {
                BC_CORE_ERROR("TransformComponent::UpdateLocalMatrix: Could Not Update ECS - Entity is Invalid.");
                return;
            }

            if (auto project = Application::GetProject(); project && project->GetSceneManager()->GetPhysicsSystem())
            {
                auto physics_system = project->GetSceneManager()->GetPhysicsSystem();
                auto child_rigidbodies = GetComponentsInChildren<RigidbodyComponent>();
                for (const auto& child_entity : child_rigidbodies)
                {
                    physics_system->MarkEntityRigidbodyTransformDirty(child_entity);
                }
                physics_system->MarkEntityRigidbodyTransformDirty(entity);
            }

            OnTransformUpdated(scale_updated);
        }
    }

    void TransformComponent::OnTransformUpdated(bool scale_updated)
    {
        auto entity = GetEntity();
        if (!entity)
        {
            BC_CORE_ERROR("TransformComponent::OnTransformUpdated: Could Not Update ECS - Entity is Invalid.");
            return;
        }

        // Set the Global Transform Updated Flag
        AddFlag(TransformFlag_GlobalTransformUpdated);

        if (entity.HasComponent<MeshRendererComponent>()) 
        {
            auto& component = entity.GetComponent<MeshRendererComponent>();
            component.UpdateOctree();
            component.UpdateBoundingBox();
        }

        if (entity.HasComponent<SkinnedMeshRendererComponent>()) 
        {

            auto& component = entity.GetComponent<SkinnedMeshRendererComponent>();
            component.UpdateOctree();
            component.UpdateBoundingBox();
        }

        // Do this inside OnTransformUpdate as it recurses down the hierarchy
        if (auto project = Application::GetProject(); project && project->GetSceneManager()->GetPhysicsSystem() && !entity.HasComponent<RigidbodyComponent>())
        {
            auto physics_system = project->GetSceneManager()->GetPhysicsSystem();
            if (scale_updated)
            {
                physics_system->MarkEntityShapeScaleChanged(entity);
            }
            if (entity.HasAnyComponent<
                    BoxColliderComponent, 
                    SphereColliderComponent, 
                    CapsuleColliderComponent, 
                    ConvexMeshColliderComponent, 
                    HeightFieldColliderComponent, 
                    TriangleMeshColliderComponent>())
            {
                // Mark Shapes Dirty if No Parent Rigidbody
                if (auto component = entity.TryGetComponent<BoxColliderComponent>(); component && component->GetShape()->GetHandle())
                {
                    GUID rigid_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(component->GetShape()->GetHandle()));
                    if (rigid_guid == entity.GetGUID()) // only mark dirty if local rigiddynamic created on shape
                        physics_system->MarkEntityLocalShapeTransformDirty(entity);
                }
                if (auto component = entity.TryGetComponent<SphereColliderComponent>(); component && component->GetShape()->GetHandle())
                {
                    GUID rigid_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(component->GetShape()->GetHandle()));
                    if (rigid_guid == entity.GetGUID()) // only mark dirty if local rigiddynamic created on shape
                        physics_system->MarkEntityLocalShapeTransformDirty(entity);
                }
                if (auto component = entity.TryGetComponent<CapsuleColliderComponent>(); component && component->GetShape()->GetHandle())
                {
                    GUID rigid_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(component->GetShape()->GetHandle()));
                    if (rigid_guid == entity.GetGUID()) // only mark dirty if local rigiddynamic created on shape
                        physics_system->MarkEntityLocalShapeTransformDirty(entity);
                }
                if (auto component = entity.TryGetComponent<ConvexMeshColliderComponent>(); component && component->GetShape()->GetHandle())
                {
                    GUID rigid_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(component->GetShape()->GetHandle()));
                    if (rigid_guid == entity.GetGUID()) // only mark dirty if local rigiddynamic created on shape
                        physics_system->MarkEntityLocalShapeTransformDirty(entity);
                }
                if (auto component = entity.TryGetComponent<HeightFieldColliderComponent>(); component && component->GetShape()->GetHandle())
                {
                    GUID rigid_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(component->GetShape()->GetHandle()));
                    if (rigid_guid == entity.GetGUID()) // only mark dirty if local rigiddynamic created on shape
                        physics_system->MarkEntityLocalShapeTransformDirty(entity);
                }
                if (auto component = entity.TryGetComponent<TriangleMeshColliderComponent>(); component && component->GetShape()->GetHandle())
                {
                    GUID rigid_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(component->GetShape()->GetHandle()));
                    if (rigid_guid == entity.GetGUID()) // only mark dirty if local rigiddynamic created on shape
                        physics_system->MarkEntityLocalShapeTransformDirty(entity);
                }
            }
        }

        // Recursive down the tree
        // TODO: Jobify
        
        auto project = Application::GetProject();
        if (!project)
            return;

        for (const auto& child_guid : entity.GetComponent<MetaComponent>().GetChildrenGUID()) 
        {
            Entity child_entity = project->GetSceneManager()->GetEntity(child_guid);
            if (!child_entity)
            {
                continue;
            }

            child_entity.GetTransform().OnTransformUpdated();
        }
        
    }

#pragma endregion

#pragma region Setters

    void TransformComponent::SetPosition(const glm::vec3& new_position, bool global)
    {
        if (global)
        {
            auto entity = GetEntity();
            if (!entity)
            {
                BC_CORE_WARN("TransformComponent::SetPosition: Could Not Set Position - Entity is Invalid.");
                return;
            }

            auto meta_component = entity.GetComponent<MetaComponent>();
            if (!meta_component.HasParent())
            {
                SetPosition(new_position, false); // Call Again as Local - root entity
                return;
            }

            auto parent_entity = Application::GetProject()->GetSceneManager()->GetEntity(meta_component.GetParentGUID());
            if (!parent_entity)
            {
                BC_CORE_WARN("TransformComponent::SetPosition: Could Not Set Position - Parent Entity is Not Null But Is Invalid.");
                return;
            }

            // Convert Global Position to Relative Position to Parent
            glm::mat4 parent_global_matrix = GetComponentInParent<TransformComponent>().GetGlobalMatrix();
            glm::mat4 parent_global_matrix_inverse = glm::inverse(parent_global_matrix);
            glm::vec4 local_position_v4 = parent_global_matrix_inverse * glm::vec4(new_position, 1.0f);

            // Treat New Relative Position as Local, Call Again as Local
            SetPosition(local_position_v4, false);
        }
        else
        {
            m_LocalPosition = new_position;
            AddFlag(TransformFlag_PropertiesUpdated);
            UpdateLocalMatrix();
        }
    }

    void TransformComponent::SetPositionX(float new_x, bool global)
    {
        glm::vec3 new_position = global ? GetGlobalPosition() : m_LocalPosition;
        new_position.x = new_x;
        SetPosition(new_position, global);
    }

    void TransformComponent::SetPositionY(float new_y, bool global)
    {
        glm::vec3 new_position = global ? GetGlobalPosition() : m_LocalPosition;
        new_position.y = new_y;
        SetPosition(new_position, global);
    }

    void TransformComponent::SetPositionZ(float new_z, bool global)
    {
        glm::vec3 new_position = global ? GetGlobalPosition() : m_LocalPosition;
        new_position.z = new_z;
        SetPosition(new_position, global);
    }

    void TransformComponent::SetOrientation(const glm::quat& new_orientation, bool global)
    {
        if (global)
        {
            auto entity = GetEntity();
            if (!entity)
            {
                BC_CORE_WARN("TransformComponent::SetOrientation: Could Not Set Orientation - Entity is Invalid.");
                return;
            }

            auto meta_component = entity.GetComponent<MetaComponent>();
            if (!meta_component.HasParent())
            {
                SetOrientation(new_orientation, false); // Call Again as Local - root entity
                return;
            }

            auto parent_entity = Application::GetProject()->GetSceneManager()->GetEntity(meta_component.GetParentGUID());
            if (!parent_entity)
            {
                BC_CORE_WARN("TransformComponent::SetOrientation: Could Not Set Orientation - Parent Entity is Not Null But Is Invalid.");
                return;
            }

            glm::quat parent_global_orientation = parent_entity.GetComponent<TransformComponent>().GetGlobalOrientation();
            glm::quat local_orientation = glm::inverse(parent_global_orientation) * new_orientation;

            SetOrientation(local_orientation, false);
        }
        else
        {
            m_LocalOrientation = glm::normalize(new_orientation);
            m_LocalEulerHint = glm::degrees(glm::eulerAngles(m_LocalOrientation));
            AddFlag(TransformFlag_PropertiesUpdated);
            UpdateLocalMatrix();
        }
    }

    void TransformComponent::SetOrientationEuler(const glm::vec3& new_orientation_euler, bool global)
    {
        glm::quat new_orientation = glm::quat(glm::radians(new_orientation_euler));
        SetOrientation(new_orientation, global);
        m_LocalEulerHint = new_orientation_euler;
    }

    void TransformComponent::SetOrientationEulerX(float new_x, bool global)
    {
        glm::vec3 new_euler = global ? glm::degrees(glm::eulerAngles(GetGlobalOrientation())) : m_LocalEulerHint;
        new_euler.x = new_x;
        SetOrientationEuler(new_euler, global);
        m_LocalEulerHint = new_euler;
    }

    void TransformComponent::SetOrientationEulerY(float new_y, bool global)
    {
        glm::vec3 new_euler = global ? glm::degrees(glm::eulerAngles(GetGlobalOrientation())) : m_LocalEulerHint;
        new_euler.y = new_y;
        SetOrientationEuler(new_euler, global);
        m_LocalEulerHint = new_euler;
    }

    void TransformComponent::SetOrientationEulerZ(float new_z, bool global)
    {
        glm::vec3 new_euler = global ? glm::degrees(glm::eulerAngles(GetGlobalOrientation())) : m_LocalEulerHint;
        new_euler.z = new_z;
        SetOrientationEuler(new_euler, global);
        m_LocalEulerHint = new_euler;
    }

    void TransformComponent::SetScale(const glm::vec3& new_scale, bool global)
    {
        if (global)
        {
            auto entity = GetEntity();
            if (!entity)
            {
                BC_CORE_WARN("TransformComponent::SetScale: Could Not Set Scale - Entity is Invalid.");
                return;
            }

            auto meta_component = entity.GetComponent<MetaComponent>();
            if (!meta_component.HasParent())
            {
                SetScale(new_scale, false); // Call Again as Local - root entity
                return;
            }

            auto parent_entity = Application::GetProject()->GetSceneManager()->GetEntity(meta_component.GetParentGUID());
            if (!parent_entity)
            {
                BC_CORE_WARN("TransformComponent::SetScale: Could Not Set Scale - Parent Entity is Not Null But Is Invalid.");
                return;
            }

            // Scale Division
            glm::vec3 parent_global_scale = parent_entity.GetComponent<TransformComponent>().GetGlobalScale();
            SetScale(new_scale / parent_global_scale, false);
        }
        else
        {
            m_LocalScale = new_scale;
            AddFlag(TransformFlag_PropertiesUpdated);
            UpdateLocalMatrix(true);
        }
    }

    void TransformComponent::SetScaleX(float new_x, bool global)
    {
        glm::vec3 new_scale = global ? GetGlobalScale() : m_LocalScale;
        new_scale.x = new_x;
        SetScale(new_scale, global);
    }

    void TransformComponent::SetScaleY(float new_y, bool global)
    {
        glm::vec3 new_scale = global ? GetGlobalScale() : m_LocalScale;
        new_scale.y = new_y;
        SetScale(new_scale, global);
    }

    void TransformComponent::SetScaleZ(float new_z, bool global)
    {
        glm::vec3 new_scale = global ? GetGlobalScale() : m_LocalScale;
        new_scale.z = new_z;
        SetScale(new_scale, global);
    }

    void TransformComponent::Translate(const glm::vec3& position_delta, bool global)
    {
        SetPosition((global ? GetGlobalPosition() : m_LocalPosition) + position_delta, global);
    }

    void TransformComponent::TranslateX(float position_x_delta, bool global)
    {
        SetPositionX((global ? GetGlobalPosition().x : m_LocalPosition.x) + position_x_delta, global);
    }

    void TransformComponent::TranslateY(float position_y_delta, bool global)
    {
        SetPositionY((global ? GetGlobalPosition().y : m_LocalPosition.y) + position_y_delta, global);
    }

    void TransformComponent::TranslateZ(float position_z_delta, bool global)
    {
        SetPositionZ((global ? GetGlobalPosition().z : m_LocalPosition.z) + position_z_delta, global);
    }

    void TransformComponent::Rotate(const glm::quat& orientation_delta, bool global)
    {
        SetOrientation(orientation_delta * (global ? GetGlobalOrientation() : m_LocalOrientation), global);
    }

    void TransformComponent::RotateEuler(const glm::vec3& orientation_euler_delta, bool global)
    {
        auto cache_euler_hint = m_LocalEulerHint;
        glm::quat delta = glm::quat(glm::radians(orientation_euler_delta));
        Rotate(delta, global);
        cache_euler_hint += orientation_euler_delta;
        m_LocalEulerHint = cache_euler_hint;
    }

    void TransformComponent::RotateEulerX(float orientation_x_euler_delta, bool global)
    {
        auto cache_euler_hint = m_LocalEulerHint;
        glm::quat delta = glm::angleAxis(glm::radians(orientation_x_euler_delta), glm::vec3(1.0f, 0.0f, 0.0f));
        Rotate(delta, global);
        cache_euler_hint.x += orientation_x_euler_delta;
        m_LocalEulerHint = cache_euler_hint;
    }

    void TransformComponent::RotateEulerY(float orientation_y_euler_delta, bool global)
    {
        auto cache_euler_hint = m_LocalEulerHint;
        glm::quat delta = glm::angleAxis(glm::radians(orientation_y_euler_delta), glm::vec3(0.0f, 1.0f, 0.0f));
        Rotate(delta, global);
        cache_euler_hint.y += orientation_y_euler_delta;
        m_LocalEulerHint = cache_euler_hint;
    }

    void TransformComponent::RotateEulerZ(float orientation_z_euler_delta, bool global)
    {
        auto cache_euler_hint = m_LocalEulerHint;
        glm::quat delta = glm::angleAxis(glm::radians(orientation_z_euler_delta), glm::vec3(0.0f, 0.0f, 1.0f));
        Rotate(delta, global);
        cache_euler_hint.z += orientation_z_euler_delta;
        m_LocalEulerHint = cache_euler_hint;
    }

    void TransformComponent::Scale(const glm::vec3& scale_delta, bool global)
    {
        SetScale((global ? GetGlobalScale() : m_LocalScale) + scale_delta, global);
    }

    void TransformComponent::ScaleX(float scale_x_delta, bool global)
    {
        SetScaleX((global ? GetGlobalScale().x : m_LocalScale.x) + scale_x_delta, global);
    }

    void TransformComponent::ScaleY(float scale_y_delta, bool global)
    {
        SetScaleY((global ? GetGlobalScale().y : m_LocalScale.y) + scale_y_delta, global);
    }

    void TransformComponent::ScaleZ(float scale_z_delta, bool global)
    {
        SetScaleZ((global ? GetGlobalScale().z : m_LocalScale.z) + scale_z_delta, global);
    }

    void TransformComponent::SetForwardDirection(const glm::vec3& direction)
    {
        glm::vec3 fwd = glm::normalize(direction);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        // Handle degenerate up direction
        if (glm::abs(glm::dot(fwd, up)) > 0.999f)
            up = glm::vec3(1.0f, 0.0f, 0.0f);

        glm::vec3 right = glm::normalize(glm::cross(up, fwd));
        up = glm::normalize(glm::cross(fwd, right));

        glm::mat3 rotation_matrix(right, up, -fwd); // -fwd for right-handed system
        m_LocalOrientation = glm::quat_cast(rotation_matrix);
        m_LocalEulerHint = glm::eulerAngles(m_LocalOrientation);
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalMatrix();
    }

    void TransformComponent::SetRightDirection(const glm::vec3& direction)
    {
        glm::vec3 right = glm::normalize(direction);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        if (glm::abs(glm::dot(right, up)) > 0.999f)
            up = glm::vec3(0.0f, 0.0f, 1.0f);

        glm::vec3 fwd = glm::normalize(glm::cross(right, up));
        up = glm::normalize(glm::cross(fwd, right));

        glm::mat3 rotation_matrix(right, up, -fwd);
        m_LocalOrientation = glm::quat_cast(rotation_matrix);
        m_LocalEulerHint = glm::eulerAngles(m_LocalOrientation);
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalMatrix();
    }

    void TransformComponent::SetUpDirection(const glm::vec3& direction)
    {
        glm::vec3 up = glm::normalize(direction);
        glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);

        if (glm::abs(glm::dot(forward, up)) > 0.999f)
            forward = glm::vec3(1.0f, 0.0f, 0.0f);

        glm::vec3 right = glm::normalize(glm::cross(up, forward));
        forward = glm::normalize(glm::cross(right, up));

        glm::mat3 rotation_matrix(right, up, -forward);
        m_LocalOrientation = glm::quat_cast(rotation_matrix);
        m_LocalEulerHint = glm::eulerAngles(m_LocalOrientation);
        AddFlag(TransformFlag_PropertiesUpdated);
        UpdateLocalMatrix();
    }

    void TransformComponent::LookAt(const glm::vec3& target_position)
    {
        glm::vec3 position = GetGlobalPosition();
        glm::vec3 forward = glm::normalize(target_position - position);
        SetForwardDirection(forward);
    }
    
    void TransformComponent::SetMatrix(const glm::mat4& new_matrix, bool global)
    {
        if (global)
        {
            auto entity = GetEntity();
            if (!entity)
            {
                BC_CORE_WARN("TransformComponent::SetMatrix: Could Not Set Matrix - Entity is Invalid.");
                return;
            }

            auto meta_component = entity.GetComponent<MetaComponent>();
            if (!meta_component.HasParent())
            {
                SetMatrix(new_matrix, false); // Root
                return;
            }

            auto parent_entity = Application::GetProject()->GetSceneManager()->GetEntity(meta_component.GetParentGUID());
            if (!parent_entity)
            {
                BC_CORE_WARN("TransformComponent::SetMatrix: Could Not Set Matrix - Parent Entity is Not Null But Is Invalid.");
                return;
            }

            glm::mat4 parent_global_matrix = parent_entity.GetComponent<TransformComponent>().GetGlobalMatrix();
            glm::mat4 parent_inverse = glm::inverse(parent_global_matrix);
            glm::mat4 local_matrix = parent_inverse * new_matrix;

            SetMatrix(local_matrix, false);
        }
        else
        {
            m_LocalMatrix = new_matrix;

            // Decompose matrix into position / scale / orientation
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::vec3 position, scale;
            glm::quat orientation;
            glm::decompose(new_matrix, scale, orientation, position, skew, perspective);

            m_LocalPosition    = position;
            m_LocalScale       = scale;
            m_LocalOrientation = glm::normalize(orientation);
            m_LocalEulerHint   = glm::degrees(glm::eulerAngles(m_LocalOrientation));

            AddFlag(TransformFlag_PropertiesUpdated);

            UpdateLocalMatrix(true);
        }
    }
    
#pragma endregion

#pragma region Getters

    const glm::vec3& TransformComponent::GetLocalPosition() const
    {
        return m_LocalPosition;
    }

    const glm::quat& TransformComponent::GetLocalOrientation() const
    {
        return m_LocalOrientation;
    }

    const glm::vec3& TransformComponent::GetLocalOrientationEulerHint() const
    {
        return m_LocalEulerHint;
    }

    const glm::vec3& TransformComponent::GetLocalScale() const
    {
        return m_LocalScale;
    }

    glm::vec3 TransformComponent::GetGlobalPosition()
    {
        if (CheckFlag(TransformFlag_GlobalTransformUpdated))
            return GetPositionFromMatrix(GetGlobalMatrix());
        
        if (m_LocalMatrix != m_GlobalMatrix)
            return GetPositionFromMatrix(m_GlobalMatrix);

        return m_LocalPosition;
    }

    glm::quat TransformComponent::GetGlobalOrientation()
    {
        if (CheckFlag(TransformFlag_GlobalTransformUpdated))
            return GetOrientationFromMatrix(GetGlobalMatrix());

        if (m_LocalMatrix != m_GlobalMatrix)
            return GetOrientationFromMatrix(m_GlobalMatrix);

        return m_LocalOrientation;
    }

    glm::vec3 TransformComponent::GetGlobalOrientationEulerHint()
    {
        glm::mat4 matrix;

        if (CheckFlag(TransformFlag_GlobalTransformUpdated))
            matrix = GetGlobalMatrix();
        else if (m_LocalMatrix != m_GlobalMatrix)
            matrix = m_GlobalMatrix;
        else
            return m_LocalEulerHint;

        return glm::degrees(glm::eulerAngles(GetOrientationFromMatrix(matrix)));
    }

    glm::vec3 TransformComponent::GetGlobalScale()
    {
        if (CheckFlag(TransformFlag_GlobalTransformUpdated))
            return GetScaleFromMatrix(GetGlobalMatrix());

        if (m_LocalMatrix != m_GlobalMatrix)
            return GetScaleFromMatrix(m_GlobalMatrix);

        return m_LocalScale;
    }
    
    glm::vec3 TransformComponent::GetLocalForwardDirection() const
    {
        return glm::normalize(m_LocalOrientation * glm::vec3(0.0f, 0.0f, -1.0f));
    }

    glm::vec3 TransformComponent::GetLocalRightDirection() const
    {
        return glm::normalize(m_LocalOrientation * glm::vec3(1.0f, 0.0f, 0.0f));
    }

    glm::vec3 TransformComponent::GetLocalUpDirection() const
    {
        return glm::normalize(m_LocalOrientation * glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::vec3 TransformComponent::GetGlobalForwardDirection()
    {
        return GetForwardDirectionFromMatrix(GetGlobalMatrix());
    }

    glm::vec3 TransformComponent::GetGlobalRightDirection()
    {
        return GetRightDirectionFromMatrix(GetGlobalMatrix());
    }

    glm::vec3 TransformComponent::GetGlobalUpDirection()
    {
        return GetUpDirectionFromMatrix(GetGlobalMatrix());
    }

    const glm::mat4& TransformComponent::GetLocalMatrix()
    {
        return m_LocalMatrix;
    }

    const glm::mat4& TransformComponent::GetGlobalMatrix()
    {
        if (CheckFlag(TransformFlag_GlobalTransformUpdated))
        {
            Entity entity = GetEntity();
            if (!entity) 
            {
                BC_CORE_ERROR("TransformComponent::GetGlobalMatrix: Cannot GetGlobalMatrix - Current Entity Is Invalid.");
                m_GlobalMatrix = m_LocalMatrix;
                return m_GlobalMatrix;
            }

            auto& meta_component = entity.GetComponent<MetaComponent>();
            auto project = Application::GetProject();
            if (!project)
            {
                BC_CORE_ERROR("TransformComponent::GetGlobalMatrix: Cannot GetGlobalMatrix - Project Is Invalid.");
                m_GlobalMatrix = m_LocalMatrix;
                return m_GlobalMatrix;
            }
            
            Entity parent_entity = project->GetSceneManager()->GetEntity(meta_component.GetParentGUID());
            if (!parent_entity)
            {
                m_GlobalMatrix = m_LocalMatrix;
                return m_GlobalMatrix;
            }

            m_GlobalMatrix = parent_entity.GetTransform().GetGlobalMatrix() * m_LocalMatrix;

            RemoveFlag(TransformFlag_GlobalTransformUpdated);
        }
        return m_GlobalMatrix;
    }

#pragma endregion

#pragma region Flags

    void TransformComponent::AddFlag(TransformComponentFlag flag)       { m_StateFlags |= flag; }
    void TransformComponent::RemoveFlag(TransformComponentFlag flag)    { m_StateFlags &= ~flag; }
    void TransformComponent::ClearFlags()                               { m_StateFlags = TransformFlag_None; }

    bool TransformComponent::CheckFlag(TransformComponentFlag flag) const { return (m_StateFlags & flag) != TransformFlag_None; }
    bool TransformComponent::NoFlagsSet() const { return m_StateFlags == TransformFlag_None; }

#pragma endregion

#pragma region Static Helpers

    glm::vec3 TransformComponent::GetPositionFromMatrix(const glm::mat4& transform) 
    {
        return glm::vec3(transform[3][0], transform[3][1], transform[3][2]);
    }

    glm::quat TransformComponent::GetOrientationFromMatrix(const glm::mat4& transform) 
    {
        glm::mat3 rotation_matrix;
        rotation_matrix[0] = glm::vec3(transform[0]);
        rotation_matrix[1] = glm::vec3(transform[1]);
        rotation_matrix[2] = glm::vec3(transform[2]);

        // Extract scale
        glm::vec3 scale;
        scale.x = glm::length(rotation_matrix[0]);
        scale.y = glm::length(rotation_matrix[1]);
        scale.z = glm::length(rotation_matrix[2]);

        // Prevent division by zero by clamping scale to a small epsilon
        constexpr float epsilon = 1e-6f;
        scale = glm::max(glm::abs(scale), glm::vec3(epsilon));

        // Fix mirrored handedness if needed
        if (glm::determinant(rotation_matrix) < 0.0f)
            scale.x = -scale.x;

        // Normalize rotation matrix axes
        rotation_matrix[0] /= scale.x;
        rotation_matrix[1] /= scale.y;
        rotation_matrix[2] /= scale.z;

        return glm::quat_cast(rotation_matrix);
    }

    glm::vec3 TransformComponent::GetScaleFromMatrix(const glm::mat4& transform) 
    {
        return glm::vec3(
            glm::length(glm::vec3(transform[0][0], transform[0][1], transform[0][2])),
            glm::length(glm::vec3(transform[1][0], transform[1][1], transform[1][2])),
            glm::length(glm::vec3(transform[2][0], transform[2][1], transform[2][2]))
        );
    }

    glm::vec3 TransformComponent::GetForwardDirectionFromMatrix(const glm::mat4& matrix)
    {
        return glm::normalize(glm::vec3(matrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
    }

    glm::vec3 TransformComponent::GetRightDirectionFromMatrix(const glm::mat4& matrix)
    {
        return glm::normalize(glm::vec3(matrix * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
    }

    glm::vec3 TransformComponent::GetUpDirectionFromMatrix(const glm::mat4& matrix)
    {
        return glm::normalize(glm::vec3(matrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));
    }

#pragma endregion

#pragma region Serialisation

    void TransformComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
        out << YAML::BeginMap;
        {
            out << YAML::Key << "Position" << YAML::Value << YAML::Flow
                << YAML::BeginSeq
                << m_LocalPosition.x
                << m_LocalPosition.y
                << m_LocalPosition.z
                << YAML::EndSeq;

            out << YAML::Key << "Orientation" << YAML::Value << YAML::Flow
                << YAML::BeginSeq
                << m_LocalOrientation.w
                << m_LocalOrientation.x
                << m_LocalOrientation.y
                << m_LocalOrientation.z
                << YAML::EndSeq;

            out << YAML::Key << "OrientationEulerHint" << YAML::Value << YAML::Flow
                << YAML::BeginSeq
                << m_LocalEulerHint.x
                << m_LocalEulerHint.y
                << m_LocalEulerHint.z
                << YAML::EndSeq;

            out << YAML::Key << "Scale" << YAML::Value << YAML::Flow
                << YAML::BeginSeq
                << m_LocalScale.x
                << m_LocalScale.y
                << m_LocalScale.z
                << YAML::EndSeq;
        }
        out << YAML::EndMap;
    }

    bool TransformComponent::SceneDeserialise(const YAML::Node& data)
    {
        AddFlag(TransformFlag_GlobalTransformUpdated | TransformFlag_PropertiesUpdated);

        if (data["Position"])
        {
            auto seq = data["Position"];
            if (seq.IsSequence() && seq.size() == 3) 
            {
                m_LocalPosition.x = seq[0].as<float>();
                m_LocalPosition.y = seq[1].as<float>();
                m_LocalPosition.z = seq[2].as<float>();
            }
        }

        if (data["Orientation"])
        {
            auto seq = data["Orientation"];
            if (seq.IsSequence() && seq.size() == 4) 
            {
                m_LocalOrientation.w = seq[0].as<float>();
                m_LocalOrientation.x = seq[1].as<float>();
                m_LocalOrientation.y = seq[2].as<float>();
                m_LocalOrientation.z = seq[3].as<float>();
            }
        }
        
        if (data["OrientationEulerHint"])
        {
            auto seq = data["OrientationEulerHint"];
            if (seq.IsSequence() && seq.size() == 3) 
            {
                m_LocalEulerHint.x = seq[0].as<float>();
                m_LocalEulerHint.y = seq[1].as<float>();
                m_LocalEulerHint.z = seq[2].as<float>();
            }
        }
        
        if (data["Scale"])
        {
            auto seq = data["Scale"];
            if (seq.IsSequence() && seq.size() == 3) 
            {
                m_LocalScale.x = seq[0].as<float>();
                m_LocalScale.y = seq[1].as<float>();
                m_LocalScale.z = seq[2].as<float>();
            }
        }

        return true;
    }

#pragma endregion

#pragma endregion

#pragma region Meta Component

    MetaComponent::MetaComponent(const MetaComponent &other)
    {
        m_Name = other.m_Name;
        m_EntityID = other.m_EntityID;
        m_Parent = other.m_Parent;
        m_Children = other.m_Children;
        m_Scripts = other.m_Scripts;
        m_PrefabSourceHandle = other.m_PrefabSourceHandle;
    }

    MetaComponent::MetaComponent(MetaComponent&& other) noexcept
    {
        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        m_Name = std::move(other.m_Name);
        m_EntityID = other.m_EntityID;
        m_Parent = other.m_Parent;
        m_Children = std::move(other.m_Children);
        m_Scripts = std::move(other.m_Scripts);
        m_PrefabSourceHandle = other.m_PrefabSourceHandle;

        other.m_EntityID = NULL_GUID;
        other.m_Parent = NULL_GUID;
        other.m_PrefabSourceHandle = NULL_GUID;
    }

    MetaComponent& MetaComponent::operator=(const MetaComponent& other)
    {
        if (this == &other)
            return *this;
            
        m_Name = other.m_Name;
        m_EntityID = other.m_EntityID;
        m_Parent = other.m_Parent;
        m_Children = other.m_Children;
        m_Scripts = other.m_Scripts;
        m_PrefabSourceHandle = other.m_PrefabSourceHandle;

        return *this;
    }

    MetaComponent& MetaComponent::operator=(MetaComponent&& other) noexcept
    {
        if (this == &other)
            return *this;
            
        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        m_Name = std::move(other.m_Name);
        m_EntityID = other.m_EntityID;
        m_Parent = other.m_Parent;
        m_Children = std::move(other.m_Children);
        m_Scripts = std::move(other.m_Scripts);
        m_PrefabSourceHandle = other.m_PrefabSourceHandle;

        other.m_EntityID = NULL_GUID;
        other.m_Parent = NULL_GUID;
        other.m_PrefabSourceHandle = NULL_GUID;

        return *this;
    }

    void MetaComponent::SetUniqueName(const std::string& new_name)
    {
        std::string temp_name = new_name;
        if (new_name.empty()) 
        {
            temp_name = "Untitled Entity";
            return;
        }

        // Check if the name is already used
        auto check_names = [&](const std::string& test_name) -> bool 
        {
            auto project = Application::GetProject();
            if (!project)
                return false;

            Entity parent_entity = project->GetSceneManager()->GetEntity(m_Parent);
            if (parent_entity) 
            {
                for (const auto& child_guid : parent_entity.GetComponent<MetaComponent>().GetChildrenGUID()) 
                {
                    if (child_guid == m_EntityID)
                    {
                        continue;
                    }

                    Entity child_entity = Application::GetProject()->GetSceneManager()->GetEntity(child_guid);
                    if (!child_entity)
                    {
                        continue; // Ensure entity is valid
                    }

                    const MetaComponent& child_meta = child_entity.GetComponent<MetaComponent>();
                    if (child_meta.m_Name == test_name)
                    {
                        return true;
                    }
                }
            }
            else 
            {
                auto view = GetEntity().GetScene()->GetAllEntitiesWith<MetaComponent>();
                for (auto entity : view) 
                {
                    auto& meta_component = view.get<MetaComponent>(entity);

                    if (meta_component.HasParent() || meta_component.GetEntityGUID() == GetEntityGUID())
                        continue;
                        
                    if (meta_component.GetName() == test_name)
                        return true;
                }
            }
            return false;
        };

        // If the name is already unique, set it directly
        if (!check_names(temp_name)) 
        {
            m_Name = temp_name;
            return;
        }

        // Append a suffix to ensure uniqueness
        std::string unique_name;
        int suffix = 1;
        do 
        {
            unique_name = temp_name + " (" + std::to_string(suffix) + ")";
            suffix++;
        } while (check_names(unique_name));

        m_Name = unique_name;
    }

    void MetaComponent::AttachParent(GUID new_parent_guid)
    {
        if (new_parent_guid == NULL_GUID) 
        {
            return;
        }

        if (new_parent_guid == m_Parent) 
        {
            BC_CORE_WARN("MetaComponent::AttachParent: Cannot Attach Self as Parent!");
            return;
        }

        Entity entity = GetEntity();
        if (!entity) 
        {
            BC_CORE_WARN("MetaComponent::AttachParent: Cannot Attach Parent - Current Entity Is Invalid!");
            return;
        }

        for (const auto& child_uuid : m_Children) 
        {
            if (child_uuid == new_parent_guid) 
            {
                BC_CORE_WARN("MetaComponent::AttachParent: Cannot Attach Child Entity As New Parent!");
                return;
            }
        }

        for (auto& child_entity : GetComponentsInChildren<MetaComponent>()) 
        {
            for (const auto& child_uuid : child_entity.GetComponent<MetaComponent>().GetChildrenGUID())
            {
                if (child_uuid == new_parent_guid) 
                {
                    BC_CORE_WARN("MetaComponent::AttachParent: Cannot Attach Ancestor to Descendent!");
                    return;
                }
            }
        }

        if (auto scene_ref = entity.GetScene(); scene_ref)
        {
            scene_ref->MarkHierarchyDirty();
        }

        // 1. Is this Current Entity already connected to another parent? If so call DetachParent first
        if (HasParent())
            DetachParent();

        Entity new_parent_entity = Application::GetProject()->GetSceneManager()->GetEntity(new_parent_guid);
        if (!new_parent_entity) 
        {
            BC_CORE_ERROR("MetaComponent::AttachParent: Cannot Attach Parent - Parent Entity Is Invalid! Entity({0}) will be at the root of the scene now.", entity.GetName());
            return;
        }
        
        // TODO: If New Parent is in another scene we need to MOVE this from one scene to the other

        // 2. Convert Current Global Transform to Local Transform relative to new parent
        // Get references to the relevant components
        auto& entity_transform = entity.GetTransform();
        auto& new_parent_transform = new_parent_entity.GetTransform();

        // Calculate the local transform relative to the new parent
        glm::mat4 local_matrix = glm::inverse(new_parent_transform.GetGlobalMatrix()) * entity_transform.GetGlobalMatrix();
        
        // Decompose matrix into position / scale / orientation
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::vec3 position, scale;
        glm::quat orientation;
        glm::decompose(local_matrix, scale, orientation, position, skew, perspective);

        entity_transform.m_LocalPosition    = position;
        entity_transform.m_LocalScale       = scale;
        entity_transform.m_LocalOrientation = glm::normalize(orientation);
        entity_transform.m_LocalEulerHint   = glm::degrees(glm::eulerAngles(entity_transform.m_LocalOrientation));

        entity_transform.m_LocalMatrix = local_matrix;
        entity_transform.m_GlobalMatrix = new_parent_transform.GetGlobalMatrix() * local_matrix;
        
        entity_transform.m_StateFlags = TransformFlag_None;

        if (!entity.HasComponent<RigidbodyComponent>()) 
        {
            // TODO: Implement with physics components
            
            // // 1. I need to recursively check my children to see if there are
            // // any children entities that HAVE a SphereCollider or BoxCollider, and 
            // // DO NOT HAVE a Rigidbody.
            // std::vector<Entity> child_colliders = GetChildCollidersWithoutRigidbody(entity);

            // // 2. Flag all children with Collider Components without Rigidbodies to update
            // //    rigidbody reference in the physics system
            // for (Entity child_entity : child_colliders) 
            // {
            //     // Attach the collider to the parent's Rigidbody
            //     if (child_entity.HasComponent<SphereColliderComponent>()) 
            //     {
            //         child_entity.GetComponent<SphereColliderComponent>().AddFlag(ColliderFlag_RigidbodyUpdated);
            //     }
            //     if (child_entity.HasComponent<BoxColliderComponent>()) 
            //     {
            //         child_entity.GetComponent<BoxColliderComponent>().AddFlag(ColliderFlag_RigidbodyUpdated);
            //     }
            // }
        }

        m_Parent = new_parent_guid;
        new_parent_entity.GetComponent<MetaComponent>().m_Children.push_back(entity.GetGUID());
    }

    void MetaComponent::DetachParent()
    {
        Entity entity = GetEntity();
        if (!entity) 
        {
            BC_CORE_ERROR("MetaComponent::DetachParent: Cannot Detach Parent - Current Entity Is Invalid!");
            return;
        }

        if (auto scene_ref = entity.GetScene(); scene_ref)
        {
            scene_ref->MarkHierarchyDirty();
        }

        // 1. Calculate the child's global transform
        TransformComponent& entity_transform = entity.GetTransform();
        glm::mat4 global_matrix = entity_transform.GetGlobalMatrix();

        // Decompose matrix into position / scale / orientation
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::vec3 position, scale;
        glm::quat orientation;
        glm::decompose(global_matrix, scale, orientation, position, skew, perspective);

        // 2. Update the child's local transform to match the global transform
        entity_transform.m_LocalPosition = position;
        entity_transform.m_LocalScale = scale;
        entity_transform.m_LocalOrientation = glm::normalize(orientation);
        entity_transform.m_LocalEulerHint   = glm::degrees(glm::eulerAngles(entity_transform.m_LocalOrientation));
        
        entity_transform.m_LocalMatrix= global_matrix;
        entity_transform.m_GlobalMatrix = global_matrix;

        entity_transform.m_StateFlags = TransformFlag_None;

        if (!entity.HasComponent<RigidbodyComponent>()) 
        {
            std::unordered_set<Entity> children_colliders_no_rigidbody{};

            std::function<void(Entity&)> get_child_colliders_without_rigidbody;
            get_child_colliders_without_rigidbody = [&](Entity& current_child_entity)
            {
                if (current_child_entity.HasComponent<RigidbodyComponent>())
                    return;
                
                if (current_child_entity.HasAnyComponent<BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent, ConvexMeshColliderComponent, HeightFieldColliderComponent, TriangleMeshColliderComponent>())
                {
                    children_colliders_no_rigidbody.insert(current_child_entity);
                }

                for (const auto& child_guid : current_child_entity.GetComponent<MetaComponent>().GetChildrenGUID())
                {
                    Entity child_entity = Application::GetProject()->GetSceneManager()->GetEntity(child_guid);
                    if (!child_entity)
                        continue;
                    
                    get_child_colliders_without_rigidbody(child_entity);
                }
            };

            get_child_colliders_without_rigidbody(entity);

            // 2. Flag all children with Collider Components without Rigidbodies to update
            //    rigidbody reference in the physics system
            if (auto project = Application::GetProject(); project && project->GetSceneManager()->GetPhysicsSystem())
            {
                auto physics_system = project->GetSceneManager()->GetPhysicsSystem();
                for (const auto& child_entity : children_colliders_no_rigidbody)
                {
                    if (auto component = child_entity.TryGetComponent<BoxColliderComponent>(); component) 
                    {
                        physics_system->RegisterShape(child_entity, component->GetShape()->GetHandle(), ShapeType_Box);
                    }
                    if (auto component = child_entity.TryGetComponent<SphereColliderComponent>(); component) 
                    {
                        physics_system->RegisterShape(child_entity, component->GetShape()->GetHandle(), ShapeType_Sphere);
                    }
                    if (auto component = child_entity.TryGetComponent<CapsuleColliderComponent>(); component) 
                    {
                        physics_system->RegisterShape(child_entity, component->GetShape()->GetHandle(), ShapeType_Capsule);
                    }
                    if (auto component = child_entity.TryGetComponent<ConvexMeshColliderComponent>(); component) 
                    {
                        physics_system->RegisterShape(child_entity, component->GetShape()->GetHandle(), ShapeType_ConvexMesh);
                    }
                    if (auto component = child_entity.TryGetComponent<HeightFieldColliderComponent>(); component) 
                    {
                        physics_system->RegisterShape(child_entity, component->GetShape()->GetHandle(), ShapeType_HeightField);
                    }
                    if (auto component = child_entity.TryGetComponent<TriangleMeshColliderComponent>(); component) 
                    {
                        physics_system->RegisterShape(child_entity, component->GetShape()->GetHandle(), ShapeType_TriangleMesh);
                    }
                }
            }
        }

        if (HasParent()) 
        {
            Entity old_parent_entity = Application::GetProject()->GetSceneManager()->GetEntity(m_Parent);
            if (old_parent_entity)
            {
                auto& parent_children = old_parent_entity.GetComponent<MetaComponent>().m_Children;

                // Remove the child from the parent's children list
                parent_children.erase(
                    std::remove(parent_children.begin(), parent_children.end(), entity.GetGUID()),
                    parent_children.end()
                );
            }
        }

        // Clear the parent ID
        m_Parent = NULL_GUID;
    }

    void MetaComponent::DetachChildren()
    {
        Entity entity = GetEntity();
        if (!entity) 
        {
            BC_CORE_ERROR("MetaComponent::DetachChildren: Cannot Detach Children - Current Entity Is Invalid!");
            return;
        }

        for (const auto& child_guid : m_Children) 
        {
            Entity child = Application::GetProject()->GetSceneManager()->GetEntity(child_guid);
            if (!child || !child.HasComponent<MetaComponent>())
            {
                continue;
            }

            child.GetComponent<MetaComponent>().DetachParent();
        }

        BC_CORE_TRACE("MetaComponent::DetachChildren: Detached {0} Children From Entity({1}).", m_Children.size(), entity.GetName());

        m_Children.clear();
    }

    void MetaComponent::RehomeChildren(GUID new_parent_guid)
    {
        Entity entity = GetEntity();
        if (!entity) 
        {
            BC_CORE_ERROR("MetaComponent::RehomeChildren: Cannot Rehome Children - Current Entity Is Invalid!");
            return;
        }

        Entity new_parent_entity = Application::GetProject()->GetSceneManager()->GetEntity(new_parent_guid);
        if (!new_parent_entity) 
        {
            BC_CORE_ERROR("MetaComponent::RehomeChildren: Cannot Rehome Children - Parent Entity Is Invalid.");
            return;
        }

        for (const auto& child_guid : m_Children) 
        {
            Entity child = Application::GetProject()->GetSceneManager()->GetEntity(child_guid);
            if (!child || !child.HasComponent<MetaComponent>())
            {
                continue;
            }

            child.GetComponent<MetaComponent>().AttachParent(new_parent_guid);
        }

        BC_CORE_TRACE("Rehomed {0} Children From Entity({1}) to Entity({2}).", m_Children.size(), entity.GetName(), new_parent_entity.GetName());

        m_Children.clear();
    }

    Entity MetaComponent::FindChild(GUID child_guid) const
    {
        Entity entity = GetEntity();
        if (!entity) 
        {
            BC_CORE_ERROR("MetaComponent::FindChild: Cannot Find Child - Current Entity Is Invalid!");
            return Entity{};
        }

        if (entity.GetGUID() == child_guid)
        {
            BC_CORE_WARN("MetaComponent::FindChild: Cannot Pass Child GUID to Self.");
            return Entity{};
        }

        Entity found_child = Application::GetProject()->GetSceneManager()->GetEntity(child_guid);
        if (!found_child)
            return Entity{};

        std::function<bool(Entity)> recurse_children;
        recurse_children = [&](Entity current_entity) -> bool
        {            
            auto& meta_component = current_entity.GetComponent<MetaComponent>();
            for (GUID _child_guid : meta_component.GetChildrenGUID())
            {
                if (_child_guid == child_guid)
                {
                    found_child = Application::GetProject()->GetSceneManager()->GetEntity(child_guid);
                    return true;
                }
            }

            for (GUID _child_guid : meta_component.GetChildrenGUID())
            {
                Entity _child_entity = Application::GetProject()->GetSceneManager()->GetEntity(_child_guid);
                if (!_child_entity || !_child_entity.HasComponent<MetaComponent>())
                    continue;
                
                if (recurse_children(_child_entity))
                    return true;
            }

            return false;
        };

        bool found = recurse_children(entity);
        if (found && found_child)
        {
            return found_child;
        }

        return Entity{};
    }

    Entity MetaComponent::FindChild(const std::string& child_name) const
    {
        Entity entity = GetEntity();
        if (!entity) 
        {
            BC_CORE_ERROR("MetaComponent::FindChild: Cannot Find Child - Current Entity Is Invalid!");
            return Entity{};
        }

        if (entity.GetName() == child_name)
        {
            BC_CORE_WARN("MetaComponent::FindChild: Cannot Pass Child Name to Self.");
            return Entity{};
        }

        Entity found_child = Application::GetProject()->GetSceneManager()->GetEntity(child_name);
        if (!found_child)
            return Entity{};
        
        std::function<bool(Entity)> recurse_children;
        recurse_children = [&](Entity current_entity) -> bool
        {
            auto& meta_component = current_entity.GetComponent<MetaComponent>();

            std::vector<Entity> cached_entities = {};
            cached_entities.reserve(meta_component.GetChildrenGUID().size());

            for (GUID _child_guid : meta_component.GetChildrenGUID())
            {
                Entity _child_entity = Application::GetProject()->GetSceneManager()->GetEntity(_child_guid);
                cached_entities.push_back(_child_entity);

                if (!_child_entity || !_child_entity.HasComponent<MetaComponent>())
                    continue;

                if (_child_entity.GetName() == child_name)
                {
                    return true;
                }
            }

            for (Entity _child_entity : cached_entities)
            {                
                if (recurse_children(_child_entity))
                    return true;
            }

            return false;
        };

        if (recurse_children(entity) && found_child)
        {
            return found_child;
        }
        
        return Entity{};
    }

    GUID MetaComponent::GetRootParentGUID() const
    {
        auto parent_components = GetComponentsInParents<MetaComponent>();

        for (auto it = parent_components.rbegin(); it != parent_components.rend(); ++it)
        {
            Entity& parent = *it;
            if (!parent)
                continue;

            if (!parent.GetComponent<MetaComponent>().HasParent())
                return parent.GetGUID();
        }

        auto entity = GetEntity();
        if (!entity)
            return NULL_GUID;

        return entity.GetGUID();
    }

#pragma region Serialisation

    void MetaComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
        out << YAML::BeginMap;
        {
            out << YAML::Key << "Name" << YAML::Value << m_Name;
            out << YAML::Key << "GUID" << YAML::Value << m_EntityID;
            out << YAML::Key << "ParentGUID" << YAML::Value << m_Parent;
            out << YAML::Key << "PrefabAssetHandle" << YAML::Value << m_PrefabSourceHandle;

            out << YAML::Key << "Children" << YAML::Value;
            out << YAML::BeginSeq;
            for (GUID child_guid : m_Children) 
            {
                out << YAML::BeginMap;
                out << YAML::Key << "ChildGUID" << YAML::Value << child_guid;
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;

            out << YAML::Key << "Scripts" << YAML::Value;
            out << YAML::BeginMap;
            for (const auto& [script_name, active] : m_Scripts)
            {
                out << YAML::Key << script_name << YAML::Value << active;
            }
            out << YAML::EndMap;

        }
        out << YAML::EndMap;
    }

    bool MetaComponent::SceneDeserialise(const YAML::Node& data)
    {
        if (data["Name"])
        {
            m_Name = data["Name"].as<std::string>();
        }
        
        if (data["GUID"])
        {
            m_EntityID = data["GUID"].as<uint64_t>();
        }
        
        if (data["ParentGUID"])
        {
            m_Parent = data["ParentGUID"].as<uint64_t>();
        }
        
        if (data["PrefabAssetHandle"])
        {
            m_PrefabSourceHandle = data["PrefabAssetHandle"].as<uint64_t>();
        }

        if (data["Children"] && data["Children"].IsSequence())
        {
            for (auto& child_node : data["Children"])
            {
                m_Children.push_back(child_node["ChildGUID"].as<uint64_t>());
            }
        }
        
        if (data["Scripts"] && data["Scripts"].IsMap())
        {
            for (const auto& script_node : data["Scripts"])
            {
                std::string script_name = script_node.first.as<std::string>();
                bool is_active = script_node.second.as<bool>();
                AddScript(script_name, is_active);
            }
        }

        return true;
    }
    
#pragma endregion

#pragma endregion

}