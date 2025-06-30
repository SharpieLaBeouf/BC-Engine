#include "BC_PCH.h"
#include "ContentBrowserPanel.h"


namespace BC
{
    static const std::unordered_set<std::string> s_SupportedOpenInEditorFiles = {
        ".scene",
        ".animator",
        ".humanoid",
        ".humanoidmask"
    };

    void ContentBrowserPanel::DirectoryFolderDropSource(const std::filesystem::path &file_path)
    {
        if (ImGui::BeginDragDropSource())
        {
            std::filesystem::path path = file_path;
            std::string path_str = path.string(); // Convert to string
            ImGui::SetDragDropPayload("CONTENT_ITEM_FOLDER", path_str.c_str(), path_str.size() + 1); // Null-terminate string

            ImGui::EndDragDropSource();
        }
    }

    bool ContentBrowserPanel::DirectoryFolderDropTarget(const std::filesystem::path& file_path)
    {
        if (ImGui::BeginDragDropTarget()) {

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_ITEM_FILE"))
            {
                // Convert the payload data (string) back into a filesystem path
                std::string dropped_path_str(static_cast<const char*>(payload->Data), payload->DataSize);
                std::filesystem::path dropped_path = dropped_path_str; // Convert to path

                try 
                {
                    // Combine dropped_path with entry.path() to get the target path
                    std::filesystem::path target_path = file_path / dropped_path.filename();

                    // Move the folder to the new target folder
                    std::filesystem::rename(dropped_path, target_path); // Moves the folder

                    return false; // Wasn't a folder
                }
                catch (const std::filesystem::filesystem_error& e) 
                {
                    BC_APP_WARN("Could Not Drop {} Folder into {} Folder: {}", dropped_path.string().c_str(), file_path.string().c_str(), e.what());
                }
            }

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_ITEM_FOLDER"))
            {
                // Convert the payload data (string) back into a filesystem path
                std::string dropped_path_str(static_cast<const char*>(payload->Data), payload->DataSize);
                std::filesystem::path dropped_path = dropped_path_str; // Convert to path

                try {
                    // Combine dropped_path with entry.path() to get the target path
                    std::filesystem::path target_path = file_path / dropped_path.filename();

                    // Move the folder to the new target folder
                    std::filesystem::rename(dropped_path, target_path); // Moves the folder
                    
                    return true;

                }
                catch (const std::filesystem::filesystem_error& e) 
                {
                    BC_APP_WARN("Could Not Drop {} Folder into {} Folder: {}", dropped_path.string().c_str(), file_path.string().c_str(), e.what());
                }
            }

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_HANDLE"))
            {
                GUID dropped_entity_guid = *(const GUID*)payload->Data;
                Entity dropped_entity = Application::GetProject()->GetSceneManager()->GetEntity(dropped_entity_guid);

                if (dropped_entity) 
                {
                    // TODO: Create Prefab at current directory
                }
            }

            ImGui::EndDragDropTarget();
        }

        return false;
    }

    void ContentBrowserPanel::DrawSidePanel()
    {
		// ------------- Directory Browser Side Bar -------------
		if (ImGui::BeginChild("SidePanelChild", {}, ImGuiChildFlags_Border | ImGuiChildFlags_FrameStyle))
		{
			std::function<bool(const std::filesystem::path&)> recursive_directory_display;

			recursive_directory_display = [&](const std::filesystem::path& current_directory_path) -> bool 
            {
				for (const auto& entry : std::filesystem::directory_iterator(current_directory_path)) 
                {
					std::filesystem::path entry_path = entry.path();

					if (Util::FilePathContains(entry_path, "Scripts/Bin") 			 ||
						Util::FilePathContains(entry_path, "Scripts/Generated") 	 ||
						Util::FilePathContains(entry_path, "Scripts/ScriptCoreAPI")	)
						continue;

					if (entry.is_directory() && !Util::IsPathHidden(entry.path())) 
                    {
						bool is_leaf_node = !Util::PathHasSubDirectory(entry_path);

						ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick;
						flags |= entry_path == GetCurrentDirectory() ? ImGuiTreeNodeFlags_Selected : 0;
						flags |= is_leaf_node ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_OpenOnArrow;
						
						std::string label = entry_path.filename().string();
						std::string id = entry_path.string(); // Guaranteed unique
						ImGui::PushID(id.c_str());
						bool tree_node_opened = ImGui::TreeNodeEx(label.c_str(), flags);
						ImGui::PopID();

						DirectoryFolderDropSource(entry_path);
						if (DirectoryFolderDropTarget(entry_path))
						{
							SetCurrentDirectory(entry_path);
							ImGui::TreePop();
							return false;
						}
						
						// This path tree node has been clicked
						if (ImGui::IsItemClicked()) 
                        {
							SetCurrentDirectory(entry_path);
						}

						if (tree_node_opened) 
                        {
							if (!recursive_directory_display(entry_path))
							{
								ImGui::TreePop();
								return false;
							}
							ImGui::TreePop();
						}
					}
				}
				return true;
			};

			// --------------- SIDE BAR ROOT FOLDERS ---------------
			ImGui::SeparatorText(Application::GetProject()->GetName().c_str()); 
			for (int i = 0; i < 3; i++) 
            {
				std::filesystem::path side_bar_root_path;
				
				switch (i) 
                {
					case 0: side_bar_root_path = Application::GetProject()->GetDirectory() / "Assets"; break;	// --------------- ASSETS ---------------
					case 1: side_bar_root_path = Application::GetProject()->GetDirectory() / "Scenes"; break;	// --------------- SCENES ---------------
					case 2: side_bar_root_path = Application::GetProject()->GetDirectory() / "Scripts"; break;	// --------------- SCRIPTS ---------------
				}

				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick;
				flags |= GetCurrentDirectory() == side_bar_root_path ? ImGuiTreeNodeFlags_Selected : 0;

				if (i != 2)
				{
					flags |= Util::PathHasSubDirectory(side_bar_root_path) ? ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_Leaf;
				}
				else
				{
					// Ensure Correct Tree Node Flags Set - this ensures that
					// any folders that are deemed "excluded" are not included
					// in the side bar
					bool only_excluded = true;
					for (const auto& entry : std::filesystem::directory_iterator(side_bar_root_path))
					{
						if (!entry.is_directory())
							continue;

						auto& path = entry.path();
						if (Util::FilePathContains(path, "Scripts/Bin") 			||
							Util::FilePathContains(path, "Scripts/Generated") 	 	||
							Util::FilePathContains(path, "Scripts/ScriptCoreAPI")	)
							{
								continue;
							}
						only_excluded = false;
						break;
					}

					flags |= only_excluded ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
				}

				bool folder_opened = ImGui::TreeNodeEx(side_bar_root_path.filename().string().c_str(), flags);
				
				if (DirectoryFolderDropTarget(side_bar_root_path))
				{
					SetCurrentDirectory(side_bar_root_path);
				}

				// Capture if Clicking the Label Component of TreeNode to Select This Entity
				{
					const ImGuiStyle& style = ImGui::GetStyle();
					float font_size = ImGui::GetFontSize();
					float padding_x = style.FramePadding.x;
					float text_offset_x = font_size + ((flags & ImGuiTreeNodeFlags_Framed) ? padding_x * 3.0f : padding_x * 2.0f);

					ImVec2 item_min = ImGui::GetItemRectMin();
					ImVec2 item_max = ImGui::GetItemRectMax();
					ImVec2 label_min(item_min.x + text_offset_x, item_min.y);
					ImVec2 label_max(item_max.x, item_max.y);

					if (ImGui::IsMouseHoveringRect(label_min, label_max) && ImGui::IsMouseClicked(0))
						SetCurrentDirectory(side_bar_root_path);
				}

				if (!folder_opened)
					continue;
				
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));
				recursive_directory_display(side_bar_root_path);
				ImGui::PopStyleVar();

				ImGui::TreePop();
			}

		}
		ImGui::EndChild();
    }

    void ContentBrowserPanel::DrawTopNavigation()
    {
        std::filesystem::path current_lexical_normal = GetCurrentDirectory().lexically_normal();
        std::filesystem::path parent_lexical_normal;

        std::filesystem::path relative_path = std::filesystem::relative(current_lexical_normal, Application::GetProject()->GetDirectory());
        std::filesystem::path accumulated_path = Application::GetProject()->GetDirectory(); // Start with the parent directory's absolute path.
        for (auto it = relative_path.begin(); it != relative_path.end(); ++it) 
        {
            accumulated_path /= *it; // Build the full path step by step.

            if (std::next(it) == relative_path.end()) {

                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

                ImGui::Button(it->filename().string().c_str());

                ImGui::PopStyleColor(3);
                ImGui::PopStyleVar();

            }
            else 
            {
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

                if (ImGui::Button((accumulated_path.filename().string() + "##" + accumulated_path.string()).c_str())) 
                {
					SetCurrentDirectory(accumulated_path);
                }

                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar();

                ImGui::SameLine();
                ImGui::Text("/");
                ImGui::SameLine();
            }
        }
    }

    void ContentBrowserPanel::DrawContentBrowser()
    {
		if (ImGui::BeginChild("File Browser", {}, ImGuiChildFlags_Border | ImGuiChildFlags_FrameStyle)) {

			float available_width = ImGui::GetContentRegionAvail().x;

			float icon_size = 64.0f + ImGui::GetStyle().ItemSpacing.x * 2.0f;
			int num_columns = std::truncf(available_width / icon_size) > 0.0f ? (int)std::truncf(available_width / icon_size) : 4;

			// Start the table with the specified number of columns
			if (ImGui::BeginTable("FileTable", num_columns, ImGuiTableFlags_NoSavedSettings)) 
			{
				static bool image_button_clicked = false;
				static std::filesystem::path path_clicked = "";

				static bool is_renaming_path = false;
				static std::string new_path_file_name;
				static std::filesystem::path renaming_path = "";
				static bool first_focus = true; // Track if this is the first time the InputText box gets focus

				// Check for right-click on blank space
				if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsWindowHovered()) 
				{
					ImGui::OpenPopup("Content_Browser_ContextMenu");
					image_button_clicked = false;
				}

				// Sort Directories first, then files
				std::vector<std::filesystem::directory_entry> directories;
				std::vector<std::filesystem::directory_entry> files;

				for (const auto& entry : std::filesystem::directory_iterator(GetCurrentDirectory())) 
                {
					if (std::filesystem::is_directory(entry) && !Util::IsPathHidden(entry.path())) 
                    {
						directories.push_back(entry);
					}
					else 
                    {
						if (Util::IsPathHidden(entry.path()))
							continue;

						if(entry.path().extension() != ".meta")
							files.push_back(entry);
					}
				}

				// Sort directories and files if necessary
				std::sort(directories.begin(), directories.end(), [](const auto& lhs, const auto& rhs) 
                {
					return lhs.path() < rhs.path();
                });
				std::sort(files.begin(), files.end(), [](const auto& lhs, const auto& rhs) 
                {
					return lhs.path() < rhs.path();
                });

				// Combine directories and files back
				directories.insert(directories.end(), files.begin(), files.end());

				for (const auto& entry : directories) 
				{
					// Start a new row (ImGui will automatically handle row creation)
					ImGui::TableNextColumn();  // Move to the next column in the table

					if (entry.is_directory()) // ----------- DIRECTORY -----------
                    { 
						ImGui::SetNextItemWidth(icon_size);

						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
						ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
						ImGui::ImageButton(entry.path().string().c_str(), (ImTextureID)m_FolderIcon->GetDescriptor(), { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 });
						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) 
                        {
							SetCurrentDirectory(entry.path());
						}

						DirectoryFolderDropSource(entry.path());
						DirectoryFolderDropTarget(entry.path());

						ImGui::PopStyleVar(2);

						if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsItemHovered()) 
                        {
							image_button_clicked = true;
							path_clicked = entry.path();
							ImGui::OpenPopup("Content_Browser_ItemContextMenu");
						}
					}
					else if (entry.is_regular_file()) // ----------- REGULAR FILE -----------
                    { 
						ImGui::SetNextItemWidth(icon_size);

						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
						ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
						ImTextureID texture_id = m_FileIcon->GetDescriptor();

						if (AssetManager::IsExtensionSupported(entry.path().extension()) && AssetManager::GetAssetTypeFromFileExtension(entry.path().extension()) == AssetType::Texture2D)
						{
                            // TODO: Implement
							// auto texture_asset = AssetManager::GetAsset<Texture2D>(Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(entry.path(), Project::GetActiveProject()->GetAssetDirectory()));
							// if (texture_asset)
							// 	texture_id = texture_asset->GetID();
						}

						ImGui::ImageButton(entry.path().string().c_str(), (ImTextureID)(uintptr_t)texture_id, {64.0f, 64.0f}, {0, 1}, {1, 0});

                        // TODO: Implement Selection Context
						// if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && AssetManager::GetAssetTypeFromFileExtension(entry.path().extension()) == AssetType::Material_Standard)
						// 	editor_layer.m_MaterialContext = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(entry.path(), Project::GetActiveProject()->GetAssetDirectory());

						if (ImGui::BeginDragDropSource())
						{
							std::string path_str = entry.path().string(); // Convert to string
							ImGui::SetDragDropPayload("CONTENT_ITEM_FILE", path_str.c_str(), path_str.size() + 1); // Null-terminate string
							ImGui::EndDragDropSource();
						}
						
						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) 
						{
							auto extension = entry.path().extension().string();
							bool supported = s_SupportedOpenInEditorFiles.contains(extension);

							if (supported)
							{
                                // TODO: Implement
								// if (extension == ".scene")
								// {
								// 	editor_layer.OpenScene(entry.path());
								// }
								// else if (extension == ".animator")
								// {
								// 	AnimatorPanel::SetAnimatorAssetContext(Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(entry.path(), Project::GetActiveProject()->GetAssetDirectory()));
								// 	editor_layer.m_ActiveGUIWindows["AnimatorStateMachine"] = true;
								// }
								// else if (extension == ".humanoid")
								// {
									
								// 	editor_layer.m_HumanoidContext = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(entry.path(), Project::GetActiveProject()->GetAssetDirectory());
								// 	editor_layer.m_ActiveGUIWindows["Humanoid Configuration"] = true;
								// }
								// else if (extension == ".humanoidmask")
								// {
								// 	editor_layer.m_HumanoidMaskContext = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(entry.path(), Project::GetActiveProject()->GetAssetDirectory());
								// 	editor_layer.m_ActiveGUIWindows["Humanoid Mask Configuration"] = true;
								// }
							}
							else 
							{ 
								// Open with default system application
								std::string command;

							#if defined(BC_PLATFORM_WINDOWS)
								command = "start \"\" \"" + entry.path().string() + "\"";
							#elif defined(BC_PLATFORM_LINUX)
								command = "xdg-open \"" + entry.path().string() + "\"";
							#else
								// Unsupported platform
								BC_APP_ERROR("Opening files with default application is not supported on this platform.");
							#endif

								std::system(command.c_str());
							}

						}
						
						ImGui::PopStyleVar(2);

						if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsItemHovered()) 
                        {
							image_button_clicked = true;
							path_clicked = entry.path();
							ImGui::OpenPopup("Content_Browser_ItemContextMenu");
						}
					}

					// ----------- FILE NAME -----------

					// IF WE ARE RENAMING THE PATH
					if(renaming_path == entry.path() && is_renaming_path)
					{
						static char buf[256]{};

						if (first_focus) 
                        {
							strncpy(buf, new_path_file_name.c_str(), sizeof(buf));
    						ImGui::SetKeyboardFocusHere();
						}

						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
						if (ImGui::InputText("##RenamingInputText", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) 
                        {
							// Rename file/folder
							try 
                            {
								// Construct the new path by replacing the filename or folder name
								std::filesystem::path old_path = entry.path();
								std::filesystem::path new_path = old_path.parent_path() / buf;

								// Perform the rename operation
								std::filesystem::rename(old_path, new_path);
							}
							catch (const std::filesystem::filesystem_error& e) 
                            {
								BC_APP_WARN("Could Not Delete {0}: {1}.", std::filesystem::is_directory(entry.path()) ? "Folder" : "File", e.what());
							}

							is_renaming_path = false;
							new_path_file_name.clear();
							renaming_path.clear();
							first_focus = true; // Reset for the next renaming operation
						}
						else if (first_focus) 
                        {
							first_focus = false;
						}

						if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemHovered()) 
                        {
							is_renaming_path = false;
							new_path_file_name.clear();
							renaming_path.clear();
							first_focus = true; // Reset for the next renaming operation
						}
					}
					// JUST SHOW THE PATH
					else 
                    {
						ImGui::TextWrapped(entry.path().filename().string().c_str());
					}

				}

				static bool delete_file_path = false;
				static std::filesystem::path path_to_delete{};

				if (image_button_clicked) {
					
					// Handle item-specific context menu
					if (ImGui::BeginPopup("Content_Browser_ItemContextMenu")) {

						if (ImGui::MenuItem(std::filesystem::is_directory(path_clicked) ? "Open Directory In File Explorer" : "Open File")) 
                        {
							std::string command = "explorer \"" + path_clicked.string() + "\"";
							std::system(command.c_str());
						}

						if (ImGui::MenuItem("Rename")) 
                        {
							is_renaming_path = true;
							renaming_path = path_clicked;
							new_path_file_name = path_clicked.filename().string();
							first_focus = true;
						}

						if (ImGui::MenuItem(std::filesystem::is_directory(path_clicked) ? "Delete Folder" : "Delete File")) 
                        {
							delete_file_path = true;
							path_to_delete = path_clicked;
						}

						ImGui::EndPopup();
					}
					else 
                    {
						image_button_clicked = false;
						path_clicked = "";
					}
				}
				else 
				{
					// Handle blank space context menu
					if (ImGui::BeginPopup("Content_Browser_ContextMenu")) 
                    {
						if (ImGui::MenuItem("Create New Folder")) 
                        {
							std::filesystem::path file_path = GetCurrentDirectory() / "New Folder";

							if (!std::filesystem::exists(file_path))
							{
								std::filesystem::create_directories(file_path);
								is_renaming_path = true;
								renaming_path = file_path;
								new_path_file_name = file_path.filename().string();
								first_focus = true;
							}
						}

						// Scripts Folder
						if (GetCurrentDirectory().lexically_normal().string().find((Application::GetProject()->GetDirectory() / "Scripts").lexically_normal().string()) != std::string::npos) 
						{
							if (ImGui::MenuItem("Create New Script")) 
                            {
                                
							}
						}

						// Assets Folder
						if (GetCurrentDirectory().lexically_normal().string().find((Application::GetProject()->GetDirectory() / "Assets").lexically_normal().string()) != std::string::npos) 
                        {
							if (ImGui::MenuItem("Create New Material")) 
                            {

							}

							if (ImGui::MenuItem("Create New Skybox Material")) 
							{

							}

							if (ImGui::MenuItem("Create New Shader")) 
                            {
                                
							}

							if (ImGui::MenuItem("Create New Compute Shader")) 
                            {
                                
							}

							if (ImGui::MenuItem("Create New Animator Controller"))
							{
                                
							}
						}

						// Scenes Folder
						if (GetCurrentDirectory().lexically_normal().string().find((Application::GetProject()->GetDirectory() / "Scenes").lexically_normal().string()) != std::string::npos) 
                        {
							if (ImGui::MenuItem("Create New Scene"))
							{
								// TODO: Implement
							}
						}

						ImGui::Separator();

						if (ImGui::MenuItem("Open Directory In File Explorer")) 
                        {
							std::string command = "explorer \"" + GetCurrentDirectory().string() + "\"";
							std::system(command.c_str());
						}

						ImGui::EndPopup();
					}
				}

				if (delete_file_path)
				{
					ImGui::OpenPopup("Delete?##ContentBrowserDeletePath");

					// Calculate window size: 25% of the screen width, minimum 300.0f
					float windowWidth = glm::max(300.0f, ImGui::GetContentRegionAvail().x * 0.25f);
					float windowHeight = 100.0f; // Fixed height
					ImVec2 windowSize(windowWidth, windowHeight);
					ImGui::SetNextWindowSize(windowSize);

					ImGuiViewport* viewport = ImGui::GetMainViewport();
					ImVec2 viewportPos = viewport->Pos;
					ImVec2 viewportSize = viewport->Size;

					// Calculate centered position
					ImVec2 windowPos(
						viewportPos.x + (viewportSize.x - windowWidth) * 0.5f, // Center horizontally
						viewportPos.y + (viewportSize.y - windowHeight) * 0.5f // Center vertically
					);
					ImGui::SetNextWindowPos(windowPos);
				}

				if (ImGui::BeginPopupModal("Delete?##ContentBrowserDeletePath", &delete_file_path, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {

					ImGui::Dummy({ 0.0f, 5.0f });

					// Center the "Are you sure?" text
					ImVec2 windowSize = ImGui::GetWindowSize();
					float textWidth = ImGui::CalcTextSize("Are you sure?").x;
					ImGui::SetCursorPosX((windowSize.x - textWidth) * 0.5f);
					ImGui::Text("Are you sure?");

					ImGui::Dummy({ 0.0f, 5.0f });

					// Calculate button width and center them
					float buttonWidth = ImGui::GetContentRegionAvail().x * 0.4f;
					float centerOffset = (windowSize.x - buttonWidth * 2 - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

					// Center and display buttons
					ImGui::SetCursorPosX(centerOffset);
					if (ImGui::Button("Yes", ImVec2(buttonWidth, 0.0f))) 
                    {
						try 
                        {
							std::filesystem::remove_all(path_to_delete);
						}
						catch (const std::filesystem::filesystem_error& e) 
                        {
							BC_APP_WARN("Could Not Delete {0}: {1}.", std::filesystem::is_directory(path_clicked) ? "Folder" : "File", e.what());
						}

						delete_file_path = false;
						path_to_delete = "";
					}

					ImGui::SameLine();
					if (ImGui::Button("No", ImVec2(buttonWidth, 0.0f))) {
						delete_file_path = false;
						path_to_delete = "";
					}

					ImGui::EndPopup();
				}

				// End the table
				ImGui::EndTable();
			}
		}
		ImGui::EndChild();

		//DirectoryFolderDropTarget(m_CurrentDirectory);
    }

}