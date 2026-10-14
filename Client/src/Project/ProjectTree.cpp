#include <BootstrapForge/Client/Project/ProjectTree.hpp>

#include <QDir>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QModelIndex>
#include <QStackedWidget>
#include <QTreeView>
#include <QVBoxLayout>

namespace BootstrapForge::Client::Project
{
    ProjectTree::ProjectTree(
        QWidget* parent)
        :
        QWidget(parent)
    {
        m_model =
            new QFileSystemModel(this);

        m_tree =
            new QTreeView(this);

        ConfigureModel();

        ConfigureTree();

        ConnectSignals();

        SelectWorkspaceRoot();
    }

    QString ProjectTree::WorkspaceRoot() const
    {
        return m_workspaceRoot;
    }

    bool ProjectTree::SetWorkspaceRoot(
        const QString& path)
    {
        const QFileInfo directoryInfo(path);

        if (!directoryInfo.exists() ||
            !directoryInfo.isDir())
        {
            return false;
        }

        const QString absolutePath =
            directoryInfo.absoluteFilePath();

        if (!IsWorkspaceRoot(absolutePath))
        {
            return false;
        }

        m_workspaceRoot =
            QDir::cleanPath(
                absolutePath);

        SelectWorkspaceRoot();

        return true;
    }

    void ProjectTree::Refresh()
    {
        if (m_workspaceRoot.isEmpty())
        {
            return;
        }

        const QModelIndex rootIndex =
            m_model->setRootPath(m_workspaceRoot);
        m_tree->setRootIndex(rootIndex);
    }

    bool ProjectTree::IsWorkspaceRoot(
        const QString& path)
    {
        const QFileInfo directoryInfo(path);
        return directoryInfo.exists() && directoryInfo.isDir();
    }

    void ProjectTree::ConfigureModel()
    {
        m_model->setReadOnly(
            true);

        m_model->setFilter(
            QDir::AllEntries |
            QDir::NoDotAndDotDot |
            QDir::AllDirs);

        m_model->setOption(
            QFileSystemModel::DontResolveSymlinks,
            true);

        m_model->setOption(
            QFileSystemModel::DontUseCustomDirectoryIcons,
            true);
    }

    void ProjectTree::ConfigureTree()
    {
        m_tree->setModel(
            m_model);

        m_tree->setHeaderHidden(
            true);

        m_tree->setUniformRowHeights(
            true);

        m_tree->setAnimated(
            true);

        m_tree->setSortingEnabled(
            true);

        m_tree->sortByColumn(
            0,
            Qt::AscendingOrder);

        m_tree->setSelectionMode(
            QAbstractItemView::SingleSelection);

        m_tree->setEditTriggers(
            QAbstractItemView::NoEditTriggers);

        m_tree->setAlternatingRowColors(
            false);

        for (int column = 1;
            column < m_model->columnCount();
            ++column)
        {
            m_tree->hideColumn(
                column);
        }

        auto* header =
            m_tree->header();

        header->setStretchLastSection(
            true);

        header->setSectionResizeMode(
            0,
            QHeaderView::Stretch);

        m_stack = new QStackedWidget(this);
        m_emptyStateLabel = new QLabel(
            QStringLiteral("No project open\n\nCreate or open a project to view its files."),
            m_stack);
        m_emptyStateLabel->setAlignment(Qt::AlignCenter);
        m_emptyStateLabel->setWordWrap(true);
        m_stack->addWidget(m_emptyStateLabel);
        m_stack->addWidget(m_tree);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_stack);
    }

    void ProjectTree::ConnectSignals()
    {
        connect(
            m_tree->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this,
            [this](
                const QModelIndex& current,
                const QModelIndex&)
            {
                if (!current.isValid())
                {
                    return;
                }

                const QString path =
                    m_model->filePath(
                        current);

                emit PathSelected(
                    path);
            });

        connect(
            m_tree,
            &QTreeView::doubleClicked,
            this,
            [this](
                const QModelIndex& index)
            {
                if (!index.isValid())
                {
                    return;
                }

                const QString path =
                    m_model->filePath(
                        index);

                emit PathActivated(
                    path);
            });
    }

    void ProjectTree::SelectWorkspaceRoot()
    {
        if (m_workspaceRoot.isEmpty())
        {
            m_model->setRootPath(QString{});
            m_tree->setRootIndex(QModelIndex{});
            m_stack->setCurrentWidget(m_emptyStateLabel);
            return;
        }

        const QModelIndex rootIndex =
            m_model->setRootPath(
                m_workspaceRoot);

        m_tree->setRootIndex(
            rootIndex);

        m_tree->setCurrentIndex(
            rootIndex);
        m_stack->setCurrentWidget(m_tree);
    }

} // namespace BootstrapForge::Client::Project
