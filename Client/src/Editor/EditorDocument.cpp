#include <BootstrapForge/Client/Editor/EditorDocument.hpp>

#include <QFile>
#include <QFileInfo>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QPainter>
#include <QResizeEvent>
#include <QSaveFile>
#include <QSettings>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextEdit>
#include <QWidget>

#include <algorithm>

namespace BootstrapForge::Client::Editor
{
    namespace
    {
        constexpr qint64 MaximumEditableFileSize =
            16 * 1024 * 1024;

        constexpr char Utf8Bom[] =
            "\xEF\xBB\xBF";

        class LineNumberArea final : public QWidget
        {
        public:

            explicit LineNumberArea(
                EditorDocument& editor)
                :
                QWidget(&editor),
                m_editor(editor)
            {
                setObjectName(
                    QStringLiteral(
                        "LineNumberArea"));
            }

            [[nodiscard]]
            QSize sizeHint() const override
            {
                return QSize(
                    m_editor.LineNumberAreaWidth(),
                    0);
            }

        protected:

            void paintEvent(
                QPaintEvent* event) override
            {
                QPainter painter(this);

                painter.fillRect(
                    event->rect(),
                    QColor(
                        37,
                        37,
                        38));

                const QRect clipRect =
                    event->rect();

                painter.setClipRect(
                    clipRect);

                QTextDocument* document =
                    m_editor.document();

                QTextBlock block =
                    document->firstBlock();

                const int firstBlockNumber =
                    block.blockNumber();

                Q_UNUSED(firstBlockNumber);

                const QPointF contentOffset =
                    m_editor.ContentOffset();

                const int fontHeight =
                    m_editor.fontMetrics().height();

                int blockNumber = 1;

                for (;
                    block.isValid();
                    block = block.next(),
                    ++blockNumber)
                {
                    const QRectF blockGeometry =
                        document->documentLayout()
                        ->blockBoundingRect(block);

                    const QRectF translatedGeometry =
                        blockGeometry.translated(
                            contentOffset);

                    const int top =
                        qRound(
                            translatedGeometry.top());

                    const int bottom =
                        qRound(
                            translatedGeometry.bottom());

                    if (bottom < clipRect.top())
                    {
                        continue;
                    }

                    if (top > clipRect.bottom())
                    {
                        break;
                    }

                    const QString number =
                        QString::number(
                            blockNumber);

                    const QRect textRect(
                        0,
                        top,
                        width() - 8,
                        fontHeight);

                    const QTextCursor cursor =
                        m_editor.textCursor();

                    const int currentBlockNumber =
                        cursor.blockNumber() + 1;

                    const bool isCurrentLine =
                        blockNumber ==
                        currentBlockNumber;

                    painter.setPen(
                        isCurrentLine
                        ? QColor(
                            220,
                            220,
                            220)
                        : QColor(
                            120,
                            120,
                            120));

                    painter.drawText(
                        textRect,
                        Qt::AlignRight |
                        Qt::AlignVCenter,
                        number);
                }
            }

        private:

            EditorDocument& m_editor;
        };
    }

    EditorDocument::EditorDocument(
        QWidget* parent)
        :
        QPlainTextEdit(parent)
    {
        ConfigureEditor();

        m_lineNumberArea =
            new LineNumberArea(
                *this);
        m_lineNumberArea->setVisible(m_showLineNumbers);

        connect(
            this,
            &QPlainTextEdit::blockCountChanged,
            this,
            &EditorDocument::UpdateLineNumberAreaWidth);

        connect(
            this,
            &QPlainTextEdit::updateRequest,
            this,
            &EditorDocument::UpdateLineNumberArea);

        connect(
            this,
            &QPlainTextEdit::cursorPositionChanged,
            this,
            &EditorDocument::HighlightCurrentLine);

        UpdateLineNumberAreaWidth(
            blockCount());

        HighlightCurrentLine();
    }

    const QString&
        EditorDocument::FilePath() const noexcept
    {
        return m_filePath;
    }

    bool EditorDocument::LoadFromFile(
        const QString& path,
        QString& errorMessage)
    {
        const QFileInfo fileInfo(path);

        if (!fileInfo.exists())
        {
            errorMessage =
                QStringLiteral(
                    "File does not exist: %1")
                .arg(path);

            return false;
        }

        if (!fileInfo.isFile())
        {
            errorMessage =
                QStringLiteral(
                    "Path is not a regular file: %1")
                .arg(path);

            return false;
        }

        if (!fileInfo.isReadable())
        {
            errorMessage =
                QStringLiteral(
                    "File is not readable: %1")
                .arg(path);

            return false;
        }

        if (fileInfo.size() >
            MaximumEditableFileSize)
        {
            errorMessage =
                QStringLiteral(
                    "File is larger than the "
                    "16 MiB editor limit: %1")
                .arg(path);

            return false;
        }

        QFile file(path);

        if (!file.open(
            QIODevice::ReadOnly))
        {
            errorMessage =
                QStringLiteral(
                    "Could not open file for reading: %1")
                .arg(
                    file.errorString());

            return false;
        }

        QByteArray fileData =
            file.readAll();

        if (file.error() !=
            QFileDevice::NoError)
        {
            errorMessage =
                QStringLiteral(
                    "Could not read file: %1")
                .arg(
                    file.errorString());

            return false;
        }

        if (IsLikelyBinary(fileData))
        {
            errorMessage =
                QStringLiteral(
                    "The file appears to contain "
                    "binary data and cannot be opened "
                    "in the text editor.");

            return false;
        }

        m_hasUtf8Bom =
            fileData.startsWith(
                QByteArray(
                    Utf8Bom,
                    3));

        if (m_hasUtf8Bom)
        {
            fileData.remove(
                0,
                3);
        }

        if (fileData.contains(
            QByteArray("\r\n")))
        {
            m_lineEnding =
                QByteArray("\r\n");
        }
        else if (fileData.contains(
            QByteArray("\r")))
        {
            m_lineEnding =
                QByteArray("\r");
        }
        else
        {
            m_lineEnding =
                QByteArray("\n");
        }

        const QString text =
            QString::fromUtf8(
                fileData);

        setPlainText(
            text);

        m_filePath =
            fileInfo.absoluteFilePath();

        document()->setModified(
            false);

        HighlightCurrentLine();

        errorMessage.clear();

        return true;
    }

    bool EditorDocument::SaveToFile(
        QString& errorMessage)
    {
        if (m_filePath.isEmpty())
        {
            errorMessage =
                QStringLiteral(
                    "The editor document has no file path.");

            return false;
        }

        QString normalizedText =
            toPlainText();

        QByteArray fileData =
            normalizedText.toUtf8();

        if (m_lineEnding ==
            QByteArray("\r\n"))
        {
            fileData.replace(
                QByteArray("\n"),
                QByteArray("\r\n"));
        }
        else if (m_lineEnding ==
            QByteArray("\r"))
        {
            fileData.replace(
                QByteArray("\n"),
                QByteArray("\r"));
        }

        if (m_hasUtf8Bom)
        {
            fileData.prepend(
                QByteArray(
                    Utf8Bom,
                    3));
        }

        QSaveFile file(
            m_filePath);

        file.setDirectWriteFallback(
            true);

        if (!file.open(
            QIODevice::WriteOnly))
        {
            errorMessage =
                QStringLiteral(
                    "Could not open file for saving: %1")
                .arg(
                    file.errorString());

            return false;
        }

        const qint64 bytesWritten =
            file.write(fileData);

        if (bytesWritten !=
            fileData.size())
        {
            errorMessage =
                QStringLiteral(
                    "Could not write the complete file: %1")
                .arg(
                    file.errorString());

            return false;
        }

        if (!file.commit())
        {
            errorMessage =
                QStringLiteral(
                    "Could not commit file changes: %1")
                .arg(
                    file.errorString());

            return false;
        }

        document()->setModified(
            false);

        errorMessage.clear();

        return true;
    }

    bool EditorDocument::IsModified() const noexcept
    {
        return document()->isModified();
    }

    int EditorDocument::LineNumberAreaWidth() const
    {
        if (!m_showLineNumbers)
        {
            return 0;
        }

        int digits = 1;

        int maximum =
            std::max(
                1,
                blockCount());

        while (maximum >= 10)
        {
            maximum /= 10;
            ++digits;
        }

        const int digitWidth =
            fontMetrics().horizontalAdvance(
                QLatin1Char('9'));

        return
            12 +
            digitWidth * digits;
    }

    QPointF EditorDocument::ContentOffset() const noexcept
    {
        return contentOffset();
    }

    void EditorDocument::resizeEvent(
        QResizeEvent* event)
    {
        QPlainTextEdit::resizeEvent(
            event);

        if (m_lineNumberArea == nullptr)
        {
            return;
        }

        const QRect editorContentsRect =
            QPlainTextEdit::contentsRect();

        m_lineNumberArea->setGeometry(
            QRect(
                editorContentsRect.left(),
                editorContentsRect.top(),
                LineNumberAreaWidth(),
                editorContentsRect.height()));
    }

    bool EditorDocument::IsLikelyBinary(
        const QByteArray& data)
    {
        if (data.isEmpty())
        {
            return false;
        }

        const qsizetype sampleSize =
            std::min<qsizetype>(
                data.size(),
                8192);

        for (qsizetype index = 0;
            index < sampleSize;
            ++index)
        {
            if (data.at(index) == '\0')
            {
                return true;
            }
        }

        return false;
    }

    void EditorDocument::UpdateLineNumberAreaWidth(
        int)
    {
        setViewportMargins(
            LineNumberAreaWidth(),
            0,
            0,
            0);

        if (m_lineNumberArea != nullptr)
        {
            const QRect editorContentsRect =
                QPlainTextEdit::contentsRect();

            m_lineNumberArea->setGeometry(
                QRect(
                    editorContentsRect.left(),
                    editorContentsRect.top(),
                    LineNumberAreaWidth(),
                    editorContentsRect.height()));
        }
    }

    void EditorDocument::UpdateLineNumberArea(
        const QRect& rect,
        int verticalScrollDistance)
    {
        if (m_lineNumberArea == nullptr)
        {
            return;
        }

        if (verticalScrollDistance != 0)
        {
            m_lineNumberArea->scroll(
                0,
                verticalScrollDistance);
        }
        else
        {
            m_lineNumberArea->update(
                0,
                rect.y(),
                m_lineNumberArea->width(),
                rect.height());
        }

        if (rect.contains(
            viewport()->rect()))
        {
            UpdateLineNumberAreaWidth(
                0);
        }
    }

    void EditorDocument::HighlightCurrentLine()
    {
        QList<QTextEdit::ExtraSelection>
            selections;

        if (!isReadOnly())
        {
            QTextEdit::ExtraSelection selection;

            selection.format.setBackground(
                QColor(
                    45,
                    45,
                    48));

            selection.format.setProperty(
                QTextFormat::FullWidthSelection,
                true);

            selection.cursor =
                textCursor();

            selection.cursor.clearSelection();

            selections.append(
                selection);
        }

        setExtraSelections(
            selections);

        if (m_lineNumberArea != nullptr)
        {
            m_lineNumberArea->update();
        }
    }

    void EditorDocument::ConfigureEditor()
    {
        setTabChangesFocus(
            false);

        setUndoRedoEnabled(
            true);

        setCenterOnScroll(
            true);

        setCursorWidth(
            2);

        ApplyEditorSettings();
    }

    void EditorDocument::ApplyEditorSettings()
    {
        QSettings settings(
            QSettings::IniFormat,
            QSettings::UserScope,
            QStringLiteral("BootstrapForge"),
            QStringLiteral("BootstrapForge.Client"));

        const int fontSize = std::clamp(
            settings.value(QStringLiteral("editor/fontSize"), 11).toInt(),
            8,
            28);
        const int tabWidth = std::clamp(
            settings.value(QStringLiteral("editor/tabWidth"), 4).toInt(),
            2,
            8);
        const bool wordWrap = settings.value(
            QStringLiteral("editor/wordWrap"), false).toBool();
        m_showLineNumbers = settings.value(
            QStringLiteral("editor/showLineNumbers"), true).toBool();

        QFont editorFont = QFontDatabase::systemFont(
            QFontDatabase::FixedFont);
        editorFont.setPointSize(fontSize);
        setFont(editorFont);

        const QFontMetricsF metrics(editorFont);
        setTabStopDistance(
            metrics.horizontalAdvance(QLatin1Char(' ')) * tabWidth);
        setLineWrapMode(wordWrap
            ? QPlainTextEdit::WidgetWidth
            : QPlainTextEdit::NoWrap);

        if (m_lineNumberArea != nullptr)
        {
            m_lineNumberArea->setVisible(m_showLineNumbers);
        }
        UpdateLineNumberAreaWidth(blockCount());
        HighlightCurrentLine();
    }

} // namespace BootstrapForge::Client::Editor
