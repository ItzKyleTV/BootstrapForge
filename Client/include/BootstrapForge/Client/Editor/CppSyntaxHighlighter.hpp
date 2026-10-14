
#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>

class QTextDocument;

namespace BootstrapForge::Client::Editor
{
    class CppSyntaxHighlighter final : public QSyntaxHighlighter
    {
    public:
        explicit CppSyntaxHighlighter(
            QTextDocument* document);

    protected:
        void highlightBlock(
            const QString& text) override;

    private:
        struct HighlightingRule
        {
            QRegularExpression pattern;
            QTextCharFormat format;
        };

        void AddRule(
            const QString& pattern,
            const QColor& color,
            bool bold = false,
            bool italic = false);

        void ConfigureRules();

        QVector<HighlightingRule> m_rules;

        QTextCharFormat m_lineCommentFormat;
        QTextCharFormat m_blockCommentFormat;
        QTextCharFormat m_stringFormat;
        QTextCharFormat m_characterFormat;
        QTextCharFormat m_preprocessorFormat;
    };
}
