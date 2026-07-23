#include <BootstrapForge/Application/Identity/ResourceFactory.hpp>

#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>

#include <BootstrapForge/Engine/Differ/ArchitectureDiffer.hpp>
#include <BootstrapForge/Engine/Formatting/OperationKindFormatter.hpp>

#include <BootstrapForge/Infrastructure/Cryptography/Hashing/Blake3Hasher.hpp>

#include <iostream>

using namespace BootstrapForge;

int main()
{
    Infrastructure::Cryptography::Hashing::Blake3Hasher hasher;

    Application::Identity::ResourceFactory resourceFactory(hasher);

    const Domain::Models::ArchitectureModel desired(
        {
            resourceFactory.Create("/"),
            resourceFactory.Create("/src"),
            resourceFactory.Create("/src/main.cpp")
        },
        {}
    );

    const Domain::Models::ArchitectureModel current;

    Engine::Differ::ArchitectureDiffer differ;

    const auto plan = differ.Compare(desired, current);

    std::cout << "Execution Plan\n";
    std::cout << "--------------\n";

    for (const auto& operation : plan.Operations())
    {
        std::cout
            << Engine::Formatting::ToString(operation.Kind())
            << " : "
            << operation.Name().Value()
            << '\n';
    }

    return 0;
}