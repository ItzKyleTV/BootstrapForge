#pragma once

#include <QWidget>

#include <QString>

class QFileSystemModel;
class QTreeView;

namespace BootstrapForge::Client::Project
{
    class ProjectTree final : public QWidget
    {
        Q_OBJECT

    public:

        explicit ProjectTree(
            QWidget* parent = nullptr);

        ~ProjectTree() override = default;

        [[nodiscard]]
        QString WorkspaceRoot() const;

        bool SetWorkspaceRoot(
            const QString& path);

    signals:

        void PathSelected(
            const QString& path);

        void PathActivated(
            const QString& path);

    private:

        [[nodiscard]]
        static QString DiscoverWorkspaceRoot();

        [[nodiscard]]
        static bool IsWorkspaceRoot(
            const QString& path);

        void ConfigureModel();

        void ConfigureTree();

        void ConnectSignals();

        void SelectWorkspaceRoot();

        QFileSystemModel* m_model = nullptr;

        QTreeView* m_tree = nullptr;

        QString m_workspaceRoot;
    };

} // namespace BootstrapForge::Client::Project