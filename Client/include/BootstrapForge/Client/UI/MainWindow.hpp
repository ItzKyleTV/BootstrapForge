#pragma once

#include <QMainWindow>

namespace BootstrapForge::Client::UI
{
    class MainWindow final : public QMainWindow
    {
    public:

        explicit MainWindow(QWidget* parent = nullptr);

        ~MainWindow() override = default;

    private:

        void ConfigureWindow();
        void CreateCentralWidget();
    };

} // namespace BootstrapForge::Client::UI