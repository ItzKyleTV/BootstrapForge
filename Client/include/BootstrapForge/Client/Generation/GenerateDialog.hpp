#pragma once

#include <QDialog>
#include <QString>

#include <BootstrapForge/Application/Generation/GenerationPlan.hpp>

#include <optional>

class QLabel;
class QComboBox;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QStackedWidget;
class QTreeWidget;

namespace BootstrapForge::Client::Generation
{
    class GenerateDialog final : public QDialog
    {
    public:

        explicit GenerateDialog(
            QString workspaceRoot,
            QWidget* parent = nullptr);

        [[nodiscard]]
        QString ExecutionSummary() const;

    private:

        void PreviewCommand();
        void ApplyPlan();
        void SelectPackageManifest();
        void InvalidatePreview();
        [[nodiscard]]
        QString BuildCommandSource(
            QString& errorMessage) const;

        QString m_workspaceRoot;
        QComboBox* m_modeCombo = nullptr;
        QStackedWidget* m_inputStack = nullptr;
        QPlainTextEdit* m_commandEditor = nullptr;
        QLineEdit* m_pathEdit = nullptr;
        QLineEdit* m_filePathEdit = nullptr;
        QLineEdit* m_packageManifestEdit = nullptr;
        QPlainTextEdit* m_fileContentEdit = nullptr;
        QTreeWidget* m_planTree = nullptr;
        QLabel* m_contentPreviewLabel = nullptr;
        QLabel* m_packageInfoLabel = nullptr;
        QPlainTextEdit* m_contentPreview = nullptr;
        QLabel* m_statusLabel = nullptr;
        QPushButton* m_applyButton = nullptr;
        std::optional<Application::Generation::GenerationPlan> m_plan;
        QString m_executionSummary;
    };
}
