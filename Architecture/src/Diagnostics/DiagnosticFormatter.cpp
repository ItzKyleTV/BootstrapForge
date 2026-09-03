#include <BootstrapForge/Architecture/Diagnostics/DiagnosticFormatter.hpp>

#include <BootstrapForge/Architecture/Parsing/SourceLocation.hpp>

#include <string>

namespace BootstrapForge::Architecture::Diagnostics
{
    namespace
    {
        const char* SeverityName(
            DiagnosticSeverity severity) noexcept
        {
            switch (severity)
            {
            case DiagnosticSeverity::Note:
                return "note";

            case DiagnosticSeverity::Warning:
                return "warning";

            case DiagnosticSeverity::Error:
                return "error";

            case DiagnosticSeverity::Fatal:
                return "fatal";
            }

            return "unknown";
        }
    }

    std::string DiagnosticFormatter::Format(
        const Diagnostic& diagnostic)
    {
        const Parsing::SourceLocation& location =
            diagnostic.Span().Begin();

        std::string result;

        result += SeverityName(
            diagnostic.Severity());

        result += ": ";

        result += diagnostic.Message();

        result += "\n";

        result += "  --> ";

        result += std::to_string(
            location.Line);

        result += ":";

        result += std::to_string(
            location.Column);

        return result;
    }

} // namespace BootstrapForge::Architecture::Diagnostics