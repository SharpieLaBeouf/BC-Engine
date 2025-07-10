#include "BC_PCH.h"
#include "Material.h"

#include <yaml-cpp/yaml.h>

namespace BC
{
    std::shared_ptr<Material> Material::CreateMaterial(const std::string& serialised_material_data)
    {
        std::shared_ptr<Material> material;

        YAML::Node data_node = YAML::Load(serialised_material_data);

        material = std::make_shared<Material>();
        material->DeserialiseAsset(data_node);

        return material;
    }
    
    std::shared_ptr<Skybox> Skybox::CreateSkybox(const std::string& serialised_skybox_data)
    {
        std::shared_ptr<Skybox> skybox;
        BC_CATCH_BEGIN();

        YAML::Node data_node = YAML::Load(serialised_skybox_data);

        skybox = std::make_shared<Skybox>();
        skybox->DeserialiseAsset(data_node);

        BC_CATCH_END_RETURN(nullptr);

        return skybox;
    }
}