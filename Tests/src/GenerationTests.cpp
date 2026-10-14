#include <BootstrapForge/Architecture/AST/GenerationNodes.hpp>
#include <BootstrapForge/Architecture/Diagnostics/DiagnosticEngine.hpp>
#include <BootstrapForge/Architecture/Grammar/Grammar.hpp>
#include <BootstrapForge/Architecture/Parsing/GenerateCommandParser.hpp>
#include <BootstrapForge/Application/Generation/GenerationPlan.hpp>
#include <BootstrapForge/Application/Generation/GenerationPlanner.hpp>
#include <BootstrapForge/Infrastructure/Generation/GenerationExecutor.hpp>
#include <BootstrapForge/Tests/TestSupport.hpp>

#include <atomic>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace
{
    using namespace BootstrapForge;

    class TemporaryDirectory final
    {
    public:

        TemporaryDirectory()
        {
            static std::atomic<unsigned long long> next{0};
            do
            {
                m_path = std::filesystem::temp_directory_path() /
                    ("BootstrapForge-GenerationTests-" +
                        std::to_string(next.fetch_add(1)));
            } while (std::filesystem::exists(m_path));
            std::filesystem::create_directories(m_path);
        }

        ~TemporaryDirectory()
        {
            std::error_code ignored;
            std::filesystem::remove_all(m_path, ignored);
        }

        const std::filesystem::path& Path() const noexcept
        {
            return m_path;
        }

    private:

        std::filesystem::path m_path;
    };

    bool BuildPlan(
        std::string_view source,
        Application::Generation::GenerationPlan& output,
        std::string& error)
    {
        Architecture::Grammar::Grammar grammar;
        Architecture::Diagnostics::DiagnosticEngine diagnostics;
        Architecture::Parsing::GenerateCommandParser parser(
            grammar, diagnostics, source);
        auto command = parser.Parse();
        if (!command || diagnostics.HasErrors())
        {
            if (!diagnostics.Diagnostics().Diagnostics().empty())
            {
                error = diagnostics.Diagnostics().Diagnostics().front().Message();
            }
            else
            {
                error = "command parse failed";
            }
            return false;
        }

        Application::Generation::GenerationPlanner planner;
        auto plan = planner.Build(*command, diagnostics);
        if (!plan || diagnostics.HasErrors())
        {
            if (!diagnostics.Diagnostics().Diagnostics().empty())
            {
                error = diagnostics.Diagnostics().Diagnostics().front().Message();
            }
            else
            {
                error = "plan validation failed";
            }
            return false;
        }

        output = std::move(*plan);
        error.clear();
        return true;
    }

    std::string ReadAll(const std::filesystem::path& path)
    {
        std::ifstream input(path, std::ios::binary);
        return std::string(
            std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>());
    }

    void WriteAll(
        const std::filesystem::path& path,
        const std::string& value)
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream output(path, std::ios::binary | std::ios::trunc);
        output.write(value.data(), static_cast<std::streamsize>(value.size()));
        BootstrapForge::Tests::Require(
            output.good(), "test setup should write its temporary file");
    }

    void TestNestedCommandAndContent()
    {
        const std::string source = R"(generate {
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
        file(ArchitectureDiffer.cpp) = """
int answer() {
    return 42;
}
"""
      }
    }
  }
  branch(Game) {}
  branch(Empty) {}
})";

        Architecture::Grammar::Grammar grammar;
        Architecture::Diagnostics::DiagnosticEngine diagnostics;
        Architecture::Parsing::GenerateCommandParser parser(
            grammar, diagnostics, source);
        auto command = parser.Parse();
        BootstrapForge::Tests::Require(
            command != nullptr && !diagnostics.HasErrors(),
            "nested generate command should parse");
        BootstrapForge::Tests::Require(
            command->Children().size() == 3,
            "one command should retain multiple top-level branches");

        const auto* branch = dynamic_cast<const Architecture::AST::BranchNode*>(
            command->Children()[0].get());
        BootstrapForge::Tests::Require(branch != nullptr, "root child should remain a Branch node");
        const auto* firstSubbranch = dynamic_cast<const Architecture::AST::SubbranchNode*>(
            branch->Children()[0].get());
        BootstrapForge::Tests::Require(
            firstSubbranch != nullptr && firstSubbranch->Name() == "include",
            "keyword names should be accepted and node types preserved");

        Application::Generation::GenerationPlan plan;
        std::string error;
        BootstrapForge::Tests::Require(
            BuildPlan(source, plan, error), "valid nested plan should build: " + error);

        bool foundHeader = false;
        bool foundSource = false;
        bool foundEmptyBranch = false;
        for (const auto& entry : plan.Entries())
        {
            const auto path = entry.RelativePath.generic_string();
            if (path == "Engine/include/BootstrapForge/Engine/Differ/ArchitectureDiffer.hpp")
            {
                foundHeader = entry.Kind == Application::Generation::GenerationEntryKind::File &&
                    entry.Content == std::optional<std::string>("");
            }
            if (path == "Engine/src/Differ/ArchitectureDiffer.cpp")
            {
                foundSource = entry.Content.has_value() &&
                    entry.Content->find("return 42;") != std::string::npos;
            }
            if (path == "Empty")
            {
                foundEmptyBranch = entry.Kind == Application::Generation::GenerationEntryKind::Directory;
            }
        }

        BootstrapForge::Tests::Require(foundHeader, "dotted filename and empty placeholder should be planned");
        BootstrapForge::Tests::Require(foundSource, "multiline code should be retained in the plan");
        BootstrapForge::Tests::Require(foundEmptyBranch, "empty containers should create a directory plan entry");
    }

    void TestMalformedAndFileChildrenRejected()
    {
        for (const std::string source : {
            "generate { branch(A) { file(x.cpp) { folder(Nope) {} } } }",
            "generate { subbranch(A) {} }",
            "generate { branch(A) { subbranch(B) { branch(C) {} } } }",
            "generate { branch(A) { file(A.cpp) = \"not raw\" } }"})
        {
            Architecture::Grammar::Grammar grammar;
            Architecture::Diagnostics::DiagnosticEngine diagnostics;
            Architecture::Parsing::GenerateCommandParser parser(
                grammar, diagnostics, source);
            auto command = parser.Parse();
            BootstrapForge::Tests::Require(
                command == nullptr && diagnostics.HasErrors(),
                "malformed or structurally invalid input must be rejected before planning");
        }
    }

    void TestTraversalAndCaseInsensitiveDuplicatesRejected()
    {
        for (const std::string source : {
            "generate { branch(\"..\") {} }",
            "generate { branch(Engine) { file(Thing.cpp) {} folder(thing.cpp) {} } }",
            "generate { branch(Engine) {} branch(engine) {} }"})
        {
            Application::Generation::GenerationPlan plan;
            std::string error;
            BootstrapForge::Tests::Require(
                !BuildPlan(source, plan, error) && !error.empty(),
                "traversal and case-insensitive collisions should fail preflight");
        }
    }

    void TestDeterministicPlan()
    {
        Application::Generation::GenerationPlan first;
        Application::Generation::GenerationPlan second;
        std::string error;
        BootstrapForge::Tests::Require(BuildPlan(
            "generate { branch(Z) {} branch(A) { folder(B) { file(z.cpp) file(a.cpp) } } }",
            first, error), "first plan should build");
        BootstrapForge::Tests::Require(BuildPlan(
            "generate { branch(A) { folder(B) { file(a.cpp) file(z.cpp) } } branch(Z) {} }",
            second, error), "second plan should build");
        BootstrapForge::Tests::Require(
            first.Entries().size() == second.Entries().size(),
            "equivalent trees should have the same operation count");
        for (std::size_t index = 0; index < first.Entries().size(); ++index)
        {
            BootstrapForge::Tests::Require(
                first.Entries()[index].RelativePath == second.Entries()[index].RelativePath &&
                    first.Entries()[index].Kind == second.Entries()[index].Kind,
                "equivalent trees should produce a deterministic sorted plan");
        }
    }

    void TestUnquotedSpacedNameIsPreserved()
    {
        Application::Generation::GenerationPlan plan;
        std::string error;
        BootstrapForge::Tests::Require(
            BuildPlan("generate { branch(My Branch) {} }", plan, error),
            "unquoted names containing spaces should parse");
        BootstrapForge::Tests::Require(
            plan.Entries().size() == 1 &&
                plan.Entries().front().RelativePath.generic_string() == "My Branch",
            "unquoted name characters and spacing should be preserved");
    }

    void TestDirectRootPathAndFileGeneration()
    {
        Application::Generation::GenerationPlan pathPlan;
        std::string error;
        BootstrapForge::Tests::Require(BuildPlan(
            "generate { folder(src) { folder(Engine) {} } }",
            pathPlan, error), "direct workspace-relative path should build");
        BootstrapForge::Tests::Require(
            pathPlan.Entries().size() == 2 &&
                pathPlan.Entries()[0].RelativePath.generic_string() == "src" &&
                pathPlan.Entries()[1].RelativePath.generic_string() == "src/Engine",
            "path mode should not add a synthetic branch directory");

        Application::Generation::GenerationPlan filePlan;
        BootstrapForge::Tests::Require(BuildPlan(
            "generate { file(CMakeLists.txt) = \"\"\"project(Example)\"\"\" }",
            filePlan, error), "a file directly in the workspace root should build");
        BootstrapForge::Tests::Require(
            filePlan.Entries().size() == 1 &&
                filePlan.Entries()[0].RelativePath.generic_string() == "CMakeLists.txt" &&
                filePlan.Entries()[0].Content == "project(Example)",
            "root file plan should preserve the path and content");

        TemporaryDirectory workspace;
        Infrastructure::Generation::GenerationExecutor executor;
        const auto pathReport = executor.Execute(
            pathPlan,
            workspace.Path(),
            {},
            [](const auto&, const std::string&)
            {
                return Infrastructure::Generation::FailureDecision::Stop;
            });
        BootstrapForge::Tests::Require(
            !pathReport.Stopped &&
                std::filesystem::is_directory(workspace.Path() / "src" / "Engine"),
            "path plan should create the exact workspace-relative directory");

        const auto fileReport = executor.Execute(
            filePlan,
            workspace.Path(),
            {},
            [](const auto&, const std::string&)
            {
                return Infrastructure::Generation::FailureDecision::Stop;
            });
        BootstrapForge::Tests::Require(
            !fileReport.Stopped &&
                ReadAll(workspace.Path() / "CMakeLists.txt") == "project(Example)",
            "file plan should create a file at the workspace root with exact content");

        Application::Generation::GenerationPlan emptyFilePlan;
        BootstrapForge::Tests::Require(BuildPlan(
            "generate { file(placeholder.hpp) }",
            emptyFilePlan, error), "empty placeholder file command should build");
        const auto emptyFileReport = executor.Execute(
            emptyFilePlan,
            workspace.Path(),
            {},
            [](const auto&, const std::string&)
            {
                return Infrastructure::Generation::FailureDecision::Stop;
            });
        BootstrapForge::Tests::Require(
            !emptyFileReport.Stopped &&
                std::filesystem::exists(workspace.Path() / "placeholder.hpp") &&
                std::filesystem::file_size(workspace.Path() / "placeholder.hpp") == 0,
            "a file without contents should become an empty placeholder at the root");

        Application::Generation::GenerationPlan collisionPlan;
        BootstrapForge::Tests::Require(!BuildPlan(
            "generate { file(src) folder(src) {} }",
            collisionPlan, error),
            "a root file and directory cannot claim the same destination");
    }

    void TestCreateReuseAndPlaceholderContent()
    {
        TemporaryDirectory workspace;
        std::filesystem::create_directories(workspace.Path() / "ENGINE");

        Application::Generation::GenerationPlan plan;
        std::string error;
        BootstrapForge::Tests::Require(BuildPlan(
            "generate { branch(Engine) { folder(src) { file(main.cpp) = \"\"\"int main() {}\"\"\" } } }",
            plan, error), "create plan should build");

        Infrastructure::Generation::GenerationExecutor executor;
        int failureCalls = 0;
        const auto report = executor.Execute(
            plan,
            workspace.Path(),
            [](const auto&, const auto&, const auto&)
            {
                return Infrastructure::Generation::CollisionDecision::Stop;
            },
            [&failureCalls](const auto&, const std::string&)
            {
                ++failureCalls;
                return Infrastructure::Generation::FailureDecision::Stop;
            });

        BootstrapForge::Tests::Require(!report.Stopped, "safe generation should complete");
        BootstrapForge::Tests::Require(failureCalls == 0, "no failure should require a decision");
        BootstrapForge::Tests::Require(
            std::filesystem::is_directory(workspace.Path() / "ENGINE" / "src"),
            "existing case-insensitive directories should be reused");
        BootstrapForge::Tests::Require(
            ReadAll(workspace.Path() / "ENGINE" / "src" / "main.cpp") == "int main() {}",
            "file content should be written exactly");
    }

    void TestCollisionPreviewSkipAndReplace()
    {
        TemporaryDirectory workspace;
        const auto existing = workspace.Path() / "ENGINE" / "SRC" / "main.cpp";
        WriteAll(existing, "old contents");

        Application::Generation::GenerationPlan plan;
        std::string error;
        BootstrapForge::Tests::Require(BuildPlan(
            "generate { branch(engine) { subbranch(src) { file(MAIN.cpp) = \"\"\"new contents\"\"\" } } }",
            plan, error), "collision plan should build");

        Infrastructure::Generation::GenerationExecutor executor;
        bool previewed = false;
        auto skipped = executor.Execute(
            plan,
            workspace.Path(),
            [&previewed](const auto&, const std::string& oldValue, const std::string& newValue)
            {
                previewed = oldValue == "old contents" && newValue == "new contents";
                return Infrastructure::Generation::CollisionDecision::Skip;
            },
            [](const auto&, const std::string&)
            {
                return Infrastructure::Generation::FailureDecision::Stop;
            });
        BootstrapForge::Tests::Require(previewed, "collision handler should receive both versions");
        BootstrapForge::Tests::Require(ReadAll(existing) == "old contents", "skip must preserve file bytes");

        {
            TemporaryDirectory freshWorkspace;
            WriteAll(
                freshWorkspace.Path() / "Existing" / "a.txt",
                "existing version");
            Application::Generation::GenerationPlan stopPlan;
            BootstrapForge::Tests::Require(BuildPlan(
                "generate { branch(Existing) { file(a.txt) = \"\"\"new version\"\"\" } branch(Later) {} }",
                stopPlan, error), "stop plan should build");
            const auto stopped = executor.Execute(
                stopPlan,
                freshWorkspace.Path(),
                [](const auto&, const std::string&, const std::string&)
                {
                    return Infrastructure::Generation::CollisionDecision::Stop;
                },
                [](const auto&, const std::string&)
                {
                    return Infrastructure::Generation::FailureDecision::Stop;
                });
            BootstrapForge::Tests::Require(stopped.Stopped, "declining a collision should freeze the command");
            BootstrapForge::Tests::Require(
                !std::filesystem::exists(freshWorkspace.Path() / "Later"),
                "all collisions should be previewed before filesystem changes");
            BootstrapForge::Tests::Require(
                ReadAll(freshWorkspace.Path() / "Existing" / "a.txt") == "existing version",
                "declining should leave the colliding file unchanged");
        }

        auto replaced = executor.Execute(
            plan,
            workspace.Path(),
            [](const auto&, const std::string&, const std::string&)
            {
                return Infrastructure::Generation::CollisionDecision::Replace;
            },
            [](const auto&, const std::string&)
            {
                return Infrastructure::Generation::FailureDecision::Stop;
            });
        BootstrapForge::Tests::Require(!replaced.Stopped, "confirmed replacement should complete");
        BootstrapForge::Tests::Require(ReadAll(existing) == "new contents", "confirmed change should replace exact case-insensitive collision");
    }

    void TestFailureRetrySkipAndStop()
    {
        Application::Generation::GenerationPlan plan;
        std::string error;
        BootstrapForge::Tests::Require(BuildPlan(
            "generate { branch(Blocked) {} branch(Survivor) {} }",
            plan, error), "failure plan should build");

        {
            TemporaryDirectory workspace;
            WriteAll(workspace.Path() / "blocked", "blocking file");
            Infrastructure::Generation::GenerationExecutor executor;
            const auto report = executor.Execute(
                plan,
                workspace.Path(),
                {},
                [](const auto&, const std::string& cause)
                {
                    BootstrapForge::Tests::Require(!cause.empty(), "skip decision should show failure cause");
                    return Infrastructure::Generation::FailureDecision::Skip;
                });
            BootstrapForge::Tests::Require(!report.Stopped, "skip should allow later operations to continue");
            BootstrapForge::Tests::Require(
                std::filesystem::is_directory(workspace.Path() / "Survivor"),
                "skip should continue with later plan entries");
            BootstrapForge::Tests::Require(
                !report.Steps.empty() && report.Steps.front().Skipped,
                "skipped failure should be recorded in the report");
        }

        {
            TemporaryDirectory workspace;
            WriteAll(workspace.Path() / "blocked", "blocking file");
            Infrastructure::Generation::GenerationExecutor executor;
            int attempts = 0;
            const auto report = executor.Execute(
                plan,
                workspace.Path(),
                {},
                [&attempts, &workspace](const auto&, const std::string& cause)
                {
                    BootstrapForge::Tests::Require(!cause.empty(), "failure should expose its cause");
                    if (++attempts == 1)
                    {
                        std::filesystem::remove(workspace.Path() / "blocked");
                        return Infrastructure::Generation::FailureDecision::Retry;
                    }
                    return Infrastructure::Generation::FailureDecision::Stop;
                });
            BootstrapForge::Tests::Require(!report.Stopped, "retry after fixing cause should continue");
            BootstrapForge::Tests::Require(std::filesystem::is_directory(workspace.Path() / "Blocked"), "retry should apply failed directory");
            BootstrapForge::Tests::Require(std::filesystem::is_directory(workspace.Path() / "Survivor"), "later operations should run after retry");
        }

        {
            TemporaryDirectory workspace;
            WriteAll(workspace.Path() / "blocked", "blocking file");
            Infrastructure::Generation::GenerationExecutor executor;
            const auto report = executor.Execute(
                plan,
                workspace.Path(),
                {},
                [](const auto&, const std::string&)
                {
                    return Infrastructure::Generation::FailureDecision::Stop;
                });
            BootstrapForge::Tests::Require(report.Stopped, "stop decision should freeze remaining work");
            BootstrapForge::Tests::Require(!std::filesystem::exists(workspace.Path() / "Survivor"), "stop must not run later operations");
        }
    }
}

int main()
{
    using BootstrapForge::Tests::Require;

    TestNestedCommandAndContent();
    std::cout << "[PASS] Nested command, siblings, empty containers, and code content\n";

    TestMalformedAndFileChildrenRejected();
    std::cout << "[PASS] Malformed input and file-child rejection\n";

    TestTraversalAndCaseInsensitiveDuplicatesRejected();
    std::cout << "[PASS] Traversal and case-insensitive duplicate rejection\n";

    TestDeterministicPlan();
    std::cout << "[PASS] Deterministic plan order\n";

    TestUnquotedSpacedNameIsPreserved();
    std::cout << "[PASS] Unquoted names preserve spaces\n";

    TestDirectRootPathAndFileGeneration();
    std::cout << "[PASS] Workspace-root paths and files\n";

    TestCreateReuseAndPlaceholderContent();
    std::cout << "[PASS] Directory reuse and file content creation\n";

    TestCollisionPreviewSkipAndReplace();
    std::cout << "[PASS] Collision preview, skip, and replacement\n";

    TestFailureRetrySkipAndStop();
    std::cout << "[PASS] Failure cause, retry, and stop behavior\n";
    std::cout << "All BootstrapForge generation tests passed.\n";
    return EXIT_SUCCESS;
}
