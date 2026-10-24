#include <BootstrapForge/Infrastructure/Generation/CodePackageLoader.hpp>
#include <BootstrapForge/Tests/TestSupport.hpp>

#include <atomic>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

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
                    ("BootstrapForge-CodePackageTests-" +
                        std::to_string(next.fetch_add(1)));
            } while (std::filesystem::exists(m_path));
            std::filesystem::create_directories(m_path);
        }

        ~TemporaryDirectory()
        {
            std::error_code ignored;
            std::filesystem::remove_all(m_path, ignored);
        }

        const std::filesystem::path& Path() const noexcept { return m_path; }

    private:
        std::filesystem::path m_path;
    };

    void WriteFile(const std::filesystem::path& path, const std::string& contents)
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream output(path, std::ios::binary | std::ios::trunc);
        output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
        Tests::Require(output.good(), "package fixture file should be written");
    }

    std::optional<Infrastructure::Generation::LoadedCodePackage> Load(
        const std::filesystem::path& root,
        const std::string& manifest,
        std::string& error)
    {
        WriteFile(root / "package.json", manifest);
        Infrastructure::Generation::CodePackageLoader loader;
        return loader.Load(root / "package.json", error);
    }

    void TestPackageAndComponentFilesBecomePreviewablePlan()
    {
        TemporaryDirectory packageRoot;
        WriteFile(packageRoot.Path() / "templates/include/Widget.hpp", "#pragma once\n");
        WriteFile(packageRoot.Path() / "templates/src/Widget.cpp", "int widget() { return 7; }\n");
        WriteFile(packageRoot.Path() / "templates/CMakeLists.txt", "add_library(widget src/Widget.cpp)\n");

        const std::string manifest = R"json({
  "schemaVersion": 1,
  "id": "example.widget",
  "name": "Widget Component",
  "version": "1.2.0",
  "kind": "component",
  "description": "A small reusable code component.",
  "dependencies": ["example.core"],
  "directories": ["assets/empty"],
  "files": [
    {"source": "templates/include/Widget.hpp", "destination": "include/Widget.hpp"},
    {"source": "templates/src/Widget.cpp", "destination": "src/Widget.cpp"},
    {"source": "templates/CMakeLists.txt", "destination": "CMakeLists.txt"}
  ]
})json";

        std::string error;
        auto package = Load(packageRoot.Path(), manifest, error);
        Tests::Require(package.has_value(), "valid component package should load: " + error);
        Tests::Require(package->Info.Id == "example.widget" &&
            package->Info.Kind == "component" && package->Info.Version == "1.2.0",
            "package identity and component metadata should be preserved");
        Tests::Require(package->Info.Dependencies.size() == 1 &&
            package->Info.Dependencies.front() == "example.core",
            "dependency metadata should be available to the preview");

        const auto& entries = package->Plan.Entries();
        Tests::Require(entries.size() == 7,
            "package plan should contain three files, implicit parents, and an empty directory");
        bool foundHeader = false;
        bool foundSource = false;
        bool foundCMake = false;
        bool foundEmptyDirectory = false;
        for (const auto& entry : entries)
        {
            const auto path = entry.RelativePath.generic_string();
            foundHeader = foundHeader || (path == "include/Widget.hpp" &&
                entry.Content == std::optional<std::string>("#pragma once\n"));
            foundSource = foundSource || (path == "src/Widget.cpp" &&
                entry.Content == std::optional<std::string>("int widget() { return 7; }\n"));
            foundCMake = foundCMake || (path == "CMakeLists.txt" &&
                entry.Content == std::optional<std::string>("add_library(widget src/Widget.cpp)\n"));
            foundEmptyDirectory = foundEmptyDirectory || (path == "assets/empty" &&
                entry.Kind == Application::Generation::GenerationEntryKind::Directory);
        }
        Tests::Require(foundHeader && foundSource && foundCMake && foundEmptyDirectory,
            "package code, build files, and empty directories should become plan entries");
    }

    void TestUnsafeSourcesAndDestinationsAreRejected()
    {
        TemporaryDirectory packageRoot;
        WriteFile(packageRoot.Path() / "safe.txt", "safe");
        const std::string prefix = R"json({"schemaVersion":1,"id":"bad","name":"Bad","version":"1","kind":"package","files":[)json";
        const std::string suffix = R"json(]})json";
        std::string error;

        auto traversal = Load(packageRoot.Path(), prefix +
            R"json({"source":"safe.txt","destination":"../outside.txt"})json" + suffix,
            error);
        Tests::Require(!traversal && !error.empty(),
            "workspace path traversal in a destination should be rejected");

        auto sourceTraversal = Load(packageRoot.Path(), prefix +
            R"json({"source":"../outside.txt","destination":"safe.txt"})json" + suffix,
            error);
        Tests::Require(!sourceTraversal && !error.empty(),
            "source paths that escape the package should be rejected");
    }

    void TestCaseInsensitiveAndFileDirectoryCollisionsAreRejected()
    {
        TemporaryDirectory packageRoot;
        WriteFile(packageRoot.Path() / "one.txt", "one");
        WriteFile(packageRoot.Path() / "two.txt", "two");
        std::string error;
        const std::string prefix = R"json({"schemaVersion":1,"id":"bad","name":"Bad","version":"1","kind":"package","files":[)json";
        const std::string suffix = R"json(]})json";

        auto duplicate = Load(packageRoot.Path(), prefix +
            R"json({"source":"one.txt","destination":"src/Widget.cpp"},{"source":"two.txt","destination":"SRC/widget.cpp"})json" + suffix,
            error);
        Tests::Require(!duplicate && !error.empty(),
            "case-insensitive duplicate destinations should be rejected");

        auto fileDirectory = Load(packageRoot.Path(), prefix +
            R"json({"source":"one.txt","destination":"src/Widget.cpp"},{"source":"two.txt","destination":"src"})json" + suffix,
            error);
        Tests::Require(!fileDirectory && !error.empty(),
            "a destination used as both a file and directory should be rejected");
    }
}

int main()
{
    TestPackageAndComponentFilesBecomePreviewablePlan();
    std::cout << "[PASS] Package/component manifest becomes a content preview plan\n";
    TestUnsafeSourcesAndDestinationsAreRejected();
    std::cout << "[PASS] Package source and workspace traversal rejection\n";
    TestCaseInsensitiveAndFileDirectoryCollisionsAreRejected();
    std::cout << "[PASS] Case-insensitive and file/directory collisions\n";
    std::cout << "All BootstrapForge code package tests passed.\n";
    return EXIT_SUCCESS;
}
