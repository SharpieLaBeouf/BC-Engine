#include "BC_PCH.h"
#include "ScriptDefines.h"

namespace BC
{

    ScriptFieldInstance::ScriptFieldInstance()
    {
        memset(m_RawBuffer, 0, sizeof(m_RawBuffer));
    }

    ScriptFieldInstance::ScriptFieldInstance(const ScriptFieldInfo &field_info)
    {
        memset(m_RawBuffer, 0, sizeof(m_RawBuffer));
    }

    ScriptClassInstance::ScriptClassInstance(std::shared_ptr<ScriptClassInfo> class_info, GUID entity_guid) :
        m_ClassInfo(class_info),
        m_EntityID(entity_guid)
    {
        m_InstancePtr = (Util::Scripting::IScript*)class_info->create_function(m_EntityID);
    }

    ScriptClassInstance::~ScriptClassInstance()
    {
        if(auto class_info = m_ClassInfo.lock(); class_info)
            class_info->release_function(m_EntityID);

        m_InstancePtr = nullptr;
    }

    bool ScriptClassInstance::SetFieldValueRaw(const std::string &field_name, const void *data, size_t size)
    {
        if (auto class_info = m_ClassInfo.lock(); class_info)
        {
            for (int i = 0; i < class_info->field_count; ++i)
            {
                if (class_info->fields[i].name == field_name)
                {
                    std::memcpy((uint8_t*)m_InstancePtr + class_info->fields[i].offset, data, size);
                    return true;
                }
            }
        }
        return false;
    }
}