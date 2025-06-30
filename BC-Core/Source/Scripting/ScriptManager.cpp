#include "BC_PCH.h"
#include "ScriptManager.h"

#include "ScriptRegister.h"

namespace BC
{

#pragma region General Methods

    StringHash ScriptManager::GetHash(const std::string &script_name, GUID entity_guid, size_t script_index)
    {
        StringHash hash = Util::HashStringInsensitive(script_name.c_str());
        hash = Util::HashCombine(hash, static_cast<StringHash>(entity_guid));
        hash = Util::HashCombine(hash, static_cast<StringHash>(script_index));
        return hash;
    }
    
    std::shared_ptr<ScriptClassInfo> ScriptManager::GetScriptClassInfo(const std::string &name)
    {
        if (!m_Assembly)
        {
            BC_CORE_ERROR("ScriptManager::GetScriptClassInfo: Script Assembly Not Loaded.");
            return nullptr;
        }

        auto it = m_ScriptClassInfoMap.find(name);
        if (it != m_ScriptClassInfoMap.end())
            return it->second;

        return nullptr;
    }

    ScriptClassInstance* ScriptManager::GetScriptClassInstance(const std::string &script_name, GUID entity_guid, size_t script_index)
    {
        if (!m_Assembly)
        {
            BC_CORE_ERROR("ScriptManager::GetScriptClassInstance: Script Assembly Not Loaded.");
            return nullptr;
        }

        auto it = m_ScriptInstanceMap.find(GetHash(script_name, entity_guid, script_index));
        if (it != m_ScriptInstanceMap.end())
            return it->second.get();
            
        return nullptr;
    }

    ScriptManager::ScriptFieldMap* ScriptManager::GetScriptFieldInstanceMap(const std::string &script_name, GUID entity_guid, size_t script_index)
    {
        if (!m_Assembly)
        {
            BC_CORE_ERROR("ScriptManager::GetScriptFieldInstanceMap: Script Assembly Not Loaded.");
            return nullptr;
        }

        StringHash key = GetHash(script_name, entity_guid, script_index);
        if (m_ScriptFieldInstanceMap.find(key) == m_ScriptFieldInstanceMap.end())
        {
            m_ScriptFieldInstanceMap[key] = {}; // Create Script Field Map
        }

        return &m_ScriptFieldInstanceMap[key];
    }

    void ScriptManager::RemoveScriptInstance(const std::string &script_name, GUID entity_guid, size_t script_index)
    {
        StringHash key = GetHash(script_name, entity_guid, script_index);
        auto it = m_ScriptInstanceMap.find(key);
        if (it != m_ScriptInstanceMap.end())
            m_ScriptInstanceMap.erase(it);
    }

    void ScriptManager::RemoveScriptFieldInstance(const std::string &script_name, GUID entity_guid, size_t script_index)
    {
        StringHash key = GetHash(script_name, entity_guid, script_index);
        auto it = m_ScriptFieldInstanceMap.find(key);
        if (it != m_ScriptFieldInstanceMap.end())
            m_ScriptFieldInstanceMap.erase(it);
    }

    ScriptFieldInstance *ScriptManager::GetScriptFieldInstance(const std::string &script_name, GUID entity_guid, size_t script_index, const std::string &field_name)
    {
        // 1. Validate Assembly Loaded
        if (!m_Assembly)
        {
            BC_CORE_ERROR("ScriptManager::GetScriptFieldInstance: Script Assembly Not Loaded.");
            return nullptr;
        }
        
        // 2. Validate Script Class Exists
        auto class_info = GetScriptClassInfo(script_name);
        if (!class_info)
        {
            BC_CORE_ERROR("ScriptManager::GetScriptFieldInstance: Script Class Does Not Exist");
            return nullptr;
        }

        // 2. Validate Field Exists
        int field_index = -1;
        for (int i = 0; i < class_info->field_count; ++i)
        {
            if (class_info->fields[i].name == field_name)
            {
                field_index = i;
                break;
            }
        }

        if (field_index == -1)
        {
            BC_CORE_ERROR("ScriptManager::GetScriptFieldInstance: Script Field Does Not Exist in Class");
            return nullptr;
        }

        // 3. Validate Script Field Instance Map
        StringHash key = GetHash(script_name, entity_guid, script_index);
        if (m_ScriptFieldInstanceMap.find(key) == m_ScriptFieldInstanceMap.end())
        {
            m_ScriptFieldInstanceMap[key] = {}; // Create Script Field Map
            m_ScriptFieldInstanceMap[key][field_name] = ScriptFieldInstance(class_info->fields[field_index]); // Insert Field into Field Map
        }

        // 4. Return Pointer to ScriptFieldIsntance
        return &m_ScriptFieldInstanceMap[key][field_name];
    }

#pragma endregion

#pragma region Create Instance

    bool ScriptManager::CreateScriptInstance(const std::string& script_name, GUID entity_guid, size_t script_index)
    {
        StringHash key = GetHash(script_name, entity_guid, script_index);
        if (m_ScriptInstanceMap.find(key) != m_ScriptInstanceMap.end())
        {
            BC_CORE_ERROR
            (
                "ScriptManager::CreateScriptInstance: Script Instance Already Exists: Entity - {}, Script Name: {}, Script Index: {}.",
                std::to_string(entity_guid),
                script_name,
                std::to_string(script_index)
            );
            return false;
        }

        auto class_info = GetScriptClassInfo(script_name);
        if (!class_info)
        {
            BC_CORE_ERROR("ScriptManager::CreateScriptInstance: Script Class Does Not Exist - '{}'.", script_name);
            return false;
        }

        m_ScriptInstanceMap[key] = std::make_unique<ScriptClassInstance>(class_info, entity_guid);

        if(m_ScriptFieldInstanceMap.find(key) == m_ScriptFieldInstanceMap.end())
            m_ScriptFieldInstanceMap[key] = {};

        BC_ASSERT(m_ScriptInstanceMap[key] && m_ScriptInstanceMap[key]->GetInstance(), "ScriptManager::CreateScriptInstance: Could Not Create Instance of Script Class");
        if(!m_ScriptInstanceMap[key] || !m_ScriptInstanceMap[key]->GetInstance())
        {
            BC_CORE_ERROR("ScriptManager::CreateScriptInstance: Could Not Create Instance of Script Class - '{}'.", script_name);
            return false;
        }

        return true;
    }

#pragma endregion

#pragma region Assembly Load/Unload

    void ScriptManager::FreeAssembly()
    {
        m_ScriptInstanceMap.clear();
        m_ScriptClassInfoMap.clear();

        if (!m_Assembly) 
        {
            return;
        }

    #if defined(BC_PLATFORM_WINDOWS)

        FreeLibrary(m_Assembly);

    #elif defined(BC_PLATFORM_LINUX)
    
        dlclose(m_Assembly);

    #endif

        m_Assembly = nullptr;
    }

    bool ScriptManager::ReloadAssembly(const std::filesystem::path &assembly_path)
    {
        FreeAssembly();
        return LoadAssembly(assembly_path);
    }

    bool ScriptManager::LoadAssembly(const std::filesystem::path &assembly_path)
    {
        // 1. Load Library
    #if defined(BC_PLATFORM_WINDOWS)

        m_Assembly = LoadLibraryA(assembly_path.string().c_str());

    #elif defined(BC_PLATFORM_LINUX)

        m_Assembly = dlopen(assembly_path.string().c_str(), RTLD_NOW);

    #endif

        // 2. Validate Success
        if (!m_Assembly)
        {
            BC_CORE_ERROR("ScriptManager::LoadAssembly: Failed to Load Assembly: {}", assembly_path.string());
            return false;
        }

    #if defined(BC_PLATFORM_WINDOWS)

        auto load_script_func = (void(*)())GetProcAddress(m_Assembly, "LoadScripts");
        auto get_script_info_func = (const ScriptClassInfo*(*)(size_t*))GetProcAddress(m_Assembly, "GetScriptClassInfoArray");

    #elif defined(BC_PLATFORM_LINUX)

        auto load_script_func = reinterpret_cast<void(*)()>(dlsym(m_Assembly, "LoadScripts"));
        auto get_script_info_func = reinterpret_cast<const ScriptClass*(*)(size_t*)>(dlsym(m_Assembly, "GetScriptClassInfoArray"));
    
    #endif

        if (!ScriptRegister::RegisterAll(m_Assembly))
            return false;

        if (!load_script_func || !get_script_info_func)
        {
            BC_CORE_ERROR("ScriptManager::LoadAssembly: Missing Expected Exports in Assembly");
            return false;
        }

        // Load Script Reflection Data
        load_script_func();

        // Retrieve Scripts and Field Information
        size_t scripts_count = 0;
        const ScriptClassInfo* script_classes = get_script_info_func(&scripts_count);
        for (size_t i = 0; i < scripts_count; ++i)
        {
            std::shared_ptr<ScriptClassInfo> script_class = std::make_shared<ScriptClassInfo>(script_classes[i]);
            m_ScriptClassInfoMap[script_classes[i].name] = script_class;

            // Initialise w/ Default Values - Create Default Instance, and Retrieve Default Values
            auto temp_instance = std::make_unique<ScriptClassInstance>(script_class, PLACEHOLDER_0_GUID);

            uint8_t buffer[64] = {}; // Max Field Size 64 BYTES -> Mat4
            auto& script_field_map = m_ScriptFieldInstanceMap[GetHash(script_class->name, PLACEHOLDER_0_GUID, 0)];
            for (int j = 0; j < script_class->field_count; ++j)
            {
                const ScriptFieldInfo& script_field = script_class->fields[j];
                ScriptFieldInstance& script_field_instance = script_field_map[script_field.name];
                script_field_instance.SetScriptFieldInfo(script_field);

                if (script_field_instance.GetScriptFieldInfo().type == Util::Scripting::ScriptFieldType_CString)
                {
                    const char* string = "";
                    temp_instance->GetFieldValue(script_field.name, string);
                    script_field_instance.SetCStringValue(string);
                }
                else
                {
                    memset(buffer, 0, sizeof(buffer)); // Zero out scratch buf
                    temp_instance->GetFieldValue(script_field.name, buffer);
                    script_field_instance.SetRawBuffer(buffer, Util::Scripting::GetFieldSize(script_field.type));
                }
            }

            // Free Script Instance
            temp_instance.reset();
        }

        return true;
    }

#pragma endregion

}