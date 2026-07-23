#include <BootstrapForge/Engine/Differ/ArchitectureDiffer.hpp>

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

        //
        // Create and Update operations
        //
        for (const auto& desiredResource : desired.Resources())
        {
            auto currentIt = std::ranges::find_if(
                current.Resources(),
                [&](const auto& currentResource)
                {
                    return currentResource.Id() == desiredResource.Id();
                });

            if (currentIt == current.Resources().end())
            {
                operations.emplace_back(
                    Operations::OperationKind::Create,
                    desiredResource.Id(),
                    desiredResource.Name());

                continue;
            }

            if (desiredResource != *currentIt)
            {
                operations.emplace_back(
                    Operations::OperationKind::Update,
                    desiredResource.Id(),
                    desiredResource.Name());
            }
        }

        //
        // Delete operations
        //
        for (const auto& currentResource : current.Resources())
        {
            const bool exists = std::ranges::any_of(
                desired.Resources(),
                [&](const auto& desiredResource)
                {
                    return desiredResource.Id() == currentResource.Id();
                });

            if (!exists)
            {
                operations.emplace_back(
                    Operations::OperationKind::Delete,
                    currentResource.Id(),
                    currentResource.Name());
            }
        }

        return Operations::ExecutionPlan(std::move(operations));
    }

} // namespace BootstrapForge::Engine::Differ