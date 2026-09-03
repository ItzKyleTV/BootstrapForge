#include <BootstrapForge/Architecture/Diagnostics/DiagnosticEngine.hpp>

#include <utility>

namespace BootstrapForge::Architecture::Diagnostics
{
    void DiagnosticEngine::Note(
        Parsing::SourceSpan span,
        std::string message)
    {
        Report(
            DiagnosticSeverity::Note,
            span,
            std::move(message));
    }

    void DiagnosticEngine::Warning(
        Parsing::SourceSpan span,
        std::string message)
    {
        Report(
            DiagnosticSeverity::Warning,
            span,
            std::move(message));
    }

    void DiagnosticEngine::Error(
        Parsing::SourceSpan span,
        std::string message)
    {
        Report(
            DiagnosticSeverity::Error,
            span,
            std::move(message));
    }

    void DiagnosticEngine::Fatal(
        Parsing::SourceSpan span,
        std::string message)
    {
        Report(
            DiagnosticSeverity::Fatal,
            span,
            std::move(message));
    }

    const DiagnosticBag&
        DiagnosticEngine::Diagnostics() const noexcept
    {
        return m_diagnostics;
    }

    DiagnosticBag&
        DiagnosticEngine::Diagnostics() noexcept
    {
        return m_diagnostics;
    }

    bool DiagnosticEngine::HasErrors() const noexcept
    {
        return m_diagnostics.HasErrors();
    }

    bool DiagnosticEngine::HasFatalErrors() const noexcept
    {
        return m_diagnostics.HasFatalErrors();
    }

    void DiagnosticEngine::Clear() noexcept
    {
        m_diagnostics.Clear();
    }

    void DiagnosticEngine::Report(
        DiagnosticSeverity severity,
        Parsing::SourceSpan span,
        std::string message)
    {
        m_diagnostics.Add(
            Diagnostic(
                severity,
                std::move(message),
                span));
    }

} // namespace BootstrapForge::Architecture::Diagnostics