#pragma once

#include <BootstrapForge/Architecture/Diagnostics/Diagnostic.hpp>
#include <BootstrapForge/Architecture/Diagnostics/DiagnosticBag.hpp>

#include <BootstrapForge/Architecture/Parsing/SourceSpan.hpp>

#include <string>
#include <utility>

namespace BootstrapForge::Architecture::Diagnostics
{
    class DiagnosticEngine final
    {
    public:

        DiagnosticEngine() = default;

        void Note(
            Parsing::SourceSpan span,
            std::string message);

        void Warning(
            Parsing::SourceSpan span,
            std::string message);

        void Error(
            Parsing::SourceSpan span,
            std::string message);

        void Fatal(
            Parsing::SourceSpan span,
            std::string message);

        [[nodiscard]]
        const DiagnosticBag& Diagnostics() const noexcept;

        [[nodiscard]]
        DiagnosticBag& Diagnostics() noexcept;

        [[nodiscard]]
        bool HasErrors() const noexcept;

        [[nodiscard]]
        bool HasFatalErrors() const noexcept;

        void Clear() noexcept;

    private:

        void Report(
            DiagnosticSeverity severity,
            Parsing::SourceSpan span,
            std::string message);

        DiagnosticBag m_diagnostics;
    };

} // namespace BootstrapForge::Architecture::Diagnostics