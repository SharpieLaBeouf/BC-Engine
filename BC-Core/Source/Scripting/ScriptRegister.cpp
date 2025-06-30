#include "BC_PCH.h"
#include "ScriptRegister.h"

namespace BC
{

    bool ScriptRegister::RegisterAll(void *script_assembly)
    {
        using RegisterHostFunctionFn = void(*)(const char* name, void* fn);

    #if defined(BC_PLATFORM_WINDOWS)
        
        auto register_function = (RegisterHostFunctionFn)GetProcAddress((HMODULE)script_assembly, "RegisterHostFunction");
    
    #elif defined(BC_PLATFORM_LINUX)
    
        auto register_function = (RegisterHostFunctionFn)dlsym(script_assembly, "RegisterHostFunction");
        
    #endif

        if (!register_function)
        {
            BC_CORE_ERROR("ScriptRegister::RegisterAll: Could Not Find RegisterHostFunction in ScriptDLL.");
            return false;
        }

    #pragma region Debug

        register_function("Debug_NativeLogMessage", reinterpret_cast<void*>(&Debug_NativeLogMessage));

    #pragma endregion

    #pragma region Time
    
        register_function("Time_GetDeltaTime", reinterpret_cast<void*>(&Time_GetDeltaTime));
        register_function("Time_GetFixedDeltaTime", reinterpret_cast<void*>(&Time_GetFixedDeltaTime));
        register_function("Time_GetUnscaledDeltaTime", reinterpret_cast<void*>(&Time_GetUnscaledDeltaTime));
        register_function("Time_GetUnscaledFixedDeltaTime", reinterpret_cast<void*>(&Time_GetUnscaledFixedDeltaTime));
        register_function("Time_GetTimeScale", reinterpret_cast<void*>(&Time_GetTimeScale));
        register_function("Time_SetTimeScale", reinterpret_cast<void*>(&Time_SetTimeScale));
        register_function("Time_GetFrameRate", reinterpret_cast<void*>(&Time_GetFrameRate));
        register_function("Time_GetCurrTime", reinterpret_cast<void*>(&Time_GetCurrTime));
		register_function("Time_SetFixedDeltaTime", reinterpret_cast<void*>(&Time_SetFixedDeltaTime));
        
    #pragma endregion

    #pragma region Input

        register_function("Input_GetKey", reinterpret_cast<void*>(&Input_GetKey));
        register_function("Input_GetKeyDown", reinterpret_cast<void*>(&Input_GetKeyDown));
        register_function("Input_GetKeyUp", reinterpret_cast<void*>(&Input_GetKeyUp));
        register_function("Input_GetMouseButton", reinterpret_cast<void*>(&Input_GetMouseButton));
        register_function("Input_GetMouseButtonDown", reinterpret_cast<void*>(&Input_GetMouseButtonDown));
        register_function("Input_GetMouseButtonUp", reinterpret_cast<void*>(&Input_GetMouseButtonUp));
        register_function("Input_GetMousePosition", reinterpret_cast<void*>(&Input_GetMousePosition));

    #pragma endregion

    #pragma region Entity
        register_function("Entity_Instantiate", reinterpret_cast<void*>(&Entity_Instantiate));
        register_function("Entity_Instantiate_Transform", reinterpret_cast<void*>(&Entity_Instantiate_Transform));
        register_function("Entity_Create", reinterpret_cast<void*>(&Entity_Create));
        register_function("Entity_Destroy", reinterpret_cast<void*>(&Entity_Destroy));
        register_function("Entity_FindByName", reinterpret_cast<void*>(&Entity_FindByName));
        register_function("Entity_FindByGUID", reinterpret_cast<void*>(&Entity_FindByGUID));
        register_function("Entity_GetParent", reinterpret_cast<void*>(&Entity_GetParent));
        register_function("Entity_SetParent", reinterpret_cast<void*>(&Entity_SetParent));
        register_function("Entity_GetName", reinterpret_cast<void*>(&Entity_GetName));
        register_function("Entity_SetName", reinterpret_cast<void*>(&Entity_SetName));
        register_function("Entity_CheckValidEntity", reinterpret_cast<void*>(&Entity_CheckValidEntity));
        register_function("Entity_AddComponent", reinterpret_cast<void*>(&Entity_AddComponent));
        register_function("Entity_RemoveComponent", reinterpret_cast<void*>(&Entity_RemoveComponent));
        register_function("Entity_HasComponent", reinterpret_cast<void*>(&Entity_HasComponent));
        register_function("Entity_GetComponentInParent", reinterpret_cast<void*>(&Entity_GetComponentInParent));
        register_function("Entity_GetComponentInChildren", reinterpret_cast<void*>(&Entity_GetComponentInChildren));
        register_function("Entity_GetComponentsInParentCount", reinterpret_cast<void*>(&Entity_GetComponentsInParentCount));
        register_function("Entity_GetComponentsInChildrenCount", reinterpret_cast<void*>(&Entity_GetComponentsInChildrenCount));
        register_function("Entity_GetComponentsInHierarchyCopy", reinterpret_cast<void*>(&Entity_GetComponentsInHierarchyCopy));
    #pragma endregion

    #pragma region TransformComponent
        register_function("TransformComponent_GetTransform", reinterpret_cast<void*>(&TransformComponent_GetTransform));
        register_function("TransformComponent_SetTransform", reinterpret_cast<void*>(&TransformComponent_SetTransform));
        register_function("TransformComponent_GetPosition", reinterpret_cast<void*>(&TransformComponent_GetPosition));
        register_function("TransformComponent_SetPosition", reinterpret_cast<void*>(&TransformComponent_SetPosition));
        register_function("TransformComponent_GetOrientation", reinterpret_cast<void*>(&TransformComponent_GetOrientation));
        register_function("TransformComponent_SetOrientation", reinterpret_cast<void*>(&TransformComponent_SetOrientation));
        register_function("TransformComponent_GetScale", reinterpret_cast<void*>(&TransformComponent_GetScale));
        register_function("TransformComponent_SetScale", reinterpret_cast<void*>(&TransformComponent_SetScale));
        register_function("TransformComponent_GetForwardDirection", reinterpret_cast<void*>(&TransformComponent_GetForwardDirection));
        register_function("TransformComponent_SetForwardDirection", reinterpret_cast<void*>(&TransformComponent_SetForwardDirection));
        register_function("TransformComponent_GetUpDirection", reinterpret_cast<void*>(&TransformComponent_GetUpDirection));
        register_function("TransformComponent_SetUpDirection", reinterpret_cast<void*>(&TransformComponent_SetUpDirection));
        register_function("TransformComponent_GetRightDirection", reinterpret_cast<void*>(&TransformComponent_GetRightDirection));
        register_function("TransformComponent_SetRightDirection", reinterpret_cast<void*>(&TransformComponent_SetRightDirection));
        register_function("TransformComponent_LookAt", reinterpret_cast<void*>(&TransformComponent_LookAt));
    #pragma endregion

    #pragma region CameraComponent
        register_function("CameraComponent_GetClearFlag", reinterpret_cast<void*>(&CameraComponent_GetClearFlag));
        register_function("CameraComponent_SetClearFlag", reinterpret_cast<void*>(&CameraComponent_SetClearFlag));
        register_function("CameraComponent_GetClearColour", reinterpret_cast<void*>(&CameraComponent_GetClearColour));
        register_function("CameraComponent_SetClearColour", reinterpret_cast<void*>(&CameraComponent_SetClearColour));
        register_function("CameraComponent_GetSkyboxHandle", reinterpret_cast<void*>(&CameraComponent_GetSkyboxHandle));
        register_function("CameraComponent_SetSkyboxHandle", reinterpret_cast<void*>(&CameraComponent_SetSkyboxHandle));
        register_function("CameraComponent_GetViewport", reinterpret_cast<void*>(&CameraComponent_GetViewport));
        register_function("CameraComponent_SetViewport", reinterpret_cast<void*>(&CameraComponent_SetViewport));
        register_function("CameraComponent_GetShouldDisplay", reinterpret_cast<void*>(&CameraComponent_GetShouldDisplay));
        register_function("CameraComponent_SetShouldDisplay", reinterpret_cast<void*>(&CameraComponent_SetShouldDisplay));
        register_function("CameraComponent_GetForceRender", reinterpret_cast<void*>(&CameraComponent_GetForceRender));
        register_function("CameraComponent_SetForceRender", reinterpret_cast<void*>(&CameraComponent_SetForceRender));
        register_function("CameraComponent_GetCameraDepth", reinterpret_cast<void*>(&CameraComponent_GetCameraDepth));
        register_function("CameraComponent_SetCameraDepth", reinterpret_cast<void*>(&CameraComponent_SetCameraDepth));
        register_function("CameraComponent_GetRenderTargetHandle", reinterpret_cast<void*>(&CameraComponent_GetRenderTargetHandle));
    #pragma endregion

    #pragma region MeshRendererComponent
        register_function("MeshRendererComponent_GetIsActive", reinterpret_cast<void*>(&MeshRendererComponent_GetIsActive));
        register_function("MeshRendererComponent_SetIsActive", reinterpret_cast<void*>(&MeshRendererComponent_SetIsActive));
        register_function("MeshRendererComponent_GetMeshAssetHandle", reinterpret_cast<void*>(&MeshRendererComponent_GetMeshAssetHandle));
        register_function("MeshRendererComponent_SetMeshAssetHandle", reinterpret_cast<void*>(&MeshRendererComponent_SetMeshAssetHandle));
        register_function("MeshRendererComponent_CopyMesh", reinterpret_cast<void*>(&MeshRendererComponent_CopyMesh));
        register_function("MeshRendererComponent_GetMeshBounds", reinterpret_cast<void*>(&MeshRendererComponent_GetMeshBounds));
        register_function("MeshRendererComponent_GetIsCastingShadows", reinterpret_cast<void*>(&MeshRendererComponent_GetIsCastingShadows));
        register_function("MeshRendererComponent_SetCastingShadows", reinterpret_cast<void*>(&MeshRendererComponent_SetCastingShadows));
        register_function("MeshRendererComponent_GetAllMaterialsCount", reinterpret_cast<void*>(&MeshRendererComponent_GetAllMaterialsCount));
        register_function("MeshRendererComponent_GetAllMaterialsCopy", reinterpret_cast<void*>(&MeshRendererComponent_GetAllMaterialsCopy));
        register_function("MeshRendererComponent_SetAllMaterials", reinterpret_cast<void*>(&MeshRendererComponent_SetAllMaterials));
        register_function("MeshRendererComponent_GetMaterial", reinterpret_cast<void*>(&MeshRendererComponent_GetMaterial));
        register_function("MeshRendererComponent_SetMaterial", reinterpret_cast<void*>(&MeshRendererComponent_SetMaterial));
    #pragma endregion

    #pragma region SkinnedMeshComponent
        register_function("SkinnedMeshComponent_GetIsActive", reinterpret_cast<void*>(&SkinnedMeshComponent_GetIsActive));
        register_function("SkinnedMeshComponent_SetIsActive", reinterpret_cast<void*>(&SkinnedMeshComponent_SetIsActive));
        register_function("SkinnedMeshComponent_GetIsCastingShadows", reinterpret_cast<void*>(&SkinnedMeshComponent_GetIsCastingShadows));
        register_function("SkinnedMeshComponent_SetCastingShadows", reinterpret_cast<void*>(&SkinnedMeshComponent_SetCastingShadows));
        register_function("SkinnedMeshComponent_GetMeshAssetHandle", reinterpret_cast<void*>(&SkinnedMeshComponent_GetMeshAssetHandle));
        register_function("SkinnedMeshComponent_SetMeshAssetHandle", reinterpret_cast<void*>(&SkinnedMeshComponent_SetMeshAssetHandle));
        register_function("SkinnedMeshComponent_CopyMesh", reinterpret_cast<void*>(&SkinnedMeshComponent_CopyMesh));
        register_function("SkinnedMeshComponent_GetMeshBounds", reinterpret_cast<void*>(&SkinnedMeshComponent_GetMeshBounds));
        register_function("SkinnedMeshComponent_GetAllMaterialsCount", reinterpret_cast<void*>(&SkinnedMeshComponent_GetAllMaterialsCount));
        register_function("SkinnedMeshComponent_GetAllMaterialsCopy", reinterpret_cast<void*>(&SkinnedMeshComponent_GetAllMaterialsCopy));
        register_function("SkinnedMeshComponent_SetAllMaterials", reinterpret_cast<void*>(&SkinnedMeshComponent_SetAllMaterials));
        register_function("SkinnedMeshComponent_GetMaterial", reinterpret_cast<void*>(&SkinnedMeshComponent_GetMaterial));
        register_function("SkinnedMeshComponent_SetMaterial", reinterpret_cast<void*>(&SkinnedMeshComponent_SetMaterial));
    #pragma endregion

    #pragma region LODMeshComponent
        register_function("LODMeshComponent_GetUseMaxDistanceOverFarPlane", reinterpret_cast<void*>(&LODMeshComponent_GetUseMaxDistanceOverFarPlane));
        register_function("LODMeshComponent_SetMaxDistanceOverFarPlane", reinterpret_cast<void*>(&LODMeshComponent_SetMaxDistanceOverFarPlane));
        register_function("LODMeshComponent_GetMaxDistance", reinterpret_cast<void*>(&LODMeshComponent_GetMaxDistance));
        register_function("LODMeshComponent_SetMaxDistance", reinterpret_cast<void*>(&LODMeshComponent_SetMaxDistance));
        register_function("LODMeshComponent_GetElementCount", reinterpret_cast<void*>(&LODMeshComponent_GetElementCount));
        register_function("LODMeshComponent_GetDistances", reinterpret_cast<void*>(&LODMeshComponent_GetDistances));
        register_function("LODMeshComponent_GetEntityCount", reinterpret_cast<void*>(&LODMeshComponent_GetEntityCount));
        register_function("LODMeshComponent_GetEntityArray", reinterpret_cast<void*>(&LODMeshComponent_GetEntityArray));
        register_function("LODMeshComponent_SetLODElements", reinterpret_cast<void*>(&LODMeshComponent_SetLODElements));
    #pragma endregion

    #pragma region SphereLightComponent
        register_function("SphereLightComponent_GetIsActive", reinterpret_cast<void*>(&SphereLightComponent_GetIsActive));
        register_function("SphereLightComponent_SetIsActive", reinterpret_cast<void*>(&SphereLightComponent_SetIsActive));
        register_function("SphereLightComponent_GetRadius", reinterpret_cast<void*>(&SphereLightComponent_GetRadius));
        register_function("SphereLightComponent_SetRadius", reinterpret_cast<void*>(&SphereLightComponent_SetRadius));
        register_function("SphereLightComponent_GetIntensity", reinterpret_cast<void*>(&SphereLightComponent_GetIntensity));
        register_function("SphereLightComponent_SetIntensity", reinterpret_cast<void*>(&SphereLightComponent_SetIntensity));
        register_function("SphereLightComponent_GetColour", reinterpret_cast<void*>(&SphereLightComponent_GetColour));
        register_function("SphereLightComponent_SetColour", reinterpret_cast<void*>(&SphereLightComponent_SetColour));
        register_function("SphereLightComponent_GetShadowFlag", reinterpret_cast<void*>(&SphereLightComponent_GetShadowFlag));
        register_function("SphereLightComponent_SetShadowFlag", reinterpret_cast<void*>(&SphereLightComponent_SetShadowFlag));
    #pragma endregion

    #pragma region ConeLightComponent
        register_function("ConeLightComponent_GetIsActive", reinterpret_cast<void*>(&ConeLightComponent_GetIsActive));
        register_function("ConeLightComponent_SetIsActive", reinterpret_cast<void*>(&ConeLightComponent_SetIsActive));
        register_function("ConeLightComponent_GetColour", reinterpret_cast<void*>(&ConeLightComponent_GetColour));
        register_function("ConeLightComponent_SetColour", reinterpret_cast<void*>(&ConeLightComponent_SetColour));
        register_function("ConeLightComponent_GetRange", reinterpret_cast<void*>(&ConeLightComponent_GetRange));
        register_function("ConeLightComponent_SetRange", reinterpret_cast<void*>(&ConeLightComponent_SetRange));
        register_function("ConeLightComponent_GetAngle", reinterpret_cast<void*>(&ConeLightComponent_GetAngle));
        register_function("ConeLightComponent_SetAngle", reinterpret_cast<void*>(&ConeLightComponent_SetAngle));
        register_function("ConeLightComponent_GetIntensity", reinterpret_cast<void*>(&ConeLightComponent_GetIntensity));
        register_function("ConeLightComponent_SetIntensity", reinterpret_cast<void*>(&ConeLightComponent_SetIntensity));
        register_function("ConeLightComponent_GetShadowFlag", reinterpret_cast<void*>(&ConeLightComponent_GetShadowFlag));
        register_function("ConeLightComponent_SetShadowFlag", reinterpret_cast<void*>(&ConeLightComponent_SetShadowFlag));
    #pragma endregion

    #pragma region DirectionalLightComponent
        register_function("DirectionalLightComponent_GetIsActive", reinterpret_cast<void*>(&DirectionalLightComponent_GetIsActive));
        register_function("DirectionalLightComponent_SetIsActive", reinterpret_cast<void*>(&DirectionalLightComponent_SetIsActive));
        register_function("DirectionalLightComponent_GetIntensity", reinterpret_cast<void*>(&DirectionalLightComponent_GetIntensity));
        register_function("DirectionalLightComponent_SetIntensity", reinterpret_cast<void*>(&DirectionalLightComponent_SetIntensity));
        register_function("DirectionalLightComponent_GetMaxShadowDistance", reinterpret_cast<void*>(&DirectionalLightComponent_GetMaxShadowDistance));
        register_function("DirectionalLightComponent_SetMaxShadowDistance", reinterpret_cast<void*>(&DirectionalLightComponent_SetMaxShadowDistance));
        register_function("DirectionalLightComponent_GetColour", reinterpret_cast<void*>(&DirectionalLightComponent_GetColour));
        register_function("DirectionalLightComponent_SetColour", reinterpret_cast<void*>(&DirectionalLightComponent_SetColour));
        register_function("DirectionalLightComponent_GetShadowFlag", reinterpret_cast<void*>(&DirectionalLightComponent_GetShadowFlag));
        register_function("DirectionalLightComponent_SetShadowFlag", reinterpret_cast<void*>(&DirectionalLightComponent_SetShadowFlag));
    #pragma endregion
        
    #pragma region RigidbodyComponent
        register_function("RigidbodyComponent_GetMass", reinterpret_cast<void*>(&RigidbodyComponent_GetMass));
        register_function("RigidbodyComponent_SetMass", reinterpret_cast<void*>(&RigidbodyComponent_SetMass));
        register_function("RigidbodyComponent_GetDrag", reinterpret_cast<void*>(&RigidbodyComponent_GetDrag));
        register_function("RigidbodyComponent_SetDrag", reinterpret_cast<void*>(&RigidbodyComponent_SetDrag));
        register_function("RigidbodyComponent_GetAngularDrag", reinterpret_cast<void*>(&RigidbodyComponent_GetAngularDrag));
        register_function("RigidbodyComponent_SetAngularDrag", reinterpret_cast<void*>(&RigidbodyComponent_SetAngularDrag));
        register_function("RigidbodyComponent_GetAutomaticCentreOfMass", reinterpret_cast<void*>(&RigidbodyComponent_GetAutomaticCentreOfMass));
        register_function("RigidbodyComponent_SetAutomaticCentreOfMass", reinterpret_cast<void*>(&RigidbodyComponent_SetAutomaticCentreOfMass));
        register_function("RigidbodyComponent_GetUseGravity", reinterpret_cast<void*>(&RigidbodyComponent_GetUseGravity));
        register_function("RigidbodyComponent_SetUseGravity", reinterpret_cast<void*>(&RigidbodyComponent_SetUseGravity));
        register_function("RigidbodyComponent_GetIsKinematic", reinterpret_cast<void*>(&RigidbodyComponent_GetIsKinematic));
        register_function("RigidbodyComponent_SetIsKinematic", reinterpret_cast<void*>(&RigidbodyComponent_SetIsKinematic));
        register_function("RigidbodyComponent_GetPositionConstraint", reinterpret_cast<void*>(&RigidbodyComponent_GetPositionConstraint));
        register_function("RigidbodyComponent_SetPositionConstraint", reinterpret_cast<void*>(&RigidbodyComponent_SetPositionConstraint));
        register_function("RigidbodyComponent_GetRotationConstraint", reinterpret_cast<void*>(&RigidbodyComponent_GetRotationConstraint));
        register_function("RigidbodyComponent_SetRotationConstraint", reinterpret_cast<void*>(&RigidbodyComponent_SetRotationConstraint));
        register_function("RigidbodyComponent_GetLinearVelocity", reinterpret_cast<void*>(&RigidbodyComponent_GetLinearVelocity));
        register_function("RigidbodyComponent_SetLinearVelocity", reinterpret_cast<void*>(&RigidbodyComponent_SetLinearVelocity));
        register_function("RigidbodyComponent_GetAngularVelocity", reinterpret_cast<void*>(&RigidbodyComponent_GetAngularVelocity));
        register_function("RigidbodyComponent_SetAngularVelocity", reinterpret_cast<void*>(&RigidbodyComponent_SetAngularVelocity));
        register_function("RigidbodyComponent_ApplyForce", reinterpret_cast<void*>(&RigidbodyComponent_ApplyForce));
        register_function("RigidbodyComponent_ApplyTorque", reinterpret_cast<void*>(&RigidbodyComponent_ApplyTorque));
    #pragma endregion

    #pragma region BoxColliderComponent
        register_function("BoxColliderComponent_GetIsTrigger", reinterpret_cast<void*>(&BoxColliderComponent_GetIsTrigger));
        register_function("BoxColliderComponent_SetIsTrigger", reinterpret_cast<void*>(&BoxColliderComponent_SetIsTrigger));
        register_function("BoxColliderComponent_GetCentre", reinterpret_cast<void*>(&BoxColliderComponent_GetCentre));
        register_function("BoxColliderComponent_SetCentre", reinterpret_cast<void*>(&BoxColliderComponent_SetCentre));
        register_function("BoxColliderComponent_GetSize", reinterpret_cast<void*>(&BoxColliderComponent_GetSize));
        register_function("BoxColliderComponent_SetSize", reinterpret_cast<void*>(&BoxColliderComponent_SetSize));
        register_function("BoxColliderComponent_GetMaterial", reinterpret_cast<void*>(&BoxColliderComponent_GetMaterial));
        register_function("BoxColliderComponent_SetMaterial", reinterpret_cast<void*>(&BoxColliderComponent_SetMaterial));
    #pragma endregion

    #pragma region SphereColliderComponent
        register_function("SphereColliderComponent_GetIsTrigger", reinterpret_cast<void*>(&SphereColliderComponent_GetIsTrigger));
        register_function("SphereColliderComponent_SetIsTrigger", reinterpret_cast<void*>(&SphereColliderComponent_SetIsTrigger));
        register_function("SphereColliderComponent_GetCentre", reinterpret_cast<void*>(&SphereColliderComponent_GetCentre));
        register_function("SphereColliderComponent_SetCentre", reinterpret_cast<void*>(&SphereColliderComponent_SetCentre));
        register_function("SphereColliderComponent_GetRadius", reinterpret_cast<void*>(&SphereColliderComponent_GetRadius));
        register_function("SphereColliderComponent_SetRadius", reinterpret_cast<void*>(&SphereColliderComponent_SetRadius));
        register_function("SphereColliderComponent_GetMaterial", reinterpret_cast<void*>(&SphereColliderComponent_GetMaterial));
        register_function("SphereColliderComponent_SetMaterial", reinterpret_cast<void*>(&SphereColliderComponent_SetMaterial));
    #pragma endregion

    #pragma region CapsuleColliderComponent
        register_function("CapsuleColliderComponent_GetIsTrigger", reinterpret_cast<void*>(&CapsuleColliderComponent_GetIsTrigger));
        register_function("CapsuleColliderComponent_SetIsTrigger", reinterpret_cast<void*>(&CapsuleColliderComponent_SetIsTrigger));
        register_function("CapsuleColliderComponent_GetCentre", reinterpret_cast<void*>(&CapsuleColliderComponent_GetCentre));
        register_function("CapsuleColliderComponent_SetCentre", reinterpret_cast<void*>(&CapsuleColliderComponent_SetCentre));
        register_function("CapsuleColliderComponent_GetRadius", reinterpret_cast<void*>(&CapsuleColliderComponent_GetRadius));
        register_function("CapsuleColliderComponent_SetRadius", reinterpret_cast<void*>(&CapsuleColliderComponent_SetRadius));
        register_function("CapsuleColliderComponent_GetHalfHeight", reinterpret_cast<void*>(&CapsuleColliderComponent_GetHalfHeight));
        register_function("CapsuleColliderComponent_SetHalfHeight", reinterpret_cast<void*>(&CapsuleColliderComponent_SetHalfHeight));
        register_function("CapsuleColliderComponent_GetMaterial", reinterpret_cast<void*>(&CapsuleColliderComponent_GetMaterial));
        register_function("CapsuleColliderComponent_SetMaterial", reinterpret_cast<void*>(&CapsuleColliderComponent_SetMaterial));
    #pragma endregion

    #pragma region ConvexMeshColliderComponent
        register_function("ConvexMeshColliderComponent_GetIsTrigger", reinterpret_cast<void*>(&ConvexMeshColliderComponent_GetIsTrigger));
        register_function("ConvexMeshColliderComponent_SetIsTrigger", reinterpret_cast<void*>(&ConvexMeshColliderComponent_SetIsTrigger));
        register_function("ConvexMeshColliderComponent_GetCentre", reinterpret_cast<void*>(&ConvexMeshColliderComponent_GetCentre));
        register_function("ConvexMeshColliderComponent_SetCentre", reinterpret_cast<void*>(&ConvexMeshColliderComponent_SetCentre));
        register_function("ConvexMeshColliderComponent_GetMaterial", reinterpret_cast<void*>(&ConvexMeshColliderComponent_GetMaterial));
        register_function("ConvexMeshColliderComponent_SetMaterial", reinterpret_cast<void*>(&ConvexMeshColliderComponent_SetMaterial));
    #pragma endregion

    #pragma region HeightFieldColliderComponent
        register_function("HeightFieldColliderComponent_GetIsTrigger", reinterpret_cast<void*>(&HeightFieldColliderComponent_GetIsTrigger));
        register_function("HeightFieldColliderComponent_SetIsTrigger", reinterpret_cast<void*>(&HeightFieldColliderComponent_SetIsTrigger));
        register_function("HeightFieldColliderComponent_GetCentre", reinterpret_cast<void*>(&HeightFieldColliderComponent_GetCentre));
        register_function("HeightFieldColliderComponent_SetCentre", reinterpret_cast<void*>(&HeightFieldColliderComponent_SetCentre));
        register_function("HeightFieldColliderComponent_GetMaterial", reinterpret_cast<void*>(&HeightFieldColliderComponent_GetMaterial));
        register_function("HeightFieldColliderComponent_SetMaterial", reinterpret_cast<void*>(&HeightFieldColliderComponent_SetMaterial));
    #pragma endregion

    #pragma region TriangleMeshColliderComponent
        register_function("TriangleMeshColliderComponent_GetIsTrigger", reinterpret_cast<void*>(&TriangleMeshColliderComponent_GetIsTrigger));
        register_function("TriangleMeshColliderComponent_SetIsTrigger", reinterpret_cast<void*>(&TriangleMeshColliderComponent_SetIsTrigger));
        register_function("TriangleMeshColliderComponent_GetCentre", reinterpret_cast<void*>(&TriangleMeshColliderComponent_GetCentre));
        register_function("TriangleMeshColliderComponent_SetCentre", reinterpret_cast<void*>(&TriangleMeshColliderComponent_SetCentre));
        register_function("TriangleMeshColliderComponent_GetMaterial", reinterpret_cast<void*>(&TriangleMeshColliderComponent_GetMaterial));
        register_function("TriangleMeshColliderComponent_SetMaterial", reinterpret_cast<void*>(&TriangleMeshColliderComponent_SetMaterial));
    #pragma endregion

        return true;
    }

#pragma region Debug

    void ScriptRegister::Debug_NativeLogMessage(const char* message, uint8_t type)
    {
        switch (type)
        {
            default:
            case 0:     BC_SCRIPT_INFO(message);     break;
            case 1:     BC_SCRIPT_WARN(message);     break;
            case 2:     BC_SCRIPT_ERROR(message);    break;
            case 3:     BC_SCRIPT_FATAL(message);    break;
        }
    }

#pragma endregion

#pragma region Time

    float ScriptRegister::Time_GetDeltaTime() { return Time::GetDeltaTime(); }
    float ScriptRegister::Time_GetFixedDeltaTime() { return Time::GetFixedDeltaTime(); }
    float ScriptRegister::Time_GetUnscaledDeltaTime() { return Time::GetUnscaledDeltaTime(); }
    float ScriptRegister::Time_GetUnscaledFixedDeltaTime() { return Time::GetUnscaledFixedDeltaTime(); }
    float ScriptRegister::Time_GetTimeScale() { return Time::GetTimeScale(); }
    void ScriptRegister::Time_SetTimeScale(float time_scale) { Time::SetTimeScale(time_scale); }
    int ScriptRegister::Time_GetFrameRate() { return Time::GetFrameRate(); }
    float ScriptRegister::Time_GetCurrTime() { return static_cast<float>(Time::GetCurrTime()); }

    void ScriptRegister::Time_SetFixedDeltaTime(float fixed_time) { Time::SetFixedDeltaTime(fixed_time); }

#pragma endregion

#pragma region Input

    bool ScriptRegister::Input_GetKey(KeyCode key_code) { return Input::GetKey(key_code); }
    bool ScriptRegister::Input_GetKeyDown(KeyCode key_code) { return Input::GetKeyDown(key_code); }
    bool ScriptRegister::Input_GetKeyUp(KeyCode key_code) { return Input::GetKeyUp(key_code); }

    bool ScriptRegister::Input_GetMouseButton(MouseButtonCode button_code) { return Input::GetMouseButton(button_code); }
    bool ScriptRegister::Input_GetMouseButtonDown(MouseButtonCode button_code) { return Input::GetMouseButtonDown(button_code); }
    bool ScriptRegister::Input_GetMouseButtonUp(MouseButtonCode button_code) { return Input::GetMouseButtonUp(button_code); }
    glm::vec2 ScriptRegister::Input_GetMousePosition() { return Input::GetMousePosition(); }

#pragma endregion

#pragma region Entity

    uint32_t ScriptRegister::Entity_Instantiate(uint32_t prefab_handle)
    {
        return 0;
    }

    uint32_t ScriptRegister::Entity_Instantiate_Transform(uint32_t prefab_handle, const _Transform *transform)
    {
        return 0;
    }
    uint32_t ScriptRegister::Entity_Create(const char *name)
    {
        return 0;
    }
    void ScriptRegister::Entity_Destroy(GUID entity_guid)
    {
    }
    uint32_t ScriptRegister::Entity_FindByName(const char *name)
    {
        return 0;
    }
    uint32_t ScriptRegister::Entity_FindByGUID(GUID entity_guid)
    {
        return 0;
    }
    GUID ScriptRegister::Entity_GetParent(GUID entity_guid)
    {
        return GUID();
    }
    void ScriptRegister::Entity_SetParent(GUID entity_guid, GUID parent_guid)
    {
    }
    const char *ScriptRegister::Entity_GetName(GUID entity_guid)
    {
        return nullptr;
    }
    void ScriptRegister::Entity_SetName(GUID entity_guid, const char *value)
    {
    }
    bool ScriptRegister::Entity_CheckValidEntity(GUID entity_guid)
    {
        return false;
    }
    void ScriptRegister::Entity_AddComponent(GUID entity_guid, ScriptFieldType component_type)
    {
    }
    void ScriptRegister::Entity_RemoveComponent(GUID entity_guid, ScriptFieldType component_type)
    {
    }
    bool ScriptRegister::Entity_HasComponent(GUID entity_guid, ScriptFieldType component_type)
    {
        return false;
    }
    uint32_t ScriptRegister::Entity_GetComponentInParent(GUID entity_guid, ScriptFieldType component_type)
    {
        return 0;
    }
    uint32_t ScriptRegister::Entity_GetComponentInChildren(GUID entity_guid, ScriptFieldType component_type)
    {
        return 0;
    }
    uint64_t ScriptRegister::Entity_GetComponentsInParentCount(GUID entity_guid, ScriptFieldType component_type, size_t *count)
    {
        return 0;
    }
    uint64_t ScriptRegister::Entity_GetComponentsInChildrenCount(GUID entity_guid, ScriptFieldType component_type, size_t *count)
    {
        return 0;
    }
    void ScriptRegister::Entity_GetComponentsInHierarchyCopy(GUID *entity_array, size_t count, uint64_t cached_key)
    {
    }

#pragma endregion

#pragma region TransformComponent

    ScriptRegister::_Transform ScriptRegister::TransformComponent_GetTransform(GUID entity_guid, bool global)
    {
        return _Transform();
    }

    void ScriptRegister::TransformComponent_SetTransform(GUID entity_guid, bool global, const _Transform *value)
    {
    }

    glm::vec3 ScriptRegister::TransformComponent_GetPosition(GUID entity_guid, bool global)
    {
        return glm::vec3();
    }

    void ScriptRegister::TransformComponent_SetPosition(GUID entity_guid, bool global, const glm::vec3 *value)
    {
    }

    glm::quat ScriptRegister::TransformComponent_GetOrientation(GUID entity_guid, bool global)
    {
        return glm::quat();
    }

    void ScriptRegister::TransformComponent_SetOrientation(GUID entity_guid, bool global, const glm::quat *value)
    {
    }

    glm::vec3 ScriptRegister::TransformComponent_GetScale(GUID entity_guid, bool global)
    {
        return glm::vec3();
    }

    void ScriptRegister::TransformComponent_SetScale(GUID entity_guid, bool global, const glm::vec3 *value)
    {
    }

    glm::vec3 ScriptRegister::TransformComponent_GetForwardDirection(GUID entity_guid, bool global)
    {
        return glm::vec3();
    }

    void ScriptRegister::TransformComponent_SetForwardDirection(GUID entity_guid, bool global, const glm::vec3 *value)
    {
    }

    glm::vec3 ScriptRegister::TransformComponent_GetUpDirection(GUID entity_guid, bool global)
    {
        return glm::vec3();
    }

    void ScriptRegister::TransformComponent_SetUpDirection(GUID entity_guid, bool global, const glm::vec3 *value)
    {
    }

    glm::vec3 ScriptRegister::TransformComponent_GetRightDirection(GUID entity_guid, bool global)
    {
        return glm::vec3();
    }

    void ScriptRegister::TransformComponent_SetRightDirection(GUID entity_guid, bool global, const glm::vec3 *value)
    {
    }

    void ScriptRegister::TransformComponent_LookAt(GUID entity_guid, const glm::vec3 *value)
    {
    }

#pragma endregion

#pragma region Camera Compoennt

    uint8_t ScriptRegister::CameraComponent_GetClearFlag(GUID entity_guid)
    {
        return 0;
    }

    void ScriptRegister::CameraComponent_SetClearFlag(GUID entity_guid, uint8_t value)
    {
    }

    glm::vec4 ScriptRegister::CameraComponent_GetClearColour(GUID entity_guid)
    {
        return glm::vec4();
    }

    void ScriptRegister::CameraComponent_SetClearColour(GUID entity_guid, const glm::vec4 *value)
    {
    }

    AssetHandle ScriptRegister::CameraComponent_GetSkyboxHandle(GUID entity_guid)
    {
        return AssetHandle();
    }

    void ScriptRegister::CameraComponent_SetSkyboxHandle(GUID entity_guid, AssetHandle value)
    {
    }

    glm::vec4 ScriptRegister::CameraComponent_GetViewport(GUID entity_guid)
    {
        return glm::vec4();
    }

    void ScriptRegister::CameraComponent_SetViewport(GUID entity_guid, const glm::vec4 *value)
    {
    }

    bool ScriptRegister::CameraComponent_GetShouldDisplay(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::CameraComponent_SetShouldDisplay(GUID entity_guid, bool value)
    {
    }

    bool ScriptRegister::CameraComponent_GetForceRender(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::CameraComponent_SetForceRender(GUID entity_guid, bool value)
    {
    }

    uint8_t ScriptRegister::CameraComponent_GetCameraDepth(GUID entity_guid)
    {
        return 0;
    }

    void ScriptRegister::CameraComponent_SetCameraDepth(GUID entity_guid, uint8_t value)
    {
    }

    AssetHandle ScriptRegister::CameraComponent_GetRenderTargetHandle(GUID entity_guid)
    {
        return AssetHandle();
    }

#pragma endregion

#pragma region MeshRendererComponent

    bool ScriptRegister::MeshRendererComponent_GetIsActive(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::MeshRendererComponent_SetIsActive(GUID entity_guid, bool value)
    {
    }

    AssetHandle ScriptRegister::MeshRendererComponent_GetMeshAssetHandle(GUID entity_guid)
    {
        return AssetHandle();
    }

    void ScriptRegister::MeshRendererComponent_SetMeshAssetHandle(GUID entity_guid, AssetHandle asset_handle)
    {
    }

    AssetHandle ScriptRegister::MeshRendererComponent_CopyMesh(GUID entity_guid)
    {
        return AssetHandle();
    }

    Bounds_AABB ScriptRegister::MeshRendererComponent_GetMeshBounds(GUID entity_guid)
    {
        return Bounds_AABB();
    }

    bool ScriptRegister::MeshRendererComponent_GetIsCastingShadows(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::MeshRendererComponent_SetCastingShadows(GUID entity_guid, bool value)
    {
    }

    void ScriptRegister::MeshRendererComponent_GetAllMaterialsCount(GUID entity_guid, size_t *material_count)
    {
    }

    void ScriptRegister::MeshRendererComponent_GetAllMaterialsCopy(GUID entity_guid, AssetHandle *material_array, size_t material_count)
    {
    }

    void ScriptRegister::MeshRendererComponent_SetAllMaterials(GUID entity_guid, const AssetHandle *material_array, size_t material_count)
    {
    }

    AssetHandle ScriptRegister::MeshRendererComponent_GetMaterial(GUID entity_guid, size_t material_index)
    {
        return AssetHandle();
    }

    void ScriptRegister::MeshRendererComponent_SetMaterial(GUID entity_guid, size_t material_index, AssetHandle material_handle)
    {
    }

#pragma endregion

#pragma region LODMeshComponent

    bool ScriptRegister::LODMeshComponent_GetUseMaxDistanceOverFarPlane(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::LODMeshComponent_SetMaxDistanceOverFarPlane(GUID entity_guid, bool value)
    {
    }

    float ScriptRegister::LODMeshComponent_GetMaxDistance(GUID entity_guid)
    {
        return 0.0f;
    }

    void ScriptRegister::LODMeshComponent_SetMaxDistance(GUID entity_guid, float value)
    {
    }

    size_t ScriptRegister::LODMeshComponent_GetElementCount(GUID entity_guid)
    {
        return size_t();
    }

    void ScriptRegister::LODMeshComponent_GetDistances(GUID entity_guid, float *distance_array, size_t element_count)
    {
    }

    size_t ScriptRegister::LODMeshComponent_GetEntityCount(GUID entity_guid, size_t element_index)
    {
        return size_t();
    }

    void ScriptRegister::LODMeshComponent_GetEntityArray(GUID entity_guid, uint32_t *entity_array, size_t entity_count, size_t element_index)
    {
    }

    void ScriptRegister::LODMeshComponent_SetLODElements(GUID entity_guid, const float *distance_array, const size_t *entity_counts, const uint32_t *entity_array, size_t element_count)
    {
    }

#pragma endregion

#pragma region SkinnedMeshComponent

    bool ScriptRegister::SkinnedMeshComponent_GetIsActive(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::SkinnedMeshComponent_SetIsActive(GUID entity_guid, bool value)
    {
    }

    bool ScriptRegister::SkinnedMeshComponent_GetIsCastingShadows(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::SkinnedMeshComponent_SetCastingShadows(GUID entity_guid, bool value)
    {
    }

    AssetHandle ScriptRegister::SkinnedMeshComponent_GetMeshAssetHandle(GUID entity_guid)
    {
        return AssetHandle();
    }

    void ScriptRegister::SkinnedMeshComponent_SetMeshAssetHandle(GUID entity_guid, AssetHandle asset_handle)
    {
    }

    AssetHandle ScriptRegister::SkinnedMeshComponent_CopyMesh(GUID entity_guid)
    {
        return AssetHandle();
    }

    Bounds_AABB ScriptRegister::SkinnedMeshComponent_GetMeshBounds(GUID entity_guid)
    {
        return Bounds_AABB();
    }

    void ScriptRegister::SkinnedMeshComponent_GetAllMaterialsCount(GUID entity_guid, size_t *material_count)
    {
    }

    void ScriptRegister::SkinnedMeshComponent_GetAllMaterialsCopy(GUID entity_guid, uint32_t *material_array, size_t material_count)
    {
    }

    void ScriptRegister::SkinnedMeshComponent_SetAllMaterials(GUID entity_guid, const uint32_t *material_array, size_t material_count)
    {
    }

    uint32_t ScriptRegister::SkinnedMeshComponent_GetMaterial(GUID entity_guid, size_t material_index)
    {
        return 0;
    }

    void ScriptRegister::SkinnedMeshComponent_SetMaterial(GUID entity_guid, size_t material_index, GUID material_handle)
    {
    }

#pragma endregion

#pragma region SphereLightComponent

    bool ScriptRegister::SphereLightComponent_GetIsActive(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::SphereLightComponent_SetIsActive(GUID entity_guid, bool value)
    {
    }

    float ScriptRegister::SphereLightComponent_GetRadius(GUID entity_guid)
    {
        return 0.0f;
    }

    void ScriptRegister::SphereLightComponent_SetRadius(GUID entity_guid, float value)
    {
    }

    float ScriptRegister::SphereLightComponent_GetIntensity(GUID entity_guid)
    {
        return 0.0f;
    }

    void ScriptRegister::SphereLightComponent_SetIntensity(GUID entity_guid, float value)
    {
    }

    glm::vec4 ScriptRegister::SphereLightComponent_GetColour(GUID entity_guid)
    {
        return glm::vec4();
    }

    void ScriptRegister::SphereLightComponent_SetColour(GUID entity_guid, const glm::vec4 *value)
    {
    }

    uint8_t ScriptRegister::SphereLightComponent_GetShadowFlag(GUID entity_guid)
    {
        return 0;
    }

    void ScriptRegister::SphereLightComponent_SetShadowFlag(GUID entity_guid, uint8_t value)
    {
    }

#pragma endregion

#pragma region ConeLightComponent

    bool ScriptRegister::ConeLightComponent_GetIsActive(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::ConeLightComponent_SetIsActive(GUID entity_guid, bool value)
    {
    }

    glm::vec4 ScriptRegister::ConeLightComponent_GetColour(GUID entity_guid)
    {
        return glm::vec4();
    }

    void ScriptRegister::ConeLightComponent_SetColour(GUID entity_guid, const glm::vec4 *value)
    {
    }

    float ScriptRegister::ConeLightComponent_GetRange(GUID entity_guid)
    {
        return 0.0f;
    }

    void ScriptRegister::ConeLightComponent_SetRange(GUID entity_guid, float value)
    {
    }

    float ScriptRegister::ConeLightComponent_GetAngle(GUID entity_guid)
    {
        return 0.0f;
    }

    void ScriptRegister::ConeLightComponent_SetAngle(GUID entity_guid, float value)
    {
    }

    float ScriptRegister::ConeLightComponent_GetIntensity(GUID entity_guid)
    {
        return 0.0f;
    }

    void ScriptRegister::ConeLightComponent_SetIntensity(GUID entity_guid, float value)
    {
    }

    uint8_t ScriptRegister::ConeLightComponent_GetShadowFlag(GUID entity_guid)
    {
        return 0;
    }

    void ScriptRegister::ConeLightComponent_SetShadowFlag(GUID entity_guid, uint8_t value)
    {
    }

#pragma endregion

#pragma region DirectionalLightComponent

    bool ScriptRegister::DirectionalLightComponent_GetIsActive(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::DirectionalLightComponent_SetIsActive(GUID entity_guid, bool value)
    {
    }

    float ScriptRegister::DirectionalLightComponent_GetIntensity(GUID entity_guid)
    {
        return 0.0f;
    }

    void ScriptRegister::DirectionalLightComponent_SetIntensity(GUID entity_guid, float value)
    {
    }

    float ScriptRegister::DirectionalLightComponent_GetMaxShadowDistance(GUID entity_guid)
    {
        return 0.0f;
    }

    void ScriptRegister::DirectionalLightComponent_SetMaxShadowDistance(GUID entity_guid, float value)
    {
    }

    glm::vec4 ScriptRegister::DirectionalLightComponent_GetColour(GUID entity_guid)
    {
        return glm::vec4();
    }

    void ScriptRegister::DirectionalLightComponent_SetColour(GUID entity_guid, const glm::vec4 *value)
    {
    }

    uint8_t ScriptRegister::DirectionalLightComponent_GetShadowFlag(GUID entity_guid)
    {
        return 0;
    }

    void ScriptRegister::DirectionalLightComponent_SetShadowFlag(GUID entity_guid, uint8_t value)
    {
    }

#pragma endregion

#pragma region Physics

#pragma region RigidbodyComponent

    float ScriptRegister::RigidbodyComponent_GetMass(GUID entity_guid)
    {
        return 0.0f;
    }

    void ScriptRegister::RigidbodyComponent_SetMass(GUID entity_guid, float value)
    {
    }

    float ScriptRegister::RigidbodyComponent_GetDrag(GUID entity_guid)
    {
        return 0.0f;
    }

    void ScriptRegister::RigidbodyComponent_SetDrag(GUID entity_guid, float value)
    {
    }

    float ScriptRegister::RigidbodyComponent_GetAngularDrag(GUID entity_guid)
    {
        return 0.0f;
    }

    void ScriptRegister::RigidbodyComponent_SetAngularDrag(GUID entity_guid, float value)
    {
    }

    bool ScriptRegister::RigidbodyComponent_GetAutomaticCentreOfMass(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::RigidbodyComponent_SetAutomaticCentreOfMass(GUID entity_guid, bool value)
    {
    }

    bool ScriptRegister::RigidbodyComponent_GetUseGravity(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::RigidbodyComponent_SetUseGravity(GUID entity_guid, bool value)
    {
    }

    bool ScriptRegister::RigidbodyComponent_GetIsKinematic(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::RigidbodyComponent_SetIsKinematic(GUID entity_guid, bool value)
    {
    }

    glm::bvec3 ScriptRegister::RigidbodyComponent_GetPositionConstraint(GUID entity_guid)
    {
        return glm::bvec3();
    }

    void ScriptRegister::RigidbodyComponent_SetPositionConstraint(GUID entity_guid, const glm::bvec3 *value)
    {
    }

    glm::bvec3 ScriptRegister::RigidbodyComponent_GetRotationConstraint(GUID entity_guid)
    {
        return glm::bvec3();
    }

    void ScriptRegister::RigidbodyComponent_SetRotationConstraint(GUID entity_guid, const glm::bvec3 *value)
    {
    }

    glm::vec3 ScriptRegister::RigidbodyComponent_GetLinearVelocity(GUID entity_guid)
    {
        return glm::vec3();
    }

    void ScriptRegister::RigidbodyComponent_SetLinearVelocity(GUID entity_guid, const glm::vec3 *value)
    {
    }

    glm::vec3 ScriptRegister::RigidbodyComponent_GetAngularVelocity(GUID entity_guid)
    {
        return glm::vec3();
    }

    void ScriptRegister::RigidbodyComponent_SetAngularVelocity(GUID entity_guid, const glm::vec3 *value)
    {
    }

    void ScriptRegister::RigidbodyComponent_ApplyForce(GUID entity_guid, const glm::vec3 *force, uint8_t force_mode)
    {
    }

    void ScriptRegister::RigidbodyComponent_ApplyTorque(GUID entity_guid, const glm::vec3 *torque)
    {
    }

#pragma endregion

#pragma region BoxColliderComponent

    bool ScriptRegister::BoxColliderComponent_GetIsTrigger(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::BoxColliderComponent_SetIsTrigger(GUID entity_guid, bool value)
    {
    }

    glm::vec3 ScriptRegister::BoxColliderComponent_GetCentre(GUID entity_guid)
    {
        return glm::vec3();
    }

    void ScriptRegister::BoxColliderComponent_SetCentre(GUID entity_guid, const glm::vec3 *value)
    {
    }

    glm::vec3 ScriptRegister::BoxColliderComponent_GetSize(GUID entity_guid)
    {
        return glm::vec3();
    }

    void ScriptRegister::BoxColliderComponent_SetSize(GUID entity_guid, const glm::vec3 *value)
    {
    }

    ScriptRegister::_PhysicsMaterial ScriptRegister::BoxColliderComponent_GetMaterial(GUID entity_guid)
    {
        return _PhysicsMaterial();
    }

    void ScriptRegister::BoxColliderComponent_SetMaterial(GUID entity_guid, const _PhysicsMaterial *value)
    {
    }

#pragma endregion

#pragma region SphereColliderComponent

    bool ScriptRegister::SphereColliderComponent_GetIsTrigger(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::SphereColliderComponent_SetIsTrigger(GUID entity_guid, bool value)
    {
    }

    glm::vec3 ScriptRegister::SphereColliderComponent_GetCentre(GUID entity_guid)
    {
        return glm::vec3();
    }

    void ScriptRegister::SphereColliderComponent_SetCentre(GUID entity_guid, const glm::vec3 *value)
    {
    }

    float ScriptRegister::SphereColliderComponent_GetRadius(GUID entity_guid)
    {
        return 0.0f;
    }

    void ScriptRegister::SphereColliderComponent_SetRadius(GUID entity_guid, float value)
    {
    }

    ScriptRegister::_PhysicsMaterial ScriptRegister::SphereColliderComponent_GetMaterial(GUID entity_guid)
    {
        return _PhysicsMaterial();
    }

    void ScriptRegister::SphereColliderComponent_SetMaterial(GUID entity_guid, const _PhysicsMaterial *value)
    {
    }

#pragma endregion

#pragma region CapsuleColliderComponent

    bool ScriptRegister::CapsuleColliderComponent_GetIsTrigger(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::CapsuleColliderComponent_SetIsTrigger(GUID entity_guid, bool value)
    {
    }

    glm::vec3 ScriptRegister::CapsuleColliderComponent_GetCentre(GUID entity_guid)
    {
        return glm::vec3();
    }

    void ScriptRegister::CapsuleColliderComponent_SetCentre(GUID entity_guid, const glm::vec3 *value)
    {
    }

    float ScriptRegister::CapsuleColliderComponent_GetRadius(GUID entity_guid)
    {
        return 0.0f;
    }

    void ScriptRegister::CapsuleColliderComponent_SetRadius(GUID entity_guid, float value)
    {
    }

    float ScriptRegister::CapsuleColliderComponent_GetHalfHeight(GUID entity_guid)
    {
        return 0.0f;
    }

    void ScriptRegister::CapsuleColliderComponent_SetHalfHeight(GUID entity_guid, float value)
    {
    }

    ScriptRegister::_PhysicsMaterial ScriptRegister::CapsuleColliderComponent_GetMaterial(GUID entity_guid)
    {
        return _PhysicsMaterial();
    }

    void ScriptRegister::CapsuleColliderComponent_SetMaterial(GUID entity_guid, const _PhysicsMaterial *value)
    {
    }

#pragma endregion

#pragma region ConvexMeshColliderComponent

    bool ScriptRegister::ConvexMeshColliderComponent_GetIsTrigger(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::ConvexMeshColliderComponent_SetIsTrigger(GUID entity_guid, bool value)
    {
    }

    glm::vec3 ScriptRegister::ConvexMeshColliderComponent_GetCentre(GUID entity_guid)
    {
        return glm::vec3();
    }

    void ScriptRegister::ConvexMeshColliderComponent_SetCentre(GUID entity_guid, const glm::vec3 *value)
    {
    }

    ScriptRegister::_PhysicsMaterial ScriptRegister::ConvexMeshColliderComponent_GetMaterial(GUID entity_guid)
    {
        return _PhysicsMaterial();
    }

    void ScriptRegister::ConvexMeshColliderComponent_SetMaterial(GUID entity_guid, const _PhysicsMaterial *value)
    {
    }

#pragma endregion

#pragma region HeightFieldColliderComponent

    bool ScriptRegister::HeightFieldColliderComponent_GetIsTrigger(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::HeightFieldColliderComponent_SetIsTrigger(GUID entity_guid, bool value)
    {
    }

    glm::vec3 ScriptRegister::HeightFieldColliderComponent_GetCentre(GUID entity_guid)
    {
        return glm::vec3();
    }

    void ScriptRegister::HeightFieldColliderComponent_SetCentre(GUID entity_guid, const glm::vec3 *value)
    {
    }

    ScriptRegister::_PhysicsMaterial ScriptRegister::HeightFieldColliderComponent_GetMaterial(GUID entity_guid)
    {
        return _PhysicsMaterial();
    }

    void ScriptRegister::HeightFieldColliderComponent_SetMaterial(GUID entity_guid, const _PhysicsMaterial *value)
    {
    }

#pragma endregion

#pragma region TriangleMeshColliderComponent

    bool ScriptRegister::TriangleMeshColliderComponent_GetIsTrigger(GUID entity_guid)
    {
        return false;
    }

    void ScriptRegister::TriangleMeshColliderComponent_SetIsTrigger(GUID entity_guid, bool value)
    {
    }

    glm::vec3 ScriptRegister::TriangleMeshColliderComponent_GetCentre(GUID entity_guid)
    {
        return glm::vec3();
    }

    void ScriptRegister::TriangleMeshColliderComponent_SetCentre(GUID entity_guid, const glm::vec3 *value)
    {
    }

    ScriptRegister::_PhysicsMaterial ScriptRegister::TriangleMeshColliderComponent_GetMaterial(GUID entity_guid)
    {
        return _PhysicsMaterial();
    }

    void ScriptRegister::TriangleMeshColliderComponent_SetMaterial(GUID entity_guid, const _PhysicsMaterial *value)
    {
    }

#pragma endregion

#pragma endregion

}