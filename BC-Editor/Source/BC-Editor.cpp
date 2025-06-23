#include "BC_PCH.h"
#include "BC-Editor.h"

#include <imgui.h>

namespace BC
{
	constexpr float TOP_BAR_HEIGHT = 75.0f;
	static bool SHOW_DEMO_WINDOW = false;

    EditorLayer::EditorLayer() : m_Camera(1, 1)
    {
		m_Camera.OnUpdate();
    }

    EditorLayer::~EditorLayer()
    {
    }

	void EditorLayer::ResizeScreenSpace(uint32_t width, uint32_t height)
	{

	}
	
	void EditorLayer::ResizeSwapchain(const SwapchainSpecification& swapchain_spec)
	{

	}

	void EditorLayer::OnAttach()
	{
		m_Panels[PanelType_Inspector] 			= std::make_unique<InspectorPanel>(); 			m_Panels[PanelType_Inspector]->SetActive(true);
		m_Panels[PanelType_Hierarchy] 			= std::make_unique<HierarchyPanel>(); 			m_Panels[PanelType_Hierarchy]->SetActive(true);
		m_Panels[PanelType_ContentBrowser] 		= std::make_unique<ContentBrowserPanel>(); 		m_Panels[PanelType_ContentBrowser]->SetActive(true);
		m_Panels[PanelType_AssetRegistry] 		= std::make_unique<AssetRegistryPanel>(); 		m_Panels[PanelType_AssetRegistry]->SetActive(true);
		m_Panels[PanelType_SceneViewport] 		= std::make_unique<SceneViewportPanel>(); 		m_Panels[PanelType_SceneViewport]->SetActive(true);
		m_Panels[PanelType_Console] 			= std::make_unique<ConsolePanel>(); 			m_Panels[PanelType_Console]->SetActive(true);
		m_Panels[PanelType_Profiler] 			= std::make_unique<ProfilerPanel>(); 			m_Panels[PanelType_Profiler]->SetActive(true);
		m_Panels[PanelType_Statistics] 			= std::make_unique<StatisticsPanel>(); 			m_Panels[PanelType_Statistics]->SetActive(true);
		m_Panels[PanelType_HumanoidConfig] 		= std::make_unique<HumanoidConfigPanel>(); 		m_Panels[PanelType_HumanoidConfig]->SetActive(true);
		m_Panels[PanelType_AnimatorNodeGraph] 	= std::make_unique<AnimatorPanel>(); 			m_Panels[PanelType_AnimatorNodeGraph]->SetActive(true);
		m_Panels[PanelType_ProjectConfig] 		= std::make_unique<ProjectConfigPanel>(); 		m_Panels[PanelType_ProjectConfig]->SetActive(true);
		m_Panels[PanelType_SceneManagerConfig] 	= std::make_unique<SceneManagerConfigPanel>(); 	m_Panels[PanelType_SceneManagerConfig]->SetActive(true);
		m_Panels[PanelType_SceneConfig] 		= std::make_unique<SceneConfigPanel>(); 		m_Panels[PanelType_SceneConfig]->SetActive(true);

		for (auto& [type, panel] : m_Panels)
		{
			panel->SetEditorLayerReference(this);

			switch (type)
			{
				case PanelType_Inspector:
				case PanelType_Hierarchy:
				case PanelType_ContentBrowser:
				case PanelType_SceneViewport:
				case PanelType_Console:
				case PanelType_Profiler:
				case PanelType_Statistics:
				{
					panel->SetActive(true);
					break;
				}
				case PanelType_AssetRegistry:
				case PanelType_ProjectConfig:
				case PanelType_SceneManagerConfig:
				case PanelType_SceneConfig:
				case PanelType_HumanoidConfig:
				case PanelType_AnimatorNodeGraph:
				{
					panel->SetActive(false);
					break;
				}
			}
		}

		m_PlayButton = Texture2D::CreateTexture("Resources/Icons/PlayButton.png");
		m_PauseButton = Texture2D::CreateTexture("Resources/Icons/PauseButton.png");
		m_StopButton = Texture2D::CreateTexture("Resources/Icons/StopButton.png");
		m_SimButton = Texture2D::CreateTexture("Resources/Icons/SimulateButton.png");
		m_StepButton = Texture2D::CreateTexture("Resources/Icons/StepButton.png");
	}
	
	void EditorLayer::OnDetach()
	{
		m_PlayButton.reset();
		m_PauseButton.reset();
		m_StopButton.reset();
		m_SimButton.reset();
		m_StepButton.reset();

		m_Panels.clear();
	}

	void EditorLayer::OnUpdate()
	{
		for (auto& [type, panel] : m_Panels)
		{
			panel->OnUpdate();
		}
	}

	void EditorLayer::OnFixedUpdate()
	{

	}
	
	void EditorLayer::OnLateUpdate()
	{

	}

	void EditorLayer::OnRenderGUI()
	{ 
		if(SHOW_DEMO_WINDOW)
			ImGui::ShowDemoWindow(&SHOW_DEMO_WINDOW);

		DrawTopBar();
		DrawDockspace();
	}

    void EditorLayer::DrawTopBar()
    {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		constexpr ImGuiWindowFlags top_bar_window_flags = 
			ImGuiWindowFlags_MenuBar 				| 
			ImGuiWindowFlags_NoDocking 				| 
			ImGuiWindowFlags_NoTitleBar 			| 
			ImGuiWindowFlags_NoCollapse 			| 
			ImGuiWindowFlags_NoResize 				| 
			ImGuiWindowFlags_NoMove 				|
			ImGuiWindowFlags_NoBringToFrontOnFocus 	| 
			ImGuiWindowFlags_NoNavFocus				|
			ImGuiWindowFlags_NoScrollbar;	

		ImVec2 top_bar_size = ImVec2(viewport->WorkSize.x, TOP_BAR_HEIGHT);		
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(top_bar_size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		if (ImGui::Begin("##TopBar", nullptr, top_bar_window_flags))
		{
			ImGui::PopStyleVar(3);

			if (ImGui::BeginMenuBar()) 
			{
				if (ImGui::BeginMenu("File")) {

					if (ImGui::BeginMenu("New")) {

						if (ImGui::MenuItem("New Project")) 
						{
							// TODO: Implement
						}

						if (ImGui::MenuItem("New Scene", "Ctrl+N"))
						{
							// TODO: Implement
						}

						ImGui::EndMenu();
					}

					if (ImGui::BeginMenu("Open")) {

						if (ImGui::MenuItem("Open Project")) 
						{
							// TODO: Implement
						}

						if (ImGui::MenuItem("Open Scene", "Ctrl+O")) 
						{
							// TODO: Implement
						}

						ImGui::EndMenu();
					}

					if (ImGui::BeginMenu("Save")) {

						if (ImGui::MenuItem("Save Project")) 
						{
							// TODO: Implement
						}

						if (ImGui::MenuItem("Save Scene", "Ctrl+S")) 
						{
							// TODO: Implement
						}

						if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S"))
						{
							// TODO: Implement
						}

						ImGui::EndMenu();
					}

					ImGui::Separator();

					if (ImGui::MenuItem("Reload Shaders")) 
					{
						// TODO: Implement
					}

					if (ImGui::MenuItem("Reload Script Assembly")) 
					{
						// TODO: Implement
					}

					ImGui::Separator();

					ImGui::MenuItem("Show ImGui Demo", NULL, &SHOW_DEMO_WINDOW);

					ImGui::Separator();

					if (ImGui::MenuItem("Exit"))
						Application::Get()->Close();

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Edit")) 
				{

					if (ImGui::MenuItem("Project Config")) 
						GetPanel<ProjectConfigPanel>()->SetActive(true);

					if (ImGui::MenuItem("Scene Manager Config"))
						GetPanel<SceneManagerConfigPanel>()->SetActive(true);
					
					if (ImGui::MenuItem("Scene Properties"))
						GetPanel<SceneConfigPanel>()->SetActive(true);

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("View")) {

					if (ImGui::BeginMenu("Windows")) 
					{
						for (const auto& [type, panel] : m_Panels)
						{
							if (type == PanelType_Unknown)
								continue;

							if (ImGui::MenuItem(Util::PanelTypeToString(panel->GetType())))
							{
								panel->SetActive(true);
							}
						}
						
						ImGui::EndMenu();
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			ImVec2 button_size = { 32.0f, 32.0f };
			float spacing = 4.0f;
			int button_count = 3;
			float total_width = button_count * button_size.x + (button_count - 1) * spacing;

			ImVec2 content_region = ImGui::GetContentRegionAvail();
			ImVec2 cursor_screen_pos = ImGui::GetCursorScreenPos();

			float start_x = (content_region.x - total_width) * 0.5f;
			float start_y = (content_region.y - button_size.y) * 0.5f;

			ImGui::SetCursorScreenPos({ cursor_screen_pos.x + start_x, cursor_screen_pos.y + start_y });


			ImTextureID first_button_image = nullptr;
			ImTextureID second_button_image = nullptr;
			ImTextureID third_button_image = m_PauseButton->GetDescriptor();

			switch (m_SceneState)
			{
				case SceneState_Edit:
				{
					first_button_image 		= static_cast<ImTextureID>(m_PlayButton->GetDescriptor());
					second_button_image 	= static_cast<ImTextureID>(m_SimButton->GetDescriptor());
					break;
				}
				case SceneState_Play:
				{
					first_button_image 		= static_cast<ImTextureID>(m_StopButton->GetDescriptor());
					second_button_image 	= static_cast<ImTextureID>(m_StepButton->GetDescriptor());
					break;
				}
				case SceneState_Sim:
				{
					first_button_image 		= static_cast<ImTextureID>(m_StopButton->GetDescriptor());
					second_button_image 	= static_cast<ImTextureID>(m_StepButton->GetDescriptor());
					break;
				}
			}

			if (ImGui::ImageButton("##SceneControl_FirstButton", first_button_image, button_size))
			{
				switch (m_SceneState)
				{
					case SceneState_Edit:
					{
						m_SceneState = SceneState_Play;
						break;
					}
					case SceneState_Play:
					{
						m_SceneState = SceneState_Edit;
						break;
					}
					case SceneState_Sim:
					{
						m_SceneState = SceneState_Edit;
						break;
					}
				}
			}
			ImGui::SameLine(0.0f, spacing);

			if (ImGui::ImageButton("##SceneControl_SecondButton", second_button_image, button_size))
			{
				switch (m_SceneState)
				{
					case SceneState_Edit:
					{
						m_SceneState = SceneState_Sim;
						break;
					}
					case SceneState_Play:
					case SceneState_Sim:
					{
						// Step Button
						
						break;
					}
				}				
			}
			ImGui::SameLine(0.0f, spacing);

			bool is_paused = Application::GetProject()->GetSceneManager()->IsPaused();

			if (is_paused)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
			}

			if (ImGui::ImageButton("##SceneControl_ThirdButton", third_button_image, button_size))
			{
				switch (m_SceneState)
				{
					case SceneState_Edit: break;

					case SceneState_Play:
					case SceneState_Sim:
					{
						// Pause Button
						Application::GetProject()->GetSceneManager()->SetPaused(!is_paused);
						break;
					}
				}
			}
			ImGui::SameLine(0.0f, spacing);
			
			if (is_paused) 
			{
				ImGui::PopStyleColor(3);
			}
		}
		ImGui::End();
    }

    void EditorLayer::DrawDockspace()
    {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		
		ImGuiID dockspace_id = 0;
		constexpr ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
		constexpr ImGuiWindowFlags dockspace_window_flags = 
			ImGuiWindowFlags_NoDecoration			| 
			ImGuiWindowFlags_NoDocking 				| 
			ImGuiWindowFlags_NoTitleBar 			| 
			ImGuiWindowFlags_NoCollapse 			| 
			ImGuiWindowFlags_NoResize 				| 
			ImGuiWindowFlags_NoMove 				|
			ImGuiWindowFlags_NoBringToFrontOnFocus 	| 
			ImGuiWindowFlags_NoNavFocus 			|
			ImGuiWindowFlags_NoBackground;	

		ImVec2 dockspace_size = ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - TOP_BAR_HEIGHT);
		ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + TOP_BAR_HEIGHT));
		ImGui::SetNextWindowSize(dockspace_size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		if(ImGui::Begin("BC Editor", NULL, dockspace_window_flags))
		{
			ImGui::PopStyleVar(3);

			// Submit the DockSpace
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				dockspace_id = ImGui::GetID("BC Editor Dockspace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}

			for (auto& [type, panel] : m_Panels)
			{
				panel->OnRenderGUI();
			}
		}
		ImGui::End();
    }

    std::vector<CameraContext> EditorLayer::GetCameraOverrides()
	{
		std::vector<CameraContext> camera_overrides = {};

		/*
		// TODO: Implement Below
		if (m_State == EditorState::Editing || m_State == EditorState::Simulating)
		{
			camera_overrides.push_back({});
			auto& editor_ctx = camera_overrides.back();

			editor_ctx.transform_matrix = m_EditorCamera->GetTransformMatrix();
			editor_ctx.view_matrix = m_EditorCamera->GetViewMatrix();
			editor_ctx.projection_matrix = m_EditorCamera->GetProjectionMatrix();

			editor_ctx.near_clip = m_EditorCamera->GetNearClip();
			editor_ctx.far_clip = m_EditorCamera->GetFarClip();

			editor_ctx.owning_camera_entity = NULL_GUID;
			editor_ctx.is_editor_camera = true;

			editor_ctx.is_orthographic = m_EditorCamera->GetProjectionType() == EditorCamera::ProjectionType::Orthographic;
			if (editor_ctx.is_orthographic)
			{
				editor_ctx.orthographic_size = m_EditorCamera->GetOrthoSize();
			}
			else
			{
				editor_ctx.fov = m_EditorCamera->GetFOV();
				editor_ctx.aspect_ratio = m_EditorCamera->GetAspectRatio();
			}

			if (m_SelectionContext->GetType() == SelectionContextType::Entity && m_SelectionContext->GetAsEntity().HasComponent<CameraComponent>())
			{
				auto& selected_entity = m_SelectionContext->GetAsEntity();
				auto& camera_comp = selected_entity.GetComponent<CameraComponent>();
				auto& transform = selected_entity.GetComponent<TransformComponent>();
				auto camera_ref = camera_comp.GetCameraInstance();

				camera_overrides.push_back({});
				auto& preview_ctx = camera_overrides.back();

				preview_ctx.transform_matrix = transform.GetGlobalMatrix();
				preview_ctx.view_matrix = glm::inverse(preview_ctx.transform_matrix);
				preview_ctx.projection_matrix = camera_ref->GetProjectionMatrix();

				preview_ctx.near_clip = camera_ref->GetNearClip();
				preview_ctx.far_clip = camera_ref->GetFarClip();
				preview_ctx.owning_camera_entity = selected_entity.GetGUID();
				preview_ctx.is_editor_camera = false;

				preview_ctx.is_orthographic = camera_ref->GetProjectionType() == SceneCamera::ProjectionType::Orthographic;
				if (preview_ctx.is_orthographic)
					preview_ctx.orthographic_size = camera_ref->GetOrthoSize();
				else
				{
					preview_ctx.fov = camera_ref->GetFOV();
					preview_ctx.aspect_ratio = camera_ref->GetAspectRatio();
				}
			}
		}
		*/

		return camera_overrides;
	}

}