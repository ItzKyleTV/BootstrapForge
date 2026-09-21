#include <BootstrapForge/Client/UI/MainWindow.hpp>

#include <QApplication>

int main(
    int argc,
    char* argv[])
{
    QApplication application(
        argc,
        argv);

    BootstrapForge::Client::UI::MainWindow
        mainWindow;

    mainWindow.show();

    return application.exec();
}