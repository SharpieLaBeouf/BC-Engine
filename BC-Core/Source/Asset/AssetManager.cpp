#include "BC_PCH.h"
#include "AssetManager.h"

namespace BC
{

#pragma region Base Asset Manager

		const AssetMetaData& AssetManagerBase::GetMetaData(AssetHandle handle)
        {
            if (!m_MetaDataRegistry.contains(handle))
            {
                static AssetMetaData s_NullMetaData = {};
                s_NullMetaData.handle = PLACEHOLDER_0_GUID;
                return s_NullMetaData;
            }
            return m_MetaDataRegistry.at(handle);
        }

        const AssetMetaData& AssetManagerBase::GetMetaData(const std::filesystem::path& asset_path)
        {
            for (const auto& [handle, meta_data] : m_MetaDataRegistry)
            {
                if (meta_data.asset_path == asset_path)
                    return meta_data;
            }
            
            static AssetMetaData s_NullMetaData = {};
            s_NullMetaData.handle = PLACEHOLDER_0_GUID;
            return s_NullMetaData;
        }

#pragma endregion

#pragma region Editor Asset Manager



#pragma endregion

#pragma region Runtime Asset Manager



#pragma endregion

}