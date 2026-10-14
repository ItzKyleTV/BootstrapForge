#pragma once

#include <BootstrapForge/Architecture/AST/GenerationNodes.hpp>
#include <BootstrapForge/Architecture/Diagnostics/DiagnosticEngine.hpp>
#include <BootstrapForge/Architecture/Grammar/Grammar.hpp>
#include <BootstrapForge/Architecture/Parsing/TokenStream.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace BootstrapForge::Architecture::Parsing
{
    class GenerateCommandParser final
    {
    public:

        GenerateCommandParser(
            const Grammar::Grammar& grammar,
            Diagnostics::DiagnosticEngine& diagnostics,
            std::string_view source);

        [[nodiscard]]
        std::unique_ptr<AST::GenerateNode> Parse();

    private:

        enum class ParentKind
        {
            Generate,
            Branch,
            Subbranch,
            Folder
        };

        class ParseFailure final : public std::runtime_error
        {
        public:

            ParseFailure(
                SourceSpan span,
                std::string message);

            [[nodiscard]]
            const SourceSpan& Span() const noexcept;

        private:

            SourceSpan m_span;
        };

        [[nodiscard]]
        std::unique_ptr<AST::GenerateNode> ParseGenerate();

        [[nodiscard]]
        std::unique_ptr<AST::GenerationContainerNode>
            ParseContainer(ParentKind parent);

        [[nodiscard]]
        std::unique_ptr<AST::FileNode> ParseFile();

        [[nodiscard]]
        std::string ParseName();

        [[nodiscard]]
        bool IsNameToken(Grammar::TokenKind kind) const noexcept;

        [[nodiscard]]
        SourceSpan CurrentSpan() const noexcept;

        const Grammar::Grammar& m_grammar;
        Diagnostics::DiagnosticEngine& m_diagnostics;
        TokenStream m_tokens;
        std::string_view m_source;
    };

} // namespace BootstrapForge::Architecture::Parsing
