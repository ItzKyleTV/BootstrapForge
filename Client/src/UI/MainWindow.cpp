#include <BootstrapForge/Client/UI/MainWindow.hpp>

#include <BootstrapForge/Client/Build/CMakeProjectController.hpp>
#include <BootstrapForge/Client/Editor/EditorDocument.hpp>
#include <BootstrapForge/Client/Editor/EditorManager.hpp>
#include <BootstrapForge/Client/Generation/GenerateDialog.hpp>
#include <BootstrapForge/Client/Project/ProjectTree.hpp>

#include <QAction>
#include <QCheckBox>
#include <QClipboard>
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
#include <QGuiApplication>
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
#include <QRegularExpression>
#include <QScreen>
#include <QSettings>
#include <QSpinBox>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStringList>
#include <QTabWidget>
#include <QTabBar>
#include <QTextCursor>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>

namespace
{
    bool HasEmbeddedRemoteCredentials(const QString& remote)
    {
        const QUrl url(remote);
        return url.isValid() && !url.userInfo().isEmpty() &&
            (url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0 ||
                url.scheme().compare(QStringLiteral("http"), Qt::CaseInsensitive) == 0 ||
                url.scheme().compare(QStringLiteral("ssh"), Qt::CaseInsensitive) == 0);
    }

    bool HasSensitiveRemoteData(const QString& remote)
    {
        return HasEmbeddedRemoteCredentials(remote) || !QUrl(remote).query().isEmpty();
    }

    bool IsGitHubRepositoryUrl(const QString& remote)
    {
        if (remote.isEmpty() || remote.contains(QRegularExpression(QStringLiteral("\\s"))))
        {
            return false;
        }

        QString repositoryPath;
        if (remote.startsWith(QStringLiteral("git@github.com:"), Qt::CaseInsensitive))
        {
            repositoryPath = remote.mid(QStringLiteral("git@github.com:").size());
        }
        else
        {
            const QUrl url(remote);
            const QString scheme = url.scheme().toLower();
            const bool https = scheme == QStringLiteral("https");
            const bool ssh = scheme == QStringLiteral("ssh");
            if (!url.isValid() || (!https && !ssh) ||
                url.host().compare(QStringLiteral("github.com"), Qt::CaseInsensitive) != 0 ||
                !url.query().isEmpty() || !url.fragment().isEmpty() ||
                (https && (!url.userInfo().isEmpty() || url.port(-1) != -1)) ||
                (ssh && (url.userName() != QStringLiteral("git") ||
                    !url.password().isEmpty() || url.port(-1) != -1)))
            {
                return false;
            }
            repositoryPath = url.path(QUrl::FullyDecoded);
        }

        const QStringList segments = repositoryPath.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        if (segments.size() != 2)
        {
            return false;
        }

        const QString owner = segments.at(0);
        QString repository = segments.at(1);
        if (repository.endsWith(QStringLiteral(".git"), Qt::CaseInsensitive))
        {
            repository.chop(4);
        }

        static const QRegularExpression ownerPattern(
            QStringLiteral("^[A-Za-z0-9](?:[A-Za-z0-9-]*[A-Za-z0-9])?$"));
        static const QRegularExpression repositoryPattern(
            QStringLiteral("^[A-Za-z0-9_.-]+$"));
        return ownerPattern.match(owner).hasMatch() &&
            repositoryPattern.match(repository).hasMatch() &&
            repository != QStringLiteral(".") &&
            repository != QStringLiteral("..");
    }

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

    void ConfigureFilePicker(
        QFileDialog& dialog,
        const QString& title,
        const QString& startingDirectory,
        const QString& nameFilter)
    {
        dialog.setOption(QFileDialog::DontUseNativeDialog, true);
        dialog.setWindowTitle(title);
        dialog.setDirectory(startingDirectory);
        dialog.setFileMode(QFileDialog::ExistingFile);
        dialog.setNameFilter(nameFilter);
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

        resizeDocks(
            {m_projectDock, m_propertiesDock},
            {260, 280},
            Qt::Horizontal);

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
        m_homeAction = new QAction(QStringLiteral("Home"), this);
        m_homeAction->setShortcut(
            QKeySequence(QStringLiteral("Alt+Home")));
        m_homeAction->setStatusTip(
            QStringLiteral("Return to the BootstrapForge start page"));

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
        m_stashChangesAction = new QAction(QStringLiteral("Stash Changes..."), this);
        m_commitChangesAction = new QAction(QStringLiteral("Commit..."), this);
        m_connectGitRepositoryAction = new QAction(
            QStringLiteral("Connect Repository..."), this);
        m_githubLoginAction = new QAction(QStringLiteral("GitHub Login..."), this);
        m_pushGitAction = new QAction(QStringLiteral("Push..."), this);
        m_applyStashAction = new QAction(QStringLiteral("Apply Stash..."), this);
        m_popStashAction = new QAction(QStringLiteral("Pop Stash..."), this);
        m_architectureAction = new QAction(QStringLiteral("Architecture Overview"), this);
        m_settingsAction = new QAction(QStringLiteral("Settings..."), this);
        m_toolchainReadinessAction = new QAction(
            QStringLiteral("Toolchain Readiness..."), this);
        m_aboutAction = new QAction(QStringLiteral("About BootstrapForge"), this);

        m_newAction->setEnabled(true);
        m_openProjectAction->setEnabled(true);
        m_openAction->setEnabled(true);
        m_saveAction->setEnabled(false);
        m_generateAction->setEnabled(false);
        m_gitStatusAction->setEnabled(false);
        m_createBranchAction->setEnabled(false);
        m_switchBranchAction->setEnabled(false);
        m_stashChangesAction->setEnabled(false);
        m_commitChangesAction->setEnabled(false);
        m_connectGitRepositoryAction->setEnabled(false);
        m_pushGitAction->setEnabled(false);
        m_applyStashAction->setEnabled(false);
        m_popStashAction->setEnabled(false);
        m_undoAction->setEnabled(false);
        m_redoAction->setEnabled(false);
        m_cutAction->setEnabled(false);
        m_copyAction->setEnabled(false);
        m_pasteAction->setEnabled(false);

    }

    void MainWindow::CreateMenus()
    {
        menuBar()->addAction(m_homeAction);

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

        gitMenu->addAction(m_connectGitRepositoryAction);
        gitMenu->addAction(m_githubLoginAction);
        gitMenu->addAction(m_pushGitAction);

        gitMenu->addSeparator();

        gitMenu->addAction(m_createBranchAction);

        gitMenu->addAction(m_switchBranchAction);

        gitMenu->addSeparator();
        gitMenu->addAction(m_commitChangesAction);
        gitMenu->addAction(m_stashChangesAction);
        gitMenu->addAction(m_applyStashAction);
        gitMenu->addAction(m_popStashAction);

        QMenu* toolsMenu =
            menuBar()->addMenu(
                QStringLiteral("&Tools"));

        toolsMenu->addAction(m_architectureAction);

        toolsMenu->addAction(m_toolchainReadinessAction);

        toolsMenu->addAction(m_settingsAction);

        QMenu* helpMenu = menuBar()->addMenu(QStringLiteral("&Help"));
        m_usagePromptAction = new QAction(
            QStringLiteral("BootstrapForge Use Prompt"), this);
        m_usagePromptAction->setStatusTip(
            QStringLiteral("Copy BootstrapForge command rules for an external AI"));
        helpMenu->addAction(m_usagePromptAction);
        helpMenu->addSeparator();
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

        m_testButton = new QToolButton(toolBar);
        m_testButton->setText(QStringLiteral("Test"));
        m_testButton->setMenu(m_testMenu);
        m_testButton->setPopupMode(QToolButton::InstantPopup);
        m_testButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
        toolBar->addWidget(m_testButton);

        toolBar->addSeparator();

        toolBar->addAction(
            m_gitStatusAction);

        toolBar->addAction(m_pushGitAction);
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

        m_propertiesDock->setMinimumWidth(240);

        m_propertiesDock->setStyleSheet(QStringLiteral(
            "QDockWidget { border: 1px solid #454545; "
            "border-left: 2px solid #5a5a5a; "
            "border-right: 2px solid #5a5a5a; }"
            "QDockWidget::title { border-bottom: 1px solid #454545; "
            "padding: 4px; }"
            "QLabel { border: none; background-color: palette(base); }"));

        m_propertiesLabel =
            new QLabel(
                QString{},
                m_propertiesDock);
        m_propertiesLabel->setFrameStyle(QFrame::NoFrame);

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
            QStringLiteral("Home"));
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
        connect(m_homeAction, &QAction::triggered,
            this, [this] { ReturnToHome(); });

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

                QFileDialog dialog(this);
                ConfigureFilePicker(
                    dialog,
                    QStringLiteral("Open File"),
                    startingDirectory,
                    QStringLiteral("All Files (*.*)"));
                if (dialog.exec() != QDialog::Accepted ||
                    dialog.selectedFiles().isEmpty())
                {
                    return;
                }

                OpenProjectFile(
                    dialog.selectedFiles().front());
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
                m_testNames = testNames;
                RebuildTestMenu(m_testNames);
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
                m_stashChangesAction->setEnabled(hasProject && !busy);
                m_commitChangesAction->setEnabled(hasProject && !busy);
                m_applyStashAction->setEnabled(hasProject && !busy);
                m_popStashAction->setEnabled(hasProject && !busy);
                UpdateGitActionAvailability();
                RebuildTestMenu(m_testNames);
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
        connect(m_stashChangesAction, &QAction::triggered,
            this, [this] { StashGitChanges(); });
        connect(m_commitChangesAction, &QAction::triggered,
            this, [this] { CommitGitChanges(); });
        connect(m_connectGitRepositoryAction, &QAction::triggered,
            this, [this] { ConnectGitRepository(); });
        connect(m_githubLoginAction, &QAction::triggered,
            this, [this] { LoginToGitHub(); });
        connect(m_pushGitAction, &QAction::triggered,
            this, [this] { PushGitCommits(); });
        connect(m_applyStashAction, &QAction::triggered,
            this, [this] { RestoreGitStash(false); });
        connect(m_popStashAction, &QAction::triggered,
            this, [this] { RestoreGitStash(true); });
        connect(m_architectureAction, &QAction::triggered,
            this, [this] { ShowArchitectureOverview(); });
        connect(m_settingsAction, &QAction::triggered,
            this, [this] { ShowSettingsDialog(); });
        connect(m_toolchainReadinessAction, &QAction::triggered,
            this, [this] { ShowToolchainReadiness(); });
        connect(m_usagePromptAction, &QAction::triggered,
            this, [this] { ShowUsagePrompt(); });
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
                m_selectedProjectItemPath = path;
                UpdateEditorProperties();

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
            [this](int index)
            {
                if (m_editorManager->IsEditorTab(index))
                {
                    m_selectedProjectItemPath.clear();
                }
                const int welcomeIndex =
                    m_workspaceTabs->indexOf(m_welcomePage);
                if (index >= 0 && welcomeIndex >= 0 &&
                    index != welcomeIndex &&
                    m_workspaceTabs->tabBar()->isTabVisible(welcomeIndex))
                {
                    m_workspaceTabs->tabBar()->setTabVisible(
                        welcomeIndex, false);
                }
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
                UpdateEditorProperties();
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
        m_testAction->setEnabled(canUseProject);
        if (m_testButton != nullptr)
        {
            m_testButton->setEnabled(canUseProject);
        }

        QAction* refreshAction = m_testMenu->addAction(
            QStringLiteral("Refresh Tests"));
        refreshAction->setEnabled(canRun);
        connect(refreshAction, &QAction::triggered, m_buildController,
            &Build::CMakeProjectController::RefreshTests);

        m_testMenu->addSeparator();
        if (testNames.isEmpty())
        {
            QString emptyMessage;
            QAction* emptyAction = m_testMenu->addAction(
                QString{});
            if (!canUseProject)
            {
                emptyMessage = QStringLiteral("Open a project to discover tests");
            }
            else if (m_buildController->IsBusy())
            {
                emptyMessage = QStringLiteral("Test information is being refreshed");
            }
            else if (!QFileInfo(QDir(m_projectTree->WorkspaceRoot()).filePath(
                QStringLiteral("CMakeLists.txt"))).isFile())
            {
                emptyMessage = QStringLiteral("No root CMakeLists.txt — this project has no CTest tests to discover");
            }
            else if (!QFileInfo(QDir(m_projectTree->WorkspaceRoot()).filePath(
                QStringLiteral(".bootstrapforge/build/CMakeCache.txt"))).isFile())
            {
                emptyMessage = QStringLiteral("Configure CMake to discover registered tests");
            }
            else
            {
                emptyMessage = QStringLiteral("No CTest tests are registered in this build");
            }
            emptyAction->setText(emptyMessage);
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

    void MainWindow::ShowWelcomePage()
    {
        if (m_welcomePage == nullptr)
        {
            return;
        }

        const QString projectRoot = m_projectTree->WorkspaceRoot();
        m_welcomeProjectLabel->setText(projectRoot.isEmpty()
            ? QStringLiteral("No project open")
            : QStringLiteral("Current project: %1")
                .arg(QDir::toNativeSeparators(projectRoot)));
        RefreshRecentProjects();

        const int welcomeIndex = m_workspaceTabs->indexOf(m_welcomePage);
        const int visibleWelcomeIndex = welcomeIndex >= 0
            ? welcomeIndex
            : m_workspaceTabs->insertTab(
                0, m_welcomePage, QStringLiteral("Home"));
        if (welcomeIndex < 0)
        {
            m_workspaceTabs->tabBar()->setTabButton(
                visibleWelcomeIndex, QTabBar::RightSide, nullptr);
        }

        if (visibleWelcomeIndex < 0)
        {
            return;
        }

        m_workspaceTabs->tabBar()->setTabVisible(visibleWelcomeIndex, true);
        m_workspaceTabs->setCurrentWidget(m_welcomePage);
        if (projectRoot.isEmpty())
        {
            SetProjectWorkspacePanelsVisible(false);
        }
        statusBar()->showMessage(QStringLiteral("Welcome to BootstrapForge."), 4000);
    }

    void MainWindow::ReturnToHome()
    {
        if (m_buildController->IsBusy())
        {
            ShowInformationMessage(QStringLiteral(
                "Wait for the current CMake operation to finish before returning Home."));
            return;
        }
        if (m_gitPushProcess != nullptr)
        {
            ShowInformationMessage(QStringLiteral(
                "Wait for the Git push to finish before returning Home."));
            return;
        }

        if (!ConfirmUnsavedDocuments())
        {
            return;
        }

        const QString previousRoot = m_projectTree->WorkspaceRoot();
        for (int index = m_workspaceTabs->count() - 1; index >= 0; --index)
        {
            if (m_editorManager->IsEditorTab(index))
            {
                if (!m_editorManager->CloseEditor(index))
                {
                    ShowErrorMessage(QStringLiteral(
                        "Could not close an editor tab; the current project remains open."));
                    return;
                }
            }
        }

        if (!m_projectTree->SetWorkspaceRoot(QString{}))
        {
            ShowErrorMessage(QStringLiteral("Could not close the current project workspace."));
            return;
        }

        m_buildController->SetProjectRoot(QString{});
        m_selectedProjectItemPath.clear();
        m_runAfterBuild = false;

        m_buildAction->setEnabled(false);
        m_runAction->setEnabled(false);
        m_configureAction->setEnabled(false);
        m_gitStatusAction->setEnabled(false);
        m_createBranchAction->setEnabled(false);
        m_switchBranchAction->setEnabled(false);
        m_stashChangesAction->setEnabled(false);
        m_commitChangesAction->setEnabled(false);
        m_applyStashAction->setEnabled(false);
        m_popStashAction->setEnabled(false);
        UpdateGitActionAvailability();
        RebuildTestMenu({});
        SetProjectWorkspacePanelsVisible(false);
        UpdateEditorProperties();
        UpdateEditorActions();
        setWindowTitle(QStringLiteral("BootstrapForge"));

        if (!previousRoot.isEmpty())
        {
            m_outputEditor->appendPlainText(
                QStringLiteral("[PROJECT] Closed: %1")
                    .arg(QDir::toNativeSeparators(previousRoot)));
        }

        ShowWelcomePage();
    }

    void MainWindow::ShowToolchainReadiness()
    {
        QDialog dialog(this);
        dialog.setWindowTitle(QStringLiteral("Toolchain Readiness"));
        dialog.resize(760, 560);

        auto* layout = new QVBoxLayout(&dialog);
        auto* summary = new QLabel(
            QStringLiteral("Detected tools and the active project's CMake configuration."),
            &dialog);
        summary->setWordWrap(true);
        layout->addWidget(summary);

        auto* report = new QPlainTextEdit(&dialog);
        report->setReadOnly(true);
        report->setLineWrapMode(QPlainTextEdit::NoWrap);
        report->setPlainText(m_buildController->ToolchainReadinessReport());
        layout->addWidget(report, 1);

        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::accept);
        layout->addWidget(buttons);
        dialog.exec();
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
        dialog.setMinimumSize(540, 360);
        auto* layout = new QVBoxLayout(&dialog);
        auto* pages = new QTabWidget(&dialog);
        layout->addWidget(pages, 1);

        auto* toolsPage = new QWidget(pages);
        auto* toolsLayout = new QVBoxLayout(toolsPage);
        auto* toolsForm = new QFormLayout();
        auto* cmakeEdit = new QLineEdit(
            settings.value(QStringLiteral("tools/cmakeExecutable")).toString(),
            toolsPage);
        auto* gitEdit = new QLineEdit(
            settings.value(QStringLiteral("tools/gitExecutable")).toString(),
            toolsPage);
        auto* qtCMakeDirEdit = new QLineEdit(
            settings.value(QStringLiteral("tools/qt6CMakeDirectory")).toString(),
            toolsPage);
        auto addExecutableRow = [&dialog, toolsForm, toolsPage](
            const QString& label, QLineEdit* edit, const QString& caption)
        {
            auto* row = new QWidget(toolsPage);
            auto* rowLayout = new QHBoxLayout(row);
            rowLayout->setContentsMargins(0, 0, 0, 0);
            auto* browse = new QPushButton(QStringLiteral("Browse..."), row);
            rowLayout->addWidget(edit, 1);
            rowLayout->addWidget(browse);
            QObject::connect(browse, &QPushButton::clicked, &dialog,
                [&dialog, edit, caption]
                {
                    const QString currentPath = edit->text().trimmed();
                    const QString startDirectory = currentPath.isEmpty()
                        ? QDir::homePath()
                        : QFileInfo(currentPath).absolutePath();
                    QFileDialog picker(&dialog);
                    ConfigureFilePicker(
                        picker,
                        caption,
                        startDirectory,
                        QStringLiteral("Executables (*.exe);;All files (*.*)"));
                    if (picker.exec() == QDialog::Accepted &&
                        !picker.selectedFiles().isEmpty())
                    {
                        edit->setText(picker.selectedFiles().front());
                    }
                });
            toolsForm->addRow(label, row);
        };
        addExecutableRow(QStringLiteral("CMake executable"), cmakeEdit,
            QStringLiteral("Choose CMake executable"));
        addExecutableRow(QStringLiteral("Git executable"), gitEdit,
            QStringLiteral("Choose Git executable"));
        auto* qtRow = new QWidget(toolsPage);
        auto* qtRowLayout = new QHBoxLayout(qtRow);
        qtRowLayout->setContentsMargins(0, 0, 0, 0);
        auto* browseQt = new QPushButton(QStringLiteral("Browse..."), qtRow);
        qtRowLayout->addWidget(qtCMakeDirEdit, 1);
        qtRowLayout->addWidget(browseQt);
        toolsForm->addRow(QStringLiteral("Qt 6 CMake package directory"), qtRow);
        connect(browseQt, &QPushButton::clicked, &dialog,
            [&dialog, qtCMakeDirEdit]
            {
                const QString currentPath = qtCMakeDirEdit->text().trimmed();
                QFileDialog picker(&dialog);
                ConfigureDirectoryPicker(
                    picker,
                    QStringLiteral("Choose the directory containing Qt6Config.cmake"),
                    currentPath.isEmpty() ? QDir::homePath() : currentPath);
                if (picker.exec() == QDialog::Accepted &&
                    !picker.selectedFiles().isEmpty())
                {
                    qtCMakeDirEdit->setText(picker.selectedFiles().front());
                }
            });
        toolsLayout->addLayout(toolsForm);
        auto* toolHint = new QLabel(
            QStringLiteral("Leave a tool path empty to use automatic discovery. Set Qt's package directory to the folder containing Qt6Config.cmake when CMake cannot find Qt. Changes take effect on the next configure."),
            toolsPage);
        toolHint->setWordWrap(true);
        toolsLayout->addWidget(toolHint);
        toolsLayout->addStretch();
        pages->addTab(toolsPage, QStringLiteral("Tools"));

        auto* editorPage = new QWidget(pages);
        auto* editorLayout = new QVBoxLayout(editorPage);
        auto* editorForm = new QFormLayout();
        auto* fontSize = new QSpinBox(editorPage);
        fontSize->setRange(8, 28);
        fontSize->setSuffix(QStringLiteral(" pt"));
        fontSize->setValue(settings.value(
            QStringLiteral("editor/fontSize"), 11).toInt());
        auto* tabWidth = new QSpinBox(editorPage);
        tabWidth->setRange(2, 8);
        tabWidth->setSuffix(QStringLiteral(" spaces"));
        tabWidth->setValue(settings.value(
            QStringLiteral("editor/tabWidth"), 4).toInt());
        auto* wordWrap = new QCheckBox(
            QStringLiteral("Wrap long lines to the editor width"), editorPage);
        wordWrap->setChecked(settings.value(
            QStringLiteral("editor/wordWrap"), false).toBool());
        auto* lineNumbers = new QCheckBox(
            QStringLiteral("Show line numbers"), editorPage);
        lineNumbers->setChecked(settings.value(
            QStringLiteral("editor/showLineNumbers"), true).toBool());
        editorForm->addRow(QStringLiteral("Font size"), fontSize);
        editorForm->addRow(QStringLiteral("Tab width"), tabWidth);
        editorForm->addRow(QString(), wordWrap);
        editorForm->addRow(QString(), lineNumbers);
        editorLayout->addLayout(editorForm);
        auto* editorHint = new QLabel(
            QStringLiteral("These preferences apply to open documents immediately "
                "and to files opened later."), editorPage);
        editorHint->setWordWrap(true);
        editorLayout->addWidget(editorHint);
        editorLayout->addStretch();
        pages->addTab(editorPage, QStringLiteral("Editor"));

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
        settings.setValue(QStringLiteral("tools/qt6CMakeDirectory"), qtCMakeDirEdit->text().trimmed());
        settings.setValue(QStringLiteral("editor/fontSize"), fontSize->value());
        settings.setValue(QStringLiteral("editor/tabWidth"), tabWidth->value());
        settings.setValue(QStringLiteral("editor/wordWrap"), wordWrap->isChecked());
        settings.setValue(QStringLiteral("editor/showLineNumbers"), lineNumbers->isChecked());
        for (Editor::EditorDocument* document :
            m_workspaceTabs->findChildren<Editor::EditorDocument*>())
        {
            document->ApplyEditorSettings();
        }
        ShowInformationMessage(QStringLiteral("Settings saved and editor preferences applied."));
        if (!m_projectTree->WorkspaceRoot().isEmpty())
        {
            m_buildController->RefreshTests();
        }
    }

    void MainWindow::ShowUsagePrompt()
    {
        const QString prompt = QString::fromUtf8(R"PROMPT(
You are generating commands for BootstrapForge, a project structure and file generation tool.

Return one valid command only. Do not include Markdown fences or explanatory prose. BootstrapForge will parse and validate the command, show a preview, and ask the user before applying filesystem changes.

Command form:
generate {
  branch(Engine) {
    subbranch(include) {
      subbranch(BootstrapForge) {
        subbranch(Engine) {
          folder(Differ) {
            file(ArchitectureDiffer.hpp)
          }
        }
      }
    }
    subbranch(src) {
      folder(Differ) {
        file(ArchitectureDiffer.cpp)
      }
    }
  }
}

Rules:
- The generate block may contain multiple branches, folders, or files.
- A branch is allowed directly under generate. It may contain subbranch, folder, or file entries.
- A subbranch may contain subbranch, folder, or file entries. A folder may contain folder or file entries.
- Containers use braces, may have multiple siblings, and may be empty.
- Files are leaves and cannot contain children.
- Names may be quoted or unquoted. Quote names with spaces or punctuation, and preserve the requested spelling and extension.
- Every destination is relative to the currently open project. Use nested containers for directories; never use absolute paths, drive letters, or '..'.
- Use file(Name.ext) for an empty placeholder. To provide initial contents, write file(Name.ext) = """ followed by the exact multiline contents and a closing """ on its own line.
- Do not put the triple-quote delimiter inside generated file contents.
- Avoid duplicate destinations, including names that differ only by letter case.
- Do not invent unsupported declaration types or create files the user did not request.

If the requested structure is ambiguous, ask the user to clarify before returning a command.
)PROMPT");

        QDialog dialog(this);
        dialog.setWindowTitle(QStringLiteral("BootstrapForge Use Prompt"));
        dialog.resize(820, 680);
        auto* layout = new QVBoxLayout(&dialog);
        auto* introduction = new QLabel(
            QStringLiteral("Copy these instructions into an external AI so it can "
                "generate commands for BootstrapForge."),
            &dialog);
        introduction->setWordWrap(true);
        layout->addWidget(introduction);

        auto* promptEditor = new QPlainTextEdit(&dialog);
        promptEditor->setPlainText(prompt.trimmed());
        promptEditor->setReadOnly(true);
        promptEditor->setLineWrapMode(QPlainTextEdit::WidgetWidth);
        layout->addWidget(promptEditor, 1);

        auto* footer = new QHBoxLayout();
        auto* copyStatus = new QLabel(&dialog);
        auto* copyButton = new QPushButton(
            QStringLiteral("Copy Prompt"), &dialog);
        auto* closeButtons = new QDialogButtonBox(
            QDialogButtonBox::Close, &dialog);
        footer->addWidget(copyStatus, 1);
        footer->addWidget(copyButton);
        footer->addWidget(closeButtons);
        layout->addLayout(footer);

        connect(copyButton, &QPushButton::clicked, &dialog,
            [&dialog, promptEditor, copyStatus]
            {
                QGuiApplication::clipboard()->setText(
                    promptEditor->toPlainText());
                copyStatus->setText(QStringLiteral("Prompt copied."));
            });
        connect(closeButtons, &QDialogButtonBox::rejected,
            &dialog, &QDialog::reject);
        dialog.exec();
    }

    void MainWindow::ShowAboutDialog()
    {
        QMessageBox::about(
            this,
            QStringLiteral("About BootstrapForge"),
            QStringLiteral("BootstrapForge\nArchitecture workspace and development environment\n\nQt %1")
                .arg(QString::fromLatin1(qVersion())));
    }

    void MainWindow::UpdateGitActionAvailability()
    {
        const bool hasProject = m_projectTree != nullptr &&
            !m_projectTree->WorkspaceRoot().isEmpty();
        const bool canUseProjectGit = hasProject &&
            m_buildController != nullptr && !m_buildController->IsBusy() &&
            m_gitPushProcess == nullptr && m_githubLoginProcess == nullptr;

        m_gitStatusAction->setEnabled(canUseProjectGit);
        m_createBranchAction->setEnabled(canUseProjectGit);
        m_switchBranchAction->setEnabled(canUseProjectGit);
        m_stashChangesAction->setEnabled(canUseProjectGit);
        m_commitChangesAction->setEnabled(canUseProjectGit);
        m_connectGitRepositoryAction->setEnabled(canUseProjectGit);
        m_pushGitAction->setEnabled(canUseProjectGit);
        m_applyStashAction->setEnabled(canUseProjectGit);
        m_popStashAction->setEnabled(canUseProjectGit);
        m_githubLoginAction->setEnabled(m_githubLoginProcess == nullptr &&
            m_gitPushProcess == nullptr);
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

    void MainWindow::CommitGitChanges()
    {
        QString status;
        QString error;
        if (!RunGitCommand(
            {QStringLiteral("status"), QStringLiteral("--short"),
                QStringLiteral("--untracked-files=normal")}, status, error))
        {
            ShowErrorMessage(error);
            return;
        }
        if (status.trimmed().isEmpty())
        {
            ShowInformationMessage(QStringLiteral("There are no changes to commit."));
            return;
        }

        bool accepted = false;
        const QString message = QInputDialog::getText(
            this,
            QStringLiteral("Commit Changes"),
            QStringLiteral("Commit message"),
            QLineEdit::Normal,
            {},
            &accepted).trimmed();
        if (!accepted)
        {
            return;
        }
        if (message.isEmpty())
        {
            ShowErrorMessage(QStringLiteral("A commit message is required."));
            return;
        }

        QDialog choice(this);
        choice.setWindowTitle(QStringLiteral("Commit Changes"));
        choice.setWindowModality(Qt::WindowModal);
        choice.setSizeGripEnabled(true);

        auto* choiceLayout = new QVBoxLayout(&choice);
        auto* reviewText = new QLabel(
            QStringLiteral("Review the working-tree changes, then choose what to commit."),
            &choice);
        reviewText->setWordWrap(true);
        choiceLayout->addWidget(reviewText);

        auto* statusView = new QPlainTextEdit(&choice);
        statusView->setReadOnly(true);
        statusView->setLineWrapMode(QPlainTextEdit::NoWrap);
        statusView->setPlainText(status.trimmed());
        statusView->setMinimumHeight(120);
        choiceLayout->addWidget(statusView, 1);

        auto* buttons = new QDialogButtonBox(&choice);
        auto* stageAll = buttons->addButton(
            QStringLiteral("Stage All and Commit"),
            QDialogButtonBox::AcceptRole);
        auto* stagedOnly = buttons->addButton(
            QStringLiteral("Commit Staged Changes"),
            QDialogButtonBox::ActionRole);
        auto* cancel = buttons->addButton(QDialogButtonBox::Cancel);
        cancel->setDefault(true);
        choiceLayout->addWidget(buttons);

        enum class CommitSelection
        {
            None,
            StageAll,
            StagedOnly
        };
        CommitSelection selection = CommitSelection::None;
        connect(stageAll, &QAbstractButton::clicked, &choice,
            [&choice, &selection]
            {
                selection = CommitSelection::StageAll;
                choice.accept();
            });
        connect(stagedOnly, &QAbstractButton::clicked, &choice,
            [&choice, &selection]
            {
                selection = CommitSelection::StagedOnly;
                choice.accept();
            });
        connect(buttons, &QDialogButtonBox::rejected,
            &choice, &QDialog::reject);

        const QScreen* targetScreen = screen();
        if (targetScreen == nullptr)
        {
            targetScreen = QGuiApplication::primaryScreen();
        }
        const QSize availableSize = targetScreen == nullptr
            ? QSize(1024, 768)
            : targetScreen->availableGeometry().size();
        const int dialogWidth = std::max(
            240, std::min(900, availableSize.width() - 48));
        const int dialogHeight = std::max(
            200, std::min(620, availableSize.height() - 72));
        choice.setMinimumSize(
            std::min(480, dialogWidth),
            std::min(320, dialogHeight));
        choice.resize(dialogWidth, dialogHeight);

        if (choice.exec() != QDialog::Accepted ||
            selection == CommitSelection::None)
        {
            return;
        }

        if (selection == CommitSelection::StageAll)
        {
            QString addOutput;
            if (!RunGitCommand(
                {QStringLiteral("add"), QStringLiteral("--all")}, addOutput, error))
            {
                ShowErrorMessage(error);
                return;
            }
        }
        else
        {
            QString stagedFiles;
            if (!RunGitCommand(
                {QStringLiteral("diff"), QStringLiteral("--cached"),
                    QStringLiteral("--name-only")}, stagedFiles, error))
            {
                ShowErrorMessage(error);
                return;
            }
            if (stagedFiles.trimmed().isEmpty())
            {
                ShowInformationMessage(QStringLiteral(
                    "There are no staged changes. Choose Stage All and Commit, or stage files in Git first."));
                return;
            }
        }

        QString commitOutput;
        if (!RunGitCommand(
            {QStringLiteral("commit"), QStringLiteral("-m"), message},
            commitOutput, error))
        {
            if (!commitOutput.trimmed().isEmpty())
            {
                m_outputEditor->appendPlainText(commitOutput.trimmed());
            }
            ShowErrorMessage(error);
            return;
        }

        m_outputEditor->appendPlainText(QStringLiteral("[GIT COMMIT]\n%1")
            .arg(commitOutput.trimmed()));
        statusBar()->showMessage(QStringLiteral("Git commit created."), 5000);
    }

    void MainWindow::ConnectGitRepository()
    {
        const QString projectRoot = m_projectTree->WorkspaceRoot();
        if (projectRoot.isEmpty())
        {
            ShowErrorMessage(QStringLiteral("Open a project before connecting a GitHub repository."));
            return;
        }
        if (GitExecutable().isEmpty())
        {
            ShowErrorMessage(QStringLiteral(
                "Git was not found. Install Git for Windows with Git Credential Manager, "
                "or set its executable path in Tools > Settings."));
            return;
        }

        QString repositoryRootOutput;
        QString error;
        bool isRepository = RunGitCommand(
            {QStringLiteral("rev-parse"), QStringLiteral("--show-toplevel")},
            repositoryRootOutput,
            error);
        QString repositoryRoot;

        if (!isRepository)
        {
            if (!error.contains(QStringLiteral("not a git repository"), Qt::CaseInsensitive))
            {
                ShowErrorMessage(error);
                return;
            }

            const auto initialize = QMessageBox::question(
                this,
                QStringLiteral("Initialize Git Repository"),
                QStringLiteral(
                    "This project is not inside a Git repository. Initialize Git here?\n\n"
                    "This creates a .git metadata folder; it does not change project files "
                    "or create a GitHub repository."),
                QMessageBox::Yes | QMessageBox::Cancel,
                QMessageBox::Cancel);
            if (initialize != QMessageBox::Yes)
            {
                return;
            }

            QString initOutput;
            if (!RunGitCommand({QStringLiteral("init")}, initOutput, error))
            {
                ShowErrorMessage(error);
                return;
            }
            repositoryRoot = QDir::cleanPath(QFileInfo(projectRoot).absoluteFilePath());
            m_outputEditor->appendPlainText(QStringLiteral("[GIT] Initialized repository: %1")
                .arg(QDir::toNativeSeparators(repositoryRoot)));
        }
        else
        {
            repositoryRoot = QDir::cleanPath(repositoryRootOutput.trimmed());
            QString normalizedProjectRoot = QFileInfo(projectRoot).canonicalFilePath();
            if (normalizedProjectRoot.isEmpty())
            {
                normalizedProjectRoot = QFileInfo(projectRoot).absoluteFilePath();
            }
            normalizedProjectRoot = QDir::cleanPath(normalizedProjectRoot);

#ifdef Q_OS_WIN
            constexpr Qt::CaseSensitivity pathCaseSensitivity = Qt::CaseInsensitive;
#else
            constexpr Qt::CaseSensitivity pathCaseSensitivity = Qt::CaseSensitive;
#endif
            if (QString::compare(repositoryRoot, normalizedProjectRoot,
                    pathCaseSensitivity) != 0)
            {
                const auto useParentRepository = QMessageBox::warning(
                    this,
                    QStringLiteral("Project Uses a Parent Repository"),
                    QStringLiteral(
                        "This project folder is inside the Git repository at:\n%1\n\n"
                        "Changing origin will affect that entire repository. Continue?" )
                        .arg(QDir::toNativeSeparators(repositoryRoot)),
                    QMessageBox::Yes | QMessageBox::Cancel,
                    QMessageBox::Cancel);
                if (useParentRepository != QMessageBox::Yes)
                {
                    return;
                }
            }
        }

        QString currentRemote;
        const bool hasOrigin = RunGitCommand(
            {QStringLiteral("remote"), QStringLiteral("get-url"), QStringLiteral("origin")},
            currentRemote,
            error);
        if (!hasOrigin && !error.contains(QStringLiteral("no such remote"), Qt::CaseInsensitive))
        {
            ShowErrorMessage(error);
            return;
        }

        const QString currentUrl = currentRemote.trimmed();
        const QString initialUrl = HasSensitiveRemoteData(currentUrl) ||
                !IsGitHubRepositoryUrl(currentUrl)
            ? QString{}
            : currentUrl;
        bool accepted = false;
        const QString remoteUrl = QInputDialog::getText(
            this,
            QStringLiteral("Connect GitHub Repository"),
            QStringLiteral(
                "Create an empty repository on GitHub, copy its HTTPS or SSH clone URL, "
                "and paste it here. This configures origin; it does not create the "
                "GitHub repository."),
            QLineEdit::Normal,
            initialUrl,
            &accepted).trimmed();
        if (!accepted)
        {
            return;
        }
        if (!IsGitHubRepositoryUrl(remoteUrl))
        {
            ShowErrorMessage(QStringLiteral(
                "Enter a GitHub.com HTTPS or SSH clone URL, such as "
                "https://github.com/owner/repository.git or "
                "git@github.com:owner/repository.git. Do not put a password or token in the URL."));
            return;
        }

        if (hasOrigin && currentUrl == remoteUrl)
        {
            ShowInformationMessage(QStringLiteral("This project is already connected to that GitHub repository."));
            return;
        }

        if (hasOrigin)
        {
            const QString previousRemote = HasSensitiveRemoteData(currentUrl)
                ? QStringLiteral("an origin URL containing credentials (hidden)")
                : currentUrl;
            const auto replaceRemote = QMessageBox::question(
                this,
                QStringLiteral("Replace Git Remote"),
                QStringLiteral("Replace origin (%1) with %2?")
                    .arg(previousRemote, remoteUrl),
                QMessageBox::Yes | QMessageBox::Cancel,
                QMessageBox::Cancel);
            if (replaceRemote != QMessageBox::Yes)
            {
                return;
            }
        }

        const QStringList arguments = hasOrigin
            ? QStringList{QStringLiteral("remote"), QStringLiteral("set-url"),
                QStringLiteral("origin"), remoteUrl}
            : QStringList{QStringLiteral("remote"), QStringLiteral("add"),
                QStringLiteral("origin"), remoteUrl};
        QString output;
        if (!RunGitCommand(arguments, output, error))
        {
            ShowErrorMessage(error);
            return;
        }

        m_outputEditor->appendPlainText(QStringLiteral("[GIT] Connected origin to %1")
            .arg(remoteUrl));
        statusBar()->showMessage(QStringLiteral("GitHub repository connected."), 5000);
    }

    void MainWindow::LoginToGitHub()
    {
        if (m_githubLoginProcess != nullptr)
        {
            return;
        }

        const QString git = GitExecutable();
        if (git.isEmpty())
        {
            ShowErrorMessage(QStringLiteral(
                "Git was not found. Install Git for Windows with Git Credential Manager, "
                "or set its executable path in Tools > Settings."));
            return;
        }

        auto* process = new QProcess(this);
        m_githubLoginProcess = process;
        process->setProgram(git);
        process->setArguments({QStringLiteral("credential-manager"),
            QStringLiteral("github"), QStringLiteral("login"),
            QStringLiteral("--browser")});

        connect(process,
            qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this,
            [this, process](int exitCode, QProcess::ExitStatus exitStatus)
            {
                if (m_githubLoginProcess == process)
                {
                    m_githubLoginProcess = nullptr;
                }
                UpdateGitActionAvailability();
                process->deleteLater();

                if (exitStatus == QProcess::NormalExit && exitCode == 0)
                {
                    m_outputEditor->appendPlainText(
                        QStringLiteral("[GITHUB] Git Credential Manager sign-in completed."));
                    statusBar()->showMessage(QStringLiteral("GitHub sign-in completed."), 5000);
                    ShowInformationMessage(QStringLiteral(
                        "GitHub sign-in completed. Git Credential Manager stores and supplies "
                        "authentication securely when you push over HTTPS."));
                    return;
                }

                ShowErrorMessage(QStringLiteral(
                    "GitHub sign-in did not complete. Make sure Git Credential Manager is "
                    "installed and that the browser authorization was completed. Exit code: %1.")
                    .arg(exitCode));
            });
        connect(process, &QProcess::errorOccurred, this,
            [this, process](QProcess::ProcessError processError)
            {
                if (processError != QProcess::FailedToStart)
                {
                    return;
                }
                if (m_githubLoginProcess == process)
                {
                    m_githubLoginProcess = nullptr;
                }
                UpdateGitActionAvailability();
                const QString detail = process->errorString();
                process->deleteLater();
                ShowErrorMessage(QStringLiteral(
                    "Could not start Git Credential Manager. Install a current Git for Windows "
                    "version that includes Git Credential Manager.\n\n%1").arg(detail));
            });

        m_outputEditor->appendPlainText(
            QStringLiteral("[GITHUB] Starting browser sign-in through Git Credential Manager...") );
        statusBar()->showMessage(QStringLiteral("Waiting for GitHub sign-in..."));
        UpdateGitActionAvailability();
        process->start();
    }

    void MainWindow::PushGitCommits()
    {
        if (m_gitPushProcess != nullptr)
        {
            return;
        }

        const QString projectRoot = m_projectTree->WorkspaceRoot();
        if (projectRoot.isEmpty())
        {
            ShowErrorMessage(QStringLiteral("Open a project before pushing to GitHub."));
            return;
        }

        QString branch;
        QString error;
        if (!RunGitCommand(
                {QStringLiteral("branch"), QStringLiteral("--show-current")},
                branch,
                error))
        {
            ShowErrorMessage(error);
            return;
        }
        branch = branch.trimmed();
        if (branch.isEmpty())
        {
            ShowErrorMessage(QStringLiteral(
                "The repository is not on a named branch. Switch to a branch before pushing."));
            return;
        }

        QString commit;
        if (!RunGitCommand(
                {QStringLiteral("rev-parse"), QStringLiteral("--verify"), QStringLiteral("HEAD")},
                commit,
                error))
        {
            ShowErrorMessage(QStringLiteral(
                "This repository has no commits yet. Create a commit before pushing."));
            return;
        }

        QString remote;
        if (!RunGitCommand(
                {QStringLiteral("remote"), QStringLiteral("get-url"), QStringLiteral("origin")},
                remote,
                error))
        {
            if (error.contains(QStringLiteral("no such remote"), Qt::CaseInsensitive))
            {
                const auto connectNow = QMessageBox::question(
                    this,
                    QStringLiteral("No GitHub Repository Connected"),
                    QStringLiteral("Connect this project to a GitHub repository before pushing?"),
                    QMessageBox::Yes | QMessageBox::Cancel,
                    QMessageBox::Yes);
                if (connectNow == QMessageBox::Yes)
                {
                    ConnectGitRepository();
                }
                return;
            }
            ShowErrorMessage(error);
            return;
        }
        remote = remote.trimmed();
        const QUrl remoteUrl(remote);
        if (HasEmbeddedRemoteCredentials(remote) || !remoteUrl.query().isEmpty())
        {
            ShowErrorMessage(QStringLiteral(
                "The origin URL appears to contain embedded credentials or a query string. "
                "Use Git > Connect Repository... to replace it with a credential-free URL; "
                "Git Credential Manager can handle sign-in."));
            return;
        }

        const auto confirmPush = QMessageBox::question(
            this,
            QStringLiteral("Push to GitHub"),
            QStringLiteral(
                "Push committed changes from branch '%1' to origin?\n\n%2\n\n"
                "Only commits are uploaded; uncommitted file changes stay on this computer. "
                "BootstrapForge will not force-push." )
                .arg(branch, remote),
            QMessageBox::Yes | QMessageBox::Cancel,
            QMessageBox::Cancel);
        if (confirmPush != QMessageBox::Yes)
        {
            return;
        }

        const QString git = GitExecutable();
        if (git.isEmpty())
        {
            ShowErrorMessage(QStringLiteral(
                "Git was not found. Set its executable path in Tools > Settings."));
            return;
        }

        auto* process = new QProcess(this);
        m_gitPushProcess = process;
        process->setProgram(git);
        process->setArguments({QStringLiteral("-C"), projectRoot,
            QStringLiteral("push"), QStringLiteral("--set-upstream"),
            QStringLiteral("origin"), branch});

        connect(process,
            qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this,
            [this, process, branch](int exitCode, QProcess::ExitStatus exitStatus)
            {
                const QString remainingOutput = QString::fromLocal8Bit(
                    process->readAllStandardOutput()).trimmed();
                const QString remainingError = QString::fromLocal8Bit(
                    process->readAllStandardError()).trimmed();
                if (!remainingOutput.isEmpty())
                {
                    m_outputEditor->appendPlainText(
                        QStringLiteral("[GIT PUSH] %1").arg(remainingOutput));
                }
                if (!remainingError.isEmpty())
                {
                    m_outputEditor->appendPlainText(
                        QStringLiteral("[GIT PUSH] %1").arg(remainingError));
                }

                if (m_gitPushProcess == process)
                {
                    m_gitPushProcess = nullptr;
                }
                UpdateGitActionAvailability();
                process->deleteLater();

                if (exitStatus == QProcess::NormalExit && exitCode == 0)
                {
                    m_outputEditor->appendPlainText(
                        QStringLiteral("[GIT] Pushed branch %1 to origin.").arg(branch));
                    statusBar()->showMessage(QStringLiteral("Pushed %1 to origin.").arg(branch), 6000);
                    return;
                }

                QString detail = remainingError.isEmpty()
                    ? remainingOutput
                    : remainingError;
                if (detail.isEmpty())
                {
                    detail = QStringLiteral("Git exited with code %1.").arg(exitCode);
                }
                ShowErrorMessage(QStringLiteral(
                    "Push failed. Check the remote URL, GitHub access, and whether the remote "
                    "branch has changes that must be fetched first. No force-push was attempted.\n\n%1")
                    .arg(detail));
            });
        connect(process, &QProcess::errorOccurred, this,
            [this, process](QProcess::ProcessError processError)
            {
                if (processError != QProcess::FailedToStart)
                {
                    return;
                }
                if (m_gitPushProcess == process)
                {
                    m_gitPushProcess = nullptr;
                }
                UpdateGitActionAvailability();
                const QString detail = process->errorString();
                process->deleteLater();
                ShowErrorMessage(QStringLiteral("Could not start Git push.\n\n%1").arg(detail));
            });

        m_outputEditor->appendPlainText(
            QStringLiteral("[GIT] Pushing branch %1 to origin...").arg(branch));
        statusBar()->showMessage(QStringLiteral("Pushing to GitHub..."));
        UpdateGitActionAvailability();
        process->start();
    }

    void MainWindow::StashGitChanges()
    {
        QString status;
        QString error;
        if (!RunGitCommand(
            {QStringLiteral("status"), QStringLiteral("--porcelain"),
                QStringLiteral("--untracked-files=normal")}, status, error))
        {
            ShowErrorMessage(error);
            return;
        }
        if (status.trimmed().isEmpty())
        {
            ShowInformationMessage(QStringLiteral("There are no tracked or untracked changes to stash."));
            return;
        }

        bool accepted = false;
        const QString message = QInputDialog::getText(
            this,
            QStringLiteral("Stash Git Changes"),
            QStringLiteral("Stash message"),
            QLineEdit::Normal,
            QStringLiteral("BootstrapForge changes"),
            &accepted).trimmed();
        if (!accepted)
        {
            return;
        }

        QString output;
        if (!RunGitCommand(
            {QStringLiteral("stash"), QStringLiteral("push"),
                QStringLiteral("--include-untracked"), QStringLiteral("-m"),
                message.isEmpty() ? QStringLiteral("BootstrapForge changes") : message},
            output, error))
        {
            ShowErrorMessage(error);
            return;
        }

        m_outputEditor->appendPlainText(QStringLiteral("[GIT STASH]\n%1")
            .arg(output.trimmed().isEmpty() ? QStringLiteral("Changes stashed.") : output.trimmed()));
        statusBar()->showMessage(QStringLiteral("Git changes stashed."), 5000);
    }

    void MainWindow::RestoreGitStash(bool pop)
    {
        QString output;
        QString error;
        if (!RunGitCommand(
            {QStringLiteral("stash"), QStringLiteral("list"),
                QStringLiteral("--format=%gd%x09%gs")},
            output, error))
        {
            ShowErrorMessage(error);
            return;
        }

        QStringList references;
        QStringList choices;
        const QStringList rows = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString& row : rows)
        {
            const qsizetype separator = row.indexOf(QLatin1Char('\t'));
            if (separator <= 0)
            {
                continue;
            }
            references.push_back(row.left(separator).trimmed());
            choices.push_back(row.left(separator).trimmed() +
                QStringLiteral(": ") + row.mid(separator + 1).trimmed());
        }
        if (references.isEmpty())
        {
            ShowInformationMessage(QStringLiteral("There are no saved Git stashes."));
            return;
        }

        bool accepted = false;
        const QString selected = QInputDialog::getItem(
            this,
            pop ? QStringLiteral("Pop Git Stash") : QStringLiteral("Apply Git Stash"),
            QStringLiteral("Choose a saved stash"),
            choices,
            0,
            false,
            &accepted);
        if (!accepted)
        {
            return;
        }
        const qsizetype index = choices.indexOf(selected);
        if (index < 0 || index >= references.size())
        {
            return;
        }

        if (pop)
        {
            const auto answer = QMessageBox::warning(
                this,
                QStringLiteral("Pop Git Stash"),
                QStringLiteral("Pop %1 into the working tree? Git removes it from the stash list only if the apply succeeds.")
                    .arg(references[index]),
                QMessageBox::Apply | QMessageBox::Cancel,
                QMessageBox::Cancel);
            if (answer != QMessageBox::Apply)
            {
                return;
            }
        }

        const QString operation = pop ? QStringLiteral("pop") : QStringLiteral("apply");
        if (!RunGitCommand(
            {QStringLiteral("stash"), operation, references[index]}, output, error))
        {
            if (!output.trimmed().isEmpty())
            {
                m_outputEditor->appendPlainText(output.trimmed());
            }
            ShowErrorMessage(error);
            return;
        }

        m_outputEditor->appendPlainText(QStringLiteral("[GIT STASH]\n%1")
            .arg(output.trimmed().isEmpty()
                ? QStringLiteral("Applied %1.").arg(references[index])
                : output.trimmed()));
        statusBar()->showMessage(
            pop ? QStringLiteral("Git stash popped.") : QStringLiteral("Git stash applied; saved stash retained."),
            5000);
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
        if (m_gitPushProcess != nullptr || m_githubLoginProcess != nullptr)
        {
            const auto closeAnyway = QMessageBox::warning(
                this,
                QStringLiteral("Git Operation in Progress"),
                QStringLiteral(
                    "A Git push or GitHub sign-in is still running. Closing now may cancel it. "
                    "Close BootstrapForge anyway?"),
                QMessageBox::Yes | QMessageBox::Cancel,
                QMessageBox::Cancel);
            if (closeAnyway != QMessageBox::Yes)
            {
                event->ignore();
                return;
            }
        }

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

        m_selectedProjectItemPath = fileInfo.absoluteFilePath();
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
        if (m_gitPushProcess != nullptr)
        {
            ShowInformationMessage(QStringLiteral(
                "Wait for the Git push to finish before switching projects."));
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
        m_selectedProjectItemPath = projectPath;
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
                m_welcomePage->hide();
                if (m_workspaceTabs->count() == 0)
                {
                    m_workspaceTabs->setCurrentIndex(-1);
                }
            }
        }
        UpdateEditorProperties();
        SetProjectWorkspacePanelsVisible(true);
        m_buildController->SetProjectRoot(projectPath);
        m_buildAction->setEnabled(true);
        m_runAction->setEnabled(true);
        m_configureAction->setEnabled(true);
        m_testAction->setEnabled(true);
        m_gitStatusAction->setEnabled(true);
        m_createBranchAction->setEnabled(true);
        m_switchBranchAction->setEnabled(true);
        m_stashChangesAction->setEnabled(true);
        m_commitChangesAction->setEnabled(true);
        m_applyStashAction->setEnabled(true);
        m_popStashAction->setEnabled(true);
        UpdateGitActionAvailability();
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
        QString filePath = m_selectedProjectItemPath;
        if (filePath.isEmpty() &&
            m_editorManager->IsEditorTab(m_workspaceTabs->currentIndex()))
        {
            filePath = m_editorManager->CurrentFilePath();
        }

        const QFileInfo fileInfo(filePath);
        if (filePath.isEmpty() || !fileInfo.exists() || !fileInfo.isFile())
        {
            m_propertiesLabel->clear();
            return;
        }

        QString modifiedState = QStringLiteral("Not open");
        for (int index = 0; index < m_workspaceTabs->count(); ++index)
        {
            if (m_editorManager->IsEditorTab(index) &&
                QString::compare(
                    QFileInfo(m_editorManager->FilePath(index)).absoluteFilePath(),
                    fileInfo.absoluteFilePath(),
                    Qt::CaseInsensitive) == 0)
            {
                modifiedState = m_editorManager->IsModified(index)
                    ? QStringLiteral("Modified")
                    : QStringLiteral("Saved");
                break;
            }
        }

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
                QDir::toNativeSeparators(fileInfo.absoluteFilePath()),
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
