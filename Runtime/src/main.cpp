#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>
#include <BootstrapForge/Domain/Models/Resource.hpp>
#include <BootstrapForge/Engine/Differ/ArchitectureDiffer.hpp>

#include "OperationKindFormatter.hpp"

#include <iostream>

using namespace BootstrapForge;

int main()
{
    //
    // Desired Architecture
    //
    Domain::Models::Resource desiredProject(
        Domain::Types::ResourceId{},
        Domain::Types::Name("BootstrapForge"),
        Domain::Types::Path("/"));

    //
    // Current Architecture
    //
    Domain::Models::Resource currentProject(
        Domain::Types::ResourceId{},
        Domain::Types::Name("Project"),
        Domain::Types::Path("/"));

    Domain::Models::ArchitectureModel desired(
        {
            desiredProject
        },
        {}
    );

    Domain::Models::ArchitectureModel current(
        {
            currentProject
        },
        {}
    );

    Engine::Differ::ArchitectureDiffer differ;

    auto plan = differ.Compare(desired, current);

    std::cout << "Execution Plan\n";
    std::cout << "--------------\n";

    if (plan.Operations().empty())
    {
        std::cout << "No operations.\n";
    }
    else
    {
        for (const auto& operation : plan.Operations())
        {
            std::cout
                << ToString(operation.Kind())
                << " : "
                << operation.Name().Value()
                << '\n';
        }
    }

    return 0;
}