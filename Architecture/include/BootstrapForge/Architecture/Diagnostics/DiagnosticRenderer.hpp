#pragma once

#include <BootstrapForge/Architecture/Diagnostics/Diagnostic.hpp>

#include <string>
#include <string_view>

namespace BootstrapForge::Architecture::Diagnostics
{
    class DiagnosticRenderer final
    {
    public:

        DiagnosticRenderer() = delete;

        [[nodiscard]]
        static std::string Render(
            const Diagnostic& diagnostic,
            std::string_view source);
    };

} // namespace BootstrapForge::Architecture::Diagnostics