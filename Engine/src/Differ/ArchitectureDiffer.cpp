#include <BootstrapForge/Engine/Differ/ArchitectureDiffer.hpp>

#include <BootstrapForge/Domain/Operations/Operation.hpp>
#include <BootstrapForge/Domain/Operations/OperationKind.hpp>

#include <algorithm>
#include <vector>

namespace BootstrapForge::Engine::Differ
{

    using namespace BootstrapForge::Domain;

    Operations::ExecutionPlan ArchitectureDiffer::Compare(
        const Models::ArchitectureModel& desired,
        const Models::ArchitectureModel& current) const
    {
        std::vector<Operations::Operation> operations;

        for (const auto& desiredResource : desired.Resources())
        {
            const bool exists = std::ranges::any_of(
                current.Resources(),
                [&](const auto& currentResource)
                {
                    return currentResource.Id() == desiredResource.Id();
                });

            if (!exists)
            {
                operations.emplace_back(
                    Operations::OperationKind::Create,
                    desiredResource.Id(),
                    desiredResource.Name());
            }
        }

        return Operations::ExecutionPlan(std::move(operations));
    }

} // namespace BootstrapForge::Engine::Differ