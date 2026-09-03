#include <BootstrapForge/Architecture/Parsing/Parser.hpp>

#include <BootstrapForge/Architecture/AST/BuildNode.hpp>
#include <BootstrapForge/Architecture/AST/GeneratorNode.hpp>
#include <BootstrapForge/Architecture/AST/ModuleNode.hpp>
#include <BootstrapForge/Architecture/AST/ProjectNode.hpp>
#include <BootstrapForge/Architecture/AST/RelationshipNode.hpp>
#include <BootstrapForge/Architecture/AST/ResourceNode.hpp>

#include <BootstrapForge/Architecture/Grammar/TokenKind.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace BootstrapForge::Architecture::Parsing
{
    Parser::Parser(
        const Grammar::Grammar& grammar,
        std::string_view source)
        :
        m_grammar(grammar),
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
            declarations.emplace_back(
                ParseDeclaration());
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
            throw std::runtime_error(
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

        m_tokens.Expect(
            Grammar::TokenKind::KeywordProject);

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

        m_tokens.Expect(
            Grammar::TokenKind::KeywordModule);

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

        m_tokens.Expect(
            Grammar::TokenKind::KeywordRelationship);

        const std::string source =
            ParseIdentifier();

        m_tokens.Expect(
            Grammar::TokenKind::Arrow);

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

        m_tokens.Expect(
            Grammar::TokenKind::KeywordBuild);

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

        m_tokens.Expect(
            Grammar::TokenKind::KeywordGenerator);

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
        const Token& token =
            m_tokens.Expect(
                Grammar::TokenKind::Identifier);

        return token.Lexeme();
    }

    std::string Parser::ParseString()
    {
        const Token& token =
            m_tokens.Expect(
                Grammar::TokenKind::StringLiteral);

        const std::string& lexeme =
            token.Lexeme();

        if (lexeme.size() < 2)
        {
            return {};
        }

        return lexeme.substr(
            1,
            lexeme.size() - 2);
    }

    SourceSpan Parser::CurrentSpan() const noexcept
    {
        return m_tokens.Current().Span();
    }

} // namespace BootstrapForge::Architecture::Parsing