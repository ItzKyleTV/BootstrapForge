#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>
#include <BootstrapForge/Domain/Models/Resource.hpp>
#include <BootstrapForge/Engine/Differ/ArchitectureDiffer.hpp>

#include <iostream>

using namespace BootstrapForge;

int main()
{
    Domain::Models::Resource project(
        Domain::Types::ResourceId{},
        Domain::Types::Name("Project"),
        Domain::Types::Path("/"));

    Domain::Models::ArchitectureModel desired(
        { project },
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
            << static_cast<int>(operation.Kind())
            << " : "
            << operation.Name().Value()
            << '\n';
    }

    return 0;
}