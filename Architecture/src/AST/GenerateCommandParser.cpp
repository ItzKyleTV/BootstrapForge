#include <BootstrapForge/Architecture/Parsing/GenerateCommandParser.hpp>

#include <BootstrapForge/Architecture/Grammar/TokenKind.hpp>

#include <memory>
#include <cctype>
#include <string>
#include <utility>

namespace BootstrapForge::Architecture::Parsing
{
    using Grammar::TokenKind;

    GenerateCommandParser::ParseFailure::ParseFailure(
        SourceSpan span,
        std::string message)
        :
        std::runtime_error(std::move(message)),
        m_span(span)
    {
    }

    const SourceSpan&
        GenerateCommandParser::ParseFailure::Span() const noexcept
    {
        return m_span;
    }

    GenerateCommandParser::GenerateCommandParser(
        const Grammar::Grammar& grammar,
        Diagnostics::DiagnosticEngine& diagnostics,
        std::string_view source)
        :
        m_grammar(grammar),
        m_diagnostics(diagnostics),
        m_tokens(grammar, source),
        m_source(source)
    {
    }

    std::unique_ptr<AST::GenerateNode>
        GenerateCommandParser::Parse()
    {
        try
        {
            auto result = ParseGenerate();

            if (!m_tokens.IsAtEnd())
            {
                throw ParseFailure(
                    CurrentSpan(),
                    "Unexpected input after the generate command.");
            }

            return result;
        }
        catch (const ParseFailure& failure)
        {
            m_diagnostics.Error(
                failure.Span(),
                failure.what());

            return nullptr;
        }
    }

    std::unique_ptr<AST::GenerateNode>
        GenerateCommandParser::ParseGenerate()
    {
        const SourceLocation begin =
            m_tokens.Current().Span().Begin();

        if (!m_tokens.Consume(TokenKind::KeywordGenerate))
        {
            throw ParseFailure(
                CurrentSpan(),
                "Expected 'generate' as the command entry point.");
        }

        if (!m_tokens.Consume(TokenKind::LeftBrace))
        {
            throw ParseFailure(
                CurrentSpan(),
                "Expected '{' after 'generate'.");
        }

        auto generate = std::make_unique<AST::GenerateNode>(
            SourceSpan(begin, begin));

        while (!m_tokens.IsAtEnd() &&
            !m_tokens.Match(TokenKind::RightBrace))
        {
            const std::size_t before = m_tokens.Position();
            if (m_tokens.Match(TokenKind::KeywordFile))
            {
                generate->AddChild(ParseFile());
            }
            else
            {
                auto container = ParseContainer(ParentKind::Generate);
                if (container)
                {
                    generate->AddChild(std::move(container));
                }
            }

            if (m_tokens.Position() == before)
            {
                throw ParseFailure(
                    CurrentSpan(),
                    "Parser made no progress inside generate.");
            }
        }

        const SourceLocation end =
            m_tokens.Current().Span().End();

        if (!m_tokens.Consume(TokenKind::RightBrace))
        {
            throw ParseFailure(
                CurrentSpan(),
                "Expected '}' to close the generate command.");
        }

        generate->SetEndLocation(end);
        return generate;
    }

    std::unique_ptr<AST::GenerationContainerNode>
        GenerateCommandParser::ParseContainer(ParentKind parent)
    {
        const TokenKind startKind = m_tokens.Current().Kind();

        const bool allowed =
            (parent == ParentKind::Generate &&
                (startKind == TokenKind::KeywordBranch ||
                    startKind == TokenKind::KeywordFolder)) ||
            ((parent == ParentKind::Branch ||
                parent == ParentKind::Subbranch) &&
                (startKind == TokenKind::KeywordSubbranch ||
                    startKind == TokenKind::KeywordFolder ||
                    startKind == TokenKind::KeywordFile)) ||
            (parent == ParentKind::Folder &&
                (startKind == TokenKind::KeywordFolder ||
                    startKind == TokenKind::KeywordFile));

        if (!allowed)
        {
            throw ParseFailure(
                CurrentSpan(),
                "This declaration is not allowed at this level of the generate tree.");
        }

        const SourceLocation begin =
            m_tokens.Current().Span().Begin();
        const bool isBranch = startKind == TokenKind::KeywordBranch;
        const bool isSubbranch = startKind == TokenKind::KeywordSubbranch;
        m_tokens.Advance();

        if (!m_tokens.Consume(TokenKind::LeftParenthesis))
        {
            throw ParseFailure(
                CurrentSpan(),
                "Expected '(' after the container keyword.");
        }

        std::string name = ParseName();

        if (!m_tokens.Consume(TokenKind::RightParenthesis))
        {
            throw ParseFailure(
                CurrentSpan(),
                "Expected ')' after the container name.");
        }

        if (!m_tokens.Consume(TokenKind::LeftBrace))
        {
            throw ParseFailure(
                CurrentSpan(),
                "Expected '{' after the container name.");
        }

        const ParentKind childParent =
            isBranch
            ? ParentKind::Branch
            : (isSubbranch
                ? ParentKind::Subbranch
                : ParentKind::Folder);

        std::unique_ptr<AST::GenerationContainerNode> container;

        if (isBranch)
        {
            container = std::make_unique<AST::BranchNode>(
                std::move(name), SourceSpan(begin, begin));
        }
        else if (isSubbranch)
        {
            container = std::make_unique<AST::SubbranchNode>(
                std::move(name), SourceSpan(begin, begin));
        }
        else
        {
            container = std::make_unique<AST::FolderNode>(
                std::move(name), SourceSpan(begin, begin));
        }

        while (!m_tokens.IsAtEnd() &&
            !m_tokens.Match(TokenKind::RightBrace))
        {
            const std::size_t before = m_tokens.Position();
            if (m_tokens.Match(TokenKind::KeywordFile))
            {
                container->AddChild(ParseFile());
            }
            else
            {
                auto child = ParseContainer(childParent);
                container->AddChild(std::move(child));
            }

            if (m_tokens.Position() == before)
            {
                throw ParseFailure(
                    CurrentSpan(),
                    "Parser made no progress inside a container.");
            }
        }

        const SourceLocation end =
            m_tokens.Current().Span().End();

        if (!m_tokens.Consume(TokenKind::RightBrace))
        {
            throw ParseFailure(
                CurrentSpan(),
                "Expected '}' to close the container.");
        }

        container->SetEndLocation(end);
        return container;
    }

    std::unique_ptr<AST::FileNode>
        GenerateCommandParser::ParseFile()
    {
        const SourceLocation begin =
            m_tokens.Current().Span().Begin();
        m_tokens.Advance();

        if (!m_tokens.Consume(TokenKind::LeftParenthesis))
        {
            throw ParseFailure(
                CurrentSpan(),
                "Expected '(' after 'file'.");
        }

        std::string name = ParseName();

        if (!m_tokens.Consume(TokenKind::RightParenthesis))
        {
            throw ParseFailure(
                CurrentSpan(),
                "Expected ')' after the file name.");
        }

        std::optional<std::string> content;
        SourceLocation end = m_tokens.Current().Span().Begin();

        if (m_tokens.Consume(TokenKind::Equals))
        {
            if (!m_tokens.Match(TokenKind::RawStringLiteral))
            {
                throw ParseFailure(
                    CurrentSpan(),
                    "Expected a triple-quoted multiline value after '='.");
            }

            const Token& value = m_tokens.Current();
            const std::string& lexeme = value.Lexeme();
            if (lexeme.size() < 6 ||
                lexeme.compare(0, 3, "\"\"\"") != 0 ||
                lexeme.compare(lexeme.size() - 3, 3, "\"\"\"") != 0)
            {
                throw ParseFailure(
                    value.Span(),
                    "Unterminated triple-quoted file content.");
            }

            content = lexeme.substr(3, lexeme.size() - 6);
            end = value.Span().End();
            m_tokens.Advance();
        }

        return std::make_unique<AST::FileNode>(
            std::move(name),
            std::move(content),
            SourceSpan(begin, end));
    }

    std::string GenerateCommandParser::ParseName()
    {
        if (m_tokens.Match(TokenKind::StringLiteral))
        {
            const Token token = m_tokens.Current();
            m_tokens.Advance();

            if (token.Lexeme().size() < 2 ||
                token.Lexeme().front() != '"' ||
                token.Lexeme().back() != '"')
            {
                throw ParseFailure(
                    token.Span(),
                    "Unterminated quoted name.");
            }

            return token.Lexeme().substr(
                1, token.Lexeme().size() - 2);
        }

        if (!IsNameToken(m_tokens.Current().Kind()))
        {
            throw ParseFailure(
                CurrentSpan(),
                "Expected a non-empty unquoted or quoted name.");
        }

        std::size_t begin =
            m_tokens.Current().Span().Begin().Offset;
        std::size_t end = begin;
        while (IsNameToken(m_tokens.Current().Kind()))
        {
            end = m_tokens.Current().Span().End().Offset;
            m_tokens.Advance();
        }

        while (begin < end &&
            std::isspace(static_cast<unsigned char>(m_source[begin])) != 0)
        {
            ++begin;
        }
        while (end > begin &&
            std::isspace(static_cast<unsigned char>(m_source[end - 1])) != 0)
        {
            --end;
        }

        return std::string(m_source.substr(begin, end - begin));
    }

    bool GenerateCommandParser::IsNameToken(
        TokenKind kind) const noexcept
    {
        switch (kind)
        {
        case TokenKind::Identifier:
        case TokenKind::IntegerLiteral:
        case TokenKind::BooleanLiteral:
        case TokenKind::KeywordArchitecture:
        case TokenKind::KeywordGenerate:
        case TokenKind::KeywordProject:
        case TokenKind::KeywordModule:
        case TokenKind::KeywordDirectory:
        case TokenKind::KeywordFile:
        case TokenKind::KeywordBranch:
        case TokenKind::KeywordSubbranch:
        case TokenKind::KeywordFolder:
        case TokenKind::KeywordRelationship:
        case TokenKind::KeywordBuild:
        case TokenKind::KeywordGenerator:
        case TokenKind::KeywordCreate:
        case TokenKind::KeywordModify:
        case TokenKind::KeywordRemove:
        case TokenKind::KeywordRename:
        case TokenKind::KeywordMove:
        case TokenKind::KeywordCopy:
        case TokenKind::KeywordInclude:
        case TokenKind::Dot:
        case TokenKind::Minus:
            return true;
        default:
            return false;
        }
    }

    SourceSpan GenerateCommandParser::CurrentSpan() const noexcept
    {
        return m_tokens.Current().Span();
    }

} // namespace BootstrapForge::Architecture::Parsing
