#include <BootstrapForge/Architecture/Diagnostics/DiagnosticRenderer.hpp>

#include <BootstrapForge/Architecture/Diagnostics/DiagnosticFormatter.hpp>

#include <cstddef>
#include <string>
#include <string_view>

namespace BootstrapForge::Architecture::Diagnostics
{
    namespace
    {
        std::string ExtractLine(
            std::string_view source,
            std::size_t lineNumber)
        {
            if (lineNumber == 0)
            {
                return {};
            }

            std::size_t currentLine = 1;
            std::size_t lineStart = 0;

            while (lineStart < source.size())
            {
                if (currentLine == lineNumber)
                {
                    const std::size_t lineEnd =
                        source.find(
                            '\n',
                            lineStart);

                    const std::size_t end =
                        lineEnd == std::string_view::npos
                        ? source.size()
                        : lineEnd;

                    return std::string(
                        source.substr(
                            lineStart,
                            end - lineStart));
                }

                const std::size_t newline =
                    source.find(
                        '\n',
                        lineStart);

                if (newline == std::string_view::npos)
                {
                    return {};
                }

                lineStart = newline + 1;
                ++currentLine;
            }

            return {};
        }
    }

    std::string DiagnosticRenderer::Render(
        const Diagnostic& diagnostic,
        std::string_view source)
    {
        const std::string formatted =
            DiagnosticFormatter::Format(
                diagnostic);

        const std::string sourceLine =
            ExtractLine(
                source,
                diagnostic.Span()
                .Begin()
                .Line);

        std::string result;

        result += formatted;

        if (!sourceLine.empty())
        {
            result += "\n\n  ";

            result += sourceLine;

            result += "\n  ";

            const std::size_t column =
                diagnostic.Span()
                .Begin()
                .Column;

            if (column > 1)
            {
                result.append(
                    column - 1,
                    ' ');
            }

            result += '^';

            const std::size_t length =
                diagnostic.Span().Length();

            if (length > 1)
            {
                result.append(
                    length - 1,
                    '^');
            }
        }

        return result;
    }

} // namespace BootstrapForge::Architecture::Diagnostics