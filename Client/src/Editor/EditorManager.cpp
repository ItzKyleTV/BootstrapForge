#include <BootstrapForge/Client/Editor/EditorManager.hpp>

#include <BootstrapForge/Client/Editor/EditorDocument.hpp>

#include <QFileInfo>
#include <QTabWidget>
#include <QWidget>

namespace BootstrapForge::Client::Editor
{
    EditorManager::EditorManager(
        QTabWidget& tabs,
        QObject* parent)
        :
        QObject(parent),
        m_tabs(tabs)
    {
    }

    bool EditorManager::OpenFile(
        const QString& path,
        QString& errorMessage)
    {
        const QFileInfo fileInfo(path);

        if (!fileInfo.exists() ||
            !fileInfo.isFile())
        {
            errorMessage =
                QStringLiteral(
                    "The selected path is not a file: %1")
                .arg(path);

            return false;
        }

        const QString absolutePath =
            fileInfo.absoluteFilePath();

        const int existingIndex =
            FindOpenDocument(
                absolutePath);

        if (existingIndex >= 0)
        {
            m_tabs.setCurrentIndex(
                existingIndex);

            errorMessage.clear();

            return true;
        }

        auto* document =
            new EditorDocument(
                &m_tabs);

        if (!document->LoadFromFile(
            absolutePath,
            errorMessage))
        {
            delete document;

            return false;
        }

        const QString fileName =
            fileInfo.fileName();

        const int index =
            m_tabs.addTab(
                document,
                fileName);

        m_tabs.setTabToolTip(
            index,
            absolutePath);

        m_tabs.setCurrentIndex(
            index);

        ConnectDocumentSignals(
            *document);

        emit DocumentOpened(
            absolutePath);

        emit CurrentDocumentChanged(
            absolutePath);

        errorMessage.clear();

        return true;
    }

    bool EditorManager::SaveCurrent(
        QString& errorMessage)
    {
        const int index =
            m_tabs.currentIndex();

        if (!IsEditorTab(index))
        {
            errorMessage =
                QStringLiteral(
                    "There is no editable document open.");

            return false;
        }

        return SaveFile(
            index,
            errorMessage);
    }

    bool EditorManager::SaveFile(
        int index,
        QString& errorMessage)
    {
        EditorDocument* document =
            DocumentAt(index);

        if (document == nullptr)
        {
            errorMessage =
                QStringLiteral(
                    "The selected tab is not an editor document.");

            return false;
        }

        if (!document->SaveToFile(
            errorMessage))
        {
            return false;
        }

        UpdateTabTitle(
            index,
            false);

        emit DocumentSaved(
            document->FilePath());

        return true;
    }

    bool EditorManager::CloseEditor(
        int index)
    {
        if (!IsEditorTab(index))
        {
            return false;
        }

        QWidget* widget =
            m_tabs.widget(index);

        m_tabs.removeTab(
            index);

        delete widget;

        const QString currentPath =
            CurrentFilePath();

        emit CurrentDocumentChanged(
            currentPath);

        return true;
    }

    bool EditorManager::IsEditorTab(
        int index) const
    {
        if (index < 0 ||
            index >= m_tabs.count())
        {
            return false;
        }

        return qobject_cast<
            EditorDocument*>(
                m_tabs.widget(index)) != nullptr;
    }

    bool EditorManager::IsModified(
        int index) const
    {
        const EditorDocument* document =
            DocumentAt(index);

        if (document == nullptr)
        {
            return false;
        }

        return document->IsModified();
    }

    QString EditorManager::FilePath(
        int index) const
    {
        const EditorDocument* document =
            DocumentAt(index);

        if (document == nullptr)
        {
            return {};
        }

        return document->FilePath();
    }

    QString EditorManager::CurrentFilePath() const
    {
        return FilePath(
            m_tabs.currentIndex());
    }

    EditorDocument*
        EditorManager::CurrentDocument() const
    {
        return DocumentAt(
            m_tabs.currentIndex());
    }

    EditorDocument*
        EditorManager::DocumentAt(
            int index) const
    {
        if (index < 0 ||
            index >= m_tabs.count())
        {
            return nullptr;
        }

        return qobject_cast<
            EditorDocument*>(
                m_tabs.widget(index));
    }

    int EditorManager::FindOpenDocument(
        const QString& path) const
    {
        const QString absolutePath =
            QFileInfo(path)
            .absoluteFilePath();

        for (int index = 0;
            index < m_tabs.count();
            ++index)
        {
            const EditorDocument* document =
                DocumentAt(index);

            if (document == nullptr)
            {
                continue;
            }

            if (QFileInfo(
                document->FilePath())
                .absoluteFilePath() ==
                absolutePath)
            {
                return index;
            }
        }

        return -1;
    }

    void EditorManager::UpdateTabTitle(
        int index,
        bool modified)
    {
        const EditorDocument* document =
            DocumentAt(index);

        if (document == nullptr)
        {
            return;
        }

        const QString fileName =
            QFileInfo(
                document->FilePath())
            .fileName();

        const QString title =
            modified
            ? QStringLiteral("*%1")
            .arg(fileName)
            : fileName;

        m_tabs.setTabText(
            index,
            title);
    }

    void EditorManager::ConnectDocumentSignals(
        EditorDocument& document)
    {
        connect(
            &document,
            &EditorDocument::modificationChanged,
            this,
            [this](
                bool modified)
            {
                auto* editor =
                    qobject_cast<
                    EditorDocument*>(
                        sender());

                if (editor == nullptr)
                {
                    return;
                }

                const int index =
                    m_tabs.indexOf(
                        editor);

                if (index < 0)
                {
                    return;
                }

                UpdateTabTitle(
                    index,
                    modified);

                emit DocumentModifiedChanged(
                    editor->FilePath(),
                    modified);
            });
    }

} // namespace BootstrapForge::Client::Editor