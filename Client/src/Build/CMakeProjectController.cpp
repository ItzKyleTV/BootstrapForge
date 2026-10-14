#include <BootstrapForge/Client/Build/CMakeProjectController.hpp>

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>

#include <algorithm>

#ifndef BOOTSTRAPFORGE_QT6_CMAKE_DIR
#define BOOTSTRAPFORGE_QT6_CMAKE_DIR ""
#endif

namespace BootstrapForge::Client::Build
{
    CMakeProjectController::CMakeProjectController(
        QObject* parent)
        :
        QObject(parent),
        m_process(new QProcess(this))
    {
        connect(
            m_process,
            &QProcess::readyReadStandardOutput,
            this,
            &CMakeProjectController::ConsumeProcessOutput);

        connect(
            m_process,
            &QProcess::readyReadStandardError,
            this,
            &CMakeProjectController::ConsumeProcessOutput);

        connect(
            m_process,
            qOverload<int, QProcess::ExitStatus>(
                &QProcess::finished),
            this,
            [this](int exitCode, QProcess::ExitStatus exitStatus)
            {
                OnProcessFinished(
                    exitCode,
                    static_cast<int>(exitStatus));
            });

        connect(
            m_process,
            &QProcess::errorOccurred,
            this,
            [this](QProcess::ProcessError error)
            {
                if (error == QProcess::FailedToStart)
                {
                    OnProcessError();
                }
            });
    }

    void CMakeProjectController::SetProjectRoot(
        const QString& path)
    {
        if (m_busy)
        {
            return;
        }

        const QFileInfo directoryInfo(path);
        m_projectRoot = directoryInfo.exists() && directoryInfo.isDir()
            ? QDir::cleanPath(directoryInfo.absoluteFilePath())
            : QString{};

        m_pendingTestName.clear();
        m_testExecutablePaths.clear();
        emit TestsDiscovered({});
    }

    QString CMakeProjectController::ProjectRoot() const
    {
        return m_projectRoot;
    }

    QString CMakeProjectController::BuildDirectory() const
    {
        if (m_projectRoot.isEmpty())
        {
            return {};
        }

        return QDir(m_projectRoot).filePath(
            QStringLiteral(".bootstrapforge/build"));
    }

    bool CMakeProjectController::IsBusy() const noexcept
    {
        return m_busy;
    }

    void CMakeProjectController::Configure()
    {
        BeginRequest(Request::Configure);
    }

    void CMakeProjectController::Build()
    {
        BeginRequest(Request::Build);
    }

    void CMakeProjectController::RefreshTests()
    {
        if (m_busy)
        {
            return;
        }

        BeginRequest(Request::DiscoverTests);
    }

    void CMakeProjectController::RunTest(
        const QString& testName)
    {
        if (testName.trimmed().isEmpty())
        {
            return;
        }

        m_pendingTestName = testName;
        BeginRequest(Request::RunTest);
    }

    void CMakeProjectController::RunAllTests()
    {
        BeginRequest(Request::RunAllTests);
    }

    QStringList CMakeProjectController::RunnableApplications() const
    {
        QStringList applications;
        const QString buildDirectory = BuildDirectory();
        if (buildDirectory.isEmpty() || !QFileInfo(buildDirectory).isDir())
        {
            return applications;
        }

        QDirIterator iterator(
            buildDirectory,
            QDir::Files | QDir::NoSymLinks,
            QDirIterator::Subdirectories);
        while (iterator.hasNext())
        {
            const QString path = QDir::cleanPath(iterator.next());
            const QFileInfo info(path);
#ifdef Q_OS_WIN
            if (info.suffix().compare(QStringLiteral("exe"), Qt::CaseInsensitive) != 0)
            {
                continue;
            }
#else
            if (!info.isExecutable())
            {
                continue;
            }
#endif

            const QString normalizedPath = QDir::cleanPath(info.absoluteFilePath());
            const bool isTestExecutable = std::any_of(
                m_testExecutablePaths.cbegin(),
                m_testExecutablePaths.cend(),
                [&normalizedPath](const QString& testPath)
                {
                    return QString::compare(
                        normalizedPath, testPath, Qt::CaseInsensitive) == 0;
                });
            if (isTestExecutable)
            {
                continue;
            }

            const QString relativePath = QDir(buildDirectory).relativeFilePath(path);
            const QStringList pathParts = relativePath.split(
                QLatin1Char('/'), Qt::SkipEmptyParts);
            const bool generatedTool = std::any_of(
                pathParts.cbegin(), pathParts.cend(),
                [](const QString& part)
                {
                    return part.compare(QStringLiteral("_deps"), Qt::CaseInsensitive) == 0 ||
                        part.compare(QStringLiteral("CMakeFiles"), Qt::CaseInsensitive) == 0 ||
                        part.compare(QStringLiteral("Testing"), Qt::CaseInsensitive) == 0;
                });
            if (!generatedTool)
            {
                applications.push_back(normalizedPath);
            }
        }

        std::sort(applications.begin(), applications.end(),
            [](const QString& left, const QString& right)
            {
                return QString::compare(left, right, Qt::CaseInsensitive) < 0;
            });
        applications.removeDuplicates();
        return applications;
    }

    bool CMakeProjectController::LaunchApplication(
        const QString& executablePath,
        QString& errorMessage) const
    {
        const QFileInfo executableInfo(executablePath);
        if (!executableInfo.isFile())
        {
            errorMessage = QStringLiteral("The selected application no longer exists: %1")
                .arg(QDir::toNativeSeparators(executablePath));
            return false;
        }

        const QString workingDirectory = m_projectRoot.isEmpty()
            ? executableInfo.absolutePath()
            : m_projectRoot;
        if (!QProcess::startDetached(
                executableInfo.absoluteFilePath(), {}, workingDirectory))
        {
            errorMessage = QStringLiteral("Could not start the selected application: %1")
                .arg(QDir::toNativeSeparators(executableInfo.absoluteFilePath()));
            return false;
        }
        return true;
    }

    void CMakeProjectController::BeginRequest(
        Request request)
    {
        if (m_busy)
        {
            return;
        }

        m_request = request;

        if (m_projectRoot.isEmpty())
        {
            const QString requestName = CurrentRequestName();
            m_request = Request::None;
            emit OutputReceived(QStringLiteral(
                "[ERROR] Open a project before using CMake build or test actions.\n"));
            emit OperationFinished(requestName, false);
            return;
        }

        m_busy = true;
        emit BusyChanged(true);

        if (request == Request::Configure ||
            request == Request::Build)
        {
            StartConfigure();
            return;
        }

        if (request == Request::DiscoverTests)
        {
            if (!HasConfiguredBuild())
            {
                emit TestsDiscovered({});
                FinishRequest(
                    true,
                    QStringLiteral("No configured CMake build was found. Configure CMake to discover tests."));
                return;
            }

            StartTestDiscovery();
            return;
        }

        if (request == Request::RunTest ||
            request == Request::RunAllTests)
        {
            if (!HasConfiguredBuild())
            {
                FinishRequest(
                    false,
                    QStringLiteral("Configure CMake before running tests."));
                return;
            }

            emit OutputReceived(
                QStringLiteral("[TEST] Building project targets before running tests.\n"));
            StartBuild();
        }
    }

    void CMakeProjectController::StartConfigure()
    {
        emit TestsDiscovered({});

        const QString cmakeLists = QDir(m_projectRoot).filePath(
            QStringLiteral("CMakeLists.txt"));
        if (!QFileInfo::exists(cmakeLists))
        {
            emit TestsDiscovered({});
            FinishRequest(
                false,
                QStringLiteral("No CMakeLists.txt exists at the project root: %1")
                    .arg(QDir::toNativeSeparators(m_projectRoot)));
            return;
        }

        const QString cmake = CMakeExecutable();
        if (cmake.isEmpty())
        {
            FinishRequest(
                false,
                QStringLiteral("CMake was not found on PATH."));
            return;
        }

        emit OutputReceived(
            QStringLiteral("[CMAKE] Configure/check: %1\n")
                .arg(QDir::toNativeSeparators(m_projectRoot)));

        QStringList configureArguments{
            QStringLiteral("-S"), m_projectRoot,
            QStringLiteral("-B"), BuildDirectory(),
            QStringLiteral("-DCMAKE_BUILD_TYPE=Debug")
        };

        const QString qt6CMakeDirectory = QDir::fromNativeSeparators(
            QString::fromUtf8(BOOTSTRAPFORGE_QT6_CMAKE_DIR));
        if (!qt6CMakeDirectory.isEmpty() &&
            QFileInfo::exists(QDir(qt6CMakeDirectory).filePath(
                QStringLiteral("Qt6Config.cmake"))))
        {
            configureArguments.push_back(
                QStringLiteral("-DQt6_DIR=%1").arg(qt6CMakeDirectory));
        }

        StartProcess(
            Phase::Configure,
            cmake,
            configureArguments);
    }

    void CMakeProjectController::StartBuild()
    {
        const QString cmake = CMakeExecutable();
        if (cmake.isEmpty())
        {
            FinishRequest(
                false,
                QStringLiteral("CMake was not found on PATH."));
            return;
        }

        emit OutputReceived(QStringLiteral("[BUILD] Building Debug configuration.\n"));
        StartProcess(
            Phase::Build,
            cmake,
            {
                QStringLiteral("--build"), BuildDirectory(),
                QStringLiteral("--config"), QStringLiteral("Debug")
            });
    }

    void CMakeProjectController::StartTestDiscovery()
    {
        const QString ctest = CTestExecutable();
        if (ctest.isEmpty())
        {
            const bool configureOrBuildSucceeded =
                m_request == Request::Configure ||
                m_request == Request::Build;
            emit OutputReceived(
                QStringLiteral("[WARN] CTest was not found beside CMake or on PATH; test discovery was skipped.\n"));
            FinishRequest(
                configureOrBuildSucceeded,
                QStringLiteral("CTest was not found beside CMake or on PATH."));
            return;
        }

        emit OutputReceived(QStringLiteral("[TEST] Discovering CTest tests.\n"));
        StartProcess(
            Phase::DiscoverTests,
            ctest,
            {
                QStringLiteral("--test-dir"), BuildDirectory(),
                QStringLiteral("--show-only=json-v1"),
                QStringLiteral("--build-config"), QStringLiteral("Debug")
            });
    }

    void CMakeProjectController::StartTestRun(
        bool runAll)
    {
        const QString ctest = CTestExecutable();
        if (ctest.isEmpty())
        {
            FinishRequest(
                false,
                QStringLiteral("CTest was not found beside CMake or on PATH."));
            return;
        }

        QStringList arguments{
            QStringLiteral("--test-dir"), BuildDirectory(),
            QStringLiteral("--build-config"), QStringLiteral("Debug"),
            QStringLiteral("--output-on-failure")
        };

        if (!runAll)
        {
            arguments
                << QStringLiteral("--tests-regex")
                << QStringLiteral("^%1$").arg(
                    QRegularExpression::escape(m_pendingTestName));
        }

        emit OutputReceived(
            runAll
                ? QStringLiteral("[TEST] Running all registered tests.\n")
                : QStringLiteral("[TEST] Running %1.\n").arg(m_pendingTestName));

        StartProcess(
            Phase::RunTests,
            ctest,
            arguments);
    }

    void CMakeProjectController::StartProcess(
        Phase phase,
        const QString& program,
        const QStringList& arguments)
    {
        m_phase = phase;
        m_standardOutput.clear();
        m_process->setWorkingDirectory(m_projectRoot);
        m_process->setProgram(program);
        m_process->setArguments(arguments);
        m_process->start();
    }

    void CMakeProjectController::ConsumeProcessOutput()
    {
        const QByteArray standardOutput =
            m_process->readAllStandardOutput();
        const QByteArray standardError =
            m_process->readAllStandardError();

        const QString outputText = QString::fromLocal8Bit(standardOutput);
        const QString errorText = QString::fromLocal8Bit(standardError);
        m_standardOutput += outputText;

        if (!outputText.isEmpty() && m_phase != Phase::DiscoverTests)
        {
            emit OutputReceived(outputText);
        }
        if (!errorText.isEmpty())
        {
            emit OutputReceived(errorText);
        }
    }

    void CMakeProjectController::OnProcessFinished(
        int exitCode,
        int exitStatus)
    {
        if (m_phase == Phase::Idle)
        {
            return;
        }

        ConsumeProcessOutput();

        const bool succeeded =
            exitStatus == static_cast<int>(QProcess::NormalExit) &&
            exitCode == 0;
        if (!succeeded)
        {
            if (m_phase == Phase::DiscoverTests &&
                (m_request == Request::Configure ||
                    m_request == Request::Build))
            {
                emit TestsDiscovered({});
                emit OutputReceived(
                    QStringLiteral("[WARN] CMake completed, but CTest test discovery failed.\n"));
                FinishRequest(
                    true,
                    QStringLiteral("CMake operation completed; test discovery failed with exit code %1.")
                        .arg(exitCode));
                return;
            }

            FinishRequest(
                false,
                QStringLiteral("%1 failed with exit code %2.")
                    .arg(CurrentRequestName())
                    .arg(exitCode));
            return;
        }

        if (m_phase == Phase::Configure)
        {
            emit OutputReceived(QStringLiteral("[CMAKE] Configuration completed.\n"));
            if (m_request == Request::Build)
            {
                StartBuild();
                return;
            }
            StartTestDiscovery();
            return;
        }

        if (m_phase == Phase::Build)
        {
            emit OutputReceived(QStringLiteral("[BUILD] Build completed.\n"));
            if (m_request == Request::RunTest ||
                m_request == Request::RunAllTests)
            {
                StartTestRun(m_request == Request::RunAllTests);
                return;
            }
            StartTestDiscovery();
            return;
        }

        if (m_phase == Phase::DiscoverTests)
        {
            const QJsonDocument document = QJsonDocument::fromJson(
                m_standardOutput.toUtf8());
            if (!document.isObject())
            {
                emit TestsDiscovered({});
                const bool configureOrBuildSucceeded =
                    m_request == Request::Configure ||
                    m_request == Request::Build;
                if (configureOrBuildSucceeded)
                {
                    emit OutputReceived(
                        QStringLiteral("[WARN] CMake completed, but CTest returned invalid discovery JSON.\n"));
                }
                FinishRequest(
                    configureOrBuildSucceeded,
                    QStringLiteral("CTest returned invalid test-discovery JSON."));
                return;
            }

            QStringList testNames;
            m_testExecutablePaths.clear();
            const QJsonArray tests = document.object()
                .value(QStringLiteral("tests")).toArray();
            for (const QJsonValue& value : tests)
            {
                const QJsonObject testObject = value.toObject();
                const QString testName = testObject
                    .value(QStringLiteral("name")).toString();
                if (!testName.isEmpty())
                {
                    testNames.push_back(testName);
                }

                const QJsonArray command = testObject
                    .value(QStringLiteral("command")).toArray();
                if (!command.isEmpty())
                {
                    QString executablePath = command.at(0).toString();
                    if (!QFileInfo(executablePath).isAbsolute())
                    {
                        executablePath = QDir(BuildDirectory()).absoluteFilePath(
                            executablePath);
                    }
                    m_testExecutablePaths.push_back(QDir::cleanPath(
                        QFileInfo(executablePath).absoluteFilePath()));
                }
            }

            emit TestsDiscovered(testNames);
            emit OutputReceived(
                QStringLiteral("[TEST] Discovered %1 registered test(s).\n")
                    .arg(testNames.size()));
            FinishRequest(
                true,
                QStringLiteral("Discovered %1 registered test(s).")
                    .arg(testNames.size()));
            return;
        }

        if (m_phase == Phase::RunTests)
        {
            FinishRequest(
                true,
                QStringLiteral("CTest completed successfully."));
        }
    }

    void CMakeProjectController::OnProcessError()
    {
        if (m_phase == Phase::Idle)
        {
            return;
        }

        if (m_phase == Phase::DiscoverTests &&
            (m_request == Request::Configure ||
                m_request == Request::Build))
        {
            emit TestsDiscovered({});
            emit OutputReceived(
                QStringLiteral("[WARN] CMake completed, but CTest could not start for test discovery.\n"));
            FinishRequest(
                true,
                QStringLiteral("CMake operation completed; test discovery could not start: %1")
                    .arg(m_process->errorString()));
            return;
        }

        FinishRequest(
            false,
            QStringLiteral("Could not start %1: %2")
                .arg(CurrentRequestName(), m_process->errorString()));
    }

    void CMakeProjectController::FinishRequest(
        bool succeeded,
        const QString& summary)
    {
        const QString requestName = CurrentRequestName();
        m_phase = Phase::Idle;
        m_request = Request::None;
        m_pendingTestName.clear();

        emit OutputReceived(
            QStringLiteral("[%1] %2\n")
                .arg(succeeded ? QStringLiteral("OK") : QStringLiteral("ERROR"), summary));

        m_busy = false;
        emit BusyChanged(false);
        emit OperationFinished(requestName, succeeded);
    }

    QString CMakeProjectController::CMakeExecutable() const
    {
        QSettings settings(
            QSettings::IniFormat,
            QSettings::UserScope,
            QStringLiteral("BootstrapForge"),
            QStringLiteral("BootstrapForge.Client"));
        const QString configured = settings.value(
            QStringLiteral("tools/cmakeExecutable")).toString();
        if (!configured.isEmpty() && QFileInfo(configured).isFile())
        {
            return configured;
        }

        QString executable = QStandardPaths::findExecutable(QStringLiteral("cmake"));
        if (executable.isEmpty())
        {
            executable = QStandardPaths::findExecutable(QStringLiteral("cmake.exe"));
        }
        if (!executable.isEmpty())
        {
            return executable;
        }

#ifdef Q_OS_WIN
        const QString programFiles = qEnvironmentVariable("ProgramFiles");
        const QString programFilesX86 = qEnvironmentVariable("ProgramFiles(x86)");
        const QStringList editions{
            QStringLiteral("Community"),
            QStringLiteral("Professional"),
            QStringLiteral("Enterprise"),
            QStringLiteral("BuildTools")};
        for (const QString& root : {programFiles, programFilesX86})
        {
            if (root.isEmpty())
            {
                continue;
            }

            const QString standalone = QDir(root).filePath(
                QStringLiteral("CMake/bin/cmake.exe"));
            if (QFileInfo(standalone).isFile())
            {
                return standalone;
            }

            for (const QString& edition : editions)
            {
                const QString candidate = QDir(root).filePath(
                    QStringLiteral("Microsoft Visual Studio/2022/%1/Common7/IDE/"
                        "CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe")
                        .arg(edition));
                if (QFileInfo(candidate).isFile())
                {
                    return candidate;
                }
            }
        }
#endif
        return {};
    }

    QString CMakeProjectController::CTestExecutable() const
    {
        const QString cmake = CMakeExecutable();
        if (!cmake.isEmpty())
        {
            const QString executable = QDir(QFileInfo(cmake).absolutePath())
                .filePath(QStringLiteral("ctest.exe"));
            if (QFileInfo::exists(executable))
            {
                return executable;
            }

            const QString extensionlessExecutable = QDir(QFileInfo(cmake).absolutePath())
                .filePath(QStringLiteral("ctest"));
            if (QFileInfo::exists(extensionlessExecutable))
            {
                return extensionlessExecutable;
            }
        }

        return QStandardPaths::findExecutable(QStringLiteral("ctest"));
    }

    QString CMakeProjectController::CurrentRequestName() const
    {
        switch (m_request)
        {
        case Request::Configure:
            return QStringLiteral("CMake check");
        case Request::Build:
            return QStringLiteral("Build");
        case Request::DiscoverTests:
            return QStringLiteral("Test discovery");
        case Request::RunTest:
        case Request::RunAllTests:
            return m_phase == Phase::Build
                ? QStringLiteral("Test build")
                : QStringLiteral("Tests");
        case Request::None:
        default:
            return QStringLiteral("CMake operation");
        }
    }

    bool CMakeProjectController::HasConfiguredBuild() const
    {
        return QFileInfo::exists(
            QDir(BuildDirectory()).filePath(QStringLiteral("CMakeCache.txt")));
    }

} // namespace BootstrapForge::Client::Build
