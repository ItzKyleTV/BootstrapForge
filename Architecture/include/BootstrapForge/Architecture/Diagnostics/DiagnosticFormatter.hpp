#pragma once

#include <BootstrapForge/Architecture/Diagnostics/Diagnostic.hpp>

#include <string>

namespace BootstrapForge::Architecture::Diagnostics
{
    class DiagnosticFormatter final
    {
    public:

        DiagnosticFormatter() = delete;

        [[nodiscard]]
        static std::string Format(
            const Diagnostic& diagnostic);
    };

} // namespace BootstrapForge::Architecture::Diagnostics