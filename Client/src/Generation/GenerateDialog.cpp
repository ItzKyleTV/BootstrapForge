#include <BootstrapForge/Client/Generation/GenerateDialog.hpp>

#include <BootstrapForge/Architecture/Diagnostics/DiagnosticEngine.hpp>
#include <BootstrapForge/Architecture/Grammar/Grammar.hpp>
#include <BootstrapForge/Architecture/Parsing/GenerateCommandParser.hpp>
#include <BootstrapForge/Application/Generation/GenerationPlanner.hpp>
#include <BootstrapForge/Infrastructure/Generation/GenerationExecutor.hpp>

#include <QAbstractItemView>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVariant>
#include <QVBoxLayout>

#include <filesystem>
#include <sstream>
#include <string>

namespace BootstrapForge::Client::Generation
{
    namespace
    {
        std::filesystem::path ToFilesystemPath(const QString& value)
        {
#ifdef _WIN32
            return std::filesystem::path(value.toStdWString());
#else
            const QByteArray utf8 = value.toUtf8();
            const auto* begin = reinterpret_cast<const char8_t*>(utf8.constData());
            return std::filesystem::path(
                std::u8string(begin, static_cast<std::size_t>(utf8.size())));
#endif
        }

        QString ToQString(const std::filesystem::path& value)
        {
#ifdef _WIN32
            return QString::fromStdWString(value.wstring());
#else
            const auto utf8 = value.generic_u8string();
            return QString::fromUtf8(
                reinterpret_cast<const char*>(utf8.data()),
                static_cast<qsizetype>(utf8.size()));
#endif
        }

        QString KindText(Application::Generation::GenerationEntryKind kind)
        {
            return kind == Application::Generation::GenerationEntryKind::Directory
                ? QStringLiteral("Directory")
                : QStringLiteral("File");
        }

        bool IsSafePathSegment(const QString& segment)
        {
            if (segment.isEmpty() || segment == QStringLiteral(".") ||
                segment == QStringLiteral("..") ||
                segment.endsWith(QLatin1Char('.')) ||
                segment.endsWith(QLatin1Char(' ')))
            {
                return false;
            }

            const QString forbidden = QStringLiteral("<>:\"/\\|?*");
            for (const QChar character : segment)
            {
                if (character.unicode() < 32 || forbidden.contains(character))
                {
                    return false;
                }
            }

            const QString deviceName = segment.section(QLatin1Char('.'), 0, 0)
                .toUpper();
            return deviceName != QStringLiteral("CON") &&
                deviceName != QStringLiteral("PRN") &&
                deviceName != QStringLiteral("AUX") &&
                deviceName != QStringLiteral("NUL") &&
                !(deviceName.size() == 4 &&
                    (deviceName.startsWith(QStringLiteral("COM")) ||
                        deviceName.startsWith(QStringLiteral("LPT"))) &&
                    deviceName.back() >= QLatin1Char('1') &&
                    deviceName.back() <= QLatin1Char('9'));
        }

        bool SplitSafeRelativePath(
            QString path,
            QStringList& segments,
            QString& errorMessage)
        {
            path.replace(QLatin1Char('\\'), QLatin1Char('/'));
            if (path.trimmed().isEmpty())
            {
                errorMessage = QStringLiteral("Enter a workspace-relative path.");
                return false;
            }
            if (path.startsWith(QLatin1Char('/')) ||
                (path.size() >= 2 && path[1] == QLatin1Char(':')))
            {
                errorMessage = QStringLiteral(
                    "Use a path relative to the open project; absolute paths are not allowed.");
                return false;
            }

            segments = path.split(QLatin1Char('/'), Qt::KeepEmptyParts);
            for (const QString& segment : segments)
            {
                if (!IsSafePathSegment(segment))
                {
                    errorMessage = QStringLiteral(
                        "The path contains an empty, unsafe, or reserved name: %1")
                        .arg(segment);
                    return false;
                }
            }
            return !segments.isEmpty();
        }

        QString QuotedName(const QString& name)
        {
            return QStringLiteral("\"") + name + QStringLiteral("\"");
        }

        Infrastructure::Generation::CollisionDecision PreviewCollision(
            QWidget* parent,
            const std::filesystem::path& relativePath,
            const std::string& existing,
            const std::string& proposed)
        {
            QDialog dialog(parent);
            dialog.setWindowTitle(QStringLiteral("File already exists"));
            dialog.resize(1000, 700);

            auto* layout = new QVBoxLayout(&dialog);
            auto* heading = new QLabel(
                QStringLiteral("The destination already exists: %1\n"
                    "Review both versions before choosing an action.")
                    .arg(ToQString(relativePath)),
                &dialog);
            layout->addWidget(heading);

            auto* splitter = new QSplitter(Qt::Horizontal, &dialog);
            auto* existingPanel = new QWidget(splitter);
            auto* existingLayout = new QVBoxLayout(existingPanel);
            existingLayout->setContentsMargins(0, 0, 0, 0);
            existingLayout->addWidget(new QLabel(
                QStringLiteral("Existing file"), existingPanel));
            auto* existingEditor = new QPlainTextEdit(existingPanel);
            existingLayout->addWidget(existingEditor, 1);

            auto* proposedPanel = new QWidget(splitter);
            auto* proposedLayout = new QVBoxLayout(proposedPanel);
            proposedLayout->setContentsMargins(0, 0, 0, 0);
            proposedLayout->addWidget(new QLabel(
                QStringLiteral("Proposed file"), proposedPanel));
            auto* proposedEditor = new QPlainTextEdit(proposedPanel);
            proposedLayout->addWidget(proposedEditor, 1);
            existingEditor->setReadOnly(true);
            proposedEditor->setReadOnly(true);
            existingEditor->setPlainText(QString::fromUtf8(
                existing.data(), static_cast<qsizetype>(existing.size())));
            proposedEditor->setPlainText(QString::fromUtf8(
                proposed.data(), static_cast<qsizetype>(proposed.size())));
            existingEditor->setPlaceholderText(QStringLiteral("(empty file)"));
            proposedEditor->setPlaceholderText(QStringLiteral("(empty file)"));
            splitter->setStretchFactor(0, 1);
            splitter->setStretchFactor(1, 1);
            layout->addWidget(splitter, 1);

            auto* buttons = new QDialogButtonBox(&dialog);
            auto* replace = buttons->addButton(
                QStringLiteral("Replace file"), QDialogButtonBox::AcceptRole);
            auto* skip = buttons->addButton(
                QStringLiteral("Keep existing / skip"), QDialogButtonBox::ActionRole);
            auto* stop = buttons->addButton(
                QStringLiteral("Stop generation"), QDialogButtonBox::RejectRole);
            layout->addWidget(buttons);

            Infrastructure::Generation::CollisionDecision decision =
                Infrastructure::Generation::CollisionDecision::Stop;
            QObject::connect(replace, &QPushButton::clicked, &dialog, [&]
            {
                decision = Infrastructure::Generation::CollisionDecision::Replace;
                dialog.accept();
            });
            QObject::connect(skip, &QPushButton::clicked, &dialog, [&]
            {
                decision = Infrastructure::Generation::CollisionDecision::Skip;
                dialog.accept();
            });
            QObject::connect(stop, &QPushButton::clicked, &dialog, [&]
            {
                decision = Infrastructure::Generation::CollisionDecision::Stop;
                dialog.reject();
            });
            dialog.exec();
            return decision;
        }

        Infrastructure::Generation::FailureDecision AskFailureAction(
            QWidget* parent,
            const std::filesystem::path& relativePath,
            const std::string& cause)
        {
            QMessageBox box(parent);
            box.setIcon(QMessageBox::Critical);
            box.setWindowTitle(QStringLiteral("Generation paused"));
            box.setText(QStringLiteral("Could not apply: %1")
                .arg(ToQString(relativePath)));
            box.setInformativeText(QString::fromUtf8(
                cause.data(), static_cast<qsizetype>(cause.size())));

            auto* retry = box.addButton(QStringLiteral("Retry"), QMessageBox::AcceptRole);
            auto* skip = box.addButton(QStringLiteral("Skip and continue"), QMessageBox::DestructiveRole);
            auto* stop = box.addButton(QStringLiteral("Stop"), QMessageBox::RejectRole);
            box.setDefaultButton(stop);
            box.exec();

            if (box.clickedButton() == retry)
            {
                return Infrastructure::Generation::FailureDecision::Retry;
            }
            if (box.clickedButton() == skip)
            {
                return Infrastructure::Generation::FailureDecision::Skip;
            }
            return Infrastructure::Generation::FailureDecision::Stop;
        }
    }

    GenerateDialog::GenerateDialog(
        QString workspaceRoot,
        QWidget* parent)
        :
        QDialog(parent),
        m_workspaceRoot(std::move(workspaceRoot))
    {
        setWindowTitle(QStringLiteral("Generate from Command"));
        resize(1000, 760);

        auto* layout = new QVBoxLayout(this);
        layout->addWidget(new QLabel(
            QStringLiteral("Choose a command, path, or file to preview changes."),
            this));

        m_modeCombo = new QComboBox(this);
        m_modeCombo->addItems({
            QStringLiteral("Command"),
            QStringLiteral("Generate Path"),
            QStringLiteral("Generate File")});
        auto* modeForm = new QFormLayout();
        modeForm->addRow(QStringLiteral("Create"), m_modeCombo);
        layout->addLayout(modeForm);

        m_inputStack = new QStackedWidget(this);

        m_commandEditor = new QPlainTextEdit(m_inputStack);
        m_commandEditor->setObjectName(QStringLiteral("GenerateCommandEditor"));
        m_commandEditor->setPlaceholderText(QStringLiteral(
            "generate {\n"
            "  branch(Engine) {\n"
            "    subbranch(src) {\n"
            "      folder(Renderer) {\n"
            "        file(Renderer.cpp) = \"\"\"\n"
            "// implementation\n"
            "\"\"\"\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "}"));
        m_inputStack->addWidget(m_commandEditor);

        auto* pathForm = new QWidget(m_inputStack);
        auto* pathLayout = new QFormLayout(pathForm);
        m_pathEdit = new QLineEdit(pathForm);
        m_pathEdit->setObjectName(QStringLiteral("GeneratePathEdit"));
        m_pathEdit->setPlaceholderText(QStringLiteral("src/Engine/Renderer"));
        pathLayout->addRow(QStringLiteral("Project-relative path"), m_pathEdit);
        pathLayout->addRow(new QLabel(
            QStringLiteral("Creates the directory path exactly as entered."),
            pathForm));
        m_inputStack->addWidget(pathForm);

        auto* fileForm = new QWidget(m_inputStack);
        auto* fileLayout = new QFormLayout(fileForm);
        m_filePathEdit = new QLineEdit(fileForm);
        m_filePathEdit->setObjectName(QStringLiteral("GenerateFilePathEdit"));
        m_filePathEdit->setPlaceholderText(QStringLiteral("src/Engine/Renderer.cpp"));
        fileLayout->addRow(QStringLiteral("Project-relative file path"), m_filePathEdit);
        m_fileContentEdit = new QPlainTextEdit(fileForm);
        m_fileContentEdit->setObjectName(QStringLiteral("GenerateFileContentEdit"));
        m_fileContentEdit->setPlaceholderText(
            QStringLiteral("Leave empty to create an empty placeholder file."));
        m_fileContentEdit->setMinimumHeight(150);
        fileLayout->addRow(QStringLiteral("Initial contents"), m_fileContentEdit);
        m_inputStack->addWidget(fileForm);
        layout->addWidget(m_inputStack, 1);

        auto* buttonsRow = new QHBoxLayout();
        auto* previewButton = new QPushButton(QStringLiteral("Preview command"), this);
        m_applyButton = new QPushButton(QStringLiteral("Apply plan"), this);
        m_applyButton->setEnabled(false);
        buttonsRow->addWidget(previewButton);
        buttonsRow->addWidget(m_applyButton);
        buttonsRow->addStretch();
        layout->addLayout(buttonsRow);

        auto* previewSplitter = new QSplitter(Qt::Horizontal, this);
        m_planTree = new QTreeWidget(previewSplitter);
        m_planTree->setColumnCount(3);
        m_planTree->setHeaderLabels({
            QStringLiteral("Workspace-relative path"),
            QStringLiteral("Action"),
            QStringLiteral("Content bytes")});
        m_planTree->setSelectionMode(QAbstractItemView::SingleSelection);
        m_planTree->setUniformRowHeights(true);

        auto* contentPanel = new QWidget(previewSplitter);
        auto* contentLayout = new QVBoxLayout(contentPanel);
        contentLayout->setContentsMargins(0, 0, 0, 0);
        m_contentPreviewLabel = new QLabel(
            QStringLiteral("Select a file to preview its generated contents."),
            contentPanel);
        m_contentPreviewLabel->setWordWrap(true);
        contentLayout->addWidget(m_contentPreviewLabel);

        m_contentPreview = new QPlainTextEdit(contentPanel);
        m_contentPreview->setReadOnly(true);
        m_contentPreview->setLineWrapMode(QPlainTextEdit::NoWrap);
        m_contentPreview->setPlaceholderText(
            QStringLiteral("(empty placeholder file)"));
        QFont previewFont = m_contentPreview->font();
        previewFont.setStyleHint(QFont::Monospace);
        m_contentPreview->setFont(previewFont);
        contentLayout->addWidget(m_contentPreview, 1);

        previewSplitter->setStretchFactor(0, 3);
        previewSplitter->setStretchFactor(1, 2);
        layout->addWidget(previewSplitter, 2);

        m_statusLabel = new QLabel(this);
        m_statusLabel->setWordWrap(true);
        layout->addWidget(m_statusLabel);

        auto* closeButtons = new QDialogButtonBox(QDialogButtonBox::Close, this);
        QObject::connect(closeButtons, &QDialogButtonBox::rejected,
            this, &QDialog::reject);
        layout->addWidget(closeButtons);

        QObject::connect(previewButton, &QPushButton::clicked,
            this, [this] { PreviewCommand(); });
        QObject::connect(m_applyButton, &QPushButton::clicked,
            this, [this] { ApplyPlan(); });
        QObject::connect(m_planTree, &QTreeWidget::currentItemChanged,
            this, [this](QTreeWidgetItem* current, QTreeWidgetItem*)
            {
                m_contentPreview->clear();
                if (current == nullptr || !m_plan)
                {
                    m_contentPreviewLabel->setText(
                        QStringLiteral("Select a file to preview its generated contents."));
                    return;
                }

                const qulonglong entryIndex =
                    current->data(0, Qt::UserRole).toULongLong();
                if (entryIndex >= m_plan->Entries().size())
                {
                    return;
                }

                const auto& entry = m_plan->Entries()[
                    static_cast<std::size_t>(entryIndex)];
                if (entry.Kind != Application::Generation::GenerationEntryKind::File)
                {
                    m_contentPreviewLabel->setText(
                        QStringLiteral("Directory: %1")
                            .arg(ToQString(entry.RelativePath)));
                    return;
                }

                m_contentPreviewLabel->setText(
                    QStringLiteral("File contents: %1")
                        .arg(ToQString(entry.RelativePath)));
                if (entry.Content && !entry.Content->empty())
                {
                    m_contentPreview->setPlainText(QString::fromUtf8(
                        entry.Content->data(),
                        static_cast<qsizetype>(entry.Content->size())));
                }
            });
        QObject::connect(m_modeCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this](int index)
            {
                m_inputStack->setCurrentIndex(index);
                InvalidatePreview();
            });
        QObject::connect(m_commandEditor, &QPlainTextEdit::textChanged,
            this, [this] { InvalidatePreview(); });
        QObject::connect(m_pathEdit, &QLineEdit::textChanged,
            this, [this] { InvalidatePreview(); });
        QObject::connect(m_filePathEdit, &QLineEdit::textChanged,
            this, [this] { InvalidatePreview(); });
        QObject::connect(m_fileContentEdit, &QPlainTextEdit::textChanged,
            this, [this] { InvalidatePreview(); });
    }

    void GenerateDialog::InvalidatePreview()
    {
        m_plan.reset();
        m_executionSummary.clear();
        m_applyButton->setEnabled(false);
        m_planTree->clear();
        m_contentPreview->clear();
        m_contentPreviewLabel->setText(
            QStringLiteral("Select a file to preview its generated contents."));
        m_statusLabel->setText(QStringLiteral(
            "Input changed; preview it again before applying."));
    }

    QString GenerateDialog::BuildCommandSource(QString& errorMessage) const
    {
        errorMessage.clear();
        if (m_modeCombo->currentIndex() == 0)
        {
            return m_commandEditor->toPlainText();
        }

        QStringList segments;
        const bool isFile = m_modeCombo->currentIndex() == 2;
        const QString path = isFile ? m_filePathEdit->text() : m_pathEdit->text();
        if (!SplitSafeRelativePath(path, segments, errorMessage))
        {
            return {};
        }

        const QString content = isFile ? m_fileContentEdit->toPlainText() : QString{};
        if (isFile && content.contains(QStringLiteral("\"\"\"")))
        {
            errorMessage = QStringLiteral(
                "File contents cannot contain the triple-quote delimiter used by generate commands.");
            return {};
        }

        QString declaration;
        if (isFile)
        {
            const QString fileName = segments.takeLast();
            declaration = QStringLiteral("file(%1)").arg(QuotedName(fileName));
            if (!content.isEmpty())
            {
                declaration += QStringLiteral(" = \"\"\"") + content +
                    QStringLiteral("\"\"\"");
            }
        }
        else
        {
            const QString leaf = segments.takeLast();
            declaration = QStringLiteral("folder(%1) {}").arg(QuotedName(leaf));
        }

        QString source = QStringLiteral("generate {\n");
        QString indent = QStringLiteral("  ");
        for (const QString& segment : segments)
        {
            source += indent + QStringLiteral("folder(%1) {\n")
                .arg(QuotedName(segment));
            indent += QStringLiteral("  ");
        }
        source += indent + declaration + QLatin1Char('\n');
        for (qsizetype index = 0; index < segments.size(); ++index)
        {
            indent.chop(2);
            source += indent + QStringLiteral("}\n");
        }
        source += QStringLiteral("}");
        return source;
    }

    QString GenerateDialog::ExecutionSummary() const
    {
        return m_executionSummary;
    }

    void GenerateDialog::PreviewCommand()
    {
        m_plan.reset();
        m_applyButton->setEnabled(false);
        m_planTree->clear();
        m_contentPreview->clear();
        m_contentPreviewLabel->setText(
            QStringLiteral("Select a file to preview its generated contents."));

        Architecture::Grammar::Grammar grammar;
        Architecture::Diagnostics::DiagnosticEngine diagnostics;
        QString errorMessage;
        const QString sourceText = BuildCommandSource(errorMessage);
        if (!errorMessage.isEmpty())
        {
            m_statusLabel->setText(QStringLiteral("Input invalid; nothing was changed.\n%1")
                .arg(errorMessage));
            return;
        }
        const QByteArray source = sourceText.toUtf8();
        Architecture::Parsing::GenerateCommandParser parser(
            grammar,
            diagnostics,
            std::string_view(source.constData(), static_cast<std::size_t>(source.size())));
        auto command = parser.Parse();

        if (!command || diagnostics.HasErrors())
        {
            QStringList messages;
            for (const auto& diagnostic : diagnostics.Diagnostics().Diagnostics())
            {
                messages.push_back(QString::fromStdString(diagnostic.Message()));
            }
            m_statusLabel->setText(QStringLiteral("Command invalid; nothing was changed.\n%1")
                .arg(messages.join(QLatin1Char('\n'))));
            return;
        }

        Application::Generation::GenerationPlanner planner;
        m_plan = planner.Build(*command, diagnostics);
        if (!m_plan || diagnostics.HasErrors())
        {
            QStringList messages;
            for (const auto& diagnostic : diagnostics.Diagnostics().Diagnostics())
            {
                messages.push_back(QString::fromStdString(diagnostic.Message()));
            }
            m_statusLabel->setText(QStringLiteral("Plan invalid; nothing was changed.\n%1")
                .arg(messages.join(QLatin1Char('\n'))));
            return;
        }

        QTreeWidgetItem* firstFileItem = nullptr;
        for (std::size_t index = 0; index < m_plan->Entries().size(); ++index)
        {
            const auto& entry = m_plan->Entries()[index];
            const QString contentBytes = entry.Content
                ? QString::number(static_cast<qulonglong>(entry.Content->size()))
                : QString{};
            auto* item = new QTreeWidgetItem({
                ToQString(entry.RelativePath),
                KindText(entry.Kind),
                contentBytes});
            item->setData(0, Qt::UserRole,
                QVariant::fromValue(static_cast<qulonglong>(index)));
            m_planTree->addTopLevelItem(item);
            if (firstFileItem == nullptr &&
                entry.Kind == Application::Generation::GenerationEntryKind::File)
            {
                firstFileItem = item;
            }
        }

        if (firstFileItem != nullptr)
        {
            m_planTree->setCurrentItem(firstFileItem);
        }

        m_statusLabel->setText(QStringLiteral("Validated %1 planned paths under %2.")
            .arg(m_plan->Entries().size())
            .arg(m_workspaceRoot));
        m_applyButton->setEnabled(true);
    }

    void GenerateDialog::ApplyPlan()
    {
        if (!m_plan)
        {
            return;
        }

        const auto answer = QMessageBox::question(
            this,
            QStringLiteral("Apply generation plan"),
            QStringLiteral("Apply %1 planned paths to:\n\n%2?\n\n"
                "Existing directories will be reused. Existing files will be previewed before replacement.")
                .arg(m_plan->Entries().size())
                .arg(m_workspaceRoot),
            QMessageBox::Apply | QMessageBox::Cancel,
            QMessageBox::Cancel);
        if (answer != QMessageBox::Apply)
        {
            return;
        }

        Infrastructure::Generation::GenerationExecutor executor;
        const auto report = executor.Execute(
            *m_plan,
            ToFilesystemPath(m_workspaceRoot),
            [this](
                const std::filesystem::path& path,
                const std::string& existing,
                const std::string& proposed)
            {
                return PreviewCollision(this, path, existing, proposed);
            },
            [this](
                const std::filesystem::path& path,
                const std::string& cause)
            {
                return AskFailureAction(this, path, cause);
            });

        std::ostringstream summary;
        std::size_t completed = 0;
        std::size_t skipped = 0;
        for (const auto& step : report.Steps)
        {
            if (step.Succeeded && !step.Skipped)
            {
                ++completed;
                summary << "Applied: " << step.RelativePath.generic_string() << '\n';
            }
            if (step.Skipped)
            {
                ++skipped;
                summary << "Skipped: " << step.RelativePath.generic_string() << '\n';
            }
            else if (!step.Succeeded)
            {
                summary << "Failed: " << step.RelativePath.generic_string()
                    << " — " << step.Detail << '\n';
            }
        }
        summary << "Applied operations: " << completed
            << "; skipped: " << skipped
            << (report.Stopped ? "; stopped with remaining work frozen." : "; plan finished.");

        m_executionSummary = QString::fromStdString(summary.str());
        m_statusLabel->setText(m_executionSummary);
        m_applyButton->setEnabled(false);
    }
}
