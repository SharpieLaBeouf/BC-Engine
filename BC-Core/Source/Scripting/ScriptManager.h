#pragma once

// Core Headers
#include "ScriptDefines.h"

#include "Util/Hash.h"
#include "Util/Platform.h"

// C++ Standard Library Headers
#include <memory>
#include <string>
#include <format>
#include <iostream>
#include <filesystem>
#include <unordered_map>

#if defined(BC_PLATFORM_WINDOWS)
#include <Windows.h>
#elif defined(BC_PLATFORM_LINUX)
#include <dlfcn.h>
#endif

// External Vendor Library Headers

namespace BC
{
    class ScriptManager
    {

    public:

        using ScriptClassInfoMap        = std::unordered_map<std::string, std::shared_ptr<ScriptClassInfo>>;
        using ScriptClassInstanceMap    = std::unordered_map<StringHash, std::unique_ptr<ScriptClassInstance>>;

        using ScriptFieldMap            = std::unordered_map<std::string, ScriptFieldInstance>;
        using ScriptFieldInstanceMap    = std::unordered_map<StringHash, ScriptFieldMap>;

    public:

        ScriptManager() = default;
        ScriptManager(const std::filesystem::path& assembly_path)
        {
            if (!std::filesystem::exists(assembly_path))
                return;
            
            ReloadAssembly(assembly_path);
        }

        ~ScriptManager()
        {
            FreeAssembly();
        }

        /// ---- Getters ----

        StringHash GetHash(const std::string& script_name, GUID entity_guid, size_t script_index);
        
        std::shared_ptr<ScriptClassInfo> GetScriptClassInfo(const std::string& name);
        const ScriptClassInfoMap& GetScriptClassInfoMap() const { return m_ScriptClassInfoMap; }
        bool ScriptClassInfoExists(const std::string& script_name) const { return m_ScriptClassInfoMap.contains(script_name); }

        ScriptClassInstance* GetScriptClassInstance(const std::string& script_name, GUID entity_guid, size_t script_index);
        const ScriptClassInstanceMap& GetScriptClassInstanceMap() const { return m_ScriptInstanceMap; }
        
        ScriptFieldInstance* GetScriptFieldInstance(const std::string& script_name, GUID entity_guid, size_t script_index, const std::string& field_name);
        ScriptFieldMap* GetScriptFieldInstanceMap(const std::string& script_name, GUID entity_guid, size_t script_index);
        const ScriptFieldInstanceMap& GetAllScriptFieldInstanceMap() const { return m_ScriptFieldInstanceMap; }

        // ---- Cleanup ----
        void RemoveScriptInstance(const std::string& script_name, GUID entity_guid, size_t script_index);
        void RemoveAllScriptInstances() { m_ScriptInstanceMap.clear(); }

        void RemoveScriptFieldInstance(const std::string& script_name, GUID entity_guid, size_t script_index);
        void RemoveAllScriptFieldInstances() { m_ScriptFieldInstanceMap.clear(); }

        // ---- Assembly ----
        void FreeAssembly();
        bool ReloadAssembly(const std::filesystem::path& assembly_path);

    private:

        bool LoadAssembly(const std::filesystem::path& assembly_path);
        bool CreateScriptInstance(const std::string& script_name, GUID entity_guid, size_t script_index);

    private:

        /// @brief Holds information on script classes, and how they can be created and released
        ScriptClassInfoMap m_ScriptClassInfoMap = {};

        /// @brief Holds instances of scripts that have been instantiated
        ScriptClassInstanceMap m_ScriptInstanceMap = {};

        /// @brief Holds isntances of fields that have been modified via editor / serialisation / deserialisation
        ScriptFieldInstanceMap m_ScriptFieldInstanceMap = {};
        
    #if defined(BC_PLATFORM_WINDOWS)
        /// @brief Script Code Assembly
        HMODULE m_Assembly = nullptr;
    #else
        /// @brief Script Code Assembly
        void* m_Assembly = nullptr;
    #endif

    };
}