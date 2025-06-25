#pragma once
#include "BC.h"

#include "PanelBase.h"

namespace BC
{

    class ContentBrowserPanel : public IEditorPanel
    {

    public:

        ContentBrowserPanel()
        {
		    m_FileIcon = Texture2D::CreateTexture("Resources/Icons/FileIcon.png");
		    m_FolderIcon = Texture2D::CreateTexture("Resources/Icons/FolderIcon.png");
            SetCurrentDirectory(Application::GetProject()->GetDirectory() / "Assets");
        }
        
        ~ContentBrowserPanel() = default;

        PanelType GetType() const override { return PanelType_ContentBrowser; }

        void OnUpdate() override { }
        void OnRenderGUI() override
        {
            if (m_Active)
            {
                if (ImGui::Begin(Util::PanelTypeToString(GetType()), &m_Active))
                {
	                if (ImGui::BeginTable("ContentBrowser", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings)) 
                    {
                        { // Setup

                            // Setup the first column and set its initial width
                            ImGui::TableSetupColumn("SidePanel", ImGuiTableColumnFlags_WidthFixed, 200.0f);
                            ImGui::TableSetupColumn("NavigationAndBrowser", ImGuiTableColumnFlags_WidthStretch);

                            // Row for first column content
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0); // Set to first column
                        }

                        DrawSidePanel();

                        ImGui::TableNextColumn();
                        ImGui::TableSetColumnIndex(1); // Set to second column

                        DrawTopNavigation();
                        DrawContentBrowser();
                    }
                    ImGui::EndTable();
                }
                ImGui::End();
            }
        }

        const std::filesystem::path& GetCurrentDirectory() const { return m_Selection.GetSelection<std::filesystem::path>(); }
        void SetCurrentDirectory(const std::filesystem::path& path) { m_Selection.SetSelection(path); }

    private:

        void DrawSidePanel();
        void DrawTopNavigation();
        void DrawContentBrowser();

        void DirectoryFolderDropSource(const std::filesystem::path& file_path);
        bool DirectoryFolderDropTarget(const std::filesystem::path& file_path);

    private:

        std::shared_ptr<Texture2D> m_FileIcon = nullptr;
        std::shared_ptr<Texture2D> m_FolderIcon = nullptr;

        friend class EditorLayer;

    };

}