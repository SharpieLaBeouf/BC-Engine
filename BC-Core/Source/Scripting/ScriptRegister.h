#pragma once

namespace BC
{
    struct ScriptRegister
    {
        static bool RegisterAll(void* script_assembly);
        
#pragma region Debug

        static void Debug_NativeLogMessage(const char* message, uint8_t type);

#pragma endregion

#pragma region Time

        static float    Time_GetDeltaTime();
        static float    Time_GetFixedDeltaTime();
        static float    Time_GetUnscaledDeltaTime();
        static float    Time_GetUnscaledFixedDeltaTime();
        static float    Time_GetTimeScale();
        static void     Time_SetTimeScale(float time_scale);
        static int      Time_GetFrameRate();
        static float    Time_GetCurrTime();

        static void     Time_SetFixedDeltaTime(float fixed_time);

#pragma endregion

#pragma region Input

        static bool Input_GetKey(KeyCode key_code);
        static bool Input_GetKeyDown(KeyCode key_code);
        static bool Input_GetKeyUp(KeyCode key_code);

        static bool Input_GetMouseButton(MouseButtonCode button_code);
        static bool Input_GetMouseButtonDown(MouseButtonCode button_code);
        static bool Input_GetMouseButtonUp(MouseButtonCode button_code);
        static glm::vec2 Input_GetMousePosition();

#pragma endregion

#pragma region Entity

        struct _Transform
        {
            glm::vec3 position;
            glm::quat rotation;
            glm::vec3 scale;
        };

        static uint32_t Entity_Instantiate(uint32_t prefab_handle);
        static uint32_t Entity_Instantiate_Transform(uint32_t prefab_handle, const _Transform* transform);
		
        static uint32_t Entity_Create(const char* name);
        static void Entity_Destroy(GUID entity_guid);

        static uint32_t Entity_FindByName(const char* name);
        static uint32_t Entity_FindByGUID(GUID entity_guid);

        static GUID Entity_GetParent(GUID entity_guid);
        static void Entity_SetParent(GUID entity_guid, GUID parent_guid);
        
        static const char* Entity_GetName(GUID entity_guid);
        static void Entity_SetName(GUID entity_guid, const char* value);

        static bool Entity_CheckValidEntity(GUID entity_guid);

        static void Entity_AddComponent(GUID entity_guid, ScriptFieldType component_type);
        static void Entity_RemoveComponent(GUID entity_guid, ScriptFieldType component_type);
        static bool Entity_HasComponent(GUID entity_guid, ScriptFieldType component_type);

        static uint32_t Entity_GetComponentInParent(GUID entity_guid, ScriptFieldType component_type);
        static uint32_t Entity_GetComponentInChildren(GUID entity_guid, ScriptFieldType component_type);
        
        // We cannot allocate array on APP side. We need to get count, allocate array 
        // on DLL side, pass array back to APP, then APP fills in array.

        static inline std::unordered_map<uint64_t, std::vector<Entity>> s_CachedFindHierarchyComponents;

        static uint64_t Entity_GetComponentsInParentCount(GUID entity_guid, ScriptFieldType component_type, size_t* count);
        static uint64_t Entity_GetComponentsInChildrenCount(GUID entity_guid, ScriptFieldType component_type, size_t* count);

        static void Entity_GetComponentsInHierarchyCopy(GUID* entity_array, size_t count, uint64_t cached_key);

        template<typename T>
        static uint64_t CacheAndReturn(Entity& entity, uint64_t base_key, size_t* count, bool get_parents)
        {
            auto& result = get_parents
                ? s_CachedFindHierarchyComponents[base_key] = entity.GetComponentsInParents<T>()
                : s_CachedFindHierarchyComponents[base_key] = entity.GetComponentsInChildren<T>();

            *count = result.size();

            if (*count == 0)
            {
                s_CachedFindHierarchyComponents.erase(base_key);
                return -1;
            }

            return base_key;
        }

#pragma endregion

#pragma region TransformComponent

        static _Transform TransformComponent_GetTransform(GUID entity_guid, bool global);
        static void TransformComponent_SetTransform(GUID entity_guid, bool global, const _Transform* value);

        static glm::vec3 TransformComponent_GetPosition(GUID entity_guid, bool global);
        static void TransformComponent_SetPosition(GUID entity_guid, bool global, const glm::vec3* value);

        static glm::quat TransformComponent_GetOrientation(GUID entity_guid, bool global);
        static void TransformComponent_SetOrientation(GUID entity_guid, bool global, const glm::quat* value);

        static glm::vec3 TransformComponent_GetScale(GUID entity_guid, bool global);
        static void TransformComponent_SetScale(GUID entity_guid, bool global, const glm::vec3* value);
        
        static glm::vec3 TransformComponent_GetForwardDirection(GUID entity_guid, bool global);
        static void TransformComponent_SetForwardDirection(GUID entity_guid, bool global, const glm::vec3* value);

        static glm::vec3 TransformComponent_GetUpDirection(GUID entity_guid, bool global);
        static void TransformComponent_SetUpDirection(GUID entity_guid, bool global, const glm::vec3* value);

        static glm::vec3 TransformComponent_GetRightDirection(GUID entity_guid, bool global);
        static void TransformComponent_SetRightDirection(GUID entity_guid, bool global, const glm::vec3* value);

        static void TransformComponent_LookAt(GUID entity_guid, const glm::vec3* value);

#pragma endregion

#pragma region Camera Component

        static uint8_t CameraComponent_GetClearFlag(GUID entity_guid);
        static void CameraComponent_SetClearFlag(GUID entity_guid, uint8_t value);

        static glm::vec4 CameraComponent_GetClearColour(GUID entity_guid);
        static void CameraComponent_SetClearColour(GUID entity_guid, const glm::vec4* value);

        static AssetHandle CameraComponent_GetSkyboxHandle(GUID entity_guid);
        static void CameraComponent_SetSkyboxHandle(GUID entity_guid, AssetHandle value);

        static glm::vec4 CameraComponent_GetViewport(GUID entity_guid);
        static void CameraComponent_SetViewport(GUID entity_guid, const glm::vec4* value);

        static bool CameraComponent_GetShouldDisplay(GUID entity_guid);
        static void CameraComponent_SetShouldDisplay(GUID entity_guid, bool value);

        static bool CameraComponent_GetForceRender(GUID entity_guid);
        static void CameraComponent_SetForceRender(GUID entity_guid, bool value);

        static uint8_t CameraComponent_GetCameraDepth(GUID entity_guid);
        static void CameraComponent_SetCameraDepth(GUID entity_guid, uint8_t value);

        static AssetHandle CameraComponent_GetRenderTargetHandle(GUID entity_guid);

#pragma endregion

#pragma region MeshRendererComponent

        static bool MeshRendererComponent_GetIsActive(GUID entity_guid);
        static void MeshRendererComponent_SetIsActive(GUID entity_guid, bool value);

        static AssetHandle MeshRendererComponent_GetMeshAssetHandle(GUID entity_guid);
        static void MeshRendererComponent_SetMeshAssetHandle(GUID entity_guid, AssetHandle asset_handle);
        static AssetHandle MeshRendererComponent_CopyMesh(GUID entity_guid);

        static Bounds_AABB MeshRendererComponent_GetMeshBounds(GUID entity_guid);

        static bool MeshRendererComponent_GetIsCastingShadows(GUID entity_guid);
        static void MeshRendererComponent_SetCastingShadows(GUID entity_guid, bool value);

        static void MeshRendererComponent_GetAllMaterialsCount(GUID entity_guid, size_t* material_count);
        static void MeshRendererComponent_GetAllMaterialsCopy(GUID entity_guid, AssetHandle* material_array, size_t material_count);
        static void MeshRendererComponent_SetAllMaterials(GUID entity_guid, const AssetHandle* material_array, size_t material_count);

        static AssetHandle MeshRendererComponent_GetMaterial(GUID entity_guid, size_t material_index);
        static void MeshRendererComponent_SetMaterial(GUID entity_guid, size_t material_index, AssetHandle material_handle);

#pragma endregion

#pragma region LOD Component

        static bool LODMeshComponent_GetUseMaxDistanceOverFarPlane(GUID entity_guid);
        static void LODMeshComponent_SetMaxDistanceOverFarPlane(GUID entity_guid, bool value);

        static float LODMeshComponent_GetMaxDistance(GUID entity_guid);
        static void LODMeshComponent_SetMaxDistance(GUID entity_guid, float value);

        static size_t LODMeshComponent_GetElementCount(GUID entity_guid);
        static void LODMeshComponent_GetDistances(GUID entity_guid, float* distance_array, size_t element_count);
        static size_t LODMeshComponent_GetEntityCount(GUID entity_guid, size_t element_index);
        static void LODMeshComponent_GetEntityArray(GUID entity_guid, uint32_t* entity_array, size_t entity_count, size_t element_index);

        static void LODMeshComponent_SetLODElements(GUID entity_guid, const float* distance_array, const size_t* entity_counts, const uint32_t* entity_array, size_t element_count);

#pragma endregion

#pragma region Skinned Mesh Component

        static bool SkinnedMeshComponent_GetIsActive(GUID entity_guid);
        static void SkinnedMeshComponent_SetIsActive(GUID entity_guid, bool value);

        static bool SkinnedMeshComponent_GetIsCastingShadows(GUID entity_guid);
        static void SkinnedMeshComponent_SetCastingShadows(GUID entity_guid, bool value);

        static AssetHandle SkinnedMeshComponent_GetMeshAssetHandle(GUID entity_guid);
        static void SkinnedMeshComponent_SetMeshAssetHandle(GUID entity_guid, AssetHandle asset_handle);
        static AssetHandle SkinnedMeshComponent_CopyMesh(GUID entity_guid);

        static Bounds_AABB SkinnedMeshComponent_GetMeshBounds(GUID entity_guid);

        static void SkinnedMeshComponent_GetAllMaterialsCount(GUID entity_guid, size_t* material_count);
        static void SkinnedMeshComponent_GetAllMaterialsCopy(GUID entity_guid, uint32_t* material_array, size_t material_count);
        static void SkinnedMeshComponent_SetAllMaterials(GUID entity_guid, const uint32_t* material_array, size_t material_count);

        static uint32_t SkinnedMeshComponent_GetMaterial(GUID entity_guid, size_t material_index);
        static void SkinnedMeshComponent_SetMaterial(GUID entity_guid, size_t material_index, GUID material_handle);

#pragma endregion

#pragma region Sphere Light Component

        static bool SphereLightComponent_GetIsActive(GUID entity_guid);
        static void SphereLightComponent_SetIsActive(GUID entity_guid, bool value);

        static float SphereLightComponent_GetRadius(GUID entity_guid);
        static void SphereLightComponent_SetRadius(GUID entity_guid, float value);

        static float SphereLightComponent_GetIntensity(GUID entity_guid);
        static void SphereLightComponent_SetIntensity(GUID entity_guid, float value);

        static glm::vec4 SphereLightComponent_GetColour(GUID entity_guid);
        static void SphereLightComponent_SetColour(GUID entity_guid, const glm::vec4* value);

        static uint8_t SphereLightComponent_GetShadowFlag(GUID entity_guid);
        static void SphereLightComponent_SetShadowFlag(GUID entity_guid, uint8_t value);

#pragma endregion

#pragma region Cone Light Component

        static bool ConeLightComponent_GetIsActive(GUID entity_guid);
        static void ConeLightComponent_SetIsActive(GUID entity_guid, bool value);

        static glm::vec4 ConeLightComponent_GetColour(GUID entity_guid);
        static void ConeLightComponent_SetColour(GUID entity_guid, const glm::vec4* value);

        static float ConeLightComponent_GetRange(GUID entity_guid);
        static void ConeLightComponent_SetRange(GUID entity_guid, float value);

        static float ConeLightComponent_GetAngle(GUID entity_guid);
        static void ConeLightComponent_SetAngle(GUID entity_guid, float value);

        static float ConeLightComponent_GetIntensity(GUID entity_guid);
        static void ConeLightComponent_SetIntensity(GUID entity_guid, float value);

        static uint8_t ConeLightComponent_GetShadowFlag(GUID entity_guid);
        static void ConeLightComponent_SetShadowFlag(GUID entity_guid, uint8_t value);

#pragma endregion

#pragma region Directional Light Component

        static bool DirectionalLightComponent_GetIsActive(GUID entity_guid);
        static void DirectionalLightComponent_SetIsActive(GUID entity_guid, bool value);

        static float DirectionalLightComponent_GetIntensity(GUID entity_guid);
        static void DirectionalLightComponent_SetIntensity(GUID entity_guid, float value);

        static float DirectionalLightComponent_GetMaxShadowDistance(GUID entity_guid);
        static void DirectionalLightComponent_SetMaxShadowDistance(GUID entity_guid, float value);

        static glm::vec4 DirectionalLightComponent_GetColour(GUID entity_guid);
        static void DirectionalLightComponent_SetColour(GUID entity_guid, const glm::vec4* value);

        static uint8_t DirectionalLightComponent_GetShadowFlag(GUID entity_guid);
        static void DirectionalLightComponent_SetShadowFlag(GUID entity_guid, uint8_t value);

#pragma endregion

#pragma region Physics

        struct _PhysicsMaterial
        {
            float m_DynamicFriction;
            float m_StaticFriction;
            float m_Bounciness;
        };

#pragma region RigidbodyComponent Component

        static float RigidbodyComponent_GetMass(GUID entity_guid);
        static void RigidbodyComponent_SetMass(GUID entity_guid, float value);

        static float RigidbodyComponent_GetDrag(GUID entity_guid);
        static void RigidbodyComponent_SetDrag(GUID entity_guid, float value);

        static float RigidbodyComponent_GetAngularDrag(GUID entity_guid);
        static void RigidbodyComponent_SetAngularDrag(GUID entity_guid, float value);

        static bool RigidbodyComponent_GetAutomaticCentreOfMass(GUID entity_guid);
        static void RigidbodyComponent_SetAutomaticCentreOfMass(GUID entity_guid, bool value);

        static bool RigidbodyComponent_GetUseGravity(GUID entity_guid);
        static void RigidbodyComponent_SetUseGravity(GUID entity_guid, bool value);

        static bool RigidbodyComponent_GetIsKinematic(GUID entity_guid);
        static void RigidbodyComponent_SetIsKinematic(GUID entity_guid, bool value);

        static glm::bvec3 RigidbodyComponent_GetPositionConstraint(GUID entity_guid);
        static void RigidbodyComponent_SetPositionConstraint(GUID entity_guid, const glm::bvec3* value);

        static glm::bvec3 RigidbodyComponent_GetRotationConstraint(GUID entity_guid);
        static void RigidbodyComponent_SetRotationConstraint(GUID entity_guid, const glm::bvec3* value);

        static glm::vec3 RigidbodyComponent_GetLinearVelocity(GUID entity_guid);
        static void RigidbodyComponent_SetLinearVelocity(GUID entity_guid, const glm::vec3* value);

        static glm::vec3 RigidbodyComponent_GetAngularVelocity(GUID entity_guid);
        static void RigidbodyComponent_SetAngularVelocity(GUID entity_guid, const glm::vec3* value);

        static void RigidbodyComponent_ApplyForce(GUID entity_guid, const glm::vec3* force, uint8_t force_mode);
        static void RigidbodyComponent_ApplyTorque(GUID entity_guid, const glm::vec3* torque);

#pragma endregion

#pragma region Box Collider Component

        static bool BoxColliderComponent_GetIsTrigger(GUID entity_guid);
        static void BoxColliderComponent_SetIsTrigger(GUID entity_guid, bool value);

        static glm::vec3 BoxColliderComponent_GetCentre(GUID entity_guid);
        static void BoxColliderComponent_SetCentre(GUID entity_guid, const glm::vec3* value);

        static glm::vec3 BoxColliderComponent_GetSize(GUID entity_guid);
        static void BoxColliderComponent_SetSize(GUID entity_guid, const glm::vec3* value);

        static _PhysicsMaterial BoxColliderComponent_GetMaterial(GUID entity_guid);
        static void BoxColliderComponent_SetMaterial(GUID entity_guid, const _PhysicsMaterial* value);

#pragma endregion

#pragma region Sphere Collider Component

        static bool SphereColliderComponent_GetIsTrigger(GUID entity_guid);
        static void SphereColliderComponent_SetIsTrigger(GUID entity_guid, bool value);

        static glm::vec3 SphereColliderComponent_GetCentre(GUID entity_guid);
        static void SphereColliderComponent_SetCentre(GUID entity_guid, const glm::vec3* value);

        static float SphereColliderComponent_GetRadius(GUID entity_guid);
        static void SphereColliderComponent_SetRadius(GUID entity_guid, float value);

        static _PhysicsMaterial SphereColliderComponent_GetMaterial(GUID entity_guid);
        static void SphereColliderComponent_SetMaterial(GUID entity_guid, const _PhysicsMaterial* value);

#pragma endregion

#pragma region Capsule Collider Component

        static bool CapsuleColliderComponent_GetIsTrigger(GUID entity_guid);
        static void CapsuleColliderComponent_SetIsTrigger(GUID entity_guid, bool value);

        static glm::vec3 CapsuleColliderComponent_GetCentre(GUID entity_guid);
        static void CapsuleColliderComponent_SetCentre(GUID entity_guid, const glm::vec3* value);

        static float CapsuleColliderComponent_GetRadius(GUID entity_guid);
        static void CapsuleColliderComponent_SetRadius(GUID entity_guid, float value);

        static float CapsuleColliderComponent_GetHalfHeight(GUID entity_guid);
        static void CapsuleColliderComponent_SetHalfHeight(GUID entity_guid, float value);

        static _PhysicsMaterial CapsuleColliderComponent_GetMaterial(GUID entity_guid);
        static void CapsuleColliderComponent_SetMaterial(GUID entity_guid, const _PhysicsMaterial* value);

#pragma endregion

#pragma region Convex Mesh Collider Component

        static bool ConvexMeshColliderComponent_GetIsTrigger(GUID entity_guid);
        static void ConvexMeshColliderComponent_SetIsTrigger(GUID entity_guid, bool value);

        static glm::vec3 ConvexMeshColliderComponent_GetCentre(GUID entity_guid);
        static void ConvexMeshColliderComponent_SetCentre(GUID entity_guid, const glm::vec3* value);

        // TODO: IMPLEMENT MESH HANDLING/BAKING/ASSET?

        static _PhysicsMaterial ConvexMeshColliderComponent_GetMaterial(GUID entity_guid);
        static void ConvexMeshColliderComponent_SetMaterial(GUID entity_guid, const _PhysicsMaterial* value);

#pragma endregion

#pragma region Height Field Mesh Collider Component

        static bool HeightFieldColliderComponent_GetIsTrigger(GUID entity_guid);
        static void HeightFieldColliderComponent_SetIsTrigger(GUID entity_guid, bool value);

        static glm::vec3 HeightFieldColliderComponent_GetCentre(GUID entity_guid);
        static void HeightFieldColliderComponent_SetCentre(GUID entity_guid, const glm::vec3* value);

        // TODO: IMPLEMENT MESH HANDLING/BAKING/ASSET?

        static _PhysicsMaterial HeightFieldColliderComponent_GetMaterial(GUID entity_guid);
        static void HeightFieldColliderComponent_SetMaterial(GUID entity_guid, const _PhysicsMaterial* value);

#pragma endregion

#pragma region Triangle Mesh Collider Component

        static bool TriangleMeshColliderComponent_GetIsTrigger(GUID entity_guid);
        static void TriangleMeshColliderComponent_SetIsTrigger(GUID entity_guid, bool value);

        static glm::vec3 TriangleMeshColliderComponent_GetCentre(GUID entity_guid);
        static void TriangleMeshColliderComponent_SetCentre(GUID entity_guid, const glm::vec3* value);

        // TODO: IMPLEMENT MESH HANDLING/BAKING/ASSET?

        static _PhysicsMaterial TriangleMeshColliderComponent_GetMaterial(GUID entity_guid);
        static void TriangleMeshColliderComponent_SetMaterial(GUID entity_guid, const _PhysicsMaterial* value);

#pragma endregion

#pragma endregion

#pragma region Animator

        static void SimpleAnimationComponent_PlayAnimation_Index(GUID entity_guid, int32_t clip_index, bool should_loop);
        static void SimpleAnimationComponent_PlayAnimation_Name(GUID entity_guid, const char* clip_name, bool should_loop);

        static void SimpleAnimationComponent_PauseAnimation(GUID entity_guid);
        static void SimpleAnimationComponent_ResumeAnimation(GUID entity_guid);
        static void SimpleAnimationComponent_StopAnimation(GUID entity_guid);

        static bool SimpleAnimationComponent_IsPlaying(GUID entity_guid);
        static bool SimpleAnimationComponent_IsLooping(GUID entity_guid);
        static void SimpleAnimationComponent_SetIsLooping(GUID entity_guid, bool should_loop);

        static float SimpleAnimationComponent_GetPlaybackSpeed(GUID entity_guid);
        static void SimpleAnimationComponent_SetPlaybackSpeed(GUID entity_guid, float playback_speed);
        
        static float SimpleAnimationComponent_GetCurrentTimestep(GUID entity_guid);
        static void SimpleAnimationComponent_SetCurrentTimestep(GUID entity_guid, float normalised_time_step);
        
        static uint32_t SimpleAnimationComponent_GetCurrentClipIndex(GUID entity_guid);
        static const char* SimpleAnimationComponent_GetCurrentClipName(GUID entity_guid);

        static void AnimatorComponent_SetBool(GUID entity_guid, uint64_t param_hash, bool value);
        static void AnimatorComponent_SetFloat(GUID entity_guid, uint64_t param_hash, float value);
        static void AnimatorComponent_SetUInt(GUID entity_guid, uint64_t param_hash, uint32_t value);
        static void AnimatorComponent_SetInt(GUID entity_guid, uint64_t param_hash, int32_t value);

        static void AnimatorComponent_ResetMachine(GUID entity_guid);

#pragma endregion

#pragma region Compute Shader

        // TODO: implement
        // static void ComputeShader_SetBuffer(GUID asset_handle, ComputeBuffer* buffer, uint32_t binding_index);
        // static void ComputeShader_Dispatch(GUID asset_handle, uint32_t x, uint32_t y, uint32_t z);
        // static void ComputeShader_SetBool(GUID asset_handle, const char* name, bool value);
        // static void ComputeShader_SetInt(GUID asset_handle, const char* name, int32_t value);
        // static void ComputeShader_SetUInt(GUID asset_handle, const char* name, uint32_t value);
        // static void ComputeShader_SetFloat(GUID asset_handle, const char* name, float value);
        // static void ComputeShader_SetVector2(GUID asset_handle, const char* name, glm::vec2 value);
        // static void ComputeShader_SetVector3(GUID asset_handle, const char* name, glm::vec3 value);
        // static void ComputeShader_SetVector4(GUID asset_handle, const char* name, glm::vec4 value);
        // static void ComputeShader_SetMat3(GUID asset_handle, const char* name, const float* value);
        // static void ComputeShader_SetMat4(GUID asset_handle, const char* name, const float* value);

#pragma endregion

#pragma region Compute Buffer

        // TODO: implement
        // static ComputeBuffer* ComputeBuffer_Create(size_t element_count, size_t element_size);
        // static void ComputeBuffer_SetData(ComputeBuffer* buffer, const void* data, size_t element_count, size_t size_of_elements);
        // static void ComputeBuffer_GetData(ComputeBuffer* buffer, void* output, size_t element_count, size_t size_of_elements);
        // static void ComputeBuffer_Release(ComputeBuffer* buffer);

#pragma endregion

#pragma region Material

        // TODO: implement
        // static uint32_t Material_Create(const char* material_name);
        // static void Material_SetShader(GUID asset_handle, GUID shader_handle);
        // static uint32_t Material_GetShader(GUID asset_handle);
        // static void Material_Destroy(GUID asset_handle);

#pragma endregion

#pragma region Texture2D

        // TODO: implement
        // static uint32_t Texture2D_Create(int width, int height, uint8_t internal_format);
        // static uint32_t Texture2D_CreateWithData(unsigned char* data, int width, int height, uint8_t internal_format, uint8_t data_format);

        // static void Texture2D_SetPixel(uint32_t asset_handle, const glm::vec4* colour, const glm::ivec2* pixel_coord);
        // static void Texture2D_SetPixelData(uint32_t asset_handle, unsigned char* pixel_data, size_t pixel_data_size, uint8_t pixel_data_format);
        // static void Texture2D_SubmitTextureChanges(uint32_t asset_handle);

        // static void Texture2D_Destroy(AssetHandle handle);

#pragma endregion

#pragma region Mesh

        // TODO: implement

        // Mesh Functions
        // static uint32_t Mesh_CreateNewMesh();
        // static void Mesh_Destroy(AssetHandle handle);

        // static void Mesh_SubmitChanges(GUID asset_handle, bool clearCPUData);
        // static void Mesh_CopyBufferDataToCPU(GUID asset_handle);
        // static void Mesh_ClearBufferDataFromCPU(GUID asset_handle);

        // static void Mesh_SetVertices(GUID asset_handle, const float* data, size_t data_size);
        // static void Mesh_SetNormals(GUID asset_handle, const float* data, size_t data_size);
        // static void Mesh_SetTextureCoords(GUID asset_handle, const float* data, size_t data_size);
        // static void Mesh_SetTangents(GUID asset_handle, const float* data, size_t data_size);
        // static void Mesh_SetBitangents(GUID asset_handle, const float* data, size_t data_size);
        // static void Mesh_SetBoneIDs(GUID asset_handle, const glm::ivec4* data, size_t data_size);
        // static void Mesh_SetBoneWeights(GUID asset_handle, const glm::vec4* data, size_t data_size);

        // // pointer to const float*, pointer to data size_t
        // static void Mesh_GetVertices(GUID asset_handle, const float** data, size_t* data_size);
        // static void Mesh_GetNormals(GUID asset_handle, const float** data, size_t* data_size);
        // static void Mesh_GetTextureCoords(GUID asset_handle, const float** data, size_t* data_size);
        // static void Mesh_GetTangents(GUID asset_handle, const float** data, size_t* data_size);
        // static void Mesh_GetBitangents(GUID asset_handle, const float** data, size_t* data_size);
		// static void Mesh_GetBoneIDs(GUID asset_handle, const glm::ivec4** data, size_t* data_size);
		// static void Mesh_GetBoneWeights(GUID asset_handle, const glm::vec4** data, size_t* data_size);

        // static void Mesh_SetTriangles(GUID asset_handle, const uint32_t* data, size_t data_size);
        // static void Mesh_GetTriangles(GUID asset_handle, const uint32_t** data, size_t* data_size);

        // static void Mesh_RecalculateNormals(GUID asset_handle);

#pragma endregion

    };
}