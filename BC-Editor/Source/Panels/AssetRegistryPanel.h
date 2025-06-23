#pragma once

#include "PanelBase.h"

namespace BC
{

    class AssetRegistryPanel : public IEditorPanel
    {

    public:

        AssetRegistryPanel() = default;
        ~AssetRegistryPanel() = default;

        PanelType GetType() const override { return PanelType_AssetRegistry; }

        void OnUpdate() override { }
        void OnRenderGUI() override
        {
            if (m_Active)
            {
                if (ImGui::Begin(Util::PanelTypeToString(GetType()), &m_Active))
                {

                }
                ImGui::End();
            }
        }

    private:

        friend class EditorLayer;

    };

}