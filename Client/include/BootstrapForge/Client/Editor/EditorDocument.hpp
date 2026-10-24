#pragma once

#include <QPlainTextEdit>

#include <QByteArray>
#include <QPointF>
#include <QString>

class QResizeEvent;
class QWidget;

namespace BootstrapForge::Client::Editor
{
    class EditorDocument final : public QPlainTextEdit
    {
        Q_OBJECT

    public:

        explicit EditorDocument(
            QWidget* parent = nullptr);

        ~EditorDocument() override = default;

        [[nodiscard]]
        const QString& FilePath() const noexcept;

        [[nodiscard]]
        bool LoadFromFile(
            const QString& path,
            QString& errorMessage);

        [[nodiscard]]
        bool SaveToFile(
            QString& errorMessage);

        [[nodiscard]]
        bool IsModified() const noexcept;

        void ApplyEditorSettings();

        [[nodiscard]]
        int LineNumberAreaWidth() const;

        [[nodiscard]]
        QPointF ContentOffset() const noexcept;

    protected:

        void resizeEvent(
            QResizeEvent* event) override;

    private:

        [[nodiscard]]
        static bool IsLikelyBinary(
            const QByteArray& data);

        void UpdateLineNumberAreaWidth(
            int newBlockCount);

        void UpdateLineNumberArea(
            const QRect& rect,
            int verticalScrollDistance);

        void HighlightCurrentLine();

        void ConfigureEditor();

        QString m_filePath;

        QByteArray m_lineEnding =
            QByteArray("\n");

        bool m_hasUtf8Bom = false;
        bool m_showLineNumbers = true;

        QWidget* m_lineNumberArea = nullptr;
    };

} // namespace BootstrapForge::Client::Editor
