#pragma once

#include <QObject>
#include <QString>

class QTabWidget;

namespace BootstrapForge::Client::Editor
{
    class EditorDocument;

    class EditorManager final : public QObject
    {
        Q_OBJECT

    public:

        explicit EditorManager(
            QTabWidget& tabs,
            QObject* parent = nullptr);

        ~EditorManager() override = default;

        [[nodiscard]]
        bool OpenFile(
            const QString& path,
            QString& errorMessage);

        [[nodiscard]]
        bool SaveCurrent(
            QString& errorMessage);

        [[nodiscard]]
        bool SaveFile(
            int index,
            QString& errorMessage);

        [[nodiscard]]
        bool CloseEditor(
            int index);

        [[nodiscard]]
        bool IsEditorTab(
            int index) const;

        [[nodiscard]]
        bool IsModified(
            int index) const;

        [[nodiscard]]
        QString FilePath(
            int index) const;

        [[nodiscard]]
        QString CurrentFilePath() const;

        [[nodiscard]]
        EditorDocument*
            CurrentDocument() const;

    signals:

        void DocumentOpened(
            const QString& path);

        void DocumentSaved(
            const QString& path);

        void DocumentModifiedChanged(
            const QString& path,
            bool modified);

        void CurrentDocumentChanged(
            const QString& path);

    private:

        [[nodiscard]]
        EditorDocument*
            DocumentAt(
                int index) const;

        [[nodiscard]]
        int FindOpenDocument(
            const QString& path) const;

        void UpdateTabTitle(
            int index,
            bool modified);

        void ConnectDocumentSignals(
            EditorDocument& document);

        QTabWidget& m_tabs;
    };

} // namespace BootstrapForge::Client::Editor