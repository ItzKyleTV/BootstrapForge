#pragma once

#include <BootstrapForge/Architecture/Diagnostics/DiagnosticSeverity.hpp>
#include <BootstrapForge/Architecture/Parsing/SourceSpan.hpp>

#include <string>
#include <utility>

namespace BootstrapForge::Architecture::Diagnostics
{
    class Diagnostic final
    {
    public:

        Diagnostic() = default;

        Diagnostic(
            DiagnosticSeverity severity,
            std::string message,
            Parsing::SourceSpan span)
            :
            m_severity(severity),
            m_message(std::move(message)),
            m_span(span)
        {
        }

        [[nodiscard]]
        DiagnosticSeverity Severity() const noexcept
        {
            return m_severity;
        }

        [[nodiscard]]
        const std::string& Message() const noexcept
        {
            return m_message;
        }

        [[nodiscard]]
        const Parsing::SourceSpan& Span() const noexcept
        {
            return m_span;
        }

        [[nodiscard]]
        bool IsNote() const noexcept
        {
            return m_severity ==
                DiagnosticSeverity::Note;
        }

        [[nodiscard]]
        bool IsWarning() const noexcept
        {
            return m_severity ==
                DiagnosticSeverity::Warning;
        }

        [[nodiscard]]
        bool IsError() const noexcept
        {
            return m_severity ==
                DiagnosticSeverity::Error;
        }

        [[nodiscard]]
        bool IsFatal() const noexcept
        {
            return m_severity ==
                DiagnosticSeverity::Fatal;
        }

    private:

        DiagnosticSeverity m_severity =
            DiagnosticSeverity::Note;

        std::string m_message;

        Parsing::SourceSpan m_span;
    };

} // namespace BootstrapForge::Architecture::Diagnostics