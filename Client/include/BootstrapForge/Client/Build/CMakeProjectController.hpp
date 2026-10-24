#pragma once

#include <QObject>
#include <QHash>
#include <QString>
#include <QStringList>

class QProcess;

namespace BootstrapForge::Client::Build
{
    class CMakeProjectController final : public QObject
    {
        Q_OBJECT

    public:

        explicit CMakeProjectController(
            QObject* parent = nullptr);

        void SetProjectRoot(
            const QString& path);

        [[nodiscard]]
        QString ProjectRoot() const;

        [[nodiscard]]
        QString BuildDirectory() const;

        [[nodiscard]]
        QString ToolchainReadinessReport() const;

        [[nodiscard]]
        bool IsBusy() const noexcept;

        void Configure();

        void Build();

        void RefreshTests();

        void RunTest(
            const QString& testName);

        void RunAllTests();

        [[nodiscard]]
        QStringList RunnableApplications() const;

        [[nodiscard]]
        bool LaunchApplication(
            const QString& executablePath,
            QString& errorMessage) const;

    signals:

        void OutputReceived(
            const QString& text);

        void TestsDiscovered(
            const QStringList& testNames);

        void BusyChanged(
            bool busy);

        void OperationFinished(
            const QString& operation,
            bool succeeded);

    private:

        enum class Request
        {
            None,
            Configure,
            Build,
            DiscoverTests,
            RunTest,
            RunAllTests
        };

        enum class Phase
        {
            Idle,
            Configure,
            Build,
            DiscoverTests,
            RunTests
        };

        void BeginRequest(
            Request request);

        void StartConfigure();

        void StartBuild();

        void StartTestDiscovery();

        void StartTestRun(
            bool runAll);

        [[nodiscard]]
        bool ValidateDiscoveredTestsForRun(
            QString& errorMessage) const;

        void StartProcess(
            Phase phase,
            const QString& program,
            const QStringList& arguments);

        void ConsumeProcessOutput();

        void OnProcessFinished(
            int exitCode,
            int exitStatus);

        void OnProcessError();

        void FinishRequest(
            bool succeeded,
            const QString& summary);

        [[nodiscard]]
        QString CMakeExecutable() const;

        [[nodiscard]]
        QString CTestExecutable() const;

        [[nodiscard]]
        QString CurrentRequestName() const;

        [[nodiscard]]
        bool HasConfiguredBuild() const;

        QString m_projectRoot;
        QString m_pendingTestName;
        QStringList m_testExecutablePaths;
        QStringList m_discoveredTestNames;
        QHash<QString, QStringList> m_testExecutablesByName;
        QString m_standardOutput;
        QProcess* m_process = nullptr;
        Request m_request = Request::None;
        Phase m_phase = Phase::Idle;
        bool m_busy = false;
    };

} // namespace BootstrapForge::Client::Build
