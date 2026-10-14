#include <BootstrapForge/Client/UI/MainWindow.hpp>

#include <BootstrapForge/Client/Build/CMakeProjectController.hpp>
#include <BootstrapForge/Client/Editor/EditorDocument.hpp>
#include <BootstrapForge/Client/Editor/EditorManager.hpp>
#include <BootstrapForge/Client/Generation/GenerateDialog.hpp>
#include <BootstrapForge/Client/Project/ProjectTree.hpp>

#include <QAction>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QInputDialog>
#include <QList>
#include <QKeySequence>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStringList>
#include <QTabWidget>
#include <QTabBar>
#include <QTextCursor>
#include <QTimer>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>

namespace
{
    QList<QUrl> CommonDirectoryPlaces()
    {
        QStringList paths{
            QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
            QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)
        };

        for (const QFileInfo& drive : QDir::drives())
        {
            paths.push_back(drive.absoluteFilePath());
        }

        QList<QUrl> places;
        QStringList includedPaths;
        for (const QString& path : paths)
        {
            const QFileInfo info(path);
            if (!info.exists() || !info.isDir())
            {
                continue;
            }

            const QString absolutePath = QDir::cleanPath(
                info.absoluteFilePath());
            const bool alreadyIncluded = std::any_of(
                includedPaths.cbegin(),
                includedPaths.cend(),
                [&absolutePath](const QString& includedPath)
                {
                    return QString::compare(
                        includedPath, absolutePath, Qt::CaseInsensitive) == 0;
                });
            if (alreadyIncluded)
            {
                continue;
            }

            includedPaths.push_back(absolutePath);
            places.push_back(QUrl::fromLocalFile(absolutePath));
        }

        return places;
    }

    void ConfigureDirectoryPicker(
        QFileDialog& dialog,
        const QString& title,
        const QString& startingDirectory)
    {
        // Set this before changing dialog properties so Qt uses its widget
        // implementation instead of a platform-native picker.
        dialog.setOption(QFileDialog::DontUseNativeDialog, true);
        dialog.setOption(QFileDialog::ShowDirsOnly, true);
        dialog.setWindowTitle(title);
        dialog.setDirectory(startingDirectory);
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setSidebarUrls(CommonDirectoryPlaces());
        dialog.setWindowModality(Qt::WindowModal);
    }

    class NewProjectDialog final : public QDialog
    {
    public:

        explicit NewProjectDialog(QWidget* parent)
            :
            QDialog(parent)
        {
            setWindowTitle(QStringLiteral("Create New Project"));
            setMinimumWidth(560);

            auto* layout = new QVBoxLayout(this);
            auto* title = new QLabel(
                QStringLiteral("Create a project workspace"), this);
            QFont titleFont = title->font();
            titleFont.setPointSize(titleFont.pointSize() + 3);
            titleFont.setBold(true);
            title->setFont(titleFont);
            layout->addWidget(title);

            layout->addWidget(new QLabel(
                QStringLiteral("BootstrapForge will create an empty folder. "
                    "Use Generate from Command to add folders and files."),
                this));

            auto* form = new QFormLayout();
            m_nameEdit = new QLineEdit(QStringLiteral("My Project"), this);
            form->addRow(QStringLiteral("Project name"), m_nameEdit);

            auto* locationRow = new QHBoxLayout();
            m_locationEdit = new QLineEdit(this);
            QString defaultLocation = QStandardPaths::writableLocation(
                QStandardPaths::DocumentsLocation);
            if (defaultLocation.isEmpty() ||
                !QFileInfo(defaultLocation).isDir())
            {
                defaultLocation = QDir::homePath();
            }
            m_locationEdit->setText(defaultLocation);
            auto* browseButton = new QPushButton(
                QStringLiteral("Browse..."), this);
            locationRow->addWidget(m_locationEdit, 1);
            locationRow->addWidget(browseButton);
            form->addRow(QStringLiteral("Create inside"), locationRow);
            layout->addLayout(form);

            m_pathPreview = new QLabel(this);
            m_pathPreview->setWordWrap(true);
            layout->addWidget(m_pathPreview);

            auto* buttons = new QDialogButtonBox(
                QDialogButtonBox::Cancel | QDialogButtonBox::Ok,
                this);
            m_createButton = buttons->button(QDialogButtonBox::Ok);
            m_createButton->setText(QStringLiteral("Create Project"));
            layout->addWidget(buttons);

            connect(m_nameEdit, &QLineEdit::textChanged,
                this, [this] { UpdateValidation(); });
            connect(m_locationEdit, &QLineEdit::textChanged,
                this, [this] { UpdateValidation(); });
            connect(browseButton, &QPushButton::clicked,
                this, [this]
                {
                    if (m_locationDialog != nullptr)
                    {
                        m_locationDialog->raise();
                        m_locationDialog->activateWindow();
                        return;
                    }

                    auto* dialog = new QFileDialog(this);
                    ConfigureDirectoryPicker(
                        *dialog,
                        QStringLiteral("Choose Project Parent Folder"),
                        m_locationEdit->text());
                    m_locationDialog = dialog;

                    connect(dialog, &QFileDialog::accepted,
                        this, [this, dialog]
                        {
                            const QStringList selectedPaths =
                                dialog->selectedFiles();
                            if (!selectedPaths.isEmpty())
                            {
                                m_locationEdit->setText(selectedPaths.front());
                            }
                        });
                    connect(dialog, &QFileDialog::finished,
                        this, [this, dialog](int)
                        {
                            if (m_locationDialog == dialog)
                            {
                                m_locationDialog = nullptr;
                            }
                            dialog->deleteLater();
                        });
                    dialog->open();
                });
            connect(buttons, &QDialogButtonBox::accepted,
                this, &QDialog::accept);
            connect(buttons, &QDialogButtonBox::rejected,
                this, &QDialog::reject);

            UpdateValidation();
        }

        QString ProjectPath() const
        {
            return QDir::cleanPath(QDir(m_locationEdit->text().trimmed())
                .filePath(m_nameEdit->text().trimmed()));
        }

    private:

        void UpdateValidation()
        {
            const QString name = m_nameEdit->text().trimmed();
            const QString location = m_locationEdit->text().trimmed();
            QString error;

            if (name.isEmpty())
            {
                error = QStringLiteral("Enter a project name.");
            }
            else if (name == QStringLiteral(".") ||
                name == QStringLiteral("..") ||
                name.endsWith(QLatin1Char('.')) ||
                name.endsWith(QLatin1Char(' ')))
            {
                error = QStringLiteral("Choose a valid folder name.");
            }
            else
            {
                const QString forbiddenCharacters =
                    QStringLiteral("<>:\"/\\|?*");
                for (const QChar character : name)
                {
                    if (character.unicode() < 32 ||
                        forbiddenCharacters.contains(character))
                    {
                        error = QStringLiteral(
                            "The project name contains a character Windows cannot use in a folder name.");
                        break;
                    }
                }
            }

            const QString deviceName = name.section(QLatin1Char('.'), 0, 0)
                .toUpper();
            if (error.isEmpty() &&
                (deviceName == QStringLiteral("CON") ||
                    deviceName == QStringLiteral("PRN") ||
                    deviceName == QStringLiteral("AUX") ||
                    deviceName == QStringLiteral("NUL") ||
                    (deviceName.size() == 4 &&
                        (deviceName.startsWith(QStringLiteral("COM")) ||
                            deviceName.startsWith(QStringLiteral("LPT"))) &&
                        deviceName.back() >= QLatin1Char('1') &&
                        deviceName.back() <= QLatin1Char('9'))))
            {
                error = QStringLiteral("That name is reserved by Windows.");
            }

            if (error.isEmpty() && !QFileInfo(location).isDir())
            {
                error = QStringLiteral("Choose an existing parent folder.");
            }

            const QString target = QDir(location).filePath(name);
            if (error.isEmpty() && QFileInfo::exists(target))
            {
                error = QStringLiteral("A folder or file with that name already exists.");
            }

            if (error.isEmpty())
            {
                m_pathPreview->setText(QStringLiteral("Project folder: %1")
                    .arg(QDir::toNativeSeparators(target)));
            }
            else
            {
                m_pathPreview->setText(error);
            }
            m_createButton->setEnabled(error.isEmpty());
        }

        QLineEdit* m_nameEdit = nullptr;
        QLineEdit* m_locationEdit = nullptr;
        QLabel* m_pathPreview = nullptr;
        QPushButton* m_createButton = nullptr;
        QFileDialog* m_locationDialog = nullptr;
    };
}

namespace BootstrapForge::Client::UI
{
    MainWindow::MainWindow(
        QWidget* parent)
        :
        QMainWindow(parent)
    {
        ConfigureWindow();

        m_buildController =
            new Build::CMakeProjectController(this);

        CreateActions();

        CreateMenus();

        CreateToolBar();

        CreateProjectDock();

        CreatePropertiesDock();

        CreateOutputDock();

        CreateCentralWidget();

        CreateStatusBar();

        ConnectActions();

        RebuildTestMenu({});
        UpdateEditorActions();

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
        m_newAction = new QAction(
            QStringLiteral("&New Project..."), this);

        m_newAction->setShortcut(
            QKeySequence::New);

        m_newAction->setStatusTip(QStringLiteral(
            "Create an empty project folder to build into"));

        m_openAction = new QAction(
            QStringLiteral("Open File..."), this);

        m_openAction->setShortcut(
            QKeySequence::Open);

        m_openAction->setStatusTip(
            QStringLiteral(
                "Open a source file"));

        m_openProjectAction = new QAction(
            QStringLiteral("Open Project..."), this);
        m_openProjectAction->setShortcut(
            QKeySequence(QStringLiteral("Ctrl+Shift+O")));
        m_openProjectAction->setStatusTip(
            QStringLiteral("Open an existing project folder"));

        m_generateAction = new QAction(
            QStringLiteral("Generate from Command..."), this);
        m_generateAction->setShortcut(
            QKeySequence(QStringLiteral("Ctrl+Shift+G")));
        m_generateAction->setStatusTip(
            QStringLiteral("Preview and apply a BootstrapForge generate command"));

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

        m_undoAction = new QAction(QStringLiteral("Undo"), this);
        m_undoAction->setShortcut(QKeySequence::Undo);
        m_redoAction = new QAction(QStringLiteral("Redo"), this);
        m_redoAction->setShortcut(QKeySequence::Redo);
        m_cutAction = new QAction(QStringLiteral("Cut"), this);
        m_cutAction->setShortcut(QKeySequence::Cut);
        m_copyAction = new QAction(QStringLiteral("Copy"), this);
        m_copyAction->setShortcut(QKeySequence::Copy);
        m_pasteAction = new QAction(QStringLiteral("Paste"), this);
        m_pasteAction->setShortcut(QKeySequence::Paste);

        m_resetLayoutAction = new QAction(
            QStringLiteral("Reset Window Layout"), this);

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

        m_configureAction = new QAction(
            QStringLiteral("Check CMake"), this);
        m_configureAction->setStatusTip(
            QStringLiteral("Configure the current CMake project and report configuration errors"));

        m_buildAction =
            new QAction(
                QStringLiteral("Build"),
                this);

        m_buildAction->setStatusTip(
            QStringLiteral(
                "Build the current project"));

        m_runAction = new QAction(QStringLiteral("Run"), this);
        m_runAction->setShortcut(QKeySequence(Qt::Key_F5));
        m_runAction->setStatusTip(
            QStringLiteral("Build the current project and launch an application executable"));

        m_testMenu = new QMenu(QStringLiteral("Test"), this);
        m_testAction = m_testMenu->menuAction();

        m_testAction->setStatusTip(
            QStringLiteral(
                "Run the project tests"));

        m_buildAction->setEnabled(false);
        m_runAction->setEnabled(false);
        m_testAction->setEnabled(false);
        m_configureAction->setEnabled(false);

        m_gitStatusAction =
            new QAction(
                QStringLiteral(
                    "Git Status"),
                this);

        m_gitStatusAction->setStatusTip(
            QStringLiteral(
                "Show Git status"));

        m_createBranchAction = new QAction(QStringLiteral("Create Branch..."), this);
        m_switchBranchAction = new QAction(QStringLiteral("Switch Branch..."), this);
        m_architectureAction = new QAction(QStringLiteral("Architecture Overview"), this);
        m_settingsAction = new QAction(QStringLiteral("Settings..."), this);
        m_aboutAction = new QAction(QStringLiteral("About BootstrapForge"), this);

        m_newAction->setEnabled(true);
        m_openProjectAction->setEnabled(true);
        m_openAction->setEnabled(true);
        m_saveAction->setEnabled(false);
        m_generateAction->setEnabled(false);
        m_gitStatusAction->setEnabled(false);
        m_createBranchAction->setEnabled(false);
        m_switchBranchAction->setEnabled(false);
        m_undoAction->setEnabled(false);
        m_redoAction->setEnabled(false);
        m_cutAction->setEnabled(false);
        m_copyAction->setEnabled(false);
        m_pasteAction->setEnabled(false);

    }

    void MainWindow::CreateMenus()
    {
        QMenu* fileMenu =
            menuBar()->addMenu(
                QStringLiteral("&File"));

        fileMenu->addAction(
            m_newAction);

        fileMenu->addAction(
            m_openProjectAction);

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

        editMenu->addAction(m_undoAction);

        editMenu->addAction(m_redoAction);

        editMenu->addSeparator();

        editMenu->addAction(m_cutAction);

        editMenu->addAction(m_copyAction);

        editMenu->addAction(m_pasteAction);

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
        viewMenu->addAction(m_resetLayoutAction);

        QMenu* projectMenu =
            menuBar()->addMenu(
                QStringLiteral("&Project"));

        projectMenu->addAction(m_newAction);
        projectMenu->addAction(m_openProjectAction);
        projectMenu->addSeparator();
        projectMenu->addAction(m_generateAction);

        QMenu* buildMenu =
            menuBar()->addMenu(
                QStringLiteral("&Build"));

        buildMenu->addAction(
            m_buildAction);

        buildMenu->addAction(m_runAction);

        buildMenu->addAction(
            m_configureAction);

        buildMenu->addMenu(m_testMenu);

        QMenu* gitMenu =
            menuBar()->addMenu(
                QStringLiteral("&Git"));

        gitMenu->addAction(
            m_gitStatusAction);

        gitMenu->addSeparator();

        gitMenu->addAction(m_createBranchAction);

        gitMenu->addAction(m_switchBranchAction);

        QMenu* toolsMenu =
            menuBar()->addMenu(
                QStringLiteral("&Tools"));

        toolsMenu->addAction(m_architectureAction);

        toolsMenu->addAction(m_settingsAction);

        QMenu* helpMenu = menuBar()->addMenu(QStringLiteral("&Help"));
        helpMenu->addAction(m_aboutAction);
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
            m_openProjectAction);

        toolBar->addAction(
            m_openAction);

        toolBar->addAction(m_generateAction);

        toolBar->addAction(
            m_saveAction);

        toolBar->addSeparator();

        toolBar->addAction(
            m_buildAction);

        toolBar->addAction(m_runAction);

        toolBar->addAction(
            m_configureAction);

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

        m_welcomePage = new QWidget(m_workspaceTabs);
        m_welcomePage->setObjectName(QStringLiteral("ProjectLauncher"));
        auto* outerLayout = new QVBoxLayout(m_welcomePage);
        outerLayout->setContentsMargins(32, 28, 32, 28);
        outerLayout->addStretch(1);

        auto* content = new QWidget(m_welcomePage);
        content->setMaximumWidth(760);
        auto* contentLayout = new QVBoxLayout(content);
        contentLayout->setSpacing(14);

        auto* eyebrow = new QLabel(
            QStringLiteral("BOOTSTRAPFORGE  /  PROJECT WORKSPACE BUILDER"),
            content);
        eyebrow->setAlignment(Qt::AlignCenter);
        eyebrow->setStyleSheet(QStringLiteral(
            "color: palette(highlight); font-weight: 600; letter-spacing: 1px;"));
        contentLayout->addWidget(eyebrow);

        auto* title = new QLabel(
            QStringLiteral("What are you building?"), content);
        title->setObjectName(QStringLiteral("ProjectLauncherTitle"));
        title->setAlignment(Qt::AlignCenter);
        QFont titleFont = title->font();
        titleFont.setPointSize(titleFont.pointSize() + 10);
        titleFont.setBold(true);
        title->setFont(titleFont);
        contentLayout->addWidget(title);

        auto* subtitle = new QLabel(
            QStringLiteral("Create a project workspace or open one you already have."),
            content);
        subtitle->setAlignment(Qt::AlignCenter);
        subtitle->setWordWrap(true);
        contentLayout->addWidget(subtitle);

        auto* actionRow = new QHBoxLayout();
        actionRow->setSpacing(12);
        auto* createButton = new QPushButton(
            QStringLiteral("Create New Project"), content);
        createButton->setObjectName(QStringLiteral("PrimaryProjectAction"));
        createButton->setMinimumSize(210, 46);
        createButton->setStyleSheet(QStringLiteral(
            "QPushButton { background-color: #4269e1; color: white; "
            "border: 0; border-radius: 6px; padding: 10px 18px; font-weight: 600; }"
            "QPushButton:hover { background-color: #5278ed; }"
            "QPushButton:pressed { background-color: #3457c4; }"));
        auto* openButton = new QPushButton(
            QStringLiteral("Open Existing Project"), content);
        openButton->setMinimumSize(210, 46);
        actionRow->addStretch();
        actionRow->addWidget(createButton);
        actionRow->addWidget(openButton);
        actionRow->addStretch();
        contentLayout->addLayout(actionRow);

        m_welcomeProjectLabel = new QLabel(
            QStringLiteral("No project open"), content);
        m_welcomeProjectLabel->setAlignment(Qt::AlignCenter);
        m_welcomeProjectLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        contentLayout->addWidget(m_welcomeProjectLabel);

        auto* recentFrame = new QFrame(content);
        recentFrame->setFrameShape(QFrame::StyledPanel);
        recentFrame->setStyleSheet(QStringLiteral(
            "QFrame { border: 1px solid palette(mid); border-radius: 8px; }"));
        auto* recentLayout = new QVBoxLayout(recentFrame);
        recentLayout->setContentsMargins(14, 12, 14, 14);
        auto* recentHeading = new QLabel(
            QStringLiteral("Recent projects"), recentFrame);
        QFont recentFont = recentHeading->font();
        recentFont.setBold(true);
        recentHeading->setFont(recentFont);
        recentHeading->setStyleSheet(QStringLiteral("border: none;"));
        recentLayout->addWidget(recentHeading);

        m_recentProjectsList = new QListWidget(recentFrame);
        m_recentProjectsList->setObjectName(QStringLiteral("RecentProjectsList"));
        m_recentProjectsList->setMinimumHeight(150);
        m_recentProjectsList->setMaximumHeight(210);
        m_recentProjectsList->setStyleSheet(QStringLiteral(
            "QListWidget { border: none; background: transparent; }"));
        recentLayout->addWidget(m_recentProjectsList);
        contentLayout->addWidget(recentFrame);

        auto* hint = new QLabel(
            QStringLiteral("New projects start empty. You choose what the AI-generated "
                "command creates, and review the plan before applying it."),
            content);
        hint->setAlignment(Qt::AlignCenter);
        hint->setWordWrap(true);
        contentLayout->addWidget(hint);

        outerLayout->addWidget(content, 0, Qt::AlignHCenter);
        outerLayout->addStretch(1);

        m_workspaceTabs->addTab(
            m_welcomePage,
            QStringLiteral("Start"));
        m_workspaceTabs->tabBar()->setTabButton(
            0, QTabBar::RightSide, nullptr);

        m_editorManager =
            new Editor::EditorManager(
                *m_workspaceTabs,
                this);

        setCentralWidget(
            m_workspaceTabs);

        connect(createButton, &QPushButton::clicked,
            this, [this] { m_newAction->trigger(); });
        connect(openButton, &QPushButton::clicked,
            this, [this] { m_openProjectAction->trigger(); });
        connect(m_recentProjectsList, &QListWidget::itemDoubleClicked,
            this, [this](QListWidgetItem* item)
            {
                if (item != nullptr)
                {
                    const QString projectPath =
                        item->data(Qt::UserRole).toString();
                    QTimer::singleShot(0, this, [this, projectPath]
                    {
                        OpenProjectWorkspace(projectPath);
                    });
                }
            });
        RefreshRecentProjects();
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
                CreateNewProject();
            });

        connect(
            m_openAction,
            &QAction::triggered,
            this,
            [this]
            {
                const QString startingDirectory =
                    m_projectTree->WorkspaceRoot().isEmpty()
                    ? QStandardPaths::writableLocation(
                        QStandardPaths::DocumentsLocation)
                    : m_projectTree->WorkspaceRoot();

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
            m_openProjectAction,
            &QAction::triggered,
            this,
            [this]
            {
                if (m_openProjectDialog != nullptr)
                {
                    m_openProjectDialog->raise();
                    m_openProjectDialog->activateWindow();
                    return;
                }

                const QString startingDirectory =
                    m_projectTree->WorkspaceRoot().isEmpty()
                    ? QStandardPaths::writableLocation(
                        QStandardPaths::DocumentsLocation)
                    : QFileInfo(m_projectTree->WorkspaceRoot()).absolutePath();
                auto* dialog = new QFileDialog(this);
                ConfigureDirectoryPicker(
                    *dialog,
                    QStringLiteral("Open Existing Project"),
                    startingDirectory);
                m_openProjectDialog = dialog;

                connect(dialog, &QFileDialog::accepted,
                    this, [this, dialog]
                    {
                        const QStringList selectedPaths =
                            dialog->selectedFiles();
                        if (!selectedPaths.isEmpty())
                        {
                            OpenProjectWorkspace(selectedPaths.front());
                        }
                    });
                connect(dialog, &QFileDialog::finished,
                    this, [this, dialog](int)
                    {
                        if (m_openProjectDialog == dialog)
                        {
                            m_openProjectDialog = nullptr;
                        }
                        dialog->deleteLater();
                    });
                dialog->open();
            });

        connect(
            m_generateAction,
            &QAction::triggered,
            this,
            [this]
            {
                const QString root = m_projectTree->WorkspaceRoot();
                if (root.isEmpty())
                {
                    ShowErrorMessage(
                        QStringLiteral("Create or open a project before generating."));
                    return;
                }

                Generation::GenerateDialog dialog(root, this);
                dialog.exec();
                if (!dialog.ExecutionSummary().isEmpty())
                {
                    m_outputEditor->appendPlainText(
                        QStringLiteral("[GENERATION]\n%1")
                            .arg(dialog.ExecutionSummary()));
                    m_projectTree->Refresh();
                }
            });

        connect(
            m_saveAction,
            &QAction::triggered,
            this,
            [this]
            {
                SaveCurrentDocument();
            });

        connect(m_undoAction, &QAction::triggered, this, [this]
        {
            if (auto* document = m_editorManager->CurrentDocument())
            {
                document->undo();
            }
        });
        connect(m_redoAction, &QAction::triggered, this, [this]
        {
            if (auto* document = m_editorManager->CurrentDocument())
            {
                document->redo();
            }
        });
        connect(m_cutAction, &QAction::triggered, this, [this]
        {
            if (auto* document = m_editorManager->CurrentDocument())
            {
                document->cut();
            }
        });
        connect(m_copyAction, &QAction::triggered, this, [this]
        {
            if (auto* document = m_editorManager->CurrentDocument())
            {
                document->copy();
            }
        });
        connect(m_pasteAction, &QAction::triggered, this, [this]
        {
            if (auto* document = m_editorManager->CurrentDocument())
            {
                document->paste();
            }
        });

        connect(m_resetLayoutAction, &QAction::triggered,
            this, [this] { ResetWindowLayout(); });

        connect(
            m_buildAction,
            &QAction::triggered,
            this,
            [this]
            {
                m_buildController->Build();
            });

        connect(m_runAction, &QAction::triggered,
            this, [this]
            {
                if (m_runAfterBuild || m_buildController->IsBusy())
                {
                    return;
                }
                m_runAfterBuild = true;
                m_buildController->Build();
            });

        connect(
            m_configureAction,
            &QAction::triggered,
            m_buildController,
            &Build::CMakeProjectController::Configure);

        connect(
            m_buildController,
            &Build::CMakeProjectController::OutputReceived,
            this,
            [this](const QString& text)
            {
                m_outputEditor->moveCursor(QTextCursor::End);
                m_outputEditor->insertPlainText(text);
                m_outputEditor->ensureCursorVisible();
            });

        connect(
            m_buildController,
            &Build::CMakeProjectController::TestsDiscovered,
            this,
            [this](const QStringList& testNames)
            {
                QTimer::singleShot(0, this, [this, testNames]
                {
                    m_testNames = testNames;
                    RebuildTestMenu(testNames);
                });
            });

        connect(
            m_buildController,
            &Build::CMakeProjectController::BusyChanged,
            this,
            [this](bool busy)
            {
                const bool hasProject =
                    !m_projectTree->WorkspaceRoot().isEmpty();
                m_newAction->setEnabled(!busy);
                m_openProjectAction->setEnabled(!busy);
                m_openAction->setEnabled(!busy);
                m_generateAction->setEnabled(hasProject && !busy);
                m_saveAction->setEnabled(!busy);
                m_buildAction->setEnabled(hasProject && !busy);
                m_runAction->setEnabled(hasProject && !busy);
                m_configureAction->setEnabled(hasProject && !busy);
                m_gitStatusAction->setEnabled(hasProject && !busy);
                m_createBranchAction->setEnabled(hasProject && !busy);
                m_switchBranchAction->setEnabled(hasProject && !busy);
                m_testAction->setEnabled(hasProject && !busy);
                UpdateEditorActions();
            });

        connect(
            m_buildController,
            &Build::CMakeProjectController::OperationFinished,
            this,
            [this](const QString& operation, bool succeeded)
            {
                statusBar()->showMessage(
                    QStringLiteral("%1 %2")
                        .arg(operation,
                            succeeded ? QStringLiteral("completed")
                                      : QStringLiteral("failed")),
                    6000);
                if (operation == QStringLiteral("Build") && m_runAfterBuild)
                {
                    m_runAfterBuild = false;
                    if (succeeded)
                    {
                        RunBuiltApplication();
                    }
                }
            });

        connect(m_gitStatusAction, &QAction::triggered,
            this, [this] { RunGitStatus(); });
        connect(m_createBranchAction, &QAction::triggered,
            this, [this] { CreateGitBranch(); });
        connect(m_switchBranchAction, &QAction::triggered,
            this, [this] { SwitchGitBranch(); });
        connect(m_architectureAction, &QAction::triggered,
            this, [this] { ShowArchitectureOverview(); });
        connect(m_settingsAction, &QAction::triggered,
            this, [this] { ShowSettingsDialog(); });
        connect(m_aboutAction, &QAction::triggered,
            this, [this] { ShowAboutDialog(); });

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
                UpdateEditorActions();
            });

        connect(
            m_editorManager,
            &Editor::EditorManager::DocumentModifiedChanged,
            this,
            [this](
                const QString& path,
                bool modified)
            {
                UpdateEditorActions();
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
                UpdateEditorActions();
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
                    UpdateEditorActions();

                    return;
                }

                statusBar()->showMessage(
                    path);

                UpdateEditorProperties();
                UpdateEditorActions();
            });
    }

    void MainWindow::RebuildTestMenu(const QStringList& testNames)
    {
        if (m_testMenu == nullptr)
        {
            return;
        }

        m_testMenu->clear();
        const bool canUseProject = !m_projectTree->WorkspaceRoot().isEmpty();
        const bool canRun = canUseProject && !m_buildController->IsBusy();
        m_testAction->setEnabled(canRun);

        QAction* refreshAction = m_testMenu->addAction(
            QStringLiteral("Refresh Tests"));
        refreshAction->setEnabled(canRun);
        connect(refreshAction, &QAction::triggered, m_buildController,
            &Build::CMakeProjectController::RefreshTests);

        m_testMenu->addSeparator();
        if (testNames.isEmpty())
        {
            QAction* emptyAction = m_testMenu->addAction(
                canUseProject
                    ? QStringLiteral("No tests discovered — check CMake or refresh")
                    : QStringLiteral("Open a project to discover tests"));
            emptyAction->setEnabled(false);
            return;
        }

        QAction* runAllAction = m_testMenu->addAction(
            QStringLiteral("Run All Tests (%1)").arg(testNames.size()));
        runAllAction->setEnabled(canRun);
        connect(runAllAction, &QAction::triggered, m_buildController,
            &Build::CMakeProjectController::RunAllTests);
        m_testMenu->addSeparator();

        for (const QString& testName : testNames)
        {
            QAction* testAction = m_testMenu->addAction(testName);
            testAction->setEnabled(canRun);
            testAction->setToolTip(QStringLiteral("Run CTest: %1").arg(testName));
            connect(testAction, &QAction::triggered, m_buildController,
                [this, testName]
                {
                    m_buildController->RunTest(testName);
                });
        }
    }

    void MainWindow::ResetWindowLayout()
    {
        addDockWidget(Qt::LeftDockWidgetArea, m_projectDock);
        addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);
        addDockWidget(Qt::BottomDockWidgetArea, m_outputDock);
        m_projectDock->setFloating(false);
        m_propertiesDock->setFloating(false);
        m_outputDock->setFloating(false);
        resizeDocks({m_projectDock, m_propertiesDock}, {260, 280}, Qt::Horizontal);
        resizeDocks({m_outputDock}, {180}, Qt::Vertical);

        const bool hasProject = !m_projectTree->WorkspaceRoot().isEmpty();
        m_projectDockAction->setChecked(hasProject);
        m_propertiesDockAction->setChecked(hasProject);
        m_outputDockAction->setChecked(true);
        m_projectDock->setVisible(hasProject);
        m_propertiesDock->setVisible(hasProject);
        m_outputDock->setVisible(true);
        if (hasProject)
        {
            m_projectDock->raise();
        }
        ShowInformationMessage(QStringLiteral("Window layout restored."));
    }

    void MainWindow::ShowArchitectureOverview()
    {
        QMessageBox::information(
            this,
            QStringLiteral("BootstrapForge Architecture"),
            QStringLiteral(
                "Generation flow\n"
                "Command input → parser and typed tree → validation and deterministic plan → preview → filesystem executor.\n\n"
                "The Architecture library stays independent of Qt. The Client provides the editor, workspace tree, previews, and build/test controls."));
    }

    void MainWindow::ShowSettingsDialog()
    {
        QSettings settings(
            QSettings::IniFormat,
            QSettings::UserScope,
            QStringLiteral("BootstrapForge"),
            QStringLiteral("BootstrapForge.Client"));

        QDialog dialog(this);
        dialog.setWindowTitle(QStringLiteral("BootstrapForge Settings"));
        auto* layout = new QVBoxLayout(&dialog);
        auto* form = new QFormLayout();
        auto* cmakeEdit = new QLineEdit(
            settings.value(QStringLiteral("tools/cmakeExecutable")).toString(),
            &dialog);
        auto* gitEdit = new QLineEdit(
            settings.value(QStringLiteral("tools/gitExecutable")).toString(),
            &dialog);
        auto addExecutableRow = [&dialog, form](
            const QString& label, QLineEdit* edit, const QString& caption)
        {
            auto* row = new QWidget(&dialog);
            auto* rowLayout = new QHBoxLayout(row);
            rowLayout->setContentsMargins(0, 0, 0, 0);
            auto* browse = new QPushButton(QStringLiteral("Browse..."), row);
            rowLayout->addWidget(edit, 1);
            rowLayout->addWidget(browse);
            QObject::connect(browse, &QPushButton::clicked, &dialog,
                [&dialog, edit, caption]
                {
                    const QString path = QFileDialog::getOpenFileName(
                        &dialog, caption, edit->text(),
                        QStringLiteral("Executables (*.exe);;All files (*.*)"));
                    if (!path.isEmpty())
                    {
                        edit->setText(path);
                    }
                });
            form->addRow(label, row);
        };
        addExecutableRow(QStringLiteral("CMake executable"), cmakeEdit,
            QStringLiteral("Choose CMake executable"));
        addExecutableRow(QStringLiteral("Git executable"), gitEdit,
            QStringLiteral("Choose Git executable"));
        layout->addLayout(form);
        layout->addWidget(new QLabel(
            QStringLiteral("Leave a field empty to use automatic discovery."),
            &dialog));

        auto* buttons = new QDialogButtonBox(
            QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
        layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if (dialog.exec() != QDialog::Accepted)
        {
            return;
        }

        settings.setValue(QStringLiteral("tools/cmakeExecutable"), cmakeEdit->text().trimmed());
        settings.setValue(QStringLiteral("tools/gitExecutable"), gitEdit->text().trimmed());
        ShowInformationMessage(QStringLiteral("Tool settings saved."));
        if (!m_projectTree->WorkspaceRoot().isEmpty())
        {
            m_buildController->RefreshTests();
        }
    }

    void MainWindow::ShowAboutDialog()
    {
        QMessageBox::about(
            this,
            QStringLiteral("About BootstrapForge"),
            QStringLiteral("BootstrapForge\nArchitecture workspace and development environment\n\nQt %1")
                .arg(QString::fromLatin1(qVersion())));
    }

    QString MainWindow::GitExecutable() const
    {
        QSettings settings(
            QSettings::IniFormat,
            QSettings::UserScope,
            QStringLiteral("BootstrapForge"),
            QStringLiteral("BootstrapForge.Client"));
        const QString configured = settings.value(
            QStringLiteral("tools/gitExecutable")).toString();
        if (!configured.isEmpty() && QFileInfo(configured).isFile())
        {
            return configured;
        }

        QString executable = QStandardPaths::findExecutable(QStringLiteral("git"));
        if (!executable.isEmpty())
        {
            return executable;
        }

        const QString programFiles = qEnvironmentVariable("ProgramFiles");
        const QString programFilesX86 = qEnvironmentVariable("ProgramFiles(x86)");
        for (const QString& root : {programFiles, programFilesX86})
        {
            if (root.isEmpty())
            {
                continue;
            }
            const QString candidate = QDir(root).filePath(
                QStringLiteral("Git/cmd/git.exe"));
            if (QFileInfo(candidate).isFile())
            {
                return candidate;
            }
        }
        return {};
    }

    bool MainWindow::RunGitCommand(
        const QStringList& arguments,
        QString& output,
        QString& errorMessage) const
    {
        const QString root = m_projectTree->WorkspaceRoot();
        if (root.isEmpty())
        {
            errorMessage = QStringLiteral("Open a project before using Git.");
            return false;
        }

        const QString git = GitExecutable();
        if (git.isEmpty())
        {
            errorMessage = QStringLiteral(
                "Git was not found. Set its executable path in Tools > Settings.");
            return false;
        }

        QProcess process;
        process.setProgram(git);
        QStringList fullArguments{QStringLiteral("-C"), root};
        fullArguments.append(arguments);
        process.setArguments(fullArguments);
        process.start();
        if (!process.waitForStarted(3000))
        {
            errorMessage = process.errorString();
            return false;
        }
        if (!process.waitForFinished(30000))
        {
            process.kill();
            process.waitForFinished();
            errorMessage = QStringLiteral("Git did not finish within 30 seconds.");
            return false;
        }

        output = QString::fromLocal8Bit(process.readAllStandardOutput());
        const QString standardError = QString::fromLocal8Bit(process.readAllStandardError());
        if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
        {
            errorMessage = standardError.trimmed().isEmpty()
                ? QStringLiteral("Git exited with code %1.").arg(process.exitCode())
                : standardError.trimmed();
            return false;
        }
        if (!standardError.trimmed().isEmpty())
        {
            output += standardError;
        }
        return true;
    }

    void MainWindow::RunGitStatus()
    {
        QString output;
        QString error;
        if (!RunGitCommand({QStringLiteral("status"), QStringLiteral("--short"),
                QStringLiteral("--branch")}, output, error))
        {
            ShowErrorMessage(error);
            return;
        }
        m_outputEditor->appendPlainText(QStringLiteral("[GIT STATUS]\n%1")
            .arg(output.trimmed().isEmpty()
                ? QStringLiteral("Working tree clean.") : output.trimmed()));
        statusBar()->showMessage(QStringLiteral("Git status updated."), 5000);
    }

    void MainWindow::CreateGitBranch()
    {
        bool accepted = false;
        const QString name = QInputDialog::getText(
            this, QStringLiteral("Create Git Branch"),
            QStringLiteral("Branch name"), QLineEdit::Normal, {}, &accepted).trimmed();
        if (!accepted || name.isEmpty())
        {
            return;
        }
        QString output;
        QString error;
        if (!RunGitCommand({QStringLiteral("switch"), QStringLiteral("--create"), name},
                output, error))
        {
            ShowErrorMessage(error);
            return;
        }
        m_outputEditor->appendPlainText(QStringLiteral("[GIT] Created and switched to branch %1.").arg(name));
        statusBar()->showMessage(QStringLiteral("Switched to %1").arg(name), 5000);
    }

    void MainWindow::SwitchGitBranch()
    {
        QString output;
        QString error;
        if (!RunGitCommand({QStringLiteral("branch"),
                QStringLiteral("--format=%(refname:short)")}, output, error))
        {
            ShowErrorMessage(error);
            return;
        }
        QStringList branches = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        if (branches.isEmpty())
        {
            ShowInformationMessage(QStringLiteral("This repository has no local branches."));
            return;
        }
        bool accepted = false;
        const QString selected = QInputDialog::getItem(
            this, QStringLiteral("Switch Git Branch"), QStringLiteral("Branch"),
            branches, 0, false, &accepted);
        if (!accepted || selected.isEmpty())
        {
            return;
        }
        if (!RunGitCommand({QStringLiteral("switch"), selected}, output, error))
        {
            ShowErrorMessage(error);
            return;
        }
        m_outputEditor->appendPlainText(QStringLiteral("[GIT] Switched to branch %1.").arg(selected));
        statusBar()->showMessage(QStringLiteral("Switched to %1").arg(selected), 5000);
    }

    void MainWindow::RunBuiltApplication()
    {
        const QStringList applications = m_buildController->RunnableApplications();
        if (applications.isEmpty())
        {
            ShowErrorMessage(QStringLiteral(
                "The build completed, but no runnable application executable was found."));
            return;
        }

        QString executablePath;
        if (applications.size() == 1)
        {
            executablePath = applications.front();
        }
        else
        {
            QStringList choices;
            for (const QString& application : applications)
            {
                choices.push_back(QDir(m_buildController->BuildDirectory())
                    .relativeFilePath(application));
            }

            bool accepted = false;
            const QString selected = QInputDialog::getItem(
                this,
                QStringLiteral("Run Application"),
                QStringLiteral("Choose an executable to run"),
                choices,
                0,
                false,
                &accepted);
            if (!accepted)
            {
                return;
            }

            const int selectedIndex = choices.indexOf(selected);
            if (selectedIndex < 0)
            {
                return;
            }
            executablePath = applications.at(selectedIndex);
        }

        QString errorMessage;
        if (!m_buildController->LaunchApplication(executablePath, errorMessage))
        {
            ShowErrorMessage(errorMessage);
            return;
        }

        m_outputEditor->appendPlainText(
            QStringLiteral("[RUN] Launched %1")
                .arg(QDir::toNativeSeparators(executablePath)));
        statusBar()->showMessage(QStringLiteral("Application launched."), 5000);
    }

    void MainWindow::UpdateEditorActions()
    {
        const bool hasDocument = m_editorManager != nullptr &&
            m_editorManager->CurrentDocument() != nullptr;
        const bool canEdit = hasDocument && !m_buildController->IsBusy();
        m_saveAction->setEnabled(canEdit);
        m_undoAction->setEnabled(canEdit);
        m_redoAction->setEnabled(canEdit);
        m_cutAction->setEnabled(canEdit);
        m_copyAction->setEnabled(canEdit);
        m_pasteAction->setEnabled(canEdit);
    }

    bool MainWindow::ConfirmUnsavedDocuments()
    {
        for (int index = 0; index < m_workspaceTabs->count(); ++index)
        {
            if (!m_editorManager->IsEditorTab(index) ||
                !m_editorManager->IsModified(index))
            {
                continue;
            }

            const QString path = m_editorManager->FilePath(index);
            const auto choice = QMessageBox::warning(
                this, QStringLiteral("Unsaved Changes"),
                QStringLiteral("Save changes to:\n\n%1").arg(path),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                QMessageBox::Save);
            if (choice == QMessageBox::Cancel)
            {
                return false;
            }
            if (choice == QMessageBox::Save)
            {
                QString error;
                if (!m_editorManager->SaveFile(index, error))
                {
                    ShowErrorMessage(error);
                    return false;
                }
            }
        }
        return true;
    }

    void MainWindow::closeEvent(QCloseEvent* event)
    {
        if (ConfirmUnsavedDocuments())
        {
            event->accept();
        }
        else
        {
            event->ignore();
        }
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
            SetProjectWorkspacePanelsVisible(false);
            m_outputEditor->appendPlainText(
                QStringLiteral(
                    "No project is open. Create or open one to begin."));
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

    void MainWindow::CreateNewProject()
    {
        NewProjectDialog dialog(this);
        if (dialog.exec() != QDialog::Accepted)
        {
            return;
        }

        const QString projectPath = dialog.ProjectPath();
        const QFileInfo parentInfo(QFileInfo(projectPath).absolutePath());
        if (!parentInfo.isDir() || !QDir().mkdir(projectPath))
        {
            ShowErrorMessage(QStringLiteral(
                "Could not create the project folder. It may already exist or the parent folder may not be writable:\n%1")
                .arg(QDir::toNativeSeparators(projectPath)));
            return;
        }

        QTimer::singleShot(0, this, [this, projectPath]
        {
            OpenProjectWorkspace(projectPath);
        });
    }

    void MainWindow::OpenProjectWorkspace(
        const QString& path)
    {
        if (m_buildController->IsBusy())
        {
            ShowInformationMessage(
                QStringLiteral("Wait for the current CMake operation to finish before switching projects."));
            return;
        }

        if (!m_projectTree->SetWorkspaceRoot(path))
        {
            ShowErrorMessage(QStringLiteral(
                "Could not open this project folder:\n%1")
                .arg(QDir::toNativeSeparators(path)));
            return;
        }

        const QString projectPath = QDir::cleanPath(
            QFileInfo(path).absoluteFilePath());
        const QString projectName = QFileInfo(projectPath).fileName().isEmpty()
            ? projectPath
            : QFileInfo(projectPath).fileName();
        if (m_welcomeProjectLabel != nullptr)
        {
            m_welcomeProjectLabel->setText(QStringLiteral("Current project: %1")
                .arg(QDir::toNativeSeparators(projectPath)));
        }
        if (m_welcomePage != nullptr)
        {
            const int welcomeIndex = m_workspaceTabs->indexOf(m_welcomePage);
            if (welcomeIndex >= 0)
            {
                m_workspaceTabs->removeTab(welcomeIndex);
            }
            delete m_welcomePage;
            m_welcomePage = nullptr;
            m_welcomeProjectLabel = nullptr;
            m_recentProjectsList = nullptr;
        }
        SetProjectWorkspacePanelsVisible(true);
        m_buildController->SetProjectRoot(projectPath);
        m_buildAction->setEnabled(true);
        m_runAction->setEnabled(true);
        m_configureAction->setEnabled(true);
        m_testAction->setEnabled(true);
        m_gitStatusAction->setEnabled(true);
        m_createBranchAction->setEnabled(true);
        m_switchBranchAction->setEnabled(true);
        m_buildController->RefreshTests();
        setWindowTitle(QStringLiteral("%1 — BootstrapForge")
            .arg(projectName));

        AddRecentProject(projectPath);
        m_outputEditor->appendPlainText(
            QStringLiteral("[PROJECT] Opened: %1")
                .arg(QDir::toNativeSeparators(projectPath)));
        statusBar()->showMessage(QStringLiteral("Project: %1")
            .arg(projectName), 5000);
    }

    void MainWindow::RefreshRecentProjects()
    {
        if (m_recentProjectsList == nullptr)
        {
            return;
        }

        QSettings settings(
            QSettings::IniFormat,
            QSettings::UserScope,
            QStringLiteral("BootstrapForge"),
            QStringLiteral("BootstrapForge.Client"));
        QStringList recent = settings.value(
            QStringLiteral("projects/recent")).toStringList();
        QStringList validProjects;
        m_recentProjectsList->clear();

        for (const QString& path : recent)
        {
            const QFileInfo info(path);
            if (!info.exists() || !info.isDir())
            {
                continue;
            }

            const QString absolutePath = QDir::cleanPath(
                info.absoluteFilePath());
            const bool duplicate = std::any_of(
                validProjects.cbegin(),
                validProjects.cend(),
                [&absolutePath](const QString& existing)
                {
                    return QString::compare(
                        existing, absolutePath, Qt::CaseInsensitive) == 0;
                });
            if (duplicate)
            {
                continue;
            }

            validProjects.push_back(absolutePath);
            auto* item = new QListWidgetItem(
                QFileInfo(absolutePath).fileName().isEmpty()
                    ? QDir::toNativeSeparators(absolutePath)
                    : QFileInfo(absolutePath).fileName(),
                m_recentProjectsList);
            item->setToolTip(QDir::toNativeSeparators(absolutePath));
            item->setData(Qt::UserRole, absolutePath);
            if (validProjects.size() == 10)
            {
                break;
            }
        }

        if (validProjects.isEmpty())
        {
            auto* emptyItem = new QListWidgetItem(
                QStringLiteral("No recent projects yet"),
                m_recentProjectsList);
            emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsEnabled);
        }

        settings.setValue(
            QStringLiteral("projects/recent"),
            validProjects);
    }

    void MainWindow::AddRecentProject(
        const QString& path)
    {
        const QString absolutePath = QDir::cleanPath(
            QFileInfo(path).absoluteFilePath());

        QSettings settings(
            QSettings::IniFormat,
            QSettings::UserScope,
            QStringLiteral("BootstrapForge"),
            QStringLiteral("BootstrapForge.Client"));
        QStringList recent = settings.value(
            QStringLiteral("projects/recent")).toStringList();
        recent.erase(std::remove_if(
            recent.begin(),
            recent.end(),
            [&absolutePath](const QString& existing)
            {
                return QString::compare(
                    QFileInfo(existing).absoluteFilePath(),
                    absolutePath,
                    Qt::CaseInsensitive) == 0;
            }), recent.end());
        recent.prepend(absolutePath);
        while (recent.size() > 10)
        {
            recent.removeLast();
        }
        settings.setValue(QStringLiteral("projects/recent"), recent);
        RefreshRecentProjects();
    }

    void MainWindow::SetProjectWorkspacePanelsVisible(
        bool visible)
    {
        m_projectDockAction->setChecked(visible);
        m_propertiesDockAction->setChecked(visible);
        m_outputDockAction->setChecked(visible);
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
