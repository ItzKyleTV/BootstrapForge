#include <BootstrapForge/Client/UI/MainWindow.hpp>

#include <QApplication>
#include <QIcon>

int main(
    int argc,
    char* argv[])
{
    QApplication application(
        argc,
        argv);
    application.setWindowIcon(
        QIcon(QStringLiteral(":/BootstrapForge/bootstrapforge-tree.png")));

    BootstrapForge::Client::UI::MainWindow
        mainWindow;

    mainWindow.show();

    return application.exec();
}
