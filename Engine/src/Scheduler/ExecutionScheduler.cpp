#include <BootstrapForge/Engine/Scheduler/ExecutionScheduler.hpp>

#include <algorithm>
#include <filesystem>
#include <vector>

namespace BootstrapForge::Engine::Scheduler
{
    namespace
    {
        [[nodiscard]]
        std::size_t Depth(const std::filesystem::path& path)
        {
            const auto value = path.generic_string();

            if (value.empty())
            {
                return 0;
            }

            return static_cast<std::size_t>(
                std::count(
                    value.begin(),
                    value.end(),
                    '/'));
        }
    }

    Domain::Operations::ScheduledExecutionPlan
        ExecutionScheduler::Schedule(
            const Domain::Operations::ExecutionPlan& plan) const
    {
        using Domain::Operations::Operation;
        using Domain::Operations::OperationKind;

        std::vector<Operation> creates;
        std::vector<Operation> updates;
        std::vector<Operation> deletes;
        std::vector<Operation> others;

        creates.reserve(plan.Operations().size());
        updates.reserve(plan.Operations().size());
        deletes.reserve(plan.Operations().size());
        others.reserve(plan.Operations().size());

        for (const auto& operation : plan.Operations())
        {
            switch (operation.Kind())
            {
            case OperationKind::Create:
                creates.push_back(operation);
                break;

            case OperationKind::Update:
                updates.push_back(operation);
                break;

            case OperationKind::Delete:
                deletes.push_back(operation);
                break;

            default:
                others.push_back(operation);
                break;
            }
        }

        //
        // Creates:
        // Parents must be created before children.
        //
        std::sort(
            creates.begin(),
            creates.end(),
            [](const Operation& left, const Operation& right)
            {
                return Depth(left.Resource().Path().Value()) <
                    Depth(right.Resource().Path().Value());
            });

        //
        // Deletes:
        // Children must be deleted before parents.
        //
        std::sort(
            deletes.begin(),
            deletes.end(),
            [](const Operation& left, const Operation& right)
            {
                return Depth(left.Resource().Path().Value()) >
                    Depth(right.Resource().Path().Value());
            });

        std::vector<Operation> scheduled;
        scheduled.reserve(plan.Operations().size());

        scheduled.insert(
            scheduled.end(),
            deletes.begin(),
            deletes.end());

        scheduled.insert(
            scheduled.end(),
            updates.begin(),
            updates.end());

        scheduled.insert(
            scheduled.end(),
            creates.begin(),
            creates.end());

        scheduled.insert(
            scheduled.end(),
            others.begin(),
            others.end());

        return Domain::Operations::ScheduledExecutionPlan(
            std::move(scheduled));
    }
}