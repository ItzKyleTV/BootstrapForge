#include <BootstrapForge/Architecture/Parsing/Lexer.hpp>

#include <string>
#include <string_view>
#include <utility>

namespace BootstrapForge::Architecture::Parsing
{
    Lexer::Lexer(
        const Grammar::Grammar& grammar,
        std::string_view source) noexcept
        :
        m_grammar(grammar),
        m_reader(source)
    {
    }

    Token Lexer::NextToken()
    {
        SkipWhitespace();

        while (!m_reader.IsEnd())
        {
            if (m_reader.Match("//"))
            {
                SkipLineComment();
                SkipWhitespace();
                continue;
            }

            break;
        }

        if (m_reader.IsEnd())
        {
            const SourceLocation location =
                m_reader.Location();

            return MakeToken(
                Grammar::TokenKind::EndOfFile,
                location);
        }

        const char character =
            m_reader.Current();

        if (IsIdentifierStart(character))
        {
            return ReadIdentifier();
        }

        if (IsDigit(character))
        {
            return ReadNumber();
        }

        if (character == '"')
        {
            if (m_reader.Match("\"\"\""))
            {
                return ReadRawString();
            }

            return ReadString();
        }

        return ReadOperator();
    }

    void Lexer::SkipWhitespace()
    {
        while (!m_reader.IsEnd() &&
            IsWhitespace(m_reader.Current()))
        {
            m_reader.Advance();
        }
    }

    void Lexer::SkipLineComment()
    {
        if (!m_reader.Match("//"))
        {
            return;
        }

        m_reader.Advance(2);

        while (!m_reader.IsEnd())
        {
            const char character =
                m_reader.Current();

            if (character == '\n')
            {
                return;
            }

            m_reader.Advance();
        }
    }

    Token Lexer::ReadIdentifier()
    {
        const SourceLocation begin =
            m_reader.Location();

        while (!m_reader.IsEnd() &&
            IsIdentifierPart(m_reader.Current()))
        {
            m_reader.Advance();
        }

        const std::string_view lexeme =
            m_reader.Source().substr(
                begin.Offset,
                m_reader.Offset() - begin.Offset);

        if (lexeme == "true" || lexeme == "false")
        {
            return MakeToken(
                Grammar::TokenKind::BooleanLiteral,
                begin,
                Grammar::LiteralKind::Boolean);
        }

        const auto keyword =
            m_grammar.Keywords().Find(lexeme);

        if (keyword.has_value())
        {
            return MakeToken(
                keyword->Token(),
                begin);
        }

        return MakeToken(
            Grammar::TokenKind::Identifier,
            begin);
    }
    Token Lexer::ReadNumber()
    {
        const SourceLocation begin =
            m_reader.Location();

        while (!m_reader.IsEnd() &&
            IsDigit(m_reader.Current()))
        {
            m_reader.Advance();
        }

        bool isFloatingPoint = false;

        if (!m_reader.IsEnd() &&
            m_reader.Current() == '.' &&
            IsDigit(m_reader.Peek()))
        {
            isFloatingPoint = true;

            m_reader.Advance();

            while (!m_reader.IsEnd() &&
                IsDigit(m_reader.Current()))
            {
                m_reader.Advance();
            }
        }

        return MakeToken(
            isFloatingPoint
            ? Grammar::TokenKind::FloatLiteral
            : Grammar::TokenKind::IntegerLiteral,
            begin,
            isFloatingPoint
            ? Grammar::LiteralKind::FloatingPoint
            : Grammar::LiteralKind::Integer);
    }

    Token Lexer::ReadString()
    {
        const SourceLocation begin =
            m_reader.Location();

        m_reader.Advance();

        while (!m_reader.IsEnd())
        {
            const char character =
                m_reader.Current();

            if (character == '"')
            {
                m_reader.Advance();
                break;
            }

            if (character == '\\')
            {
                m_reader.Advance();

                if (!m_reader.IsEnd())
                {
                    m_reader.Advance();
                }

                continue;
            }

            if (character == '\n')
            {
                break;
            }

            m_reader.Advance();
        }

        return MakeToken(
            Grammar::TokenKind::StringLiteral,
            begin,
            Grammar::LiteralKind::String);
    }

    Token Lexer::ReadRawString()
    {
        const SourceLocation begin =
            m_reader.Location();

        m_reader.Advance(3);

        while (!m_reader.IsEnd() &&
            !m_reader.Match("\"\"\""))
        {
            m_reader.Advance();
        }

        if (m_reader.Match("\"\"\""))
        {
            m_reader.Advance(3);
        }

        return MakeToken(
            Grammar::TokenKind::RawStringLiteral,
            begin,
            Grammar::LiteralKind::String);
    }

    Token Lexer::ReadOperator()
    {
        const SourceLocation begin =
            m_reader.Location();

        const std::size_t remaining =
            m_reader.Source().size() -
            m_reader.Offset();

        if (remaining >= 2)
        {
            const std::string_view twoCharacterOperator =
                m_reader.Source().substr(
                    m_reader.Offset(),
                    2);

            const auto operatorValue =
                m_grammar.Operators().Find(
                    twoCharacterOperator);

            if (operatorValue.has_value())
            {
                const Grammar::Operator& op =
                    *operatorValue;

                m_reader.Advance(2);

                return MakeToken(
                    op.Token(),
                    begin,
                    Grammar::LiteralKind::None,
                    op);
            }
        }

        const std::string_view oneCharacterOperator =
            m_reader.Source().substr(
                m_reader.Offset(),
                1);

        const auto operatorValue =
            m_grammar.Operators().Find(
                oneCharacterOperator);

        if (operatorValue.has_value())
        {
            const Grammar::Operator& op =
                *operatorValue;

            m_reader.Advance();

            return MakeToken(
                op.Token(),
                begin,
                Grammar::LiteralKind::None,
                op);
        }

        return ReadUnknown();
    }

    Token Lexer::ReadUnknown()
    {
        const SourceLocation begin =
            m_reader.Location();

        m_reader.Advance();

        return MakeToken(
            Grammar::TokenKind::Unknown,
            begin);
    }

    Token Lexer::MakeToken(
        Grammar::TokenKind kind,
        SourceLocation begin,
        Grammar::LiteralKind literalKind,
        Grammar::Operator op)
    {
        const std::size_t length =
            m_reader.Offset() -
            begin.Offset;

        const std::string lexeme =
            std::string(
                m_reader.Source().substr(
                    begin.Offset,
                    length));

        const SourceLocation end =
            m_reader.Location();

        return Token(
            kind,
            lexeme,
            SourceSpan(
                begin,
                end),
            literalKind,
            op);
    }

    bool Lexer::IsIdentifierStart(
        char character) noexcept
    {
        return
            (character >= 'a' && character <= 'z') ||
            (character >= 'A' && character <= 'Z') ||
            character == '_';
    }

    bool Lexer::IsIdentifierPart(
        char character) noexcept
    {
        return
            IsIdentifierStart(character) ||
            IsDigit(character);
    }

    bool Lexer::IsDigit(
        char character) noexcept
    {
        return
            character >= '0' &&
            character <= '9';
    }

    bool Lexer::IsWhitespace(
        char character) noexcept
    {
        return
            character == ' ' ||
            character == '\t' ||
            character == '\r' ||
            character == '\n' ||
            character == '\f' ||
            character == '\v';
    }

} // namespace BootstrapForge::Architecture::Parsing
