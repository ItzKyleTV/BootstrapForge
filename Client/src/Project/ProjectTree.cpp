#include <BootstrapForge/Client/Project/ProjectTree.hpp>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QModelIndex>
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

        m_workspaceRoot =
            DiscoverWorkspaceRoot();

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

    QString ProjectTree::DiscoverWorkspaceRoot()
    {
        const QStringList startingPaths =
        {
            QDir::currentPath(),
            QCoreApplication::applicationDirPath()
        };

        for (const QString& startingPath :
            startingPaths)
        {
            QDir directory(
                startingPath);

            if (!directory.exists())
            {
                continue;
            }

            while (true)
            {
                const QString candidate =
                    directory.absolutePath();

                if (IsWorkspaceRoot(candidate))
                {
                    return QDir::cleanPath(
                        candidate);
                }

                const QString parentPath =
                    directory.absolutePath();

                if (!directory.cdUp())
                {
                    break;
                }

                if (directory.absolutePath() ==
                    parentPath)
                {
                    break;
                }
            }
        }

        return {};
    }

    bool ProjectTree::IsWorkspaceRoot(
        const QString& path)
    {
        const QDir directory(path);

        if (!directory.exists())
        {
            return false;
        }

        const QFileInfo cmakeFile(
            directory.filePath(
                QStringLiteral(
                    "CMakeLists.txt")));

        const QStringList requiredDirectories =
        {
            QStringLiteral("Domain"),
            QStringLiteral("Engine"),
            QStringLiteral("Client")
        };

        if (!cmakeFile.exists() ||
            !cmakeFile.isFile())
        {
            return false;
        }

        for (const QString& requiredDirectory :
            requiredDirectories)
        {
            const QFileInfo directoryInfo(
                directory.filePath(
                    requiredDirectory));

            if (!directoryInfo.exists() ||
                !directoryInfo.isDir())
            {
                return false;
            }
        }

        return true;
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

        auto* layout =
            new QVBoxLayout(this);

        layout->setContentsMargins(
            0,
            0,
            0,
            0);

        layout->addWidget(
            m_tree);
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
            const QString fallbackPath =
                QDir::currentPath();

            const QModelIndex rootIndex =
                m_model->setRootPath(
                    fallbackPath);

            m_tree->setRootIndex(
                rootIndex);

            return;
        }

        const QModelIndex rootIndex =
            m_model->setRootPath(
                m_workspaceRoot);

        m_tree->setRootIndex(
            rootIndex);

        m_tree->setCurrentIndex(
            rootIndex);
    }

} // namespace BootstrapForge::Client::Project