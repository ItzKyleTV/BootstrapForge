#include <BootstrapForge/Client/UI/MainWindow.hpp>

#include <BootstrapForge/Client/Editor/EditorManager.hpp>
#include <BootstrapForge/Client/Project/ProjectTree.hpp>

#include <QAction>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

namespace BootstrapForge::Client::UI
{
    MainWindow::MainWindow(
        QWidget* parent)
        :
        QMainWindow(parent)
    {
        ConfigureWindow();

        CreateActions();

        CreateMenus();

        CreateToolBar();

        CreateProjectDock();

        CreatePropertiesDock();

        CreateOutputDock();

        CreateCentralWidget();

        CreateStatusBar();

        ConnectActions();

        ShowReadyMessage();
    }

    void MainWindow::ConfigureWindow()
    {
        setWindowTitle(
            QStringLiteral(
                "BootstrapForge"));

        resize(
            1500,
            900);

        setMinimumSize(
            1100,
            700);

        setDockNestingEnabled(
            true);

        setDockOptions(
            QMainWindow::AnimatedDocks |
            QMainWindow::AllowNestedDocks |
            QMainWindow::AllowTabbedDocks |
            QMainWindow::GroupedDragging);
    }

    void MainWindow::CreateActions()
    {
        m_newAction =
            new QAction(
                QStringLiteral("&New"),
                this);

        m_newAction->setShortcut(
            QKeySequence::New);

        m_newAction->setStatusTip(
            QStringLiteral(
                "Create a new project"));

        m_openAction =
            new QAction(
                QStringLiteral("&Open..."),
                this);

        m_openAction->setShortcut(
            QKeySequence::Open);

        m_openAction->setStatusTip(
            QStringLiteral(
                "Open a source file"));

        m_saveAction =
            new QAction(
                QStringLiteral("&Save"),
                this);

        m_saveAction->setShortcut(
            QKeySequence::Save);

        m_saveAction->setStatusTip(
            QStringLiteral(
                "Save the current document"));

        m_exitAction =
            new QAction(
                QStringLiteral("E&xit"),
                this);

        m_exitAction->setShortcut(
            QKeySequence::Quit);

        m_exitAction->setStatusTip(
            QStringLiteral(
                "Exit BootstrapForge"));

        m_projectDockAction =
            new QAction(
                QStringLiteral("Project"),
                this);

        m_projectDockAction->setCheckable(
            true);

        m_projectDockAction->setChecked(
            true);

        m_propertiesDockAction =
            new QAction(
                QStringLiteral("Properties"),
                this);

        m_propertiesDockAction->setCheckable(
            true);

        m_propertiesDockAction->setChecked(
            true);

        m_outputDockAction =
            new QAction(
                QStringLiteral("Output"),
                this);

        m_outputDockAction->setCheckable(
            true);

        m_outputDockAction->setChecked(
            true);

        m_buildAction =
            new QAction(
                QStringLiteral("Build"),
                this);

        m_buildAction->setStatusTip(
            QStringLiteral(
                "Build the current project"));

        m_testAction =
            new QAction(
                QStringLiteral("Test"),
                this);

        m_testAction->setStatusTip(
            QStringLiteral(
                "Run the project tests"));

        m_gitStatusAction =
            new QAction(
                QStringLiteral(
                    "Git Status"),
                this);

        m_gitStatusAction->setStatusTip(
            QStringLiteral(
                "Show Git status"));
    }

    void MainWindow::CreateMenus()
    {
        QMenu* fileMenu =
            menuBar()->addMenu(
                QStringLiteral("&File"));

        fileMenu->addAction(
            m_newAction);

        fileMenu->addAction(
            m_openAction);

        fileMenu->addSeparator();

        fileMenu->addAction(
            m_saveAction);

        fileMenu->addSeparator();

        fileMenu->addAction(
            m_exitAction);

        QMenu* editMenu =
            menuBar()->addMenu(
                QStringLiteral("&Edit"));

        editMenu->addAction(
            QStringLiteral("Undo"));

        editMenu->addAction(
            QStringLiteral("Redo"));

        editMenu->addSeparator();

        editMenu->addAction(
            QStringLiteral("Cut"));

        editMenu->addAction(
            QStringLiteral("Copy"));

        editMenu->addAction(
            QStringLiteral("Paste"));

        QMenu* viewMenu =
            menuBar()->addMenu(
                QStringLiteral("&View"));

        QMenu* panelsMenu =
            viewMenu->addMenu(
                QStringLiteral("Panels"));

        panelsMenu->addAction(
            m_projectDockAction);

        panelsMenu->addAction(
            m_propertiesDockAction);

        panelsMenu->addAction(
            m_outputDockAction);

        viewMenu->addSeparator();

        viewMenu->addAction(
            QStringLiteral(
                "Reset Window Layout"));

        QMenu* projectMenu =
            menuBar()->addMenu(
                QStringLiteral("&Project"));

        projectMenu->addAction(
            m_newAction);

        projectMenu->addAction(
            m_openAction);

        QMenu* buildMenu =
            menuBar()->addMenu(
                QStringLiteral("&Build"));

        buildMenu->addAction(
            m_buildAction);

        buildMenu->addAction(
            m_testAction);

        QMenu* gitMenu =
            menuBar()->addMenu(
                QStringLiteral("&Git"));

        gitMenu->addAction(
            m_gitStatusAction);

        gitMenu->addSeparator();

        gitMenu->addAction(
            QStringLiteral(
                "Create Branch"));

        gitMenu->addAction(
            QStringLiteral(
                "Switch Branch"));

        QMenu* toolsMenu =
            menuBar()->addMenu(
                QStringLiteral("&Tools"));

        toolsMenu->addAction(
            QStringLiteral(
                "Architecture"));

        toolsMenu->addAction(
            QStringLiteral(
                "Settings"));

        menuBar()->addMenu(
            QStringLiteral("&Help"));
    }

    void MainWindow::CreateToolBar()
    {
        QToolBar* toolBar =
            addToolBar(
                QStringLiteral(
                    "Main"));

        toolBar->setObjectName(
            QStringLiteral(
                "MainToolBar"));

        toolBar->setMovable(
            false);

        toolBar->setToolButtonStyle(
            Qt::ToolButtonTextOnly);

        toolBar->addAction(
            m_newAction);

        toolBar->addAction(
            m_openAction);

        toolBar->addAction(
            m_saveAction);

        toolBar->addSeparator();

        toolBar->addAction(
            m_buildAction);

        toolBar->addAction(
            m_testAction);

        toolBar->addSeparator();

        toolBar->addAction(
            m_gitStatusAction);
    }

    void MainWindow::CreateProjectDock()
    {
        m_projectDock =
            new QDockWidget(
                QStringLiteral(
                    "Project"),
                this);

        m_projectDock->setObjectName(
            QStringLiteral(
                "ProjectDock"));

        m_projectDock->setAllowedAreas(
            Qt::LeftDockWidgetArea |
            Qt::RightDockWidgetArea);

        m_projectTree =
            new Project::ProjectTree(
                m_projectDock);

        m_projectDock->setWidget(
            m_projectTree);

        addDockWidget(
            Qt::LeftDockWidgetArea,
            m_projectDock);
    }

    void MainWindow::CreatePropertiesDock()
    {
        m_propertiesDock =
            new QDockWidget(
                QStringLiteral(
                    "Properties"),
                this);

        m_propertiesDock->setObjectName(
            QStringLiteral(
                "PropertiesDock"));

        m_propertiesDock->setAllowedAreas(
            Qt::LeftDockWidgetArea |
            Qt::RightDockWidgetArea);

        m_propertiesLabel =
            new QLabel(
                QStringLiteral(
                    "No item selected."),
                m_propertiesDock);

        m_propertiesLabel->setAlignment(
            Qt::AlignTop |
            Qt::AlignLeft);

        m_propertiesLabel->setWordWrap(
            true);

        m_propertiesLabel->setContentsMargins(
            12,
            12,
            12,
            12);

        m_propertiesDock->setWidget(
            m_propertiesLabel);

        addDockWidget(
            Qt::RightDockWidgetArea,
            m_propertiesDock);
    }

    void MainWindow::CreateOutputDock()
    {
        m_outputDock =
            new QDockWidget(
                QStringLiteral(
                    "Output"),
                this);

        m_outputDock->setObjectName(
            QStringLiteral(
                "OutputDock"));

        m_outputDock->setAllowedAreas(
            Qt::BottomDockWidgetArea);

        m_outputEditor =
            new QPlainTextEdit(
                m_outputDock);

        m_outputEditor->setReadOnly(
            true);

        m_outputEditor->setLineWrapMode(
            QPlainTextEdit::NoWrap);

        QFont outputFont =
            m_outputEditor->font();

        outputFont.setStyleHint(
            QFont::Monospace);

        m_outputEditor->setFont(
            outputFont);

        m_outputDock->setWidget(
            m_outputEditor);

        addDockWidget(
            Qt::BottomDockWidgetArea,
            m_outputDock);

        resizeDocks(
            {
                m_outputDock
            },
            {
                180
            },
            Qt::Vertical);
    }

    void MainWindow::CreateCentralWidget()
    {
        m_workspaceTabs =
            new QTabWidget(
                this);

        m_workspaceTabs->setObjectName(
            QStringLiteral(
                "WorkspaceTabs"));

        m_workspaceTabs->setDocumentMode(
            true);

        m_workspaceTabs->setTabsClosable(
            true);

        QWidget* welcomePage =
            new QWidget(
                m_workspaceTabs);

        auto* layout =
            new QVBoxLayout(
                welcomePage);

        auto* title =
            new QLabel(
                QStringLiteral(
                    "BootstrapForge"));

        auto* subtitle =
            new QLabel(
                QStringLiteral(
                    "Architecture workspace and development environment"));

        auto* instructions =
            new QLabel(
                QStringLiteral(
                    "Double-click a source file in the Project panel to open it."));

        title->setAlignment(
            Qt::AlignCenter);

        subtitle->setAlignment(
            Qt::AlignCenter);

        instructions->setAlignment(
            Qt::AlignCenter);

        QFont titleFont =
            title->font();

        titleFont.setPointSize(
            titleFont.pointSize() + 8);

        titleFont.setBold(
            true);

        title->setFont(
            titleFont);

        layout->addStretch();

        layout->addWidget(
            title);

        layout->addWidget(
            subtitle);

        layout->addSpacing(
            16);

        layout->addWidget(
            instructions);

        layout->addStretch();

        m_workspaceTabs->addTab(
            welcomePage,
            QStringLiteral(
                "Welcome"));

        m_editorManager =
            new Editor::EditorManager(
                *m_workspaceTabs,
                this);

        setCentralWidget(
            m_workspaceTabs);
    }

    void MainWindow::CreateStatusBar()
    {
        statusBar()->showMessage(
            QStringLiteral(
                "Ready"));
    }

    void MainWindow::ConnectActions()
    {
        connect(
            m_exitAction,
            &QAction::triggered,
            this,
            &QMainWindow::close);

        connect(
            m_newAction,
            &QAction::triggered,
            this,
            [this]
            {
                ShowInformationMessage(
                    QStringLiteral(
                        "New project support will be connected to the client project service."));
            });

        connect(
            m_openAction,
            &QAction::triggered,
            this,
            [this]
            {
                const QString startingDirectory =
                    m_projectTree->WorkspaceRoot();

                const QString path =
                    QFileDialog::getOpenFileName(
                        this,
                        QStringLiteral(
                            "Open File"),
                        startingDirectory,
                        QStringLiteral(
                            "All Files (*.*)"));

                if (path.isEmpty())
                {
                    return;
                }

                OpenProjectFile(
                    path);
            });

        connect(
            m_saveAction,
            &QAction::triggered,
            this,
            [this]
            {
                SaveCurrentDocument();
            });

        connect(
            m_buildAction,
            &QAction::triggered,
            this,
            [this]
            {
                ShowInformationMessage(
                    QStringLiteral(
                        "Build execution will be connected to the client build service."));
            });

        connect(
            m_testAction,
            &QAction::triggered,
            this,
            [this]
            {
                ShowInformationMessage(
                    QStringLiteral(
                        "Test execution will be connected to the client testing service."));
            });

        connect(
            m_gitStatusAction,
            &QAction::triggered,
            this,
            [this]
            {
                ShowInformationMessage(
                    QStringLiteral(
                        "Git operations will be connected to the client Git service."));
            });

        connect(
            m_projectDockAction,
            &QAction::toggled,
            this,
            [this](bool visible)
            {
                ToggleDockVisibility(
                    m_projectDock,
                    visible);
            });

        connect(
            m_propertiesDockAction,
            &QAction::toggled,
            this,
            [this](bool visible)
            {
                ToggleDockVisibility(
                    m_propertiesDock,
                    visible);
            });

        connect(
            m_outputDockAction,
            &QAction::toggled,
            this,
            [this](bool visible)
            {
                ToggleDockVisibility(
                    m_outputDock,
                    visible);
            });

        connect(
            m_projectTree,
            &Project::ProjectTree::PathSelected,
            this,
            [this](const QString& path)
            {
                m_propertiesLabel->setText(
                    QStringLiteral(
                        "Selected item:\n\n%1")
                    .arg(path));

                statusBar()->showMessage(
                    path);
            });

        connect(
            m_projectTree,
            &Project::ProjectTree::PathActivated,
            this,
            [this](const QString& path)
            {
                OpenProjectFile(
                    path);
            });

        connect(
            m_workspaceTabs,
            &QTabWidget::tabCloseRequested,
            this,
            [this](int index)
            {
                CloseEditorTab(
                    index);
            });

        connect(
            m_workspaceTabs,
            &QTabWidget::currentChanged,
            this,
            [this](int)
            {
                UpdateEditorProperties();
            });

        connect(
            m_editorManager,
            &Editor::EditorManager::DocumentModifiedChanged,
            this,
            [this](
                const QString& path,
                bool modified)
            {
                m_outputEditor->appendPlainText(
                    QStringLiteral(
                        "[EDITOR] %1 %2")
                    .arg(
                        modified
                        ? QStringLiteral(
                            "modified")
                        : QStringLiteral(
                            "saved"),
                        path));
            });

        connect(
            m_editorManager,
            &Editor::EditorManager::DocumentOpened,
            this,
            [this](const QString& path)
            {
                m_outputEditor->appendPlainText(
                    QStringLiteral(
                        "[EDITOR] Opened: %1")
                    .arg(path));

                UpdateEditorProperties();
            });

        connect(
            m_editorManager,
            &Editor::EditorManager::DocumentSaved,
            this,
            [this](const QString& path)
            {
                m_outputEditor->appendPlainText(
                    QStringLiteral(
                        "[EDITOR] Saved: %1")
                    .arg(path));

                statusBar()->showMessage(
                    QStringLiteral(
                        "Saved: %1")
                    .arg(path),
                    5000);

                UpdateEditorProperties();
            });

        connect(
            m_editorManager,
            &Editor::EditorManager::CurrentDocumentChanged,
            this,
            [this](const QString& path)
            {
                if (path.isEmpty())
                {
                    statusBar()->showMessage(
                        QStringLiteral(
                            "Ready"));

                    UpdateEditorProperties();

                    return;
                }

                statusBar()->showMessage(
                    path);

                UpdateEditorProperties();
            });
    }

    void MainWindow::ShowReadyMessage()
    {
        m_outputEditor->appendPlainText(
            QStringLiteral(
                "BootstrapForge Client started."));

        m_outputEditor->appendPlainText(
            QStringLiteral(
                "Qt 6.11.2 / C++23"));

        m_outputEditor->appendPlainText(
            QStringLiteral(
                "UI shell initialized."));

        if (m_projectTree->WorkspaceRoot().isEmpty())
        {
            m_outputEditor->appendPlainText(
                QStringLiteral(
                    "BootstrapForge workspace could not be discovered."));
        }
        else
        {
            m_outputEditor->appendPlainText(
                QStringLiteral(
                    "Workspace: %1")
                .arg(
                    m_projectTree->WorkspaceRoot()));

            m_outputEditor->appendPlainText(
                QStringLiteral(
                    "Project workspace ready."));
        }
    }

    void MainWindow::ShowInformationMessage(
        const QString& message)
    {
        statusBar()->showMessage(
            message,
            5000);

        m_outputEditor->appendPlainText(
            QStringLiteral(
                "[INFO] %1")
            .arg(message));
    }

    void MainWindow::ShowErrorMessage(
        const QString& message)
    {
        statusBar()->showMessage(
            message,
            5000);

        m_outputEditor->appendPlainText(
            QStringLiteral(
                "[ERROR] %1")
            .arg(message));

        QMessageBox::critical(
            this,
            QStringLiteral(
                "BootstrapForge"),
            message);
    }

    void MainWindow::OpenProjectFile(
        const QString& path)
    {
        const QFileInfo fileInfo(path);

        if (!fileInfo.exists())
        {
            ShowErrorMessage(
                QStringLiteral(
                    "The selected path no longer exists:\n%1")
                .arg(path));

            return;
        }

        if (!fileInfo.isFile())
        {
            statusBar()->showMessage(
                QStringLiteral(
                    "Selected directory: %1")
                .arg(path),
                3000);

            return;
        }

        QString errorMessage;

        if (!m_editorManager->OpenFile(
            path,
            errorMessage))
        {
            ShowErrorMessage(
                errorMessage);

            return;
        }

        UpdateEditorProperties();
    }

    void MainWindow::SaveCurrentDocument()
    {
        QString errorMessage;

        if (!m_editorManager->SaveCurrent(
            errorMessage))
        {
            ShowErrorMessage(
                errorMessage);

            return;
        }

        UpdateEditorProperties();
    }

    void MainWindow::CloseEditorTab(
        int index)
    {
        if (!m_editorManager->IsEditorTab(
            index))
        {
            return;
        }

        if (m_editorManager->IsModified(
            index))
        {
            const QString path =
                m_editorManager->FilePath(
                    index);

            const QMessageBox::StandardButton choice =
                QMessageBox::warning(
                    this,
                    QStringLiteral(
                        "Unsaved Changes"),
                    QStringLiteral(
                        "Save changes to:\n\n%1")
                    .arg(path),
                    QMessageBox::Save |
                    QMessageBox::Discard |
                    QMessageBox::Cancel,
                    QMessageBox::Save);

            if (choice ==
                QMessageBox::Cancel)
            {
                return;
            }

            if (choice ==
                QMessageBox::Save)
            {
                QString errorMessage;

                if (!m_editorManager->SaveFile(
                    index,
                    errorMessage))
                {
                    ShowErrorMessage(
                        errorMessage);

                    return;
                }
            }
        }

        if (!m_editorManager->CloseEditor(
            index))
        {
            ShowErrorMessage(
                QStringLiteral(
                    "The editor document could not be closed."));

            return;
        }

        UpdateEditorProperties();
    }

    void MainWindow::UpdateEditorProperties()
    {
        const QString filePath =
            m_editorManager->CurrentFilePath();

        if (filePath.isEmpty())
        {
            m_propertiesLabel->setText(
                QStringLiteral(
                    "No document selected."));

            return;
        }

        const QFileInfo fileInfo(
            filePath);

        const QString modifiedState =
            m_editorManager->IsModified(
                m_workspaceTabs->currentIndex())
            ? QStringLiteral(
                "Modified")
            : QStringLiteral(
                "Saved");

        m_propertiesLabel->setText(
            QStringLiteral(
                "File\n\n"
                "%1\n\n"
                "Path\n\n"
                "%2\n\n"
                "Size\n\n"
                "%3 bytes\n\n"
                "State\n\n"
                "%4")
            .arg(
                fileInfo.fileName(),
                fileInfo.absoluteFilePath(),
                QString::number(
                    fileInfo.size()),
                modifiedState));
    }

    void MainWindow::ToggleDockVisibility(
        QDockWidget* dock,
        bool visible)
    {
        if (dock == nullptr)
        {
            return;
        }

        dock->setVisible(
            visible);
    }

} // namespace BootstrapForge::Client::UI