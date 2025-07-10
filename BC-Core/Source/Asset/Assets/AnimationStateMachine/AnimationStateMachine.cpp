#include "BC_PCH.h"
#include "AnimationStateMachine.h"

#include <yaml-cpp/yaml.h>

namespace BC::Animation
{

    std::shared_ptr<StateMachine> StateMachine::CreateStateMachine(const std::string& serialised_state_machine_data)
    {
        std::shared_ptr<StateMachine> state_machine;

        YAML::Node data_node = YAML::Load(serialised_state_machine_data);

        state_machine = std::make_shared<StateMachine>();
        state_machine->DeserialiseAsset(data_node);

        return state_machine;
    }

}