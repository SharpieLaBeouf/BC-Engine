#include "BC_PCH.h"
#include "MeshComponents.h"

// TODO: Implement

namespace BC
{
    
#pragma region LODMeshComponent
    
    LODMeshComponent::LODMeshComponent(const LODMeshComponent& other)
    {

    }

    LODMeshComponent::LODMeshComponent(LODMeshComponent&& other) noexcept
    {

    }

    LODMeshComponent& LODMeshComponent::operator=(const LODMeshComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    LODMeshComponent& LODMeshComponent::operator=(LODMeshComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        return *this;
    }

    void LODMeshComponent::SceneSerialise(YAML::Emitter& out) const
    {

    }

    bool LODMeshComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion
    
#pragma region MeshRendererComponent
    
    MeshRendererComponent::MeshRendererComponent(const MeshRendererComponent& other)
    {

    }

    MeshRendererComponent::MeshRendererComponent(MeshRendererComponent&& other) noexcept
    {

    }

    MeshRendererComponent& MeshRendererComponent::operator=(const MeshRendererComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    MeshRendererComponent& MeshRendererComponent::operator=(MeshRendererComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        return *this;
    }

    void MeshRendererComponent::SceneSerialise(YAML::Emitter& out) const
    {

    }

    bool MeshRendererComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

#pragma region SkinnedMeshRendererComponent
    
    SkinnedMeshRendererComponent::SkinnedMeshRendererComponent(const SkinnedMeshRendererComponent& other)
    {

    }

    SkinnedMeshRendererComponent::SkinnedMeshRendererComponent(SkinnedMeshRendererComponent&& other) noexcept
    {

    }

    SkinnedMeshRendererComponent& SkinnedMeshRendererComponent::operator=(const SkinnedMeshRendererComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    SkinnedMeshRendererComponent& SkinnedMeshRendererComponent::operator=(SkinnedMeshRendererComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        return *this;
    }

    void SkinnedMeshRendererComponent::SceneSerialise(YAML::Emitter& out) const
    {

    }

    bool SkinnedMeshRendererComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

}
