
#include <BootstrapForge/Client/Editor/CppSyntaxHighlighter.hpp>

#include <QColor>
#include <QTextDocument>

namespace BootstrapForge::Client::Editor
{
    CppSyntaxHighlighter::CppSyntaxHighlighter(
        QTextDocument* document)
        :
        QSyntaxHighlighter(document)
    {
        ConfigureRules();
    }

    void CppSyntaxHighlighter::AddRule(
        const QString& pattern,
        const QColor& color,
        bool bold,
        bool italic)
    {
        QTextCharFormat format;
        format.setForeground(color);

        if (bold)
        {
            format.setFontWeight(QFont::Bold);
        }

        if (italic)
        {
            format.setFontItalic(true);
        }

        m_rules.append({
            QRegularExpression(pattern),
            format
            });
    }

    void CppSyntaxHighlighter::ConfigureRules()
    {
        // Theme: dark editor with readable, restrained colors.
        const QColor keywordColor(86, 156, 214);
        const QColor typeColor(78, 201, 176);
        const QColor functionColor(220, 220, 170);
        const QColor numberColor(181, 206, 168);
        const QColor commentColor(106, 153, 85);
        const QColor stringColor(206, 145, 120);
        const QColor preprocessorColor(197, 134, 192);
        const QColor operatorColor(212, 212, 212);

        const QStringList keywords = {
            QStringLiteral("alignas"),
            QStringLiteral("alignof"),
            QStringLiteral("asm"),
            QStringLiteral("auto"),
            QStringLiteral("bool"),
            QStringLiteral("break"),
            QStringLiteral("case"),
            QStringLiteral("catch"),
            QStringLiteral("char"),
            QStringLiteral("char8_t"),
            QStringLiteral("char16_t"),
            QStringLiteral("char32_t"),
            QStringLiteral("class"),
            QStringLiteral("concept"),
            QStringLiteral("const"),
            QStringLiteral("consteval"),
            QStringLiteral("constexpr"),
            QStringLiteral("constinit"),
            QStringLiteral("const_cast"),
            QStringLiteral("continue"),
            QStringLiteral("co_await"),
            QStringLiteral("co_return"),
            QStringLiteral("co_yield"),
            QStringLiteral("decltype"),
            QStringLiteral("default"),
            QStringLiteral("delete"),
            QStringLiteral("do"),
            QStringLiteral("double"),
            QStringLiteral("dynamic_cast"),
            QStringLiteral("else"),
            QStringLiteral("enum"),
            QStringLiteral("explicit"),
            QStringLiteral("export"),
            QStringLiteral("extern"),
            QStringLiteral("false"),
            QStringLiteral("float"),
            QStringLiteral("for"),
            QStringLiteral("friend"),
            QStringLiteral("goto"),
            QStringLiteral("if"),
            QStringLiteral("inline"),
            QStringLiteral("int"),
            QStringLiteral("long"),
            QStringLiteral("mutable"),
            QStringLiteral("namespace"),
            QStringLiteral("new"),
            QStringLiteral("noexcept"),
            QStringLiteral("nullptr"),
            QStringLiteral("operator"),
            QStringLiteral("override"),
            QStringLiteral("private"),
            QStringLiteral("protected"),
            QStringLiteral("public"),
            QStringLiteral("register"),
            QStringLiteral("reinterpret_cast"),
            QStringLiteral("requires"),
            QStringLiteral("return"),
            QStringLiteral("short"),
            QStringLiteral("signed"),
            QStringLiteral("sizeof"),
            QStringLiteral("static"),
            QStringLiteral("static_assert"),
            QStringLiteral("static_cast"),
            QStringLiteral("struct"),
            QStringLiteral("switch"),
            QStringLiteral("template"),
            QStringLiteral("this"),
            QStringLiteral("thread_local"),
            QStringLiteral("throw"),
            QStringLiteral("true"),
            QStringLiteral("try"),
            QStringLiteral("typedef"),
            QStringLiteral("typeid"),
            QStringLiteral("typename"),
            QStringLiteral("union"),
            QStringLiteral("unsigned"),
            QStringLiteral("using"),
            QStringLiteral("virtual"),
            QStringLiteral("void"),
            QStringLiteral("volatile"),
            QStringLiteral("wchar_t"),
            QStringLiteral("while")
        };

        for (const QString& keyword : keywords)
        {
            AddRule(
                QStringLiteral("\\b%1\\b")
                .arg(
                    QRegularExpression::escape(keyword)),
                keywordColor,
                true);
        }

        const QStringList types = {
            QStringLiteral("QString"),
            QStringLiteral("QStringView"),
            QStringLiteral("QByteArray"),
            QStringLiteral("QVector"),
            QStringLiteral("QList"),
            QStringLiteral("QMap"),
            QStringLiteral("QHash"),
            QStringLiteral("QSet"),
            QStringLiteral("QPair"),
            QStringLiteral("QPoint"),
            QStringLiteral("QPointF"),
            QStringLiteral("QRect"),
            QStringLiteral("QRectF"),
            QStringLiteral("QSize"),
            QStringLiteral("QSizeF"),
            QStringLiteral("QObject"),
            QStringLiteral("QWidget"),
            QStringLiteral("QMainWindow"),
            QStringLiteral("QApplication"),
            QStringLiteral("QDialog"),
            QStringLiteral("QFile"),
            QStringLiteral("QFileInfo"),
            QStringLiteral("QSaveFile"),
            QStringLiteral("QTextDocument"),
            QStringLiteral("QTextCursor"),
            QStringLiteral("QTextBlock"),
            QStringLiteral("QTextCharFormat"),
            QStringLiteral("QSyntaxHighlighter"),
            QStringLiteral("QRegularExpression"),
            QStringLiteral("QVariant"),
            QStringLiteral("QSharedPointer"),
            QStringLiteral("QUniquePointer"),
            QStringLiteral("std::string"),
            QStringLiteral("std::string_view"),
            QStringLiteral("std::vector"),
            QStringLiteral("std::array"),
            QStringLiteral("std::optional"),
            QStringLiteral("std::expected"),
            QStringLiteral("std::unique_ptr"),
            QStringLiteral("std::shared_ptr"),
            QStringLiteral("std::unordered_map"),
            QStringLiteral("std::unordered_set")
        };

        for (const QString& type : types)
        {
            AddRule(
                QStringLiteral("\\b%1\\b")
                .arg(
                    QRegularExpression::escape(type)),
                typeColor);
        }

        // Function identifiers.
        AddRule(
            QStringLiteral(
                "\\b[A-Za-z_][A-Za-z0-9_]*"
                "(?=\\s*\\()"),
            functionColor);

        // Numeric literals, including hexadecimal, binary,
        // floating-point values, and common suffixes.
        AddRule(
            QStringLiteral(
                "\\b(?:"
                "0[xX][0-9A-Fa-f']+"
                "|0[bB][01']+"
                "|(?:[0-9][0-9']*)"
                "(?:\\.[0-9']*)?"
                "(?:[eE][+-]?[0-9']+)?"
                ")"
                "[uUlLfF]*\\b"),
            numberColor);

        // Preprocessor directive names.
        AddRule(
            QStringLiteral(
                "^[ \\t]*#[ \\t]*"
                "[A-Za-z_][A-Za-z0-9_]*"),
            preprocessorColor,
            true);

        m_lineCommentFormat.setForeground(commentColor);
        m_lineCommentFormat.setFontItalic(true);

        m_blockCommentFormat.setForeground(commentColor);
        m_blockCommentFormat.setFontItalic(true);

        m_stringFormat.setForeground(stringColor);
        m_characterFormat.setForeground(stringColor);

        m_preprocessorFormat.setForeground(preprocessorColor);
        m_preprocessorFormat.setFontWeight(QFont::Bold);

        Q_UNUSED(operatorColor);
    }

    void CppSyntaxHighlighter::highlightBlock(
        const QString& text)
    {
        // Apply regular token rules first.
        for (const HighlightingRule& rule : m_rules)
        {
            QRegularExpressionMatchIterator matches =
                rule.pattern.globalMatch(text);

            while (matches.hasNext())
            {
                const QRegularExpressionMatch match =
                    matches.next();

                setFormat(
                    match.capturedStart(),
                    match.capturedLength(),
                    rule.format);
            }
        }

        // Strings: escaped characters are included.
        const QRegularExpression stringPattern(
            QStringLiteral(
                "\"(?:\\\\.|[^\"\\\\])*\""));

        auto stringMatches =
            stringPattern.globalMatch(text);

        while (stringMatches.hasNext())
        {
            const auto match = stringMatches.next();

            setFormat(
                match.capturedStart(),
                match.capturedLength(),
                m_stringFormat);
        }

        const QRegularExpression characterPattern(
            QStringLiteral(
                "'(?:\\\\.|[^'\\\\])'"));

        auto characterMatches =
            characterPattern.globalMatch(text);

        while (characterMatches.hasNext())
        {
            const auto match = characterMatches.next();

            setFormat(
                match.capturedStart(),
                match.capturedLength(),
                m_characterFormat);
        }

        // Single-line comments.
        const QRegularExpression lineCommentPattern(
            QStringLiteral("//.*"));

        auto lineCommentMatches =
            lineCommentPattern.globalMatch(text);

        while (lineCommentMatches.hasNext())
        {
            const auto match = lineCommentMatches.next();

            setFormat(
                match.capturedStart(),
                match.capturedLength(),
                m_lineCommentFormat);
        }

        // Block comments can continue across text blocks.
        const QRegularExpression commentStart(
            QStringLiteral("/\\*"));

        const QRegularExpression commentEnd(
            QStringLiteral("\\*/"));

        int start = previousBlockState() == 1
            ? 0
            : text.indexOf(commentStart);

        setCurrentBlockState(0);

        while (start >= 0)
        {
            const QRegularExpressionMatch endMatch =
                commentEnd.match(
                    text,
                    start + 2);

            int length = 0;

            if (!endMatch.hasMatch())
            {
                setCurrentBlockState(1);
                length = text.length() - start;
            }
            else
            {
                length =
                    endMatch.capturedEnd() - start;
            }

            setFormat(
                start,
                length,
                m_blockCommentFormat);

            if (!endMatch.hasMatch())
            {
                break;
            }

            start = text.indexOf(
                commentStart,
                start + length);
        }
    }
}
