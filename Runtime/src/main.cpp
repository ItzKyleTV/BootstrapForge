#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>
#include <BootstrapForge/Domain/Models/Resource.hpp>
#include <BootstrapForge/Engine/Differ/ArchitectureDiffer.hpp>

#include "OperationKindFormatter.hpp"

#include <iostream>

using namespace BootstrapForge;

int main()
{
    Domain::Models::Resource project(
        Domain::Types::ResourceId{},
        Domain::Types::Name("Project"),
        Domain::Types::Path("/"));

    Domain::Models::Resource src(
        Domain::Types::ResourceId{},
        Domain::Types::Name("src"),
        Domain::Types::Path("/src"));

    Domain::Models::Resource mainCpp(
        Domain::Types::ResourceId{},
        Domain::Types::Name("main.cpp"),
        Domain::Types::Path("/src/main.cpp"));

    Domain::Models::ArchitectureModel desired(
        {
            project,
            src,
            mainCpp
        },
        {}
    );

    Domain::Models::ArchitectureModel current;

    Engine::Differ::ArchitectureDiffer differ;

    auto plan = differ.Compare(desired, current);

    std::cout << "Execution Plan\n";
    std::cout << "--------------\n";

    for (const auto& operation : plan.Operations())
    {
        std::cout
            << ToString(operation.Kind())
            << " : "
            << operation.Name().Value()
            << '\n';
    }

    return 0;
}