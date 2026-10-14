#pragma once

#include <QMainWindow>

#include <QString>

class QAction;
class QDockWidget;
class QLabel;
class QPlainTextEdit;
class QTabWidget;

namespace BootstrapForge::Client::Editor
{
    class EditorManager;
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

        void SaveCurrentDocument();

        void CloseEditorTab(
            int index);

        void UpdateEditorProperties();

        void ToggleDockVisibility(
            QDockWidget* dock,
            bool visible);

        QAction* m_newAction = nullptr;
        QAction* m_openAction = nullptr;
        QAction* m_saveAction = nullptr;
        QAction* m_exitAction = nullptr;

        QAction* m_projectDockAction = nullptr;
        QAction* m_propertiesDockAction = nullptr;
        QAction* m_outputDockAction = nullptr;

        QAction* m_buildAction = nullptr;
        QAction* m_testAction = nullptr;

        QAction* m_gitStatusAction = nullptr;

        QDockWidget* m_projectDock = nullptr;
        QDockWidget* m_propertiesDock = nullptr;
        QDockWidget* m_outputDock = nullptr;

        Project::ProjectTree* m_projectTree = nullptr;

        QLabel* m_propertiesLabel = nullptr;

        QPlainTextEdit* m_outputEditor = nullptr;

        QTabWidget* m_workspaceTabs = nullptr;

        Editor::EditorManager* m_editorManager = nullptr;
    };

} // namespace BootstrapForge::Client::UI