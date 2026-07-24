#pragma once

#include <BootstrapForge/Domain/Models/ArchitectureDelta.hpp>
#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>

namespace BootstrapForge::Engine::Differ
{
    class ArchitectureDiffer
    {
    public:

        [[nodiscard]]
        BootstrapForge::Domain::Models::ArchitectureDelta Compare(
            const BootstrapForge::Domain::Models::ArchitectureModel& desired,
            const BootstrapForge::Domain::Models::ArchitectureModel& current) const;
    };

} // namespace BootstrapForge::Engine::Differ