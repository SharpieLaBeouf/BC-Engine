#pragma once

// Core Headers
#include "Core/GUID.h"

#include "Debug/Assert.h"

// C++ Standard Library Headers
#include <string>

// External Vendor Library Headers

namespace BC
{
    using ScriptFieldType = uint8_t;
    namespace Util::Scripting
    {

        enum : ScriptFieldType
        {
            // Unknown Field Type
            ScriptFieldType_Unknown,

            /// --------------------
            ///    Inbuilt Types
            /// --------------------
            ScriptFieldType_Float,
            ScriptFieldType_Double,
            ScriptFieldType_Int8,
            ScriptFieldType_Int16,
            ScriptFieldType_Int32,
            ScriptFieldType_Int64,
            ScriptFieldType_UInt8,
            ScriptFieldType_UInt16,
            ScriptFieldType_UInt32,
            ScriptFieldType_UInt64,
            ScriptFieldType_Bool,
            ScriptFieldType_CString,

            /// --------------------
            ///     Custom Types
            /// --------------------
            ScriptFieldType_Vector2,
            ScriptFieldType_Vector3,
            ScriptFieldType_Vector4,
            ScriptFieldType_UVector2,
            ScriptFieldType_UVector3,
            ScriptFieldType_UVector4,
            ScriptFieldType_IVector2,
            ScriptFieldType_IVector3,
            ScriptFieldType_IVector4,
            ScriptFieldType_DVector2,
            ScriptFieldType_DVector3,
            ScriptFieldType_DVector4,
            ScriptFieldType_BVector2,
            ScriptFieldType_BVector3,
            ScriptFieldType_BVector4,

            ScriptFieldType_Mat3,
            ScriptFieldType_Mat4,

            /// --------------------
            ///     ECS Types
            /// --------------------

            ScriptFieldType_Entity,
            
            // Default
            ScriptFieldType_TransformComponent,
            ScriptFieldType_MetaComponent,
            
            // Camera
            ScriptFieldType_CameraComponent,
            
            // Audio
            ScriptFieldType_AudioListenerComponent,
            ScriptFieldType_AudioEmitterComponent,

            // Meshes
            ScriptFieldType_LODMeshComponent,
            ScriptFieldType_MeshRendererComponent,
            ScriptFieldType_SkinnedMeshRendererComponent,

            // Animation
            ScriptFieldType_SimpleAnimationComponent,
            ScriptFieldType_AnimatorComponent,

            // Lighting
            ScriptFieldType_SphereLightComponent,
            ScriptFieldType_ConeLightComponent,
            ScriptFieldType_DirectionalLightComponent,

            // Physics
            ScriptFieldType_RigidbodyComponent,

            ScriptFieldType_PlaneCollider,
            ScriptFieldType_BoxColliderComponent,
            ScriptFieldType_SphereColliderComponent,
            ScriptFieldType_CapsuleColliderComponent,
            
            ScriptFieldType_ConvexMeshColliderComponent,
            ScriptFieldType_HeightFieldColliderComponent,
            ScriptFieldType_TriangleMeshColliderComponent,

            /// --------------------
            ///     Asset Types
            /// --------------------
            
            ScriptFieldType_Prefab,			// Actual prefabs or model import files, e.g., fbx, obj, etc.

            // Textures
            ScriptFieldType_Texture2D,
            ScriptFieldType_TextureCubeMap,
            ScriptFieldType_RenderTarget,

            // Mesh
            ScriptFieldType_Mesh,			// Mesh -> has sub meshes which make up entire mesh

            // Animation
            ScriptFieldType_Skeleton,
            ScriptFieldType_Humanoid,
            ScriptFieldType_HumanoidMask,
            ScriptFieldType_AnimationClip,
            ScriptFieldType_AnimationStateMachine,

            // Audio
            ScriptFieldType_Audio,

            // Materials
            ScriptFieldType_Material_Standard,
            ScriptFieldType_Material_Skybox,

            // Shaders
            ScriptFieldType_Compute_Shader,
            ScriptFieldType_Shader,
            
            ScriptFieldType_PhysicsMaterial
        };

        static size_t GetFieldSize(ScriptFieldType type)
        {
            switch (type)
            {
                case ScriptFieldType_Float:     return sizeof(float);
                case ScriptFieldType_Double:    return sizeof(double);
                case ScriptFieldType_Int8:      return sizeof(int8_t);
                case ScriptFieldType_Int16:     return sizeof(int16_t);
                case ScriptFieldType_Int32:     return sizeof(int32_t);
                case ScriptFieldType_Int64:     return sizeof(int64_t);
                case ScriptFieldType_UInt8:     return sizeof(uint8_t);
                case ScriptFieldType_UInt16:    return sizeof(uint16_t);
                case ScriptFieldType_UInt32:    return sizeof(uint32_t);
                case ScriptFieldType_UInt64:    return sizeof(uint64_t);
                case ScriptFieldType_Bool:      return sizeof(bool);
                case ScriptFieldType_CString:   return sizeof(const char*);

                    // Vectors
                case ScriptFieldType_Vector2:   return sizeof(float) * 2;
                case ScriptFieldType_Vector3:   return sizeof(float) * 3;
                case ScriptFieldType_Vector4:   return sizeof(float) * 4;
                case ScriptFieldType_UVector2:  return sizeof(uint32_t) * 2;
                case ScriptFieldType_UVector3:  return sizeof(uint32_t) * 3;
                case ScriptFieldType_UVector4:  return sizeof(uint32_t) * 4;
                case ScriptFieldType_IVector2:  return sizeof(int32_t) * 2;
                case ScriptFieldType_IVector3:  return sizeof(int32_t) * 3;
                case ScriptFieldType_IVector4:  return sizeof(int32_t) * 4;
                case ScriptFieldType_DVector2:  return sizeof(double) * 2;
                case ScriptFieldType_DVector3:  return sizeof(double) * 3;
                case ScriptFieldType_DVector4:  return sizeof(double) * 4;
                case ScriptFieldType_BVector2:  return sizeof(bool) * 2;
                case ScriptFieldType_BVector3:  return sizeof(bool) * 3;
                case ScriptFieldType_BVector4:  return sizeof(bool) * 4;

                case ScriptFieldType_Mat3:      return sizeof(float) * 9;
                case ScriptFieldType_Mat4:      return sizeof(float) * 16;

                    // ECS and Asset types are always the size of uint32_t
                case ScriptFieldType_Entity:
                case ScriptFieldType_TransformComponent:
                case ScriptFieldType_MetaComponent:
                case ScriptFieldType_CameraComponent:
                case ScriptFieldType_AudioListenerComponent:
                case ScriptFieldType_AudioEmitterComponent:

                case ScriptFieldType_LODMeshComponent:
                case ScriptFieldType_MeshRendererComponent:
                case ScriptFieldType_SkinnedMeshRendererComponent:

                case ScriptFieldType_SimpleAnimationComponent:
                case ScriptFieldType_AnimatorComponent:

                case ScriptFieldType_SphereLightComponent:
                case ScriptFieldType_ConeLightComponent:
                case ScriptFieldType_DirectionalLightComponent:

                case ScriptFieldType_RigidbodyComponent:

                case ScriptFieldType_PlaneCollider:
                case ScriptFieldType_BoxColliderComponent:
                case ScriptFieldType_SphereColliderComponent:
                case ScriptFieldType_CapsuleColliderComponent:
                case ScriptFieldType_ConvexMeshColliderComponent:
                case ScriptFieldType_HeightFieldColliderComponent:
                case ScriptFieldType_TriangleMeshColliderComponent:

                case ScriptFieldType_Prefab:

                case ScriptFieldType_Texture2D:
                case ScriptFieldType_TextureCubeMap:
                case ScriptFieldType_RenderTarget:

                case ScriptFieldType_Mesh:

                case ScriptFieldType_Skeleton:
                case ScriptFieldType_Humanoid:
                case ScriptFieldType_AnimationClip:
                case ScriptFieldType_AnimationStateMachine:

                case ScriptFieldType_Audio:

                case ScriptFieldType_Material_Standard:
                case ScriptFieldType_Material_Skybox:

                case ScriptFieldType_Compute_Shader:
                case ScriptFieldType_Shader:        

                case ScriptFieldType_PhysicsMaterial:                        return sizeof(uint32_t);

                default: return 0;
            }
        }

        /// @brief Helper Interface With IAsset as Defined in Native Script Core
        class IAsset
        {

        public:

            IAsset() = default;
            virtual ~IAsset() = default;

            virtual uint8_t GetAssetType() const { return 0; }
            virtual operator uint32_t() const { return m_AssetHandle; }

        protected:

            void SetAssetHandle(uint32_t asset_handle) { m_AssetHandle = asset_handle; }

            GUID m_AssetHandle = NULL_GUID;

            friend class ScriptManager;
        };

        /// @brief Helper Interface With IComponent as Defined in Native Script Core
        class IComponent
        {

        public:

            IComponent() : m_EntityID(NULL_GUID) { }
            static uint8_t GetType() { return 0; }

        protected:

            IComponent(uint32_t entity_uuid) : m_EntityID(entity_uuid) {}
            void SetEntity(uint32_t entity_uuid) { m_EntityID = entity_uuid; }

            GUID m_EntityID;

            friend class ScriptManager;
        };

        /// @brief Helper Interface With IEntity as Defined in Native Script Core
        class IEntity 
        {
        public:

            IEntity() = default;
            virtual ~IEntity() = default;

        protected: 
            GUID m_EntityID = NULL_GUID;

            friend class ScriptManager;
        };

        using IColliderType = uint8_t;
        enum  : IColliderType 
        {
            IColliderType_Unknown = 0,
            IColliderType_PlaneColliderComponent,
            IColliderType_BoxCollider,
            IColliderType_SphereCollider,
            IColliderType_CapsuleCollider,
            IColliderType_ConvexMeshCollider,
            IColliderType_HeightFieldCollider,
            IColliderType_TriangleMeshCollider
        };

        class ICollider
        {
            IColliderType type;
            IEntity other_entity_guid;
        };

        /// @brief Helper Interface With IScript as Defined in Native Script Core
        struct IScript : public IEntity
        {

    public:

            virtual void OnCreate() {}
            virtual void OnDestroy() {}

            virtual void OnUpdate() {}
            virtual void OnLateUpdate() {}
            virtual void OnFixedUpdate() {}

            virtual void OnCollideEnter(const ICollider& other)     {}
            virtual void OnCollideStay(const ICollider& other)      {}
            virtual void OnCollideLeave(const ICollider& other)     {}

            virtual void OnTriggerEnter(const ICollider& other)     {}
            virtual void OnTriggerStay(const ICollider& other)      {}
            virtual void OnTriggerLeave(const ICollider& other)     {}

            GUID GetEntityID() const { return m_EntityID; }

            friend class ScriptManager;

        };
    }
    
    /// @brief This is the reflected information of the script field
    struct ScriptFieldInfo
    {
        const char* name        = "";
        size_t offset           = 0;
        ScriptFieldType type    = 0;
    };

    /// @brief This is information of a ScriptClass contained in the script
    /// assembly including fields, create function and release function
    struct ScriptClassInfo
    {
        const char* name;                   /// @brief This is the name of the script class
        size_t field_count;                 /// @brief This is the count of fields contained in this class
        const ScriptFieldInfo* fields;      /// @brief This is a pointer to the fields array
        void*(*create_function)(uint32_t);  /// @brief This is the function pointer to the create function of the script. @return Returns the pointer to the ScriptClassInstance created from the assembly
        void (*release_function)(uint32_t); /// @brief This is the function pointer to the release function of the script
    };

    /// @brief This is an instance of a Script Field. E.g., when in the editor /
    /// serialising / deserialising a scene, this instance is used to set the
    /// correct values when initialised in the actual ScriptClassInstance
    struct ScriptFieldInstance
    {
        ScriptFieldInstance();
        ScriptFieldInstance(const ScriptFieldInfo& field_info);

        /// @brief This is to get the value of the Raw Buffer as type T
        template<typename T>
        T GetValue()
        {
            static_assert(std::is_trivially_copyable_v<T>, "ScriptFieldInstance::GetValue<T>: Only supports trivially copyable types.");
            static_assert(sizeof(T) <= sizeof(m_RawBuffer), "ScriptFieldInstance::GetValue<T>: T Size Invalid - Must Be 64 Bytes Max.");
            return *reinterpret_cast<const T*>(m_RawBuffer);
        }

        /// @brief This is to set the value of the Raw Buffer 
        template<typename T>
        void SetValue(T value)
        {
            static_assert(std::is_trivially_copyable_v<T>, "ScriptFieldInstance::SetValue<T>: Only supports trivially copyable types.");
            static_assert(sizeof(T) <= sizeof(m_RawBuffer), "ScriptFieldInstance::SetValue<T>: T Size Invalid - Must Be 64 Bytes Max.");
            memcpy(m_RawBuffer, &value, sizeof(T));
        }

        /// @brief Get Const Void Pointer to Raw Buffer 
        const void* GetRawBuffer() const
        {
            return m_RawBuffer;
        }

        /// @brief Update Raw Buffer directly with data
        void SetRawBuffer(const void* value, size_t size)
        {
            bool valid_size = size >= 0 && size <= sizeof(m_RawBuffer);
            BC_ASSERT(valid_size, "ScriptFieldInstance::SetRawBuffer: Invalid Size.");
            if (!valid_size)
            {
                BC_CORE_ERROR("ScriptFieldInstance::SetRawBuffer: Invalid Size.");
                return;
            }

            memcpy(m_RawBuffer, value, std::min(size, sizeof(m_RawBuffer)));
        }

        /// @brief If the field instance is of a string_type
        const char* GetCStringValue() const
        {
            return m_StringValue.c_str();
        }

        /// @brief If the script field type is a string, it will set the string value
        void SetCStringValue(const std::string& value)
        {
            m_StringValue = value;
        }

        /// @brief Get the string value stored 
        void SetScriptFieldInfo(const ScriptFieldInfo& field_info)
        {
            m_FieldInfo = field_info;
        }

        const ScriptFieldInfo& GetScriptFieldInfo() const
        {
            return m_FieldInfo;
        }

    private:
    
        /// @brief The info of the underlying ScriptField
        ScriptFieldInfo m_FieldInfo = {};

        /// @brief Maximum size of a Script Field data type is mat4 -> 4 x 4 x sizeof(float) = 64
        uint8_t m_RawBuffer[64] = {};

        /// @brief Special storage for if the field is a string type
        std::string m_StringValue = {};
    };

    /// @brief This is an instance of a Script created in the Assembly. Setting
    /// and getting of runtime values will be done through this.
    struct ScriptClassInstance
    {
    
    public:

        ScriptClassInstance() = default;
        ScriptClassInstance(std::shared_ptr<ScriptClassInfo> class_info, GUID entity_guid);
        ~ScriptClassInstance();

        ScriptClassInstance(const ScriptClassInstance&) = delete;                   // Cannot Copy Class Instance -> Can Only Create, Release, or Move Instance
        ScriptClassInstance(ScriptClassInstance&&) noexcept = default;
        ScriptClassInstance& operator=(const ScriptClassInstance&) = delete;        // Cannot Copy Class Instance -> Can Only Create, Release, or Move Instance
        ScriptClassInstance& operator=(ScriptClassInstance&&) noexcept = default;

        Util::Scripting::IScript* GetInstance() { return m_InstancePtr; }

        ScriptClassInfo* GetScriptClass() const { return m_ClassInfo.lock().get(); }

        template<typename T>
        bool GetFieldValue(const std::string& field_name, T& out)
        {
            if (auto ref = m_ClassInfo.lock())
            {
                for (int i = 0; i < ref->field_count; ++i)
                {
                    if (ref->fields[i].name == field_name)
                    {
                        std::memcpy(&out, (uint8_t*)m_InstancePtr + ref->fields[i].offset, sizeof(T));
                        return true;
                    }
                }
            }
            return false;
        }

        template<typename T>
        bool SetFieldValue(const std::string& field_name, const T& value, size_t additional_offset = 0)
        {
            if (auto ref = m_ClassInfo.lock())
            {
                for (int i = 0; i < ref->field_count; ++i)
                {
                    if (ref->fields[i].name == field_name)
                    {
                        std::memcpy((uint8_t*)m_InstancePtr + ref->fields[i].offset + additional_offset, &value, sizeof(T));
                        return true;
                    }
                }
            }
            return false;
        }

    private:

        bool SetFieldValueRaw(const std::string& field_name, const void* data, size_t size);

        GUID m_EntityID = NULL_GUID;                // @brief Entity GUID this Script is For
        Util::Scripting::IScript* m_InstancePtr;    // @brief Pointer to Instance created by ScriptClassInfo::create_function in Assembly
        std::weak_ptr<ScriptClassInfo> m_ClassInfo; // @brief Class Info When Loading Assembly -> Does Not Own Class Info, Owned by ScriptManager

    };

}