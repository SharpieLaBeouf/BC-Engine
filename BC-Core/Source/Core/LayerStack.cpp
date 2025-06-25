#include "BC_PCH.h"
#include "LayerStack.h"

// Core Headers
#include "Application.h"

// C++ Standard Library Headers

// External Vendor Library Headers

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <ImGuizmo.h>

namespace BC
{

#pragma region Layer Stack

	LayerStack::~LayerStack()
	{
		for (Layer* layer : m_Layers)
		{
			layer->OnDetach();
			delete layer;
		}
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		m_Layers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer);
		if (it != m_Layers.begin() + m_LayerInsertIndex)
		{
			layer->OnDetach();
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		auto it = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), overlay);
		if (it != m_Layers.end())
		{
			overlay->OnDetach();
			m_Layers.erase(it);
		}
	}

#pragma endregion

#pragma region GUI Layer

	GUILayer::GUILayer() : Layer("GuiLayer") { }

	void GUILayer::OnAttach() 
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		style.FrameRounding = 4.0f;
		style.FrameBorderSize = 1.0f;

		style.WindowMenuButtonPosition = ImGuiDir_Right;
		style.ColorButtonPosition = ImGuiDir_Left;

		SetDarkThemeColors();

		// Setup ImGui Descriptor Pool
		auto vulkan_core = Application::GetVulkanCore();
		{
			VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};

			VkDescriptorPoolCreateInfo pool_info = 
			{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
				.pNext = VK_NULL_HANDLE,
				.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
				.maxSets = 1000 * static_cast<uint32_t>(BC_ARRAY_SIZE(pool_sizes)),
				.poolSizeCount = static_cast<uint32_t>(BC_ARRAY_SIZE(pool_sizes)),
				.pPoolSizes = pool_sizes
			};
			
			auto result = vkCreateDescriptorPool(vulkan_core->GetLogicalDevice(), &pool_info, VK_NULL_HANDLE, &m_ImGuiDescriptorPool);
			BC_ASSERT(result == VK_SUCCESS, "GUILayer::OnAttach: Could Not Create ImGui Descriptor Pool.");

			VkDescriptorSetLayoutBinding binding{};
			binding.binding = 0;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			binding.descriptorCount = 1;
			binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			binding.pImmutableSamplers = nullptr;

			VkDescriptorSetLayoutCreateInfo layout_info{};
			layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layout_info.bindingCount = 1;
			layout_info.pBindings = &binding;

			result = vkCreateDescriptorSetLayout(vulkan_core->GetLogicalDevice(), &layout_info, nullptr, &m_ImGuiDescriptorSetLayout);
			BC_ASSERT(result == VK_SUCCESS, "GUILayer::OnAttach: Could Not Create ImGui Default Descriptor Set Layout.");
		}

		GLFWwindow* window = static_cast<GLFWwindow*>(Application::GetWindow()->GetNativeWindow());

		auto result = ImGui_ImplGlfw_InitForVulkan(window, false);
		BC_ASSERT(result, "GUILayer::OnAttach: Could Not Init ImGui GLFW Implementation For Vulkan.");

		ImGui_ImplVulkan_InitInfo imgui_create_info = {};
		imgui_create_info.Instance = vulkan_core->GetInstance();
		imgui_create_info.PhysicalDevice = vulkan_core->GetPhysicalDevice();
		imgui_create_info.Device = vulkan_core->GetLogicalDevice();
		imgui_create_info.QueueFamily = vulkan_core->GetGraphicsQueueFamily();
		imgui_create_info.Queue = vulkan_core->GetGraphicsQueue();
		imgui_create_info.DescriptorPool = m_ImGuiDescriptorPool;
		imgui_create_info.RenderPass = vulkan_core->GetSwapchain().GetRenderPass();
		imgui_create_info.MinImageCount = Swapchain::s_MinImageCount;
		imgui_create_info.ImageCount = vulkan_core->GetSwapchain().GetImageCount();
		imgui_create_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		imgui_create_info.UseDynamicRendering = false;

		result = ImGui_ImplVulkan_Init(&imgui_create_info);
		BC_ASSERT(result, "GUILayer::OnAttach: Could Not Init ImGui Vulkan Implementation.");
		
		ImGui_ImplVulkan_CreateFontsTexture();
		
		VkSamplerCreateInfo sampler_info = {};
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.magFilter = VK_FILTER_LINEAR;
		sampler_info.minFilter = VK_FILTER_LINEAR;
		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_info.minLod = 0.0f;
		sampler_info.maxLod = 1.0f;

		result = vkCreateSampler(vulkan_core->GetLogicalDevice(), &sampler_info, nullptr, &m_ImGuiDefaultSampler);
		BC_ASSERT(result == VK_SUCCESS, "GUILayer::OnAttach: Could Not Create ImGui Default Sampler.");
	}

	void GUILayer::OnDetach() 
	{
		VkDevice device = Application::GetVulkanCore()->GetLogicalDevice();

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		if (m_ImGuiDefaultSampler)
		{
			vkDestroySampler(device, m_ImGuiDefaultSampler, nullptr);
			m_ImGuiDefaultSampler = VK_NULL_HANDLE;
		}

		if (m_ImGuiDescriptorSetLayout)
		{
			vkDestroyDescriptorSetLayout(device, m_ImGuiDescriptorSetLayout, nullptr);
			m_ImGuiDescriptorSetLayout = VK_NULL_HANDLE;
		}

		if (m_ImGuiDescriptorPool)
		{
			vkDestroyDescriptorPool(device, m_ImGuiDescriptorPool, nullptr);
			m_ImGuiDescriptorPool = VK_NULL_HANDLE;
		}
	}

	void GUILayer::Begin() 
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void GUILayer::End() 
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)Application::GetWindow()->GetWidth(), (float)Application::GetWindow()->GetHeight());

		// Rendering
		ImGui::Render();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
		
		auto vulkan_core = Application::GetVulkanCore();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vulkan_core->GetCurrentFrame().command_buffer);
	}

	void GUILayer::SetDarkThemeColors() {
		auto& colors = ImGui::GetStyle().Colors;

		// Text
		colors[ImGuiCol_Text]					= ImVec4{ 0.875f, 0.875f, 0.875f, 1.0f };
		colors[ImGuiCol_TextSelectedBg]			= ImVec4{ 0.314f, 0.493f, 0.575f, 0.416f };

		// Separator
		colors[ImGuiCol_Separator]				= ImVec4{ 0.0f, 0.0f, 0.0f, 1.0f };

		// Windows
		colors[ImGuiCol_WindowBg]				= ImVec4{ 0.22f, 0.22f, 0.22f, 1.0f };
		colors[ImGuiCol_DockingPreview]			= ImVec4{ 0.314f, 0.493f, 0.575f, 0.416f };

		// Borders
		colors[ImGuiCol_Border]					= ImVec4{ 0.12f, 0.12f, 0.12f, 1.0f};
		colors[ImGuiCol_BorderShadow]			= ImVec4{ 0.221f, 0.221f, 0.221f, 1.0f};

		// Headers
		colors[ImGuiCol_Header]					= ImVec4{ 0.314f, 0.493f, 0.575f, 0.416f };
		colors[ImGuiCol_HeaderHovered]			= ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive]			= ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button]					= ImVec4{ 0.165f, 0.165f, 0.165f, 1.0f };
		colors[ImGuiCol_ButtonHovered]			= ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f };
		colors[ImGuiCol_ButtonActive]			= ImVec4{ 0.588f, 0.588f, 0.588f, 1.0f };
		colors[ImGuiCol_CheckMark]				= ImVec4{ 0.314f, 0.493f, 0.575f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg]				= ImVec4{ 0.165f, 0.165f, 0.165f, 1.0f };
		colors[ImGuiCol_FrameBgHovered]			= ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive]			= ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab]					= ImVec4{ 0.141f, 0.141f, 0.141f, 1.0f };
		colors[ImGuiCol_TabHovered]				= ImVec4{ 0.22f, 0.22f, 0.22f, 1.0f };

		colors[ImGuiCol_TabSelected]			= ImVec4{ 0.22f, 0.22f, 0.22f, 1.0f };
		colors[ImGuiCol_TabSelectedOverline]	= ImVec4{ 0.314f, 0.493f, 0.575f, 0.416f };

		colors[ImGuiCol_TabDimmed]				= ImVec4{ 0.141f, 0.141f, 0.141f, 1.0f };
		colors[ImGuiCol_TabDimmedSelected]		= ImVec4{ 0.22f, 0.22f, 0.22f, 1.0f };

		// Resize
		colors[ImGuiCol_ResizeGrip]				= ImVec4{ 0.314f, 0.493f, 0.575f, 0.416f };
		colors[ImGuiCol_ResizeGripHovered]		= ImVec4{ 0.314f, 0.493f, 0.575f, 0.416f };
		colors[ImGuiCol_ResizeGripActive]		= ImVec4{ 0.314f, 0.493f, 0.575f, 0.416f };

		// Title
		colors[ImGuiCol_TitleBg]				= ImVec4{ 0.157f, 0.157f, 0.157f, 1.0f };
		colors[ImGuiCol_TitleBgActive]			= colors[ImGuiCol_TitleBg];
		colors[ImGuiCol_TitleBgCollapsed]		= colors[ImGuiCol_TitleBg];

		// Drag and Drop
		colors[ImGuiCol_DragDropTarget]			= ImVec4{ 0.314f, 0.493f, 0.575f, 0.416f };
	}

	uint32_t GUILayer::GetActiveWidgetID() const
	{
		return static_cast<uint32_t>(GImGui->ActiveId);
	}


#pragma endregion

}