#pragma once

#include <QMainWindow>

#include <QString>
#include <QStringList>

class QAction;
class QCloseEvent;
class QDockWidget;
class QFileDialog;
class QLabel;
class QListWidget;
class QMenu;
class QPlainTextEdit;
class QTabWidget;
class QWidget;

namespace BootstrapForge::Client::Editor
{
    class EditorManager;
}

namespace BootstrapForge::Client::Build
{
    class CMakeProjectController;
}

namespace BootstrapForge::Client::Project
{
    class ProjectTree;
}

namespace BootstrapForge::Client::UI
{
    class MainWindow final : public QMainWindow
    {
    public:

        explicit MainWindow(
            QWidget* parent = nullptr);

        ~MainWindow() override = default;

    private:

        void closeEvent(QCloseEvent* event) override;

        void ConfigureWindow();

        void CreateActions();

        void CreateMenus();

        void CreateToolBar();

        void CreateProjectDock();

        void CreatePropertiesDock();

        void CreateOutputDock();

        void CreateCentralWidget();

        void CreateStatusBar();

        void ConnectActions();

        void ShowReadyMessage();

        void ShowInformationMessage(
            const QString& message);

        void ShowErrorMessage(
            const QString& message);

        void OpenProjectFile(
            const QString& path);

        void CreateNewProject();

        void OpenProjectWorkspace(
            const QString& path);

        void RefreshRecentProjects();

        void AddRecentProject(
            const QString& path);

        void SetProjectWorkspacePanelsVisible(
            bool visible);

        void SaveCurrentDocument();

        void CloseEditorTab(
            int index);

        void UpdateEditorProperties();

        void ToggleDockVisibility(
            QDockWidget* dock,
            bool visible);

        void RebuildTestMenu(
            const QStringList& testNames);

        void ResetWindowLayout();

        void ShowArchitectureOverview();

        void ShowSettingsDialog();

        void ShowAboutDialog();

        void RunGitStatus();

        void CreateGitBranch();

        void SwitchGitBranch();

        void RunBuiltApplication();

        [[nodiscard]]
        QString GitExecutable() const;

        [[nodiscard]]
        bool RunGitCommand(
            const QStringList& arguments,
            QString& output,
            QString& errorMessage) const;

        void UpdateEditorActions();

        [[nodiscard]]
        bool ConfirmUnsavedDocuments();

        QAction* m_newAction = nullptr;
        QAction* m_openAction = nullptr;
        QAction* m_openProjectAction = nullptr;
        QAction* m_generateAction = nullptr;
        QAction* m_saveAction = nullptr;
        QAction* m_exitAction = nullptr;

        QAction* m_undoAction = nullptr;
        QAction* m_redoAction = nullptr;
        QAction* m_cutAction = nullptr;
        QAction* m_copyAction = nullptr;
        QAction* m_pasteAction = nullptr;

        QAction* m_resetLayoutAction = nullptr;

        QAction* m_projectDockAction = nullptr;
        QAction* m_propertiesDockAction = nullptr;
        QAction* m_outputDockAction = nullptr;

        QAction* m_configureAction = nullptr;

        QAction* m_buildAction = nullptr;
        QAction* m_runAction = nullptr;
        QAction* m_testAction = nullptr;

        QAction* m_gitStatusAction = nullptr;

        QAction* m_createBranchAction = nullptr;

        QAction* m_switchBranchAction = nullptr;

        QAction* m_architectureAction = nullptr;

        QAction* m_settingsAction = nullptr;

        QAction* m_aboutAction = nullptr;

        QMenu* m_testMenu = nullptr;

        QDockWidget* m_projectDock = nullptr;
        QDockWidget* m_propertiesDock = nullptr;
        QDockWidget* m_outputDock = nullptr;

        Project::ProjectTree* m_projectTree = nullptr;

        QLabel* m_propertiesLabel = nullptr;

        QPlainTextEdit* m_outputEditor = nullptr;

        QLabel* m_welcomeProjectLabel = nullptr;

        QListWidget* m_recentProjectsList = nullptr;

        QWidget* m_welcomePage = nullptr;

        QTabWidget* m_workspaceTabs = nullptr;

        Editor::EditorManager* m_editorManager = nullptr;

        Build::CMakeProjectController* m_buildController = nullptr;

        QFileDialog* m_openProjectDialog = nullptr;

        QStringList m_testNames;
        bool m_runAfterBuild = false;
    };

} // namespace BootstrapForge::Client::UI
