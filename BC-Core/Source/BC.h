#pragma once

// ---- Asset ----
#include "Asset/Asset.h"
#include "Asset/AssetManager.h"
#include "Asset/AssetManagerAPI.h"

#include "Asset/Assets/Material.h"
#include "Asset/Assets/RenderTarget.h"
#include "Asset/Assets/Shader.h"
#include "Asset/Assets/StaticMesh.h"
#include "Asset/Assets/Texture.h"

// ---- Audio ----
//#include "Audio/Audio.h"

// ---- Core ----
#include "Core/Application.h"
#include "Core/GUID.h"
#include "Core/Input.h"
#include "Core/Keycode.h"
#include "Core/LayerStack.h"
#include "Core/Time.h"
#include "Core/Window.h"

// ---- Debug ----
#include "Debug/Assert.h"
#include "Debug/ErrorCodes.h"
#include "Debug/Logging.h"
#include "Debug/Profiler.h"

// ---- Graphics ----
#include "Graphics/Renderer/CameraContext.h"
#include "Graphics/Renderer/GeometryEnvironment.h"
#include "Graphics/Renderer/LightEnvironment.h"
#include "Graphics/Renderer/SceneRenderer.h"
#include "Graphics/Renderer/ShadowEnvironment.h"

#include "Graphics/Vulkan/RenderCommand.h"
#include "Graphics/Vulkan/Swapchain.h"
#include "Graphics/Vulkan/VulkanCore.h"
#include "Graphics/Vulkan/VulkanUtil.h"

#include "Graphics/SceneCamera.h"

// ---- Jobs ----
#include "Jobs/Jobs.h"
#include "Jobs/JobSystem.h"

// ---- Physics ----
#include "Physics/Physics.h"

// ---- Project ----
#include "Project/Project.h"

#include "Project/Scene/Entity.h"
#include "Project/Scene/Scene.h"
#include "Project/Scene/SceneManager.h"

#include "Project/Scene/Bounds/Bounds.h"
#include "Project/Scene/Bounds/Frustum.h"
#include "Project/Scene/Bounds/Octree.h"

#include "Project/Scene/Components/Component Base.h"
#include "Project/Scene/Components/AnimationComponents.h"
#include "Project/Scene/Components/AudioComponents.h"
#include "Project/Scene/Components/CameraComponents.h"
#include "Project/Scene/Components/CoreComponents.h"
#include "Project/Scene/Components/LightComponents.h"
#include "Project/Scene/Components/MeshComponents.h"
#include "Project/Scene/Components/PhysicsComponents.h"

// ---- Scripting ----

// ---- Util ----
#include "Util/FileUtil.h"
#include "Util/Hash.h"
#include "Util/Macro.h"
#include "Util/Platform.h"
#include "Util/ThreadUtil.h"