#include <BootstrapForge/Client/UI/MainWindow.hpp>

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

namespace BootstrapForge::Client::UI
{
    MainWindow::MainWindow(QWidget* parent)
        :
        QMainWindow(parent)
    {
        ConfigureWindow();
        CreateCentralWidget();
    }

    void MainWindow::ConfigureWindow()
    {
        setWindowTitle(
            QStringLiteral("BootstrapForge"));

        resize(
            1200,
            800);

        setMinimumSize(
            900,
            600);
    }

    void MainWindow::CreateCentralWidget()
    {
        auto* centralWidget =
            new QWidget(this);

        auto* layout =
            new QVBoxLayout(centralWidget);

        auto* title =
            new QLabel(
                QStringLiteral(
                    "BootstrapForge Client"),
                centralWidget);

        auto* description =
            new QLabel(
                QStringLiteral(
                    "Architecture workspace and development environment"),
                centralWidget);

        auto* technology =
            new QLabel(
                QStringLiteral(
                    "Qt 6.11.2 • C++23"),
                centralWidget);

        title->setAlignment(
            Qt::AlignCenter);

        description->setAlignment(
            Qt::AlignCenter);

        technology->setAlignment(
            Qt::AlignCenter);

        layout->addStretch();

        layout->addWidget(
            title);

        layout->addWidget(
            description);

        layout->addWidget(
            technology);

        layout->addStretch();

        setCentralWidget(
            centralWidget);
    }

} // namespace BootstrapForge::Client::UI