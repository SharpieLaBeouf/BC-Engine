#pragma once

// Core Headers
#include "Graphics/Vulkan/Swapchain.h"
#include "Graphics/Renderer/CameraContext.h"

// C++ Standard Library Headers
#include <vector>
#include <string>

// External Vendor Library Headers
#include <vulkan/vulkan.h>

namespace BC 
{
	class Layer 
    {

	public:

		Layer(const std::string& layerName = "Generic Layer") { }
		virtual ~Layer() = default;

		virtual void ResizeScreenSpace(uint32_t width, uint32_t height) {}
		virtual void ResizeSwapchain(const SwapchainSpecification& swapchain_spec) {}

		virtual void OnAttach() {}
		virtual void OnDetach() {}

		virtual void OnUpdate() {}
		virtual void OnFixedUpdate() {}
		virtual void OnLateUpdate() {}

		virtual void OnRenderGUI() {}

		virtual std::vector<CameraContext> GetCameraOverrides() = 0;
	};
    
	class GUILayer : public Layer 
    {
    
    public:

		GUILayer();
		~GUILayer() = default;

		void OnAttach() override;
		void OnDetach() override;

		std::vector<CameraContext> GetCameraOverrides() override { return {}; }

		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }

		void SetDarkThemeColors();

		uint32_t GetActiveWidgetID() const;

		VkSampler GetImGuiDefaultSampler() const { return m_ImGuiDefaultSampler; }
		VkDescriptorPool GetImGuiDescriptorPool() const { return m_ImGuiDescriptorPool; }
		VkDescriptorSetLayout GetImGuiDescriptorSetLayout() const { return m_ImGuiDescriptorSetLayout; }

	private:

		bool m_BlockEvents = true;

		VkSampler 				m_ImGuiDefaultSampler		= VK_NULL_HANDLE;
		VkDescriptorPool 		m_ImGuiDescriptorPool 		= VK_NULL_HANDLE;
		VkDescriptorSetLayout 	m_ImGuiDescriptorSetLayout 	= VK_NULL_HANDLE;
	};

	class LayerStack 
    {

	public:

		LayerStack() = default;
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_Layers.end(); }
		std::vector<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
		std::vector<Layer*>::reverse_iterator rend() { return m_Layers.rend(); }

		std::vector<Layer*>::const_iterator begin() const { return m_Layers.begin(); }
		std::vector<Layer*>::const_iterator end()	const { return m_Layers.end(); }
		std::vector<Layer*>::const_reverse_iterator rbegin() const { return m_Layers.rbegin(); }
		std::vector<Layer*>::const_reverse_iterator rend() const { return m_Layers.rend(); }

	private:

		std::vector<Layer*> m_Layers;
		unsigned int m_LayerInsertIndex = 0;
	};

}