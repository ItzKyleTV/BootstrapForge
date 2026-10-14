#pragma once

#include <BootstrapForge/Architecture/AST/ArchitectureNode.hpp>

#include <BootstrapForge/Architecture/Diagnostics/DiagnosticEngine.hpp>

#include <BootstrapForge/Architecture/Grammar/Grammar.hpp>
#include <BootstrapForge/Architecture/Grammar/TokenKind.hpp>

#include <BootstrapForge/Architecture/Parsing/SourceSpan.hpp>
#include <BootstrapForge/Architecture/Parsing/TokenStream.hpp>

#include <memory>
#include <string>
#include <string_view>

namespace BootstrapForge::Architecture::Parsing
{
    class Parser final
    {
    public:

        Parser(
            const Grammar::Grammar& grammar,
            Diagnostics::DiagnosticEngine& diagnostics,
            std::string_view source);

        [[nodiscard]]
        std::unique_ptr<AST::ArchitectureNode> Parse();

    private:

        [[nodiscard]]
        std::unique_ptr<AST::Node>
            ParseDeclaration();

        [[nodiscard]]
        std::unique_ptr<AST::Node>
            ParseProject();

        [[nodiscard]]
        std::unique_ptr<AST::Node>
            ParseModule();

        [[nodiscard]]
        std::unique_ptr<AST::Node>
            ParseResource();

        [[nodiscard]]
        std::unique_ptr<AST::Node>
            ParseRelationship();

        [[nodiscard]]
        std::unique_ptr<AST::Node>
            ParseBuild();

        [[nodiscard]]
        std::unique_ptr<AST::Node>
            ParseGenerator();

        [[nodiscard]]
        std::string ParseIdentifier();

        [[nodiscard]]
        std::string ParseString();

        [[nodiscard]]
        SourceSpan CurrentSpan() const noexcept;

        void ReportSyntaxError(
            SourceSpan span,
            std::string message);

        [[nodiscard]]
        bool IsDeclarationStart(
            Grammar::TokenKind kind) const noexcept;

        void Synchronize();

        const Grammar::Grammar& m_grammar;

        Diagnostics::DiagnosticEngine& m_diagnostics;

        TokenStream m_tokens;
    };

} // namespace BootstrapForge::Architecture::Parsing
