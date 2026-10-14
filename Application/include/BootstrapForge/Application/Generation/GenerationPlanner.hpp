#pragma once

#include <BootstrapForge/Application/Generation/GenerationPlan.hpp>

#include <BootstrapForge/Architecture/AST/GenerationNodes.hpp>
#include <BootstrapForge/Architecture/Diagnostics/DiagnosticEngine.hpp>

#include <optional>

namespace BootstrapForge::Application::Generation
{
    class GenerationPlanner final
    {
    public:

        [[nodiscard]]
        std::optional<GenerationPlan> Build(
            const Architecture::AST::GenerateNode& command,
            Architecture::Diagnostics::DiagnosticEngine& diagnostics) const;
    };
}
