#include <BootstrapForge/Client/UI/MainWindow.hpp>

#include <QApplication>
#include <QIcon>
#include <QResource>

int main(
    int argc,
    char* argv[])
{
    Q_INIT_RESOURCE(bootstrapforge);

    QApplication application(
        argc,
        argv);
    const QIcon applicationIcon(
        QStringLiteral(":/BootstrapForge/bootstrapforge-tree.png"));
    application.setWindowIcon(applicationIcon);

    BootstrapForge::Client::UI::MainWindow
        mainWindow;
    mainWindow.setWindowIcon(applicationIcon);

    mainWindow.show();

    return application.exec();
}
