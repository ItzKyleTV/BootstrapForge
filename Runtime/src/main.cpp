#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>
#include <BootstrapForge/Engine/Differ/ArchitectureDiffer.hpp>

#include <iostream>

int main()
{
    BootstrapForge::Domain::Models::ArchitectureModel desired;
    BootstrapForge::Domain::Models::ArchitectureModel current;

    BootstrapForge::Engine::Differ::ArchitectureDiffer differ;

    auto plan = differ.Compare(desired, current);

    std::cout << "Operations: "
        << plan.Operations().size()
        << '\n';

    return 0;
}