#include <BootstrapForge/Architecture/Parsing/Parser.hpp>

#include <BootstrapForge/Architecture/AST/BuildNode.hpp>
#include <BootstrapForge/Architecture/AST/GeneratorNode.hpp>
#include <BootstrapForge/Architecture/AST/ModuleNode.hpp>
#include <BootstrapForge/Architecture/AST/ProjectNode.hpp>
#include <BootstrapForge/Architecture/AST/RelationshipNode.hpp>
#include <BootstrapForge/Architecture/AST/ResourceNode.hpp>

#include <BootstrapForge/Architecture/Grammar/TokenKind.hpp>

#include <exception>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace BootstrapForge::Architecture::Parsing
{
    namespace
    {
        class ParseFailure final : public std::exception
        {
        public:

            ParseFailure(
                SourceSpan span,
                std::string message)
                :
                m_span(span),
                m_message(std::move(message))
            {
            }

            [[nodiscard]]
            const SourceSpan& Span() const noexcept
            {
                return m_span;
            }

            [[nodiscard]]
            const char* what() const noexcept override
            {
                return m_message.c_str();
            }

        private:

            SourceSpan m_span;
            std::string m_message;
        };
    }

    Parser::Parser(
        const Grammar::Grammar& grammar,
        Diagnostics::DiagnosticEngine& diagnostics,
        std::string_view source)
        :
        m_grammar(grammar),
        m_diagnostics(diagnostics),
        m_tokens(grammar, source)
    {
    }

    std::unique_ptr<AST::ArchitectureNode>
        Parser::Parse()
    {
        const SourceLocation begin =
            m_tokens.Current().Span().Begin();

        std::vector<std::unique_ptr<AST::Node>> declarations;

        while (!m_tokens.IsAtEnd())
        {
            const std::size_t positionBefore =
                m_tokens.Position();

            try
            {
                auto declaration = ParseDeclaration();

                if (declaration != nullptr)
                {
                    declarations.emplace_back(
                        std::move(declaration));
                }
            }
            catch (const ParseFailure& failure)
            {
                ReportSyntaxError(
                    failure.Span(),
                    failure.what());

                Synchronize();
            }

            // Unsupported top-level tokens are consumed by Synchronize().
            // Each supported declaration parser consumes its declaration
            // keyword before it can report a malformed declaration, so a
            // supported start left by Synchronize() is the next iteration's
            // input and is never consumed as recovery debris.
            if (!m_tokens.IsAtEnd() &&
                m_tokens.Position() == positionBefore &&
                !IsDeclarationStart(m_tokens.Current().Kind()))
            {
                m_tokens.Advance();
            }
        }

        const SourceLocation end =
            m_tokens.Current().Span().End();

        auto architecture =
            std::make_unique<AST::ArchitectureNode>(
                SourceSpan(begin, end));

        for (auto& declaration : declarations)
        {
            architecture->AddChild(
                std::move(declaration));
        }

        return architecture;
    }

    std::unique_ptr<AST::Node>
        Parser::ParseDeclaration()
    {
        switch (m_tokens.Current().Kind())
        {
        case Grammar::TokenKind::KeywordProject:
            return ParseProject();

        case Grammar::TokenKind::KeywordModule:
            return ParseModule();

        case Grammar::TokenKind::KeywordDirectory:
        case Grammar::TokenKind::KeywordFile:
            return ParseResource();

        case Grammar::TokenKind::KeywordRelationship:
            return ParseRelationship();

        case Grammar::TokenKind::KeywordBuild:
            return ParseBuild();

        case Grammar::TokenKind::KeywordGenerator:
            return ParseGenerator();

        default:
            throw ParseFailure(
                CurrentSpan(),
                "Unexpected token '" +
                m_tokens.Current().Lexeme() +
                "' while parsing an architecture declaration.");
        }
    }

    std::unique_ptr<AST::Node>
        Parser::ParseProject()
    {
        const SourceLocation begin =
            m_tokens.Current().Span().Begin();

        m_tokens.Advance();

        const std::string name =
            ParseIdentifier();

        const SourceLocation end =
            m_tokens.Current().Span().Begin();

        return std::make_unique<AST::ProjectNode>(
            name,
            SourceSpan(begin, end));
    }

    std::unique_ptr<AST::Node>
        Parser::ParseModule()
    {
        const SourceLocation begin =
            m_tokens.Current().Span().Begin();

        m_tokens.Advance();

        const std::string name =
            ParseIdentifier();

        const SourceLocation end =
            m_tokens.Current().Span().Begin();

        return std::make_unique<AST::ModuleNode>(
            name,
            SourceSpan(begin, end));
    }

    std::unique_ptr<AST::Node>
        Parser::ParseResource()
    {
        const SourceLocation begin =
            m_tokens.Current().Span().Begin();

        const Grammar::TokenKind resourceKind =
            m_tokens.Current().Kind();

        m_tokens.Advance();

        std::string path;

        if (m_tokens.Match(
            Grammar::TokenKind::StringLiteral))
        {
            path = ParseString();
        }
        else
        {
            path = ParseIdentifier();
        }

        const SourceLocation end =
            m_tokens.Current().Span().Begin();

        return std::make_unique<AST::ResourceNode>(
            resourceKind,
            std::move(path),
            SourceSpan(begin, end));
    }

    std::unique_ptr<AST::Node>
        Parser::ParseRelationship()
    {
        const SourceLocation begin =
            m_tokens.Current().Span().Begin();

        m_tokens.Advance();

        const std::string source =
            ParseIdentifier();

        if (!m_tokens.Consume(
            Grammar::TokenKind::Arrow))
        {
            throw ParseFailure(
                CurrentSpan(),
                "Expected '->' in relationship declaration.");
        }

        const std::string target =
            ParseIdentifier();

        const SourceLocation end =
            m_tokens.Current().Span().Begin();

        return std::make_unique<AST::RelationshipNode>(
            source,
            target,
            SourceSpan(begin, end));
    }

    std::unique_ptr<AST::Node>
        Parser::ParseBuild()
    {
        const SourceLocation begin =
            m_tokens.Current().Span().Begin();

        m_tokens.Advance();

        const std::string target =
            ParseIdentifier();

        const SourceLocation end =
            m_tokens.Current().Span().Begin();

        return std::make_unique<AST::BuildNode>(
            target,
            SourceSpan(begin, end));
    }

    std::unique_ptr<AST::Node>
        Parser::ParseGenerator()
    {
        const SourceLocation begin =
            m_tokens.Current().Span().Begin();

        m_tokens.Advance();

        const std::string generator =
            ParseIdentifier();

        const std::string target =
            ParseIdentifier();

        const SourceLocation end =
            m_tokens.Current().Span().Begin();

        return std::make_unique<AST::GeneratorNode>(
            generator,
            target,
            SourceSpan(begin, end));
    }

    std::string Parser::ParseIdentifier()
    {
        if (!m_tokens.Match(
            Grammar::TokenKind::Identifier))
        {
            throw ParseFailure(
                CurrentSpan(),
                "Expected an identifier, but found '" +
                m_tokens.Current().Lexeme() +
                "'.");
        }

        const std::string identifier =
            m_tokens.Current().Lexeme();

        m_tokens.Advance();

        return identifier;
    }

    std::string Parser::ParseString()
    {
        if (!m_tokens.Match(
            Grammar::TokenKind::StringLiteral))
        {
            throw ParseFailure(
                CurrentSpan(),
                "Expected a string literal, but found '" +
                m_tokens.Current().Lexeme() +
                "'.");
        }

        const Token& token = m_tokens.Current();
        const SourceSpan literalSpan = token.Span();
        const std::string lexeme = token.Lexeme();

        m_tokens.Advance();

        if (lexeme.size() < 2 ||
            lexeme.front() != '"' ||
            lexeme.back() != '"')
        {
            throw ParseFailure(
                literalSpan,
                "Invalid string literal.");
        }

        return lexeme.substr(
            1,
            lexeme.size() - 2);
    }

    SourceSpan Parser::CurrentSpan() const noexcept
    {
        return m_tokens.Current().Span();
    }

    void Parser::ReportSyntaxError(
        SourceSpan span,
        std::string message)
    {
        m_diagnostics.Error(
            span,
            std::move(message));
    }

    bool Parser::IsDeclarationStart(
        Grammar::TokenKind kind) const noexcept
    {
        switch (kind)
        {
        case Grammar::TokenKind::KeywordProject:
        case Grammar::TokenKind::KeywordModule:
        case Grammar::TokenKind::KeywordDirectory:
        case Grammar::TokenKind::KeywordFile:
        case Grammar::TokenKind::KeywordRelationship:
        case Grammar::TokenKind::KeywordBuild:
        case Grammar::TokenKind::KeywordGenerator:
            return true;

        default:
            return false;
        }
    }

    void Parser::Synchronize()
    {
        // The current token may already be the next valid declaration,
        // for example: "project module Core".
        if (m_tokens.IsAtEnd() ||
            IsDeclarationStart(m_tokens.Current().Kind()))
        {
            return;
        }

        // Skip the malformed declaration and any unsupported tokens
        // until another supported declaration begins or EOF is reached.
        while (!m_tokens.IsAtEnd() &&
               !IsDeclarationStart(m_tokens.Current().Kind()))
        {
            m_tokens.Advance();
        }
    }

} // namespace BootstrapForge::Architecture::Parsing
