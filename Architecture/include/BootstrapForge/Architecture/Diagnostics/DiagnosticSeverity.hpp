#pragma once

#include <cstdint>

namespace BootstrapForge::Architecture::Diagnostics
{
    enum class DiagnosticSeverity : std::uint8_t
    {
        Note = 0,
        Warning,
        Error,
        Fatal
    };

} // namespace BootstrapForge::Architecture::Diagnostics