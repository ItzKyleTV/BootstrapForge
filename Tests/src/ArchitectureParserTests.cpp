#include <BootstrapForge/Architecture/AST/ArchitectureNode.hpp>
#include <BootstrapForge/Architecture/AST/BuildNode.hpp>
#include <BootstrapForge/Architecture/AST/GeneratorNode.hpp>
#include <BootstrapForge/Architecture/AST/ModuleNode.hpp>
#include <BootstrapForge/Architecture/AST/ProjectNode.hpp>
#include <BootstrapForge/Architecture/AST/RelationshipNode.hpp>
#include <BootstrapForge/Architecture/AST/ResourceNode.hpp>

#include <BootstrapForge/Architecture/Diagnostics/DiagnosticEngine.hpp>

#include <BootstrapForge/Architecture/Grammar/Grammar.hpp>
#include <BootstrapForge/Architecture/Grammar/TokenKind.hpp>

#include <BootstrapForge/Architecture/Parsing/Parser.hpp>

#include <array>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

namespace
{
    using BootstrapForge::Architecture::AST::ArchitectureNode;
    using BootstrapForge::Architecture::AST::BuildNode;
    using BootstrapForge::Architecture::AST::GeneratorNode;
    using BootstrapForge::Architecture::AST::ModuleNode;
    using BootstrapForge::Architecture::AST::ProjectNode;
    using BootstrapForge::Architecture::AST::RelationshipNode;
    using BootstrapForge::Architecture::AST::ResourceNode;

    using BootstrapForge::Architecture::Diagnostics::Diagnostic;
    using BootstrapForge::Architecture::Diagnostics::DiagnosticEngine;

    using BootstrapForge::Architecture::Grammar::Grammar;
    using BootstrapForge::Architecture::Grammar::TokenKind;

    using BootstrapForge::Architecture::Parsing::Parser;

    void Require(
        bool condition,
        const std::string& message)
    {
        if (!condition)
        {
            std::cerr << "[FAIL] " << message << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    template<typename T>
    const T& RequireNode(
        const ArchitectureNode& architecture,
        std::size_t index,
        const std::string& message)
    {
        const auto* node =
            dynamic_cast<const T*>(architecture.Child(index));

        Require(node != nullptr, message);

        return *node;
    }

    void RequireNoDiagnostics(
        const DiagnosticEngine& diagnostics,
        const std::string& message)
    {
        Require(
            diagnostics.Diagnostics().Empty(),
            message);
    }

    const Diagnostic& RequireErrorAt(
        const DiagnosticEngine& diagnostics,
        std::size_t index,
        std::size_t beginOffset,
        std::size_t endOffset,
        const std::string& message)
    {
        const auto& entries =
            diagnostics.Diagnostics().Diagnostics();

        Require(
            index < entries.size(),
            message + ": expected diagnostic is missing.");

        const Diagnostic& diagnostic = entries[index];

        Require(
            diagnostic.IsError(),
            message + ": diagnostic severity should be Error.");

        Require(
            !diagnostic.Message().empty(),
            message + ": diagnostic message should not be empty.");

        Require(
            diagnostic.Span().Begin().Offset == beginOffset,
            message + ": diagnostic begins at the wrong offset.");

        Require(
            diagnostic.Span().End().Offset == endOffset,
            message + ": diagnostic ends at the wrong offset.");

        return diagnostic;
    }

    void TestEmptyInput()
    {
        const Grammar grammar;
        DiagnosticEngine diagnostics;
        Parser parser(grammar, diagnostics, "");

        const auto architecture = parser.Parse();

        Require(
            architecture != nullptr,
            "Empty input should produce an architecture root.");

        Require(
            architecture->ChildCount() == 0,
            "Empty input should produce no declarations.");

        Require(
            std::string_view(architecture->KindName()) == "Architecture",
            "The root node should identify itself as Architecture.");

        Require(
            architecture->Span().Begin() == architecture->Span().End(),
            "The empty architecture span should have zero length.");

        RequireNoDiagnostics(
            diagnostics,
            "Empty input should produce no diagnostics.");
    }

    void TestProjectDeclaration()
    {
        const Grammar grammar;
        DiagnosticEngine diagnostics;
        Parser parser(grammar, diagnostics, "project Example");

        const auto architecture = parser.Parse();

        Require(
            architecture->ChildCount() == 1,
            "A project declaration should produce one child.");

        const auto& node =
            RequireNode<ProjectNode>(
                *architecture,
                0,
                "The project declaration should produce a ProjectNode.");

        Require(
            node.Name() == "Example",
            "The project name should be preserved.");

        Require(
            std::string_view(node.KindName()) == "Project",
            "The project node kind should be Project.");

        Require(
            node.Span().Begin().Offset == 0,
            "The project span should begin at the declaration keyword.");

        RequireNoDiagnostics(
            diagnostics,
            "A valid project declaration should produce no diagnostics.");
    }

    void TestModuleDeclaration()
    {
        const Grammar grammar;
        DiagnosticEngine diagnostics;
        Parser parser(grammar, diagnostics, "module Core");

        const auto architecture = parser.Parse();

        Require(
            architecture->ChildCount() == 1,
            "A module declaration should produce one child.");

        const auto& node =
            RequireNode<ModuleNode>(
                *architecture,
                0,
                "The module declaration should produce a ModuleNode.");

        Require(
            node.Name() == "Core",
            "The module name should be preserved.");

        RequireNoDiagnostics(
            diagnostics,
            "A valid module declaration should produce no diagnostics.");
    }

    void TestDirectoryDeclaration()
    {
        const Grammar grammar;
        DiagnosticEngine diagnostics;
        Parser parser(grammar, diagnostics, R"(directory "src/core")");

        const auto architecture = parser.Parse();

        Require(
            architecture->ChildCount() == 1,
            "A directory declaration should produce one child.");

        const auto& node =
            RequireNode<ResourceNode>(
                *architecture,
                0,
                "The directory declaration should produce a ResourceNode.");

        Require(
            node.IsDirectory(),
            "The directory resource should be classified as a directory.");

        Require(
            node.ResourceTokenKind() == TokenKind::KeywordDirectory,
            "The directory resource token kind should be preserved.");

        Require(
            node.Path() == "src/core",
            "The directory path should be preserved without surrounding quotes.");

        RequireNoDiagnostics(
            diagnostics,
            "A valid directory declaration should produce no diagnostics.");
    }

    void TestFileDeclaration()
    {
        const Grammar grammar;
        DiagnosticEngine diagnostics;
        Parser parser(grammar, diagnostics, R"(file "src/main.cpp")");

        const auto architecture = parser.Parse();

        Require(
            architecture->ChildCount() == 1,
            "A file declaration should produce one child.");

        const auto& node =
            RequireNode<ResourceNode>(
                *architecture,
                0,
                "The file declaration should produce a ResourceNode.");

        Require(
            node.IsFile(),
            "The file resource should be classified as a file.");

        Require(
            node.ResourceTokenKind() == TokenKind::KeywordFile,
            "The file resource token kind should be preserved.");

        Require(
            node.Path() == "src/main.cpp",
            "The file path should be preserved without surrounding quotes.");

        RequireNoDiagnostics(
            diagnostics,
            "A valid file declaration should produce no diagnostics.");
    }

    void TestRelationshipDeclaration()
    {
        const Grammar grammar;
        DiagnosticEngine diagnostics;
        Parser parser(
            grammar,
            diagnostics,
            "relationship Core -> Runtime");

        const auto architecture = parser.Parse();

        Require(
            architecture->ChildCount() == 1,
            "A relationship declaration should produce one child.");

        const auto& node =
            RequireNode<RelationshipNode>(
                *architecture,
                0,
                "The relationship declaration should produce a RelationshipNode.");

        Require(
            node.Source() == "Core",
            "The relationship source should be preserved.");

        Require(
            node.Target() == "Runtime",
            "The relationship target should be preserved.");

        RequireNoDiagnostics(
            diagnostics,
            "A valid relationship declaration should produce no diagnostics.");
    }

    void TestBuildDeclaration()
    {
        const Grammar grammar;
        DiagnosticEngine diagnostics;
        Parser parser(grammar, diagnostics, "build Release");

        const auto architecture = parser.Parse();

        Require(
            architecture->ChildCount() == 1,
            "A build declaration should produce one child.");

        const auto& node =
            RequireNode<BuildNode>(
                *architecture,
                0,
                "The build declaration should produce a BuildNode.");

        Require(
            node.Target() == "Release",
            "The build target should be preserved.");

        RequireNoDiagnostics(
            diagnostics,
            "A valid build declaration should produce no diagnostics.");
    }

    void TestGeneratorDeclaration()
    {
        const Grammar grammar;
        DiagnosticEngine diagnostics;
        Parser parser(grammar, diagnostics, "generator CMake Desktop");

        const auto architecture = parser.Parse();

        Require(
            architecture->ChildCount() == 1,
            "A generator declaration should produce one child.");

        const auto& node =
            RequireNode<GeneratorNode>(
                *architecture,
                0,
                "The generator declaration should produce a GeneratorNode.");

        Require(
            node.Generator() == "CMake",
            "The generator name should be preserved.");

        Require(
            node.Target() == "Desktop",
            "The generator target should be preserved.");

        RequireNoDiagnostics(
            diagnostics,
            "A valid generator declaration should produce no diagnostics.");
    }

    void TestMultipleDeclarations()
    {
        const Grammar grammar;
        DiagnosticEngine diagnostics;

        Parser parser(
            grammar,
            diagnostics,
            "project Example\n"
            "module Core\n"
            "directory src\n"
            "file \"main.cpp\"\n"
            "relationship Core -> Runtime\n"
            "build Release\n"
            "generator CMake Desktop");

        const auto architecture = parser.Parse();

        Require(
            architecture->ChildCount() == 7,
            "All seven declarations should be retained in source order.");

        Require(
            std::string_view(architecture->Child(0)->KindName()) == "Project",
            "The first declaration should remain the project.");

        Require(
            std::string_view(architecture->Child(1)->KindName()) == "Module",
            "The second declaration should remain the module.");

        Require(
            std::string_view(architecture->Child(2)->KindName()) == "Resource",
            "The third declaration should remain the directory resource.");

        Require(
            std::string_view(architecture->Child(3)->KindName()) == "Resource",
            "The fourth declaration should remain the file resource.");

        Require(
            std::string_view(architecture->Child(4)->KindName()) == "Relationship",
            "The fifth declaration should remain the relationship.");

        Require(
            std::string_view(architecture->Child(5)->KindName()) == "Build",
            "The sixth declaration should remain the build.");

        Require(
            std::string_view(architecture->Child(6)->KindName()) == "Generator",
            "The seventh declaration should remain the generator.");

        RequireNoDiagnostics(
            diagnostics,
            "Valid declarations should produce no diagnostics.");
    }

    void TestLineComments()
    {
        const Grammar grammar;
        DiagnosticEngine diagnostics;

        Parser parser(
            grammar,
            diagnostics,
            "// architecture declarations follow\n"
            "project Example // project comment\n"
            "module Core");

        const auto architecture = parser.Parse();

        Require(
            architecture->ChildCount() == 2,
            "Line comments should not become AST declarations.");

        Require(
            RequireNode<ProjectNode>(
                *architecture,
                0,
                "The first declaration should be a project.").Name() == "Example",
            "The project should parse correctly after a comment.");

        Require(
            RequireNode<ModuleNode>(
                *architecture,
                1,
                "The second declaration should be a module.").Name() == "Core",
            "The module should parse correctly after a trailing comment.");

        RequireNoDiagnostics(
            diagnostics,
            "Valid declarations with comments should produce no diagnostics.");
    }

    void TestUnexpectedDeclarationReportsAndContinues()
    {
        const Grammar grammar;
        DiagnosticEngine diagnostics;
        const std::string source =
            "include common\n"
            "module Kept";

        Parser parser(grammar, diagnostics, source);
        const auto architecture = parser.Parse();

        Require(
            architecture->ChildCount() == 1,
            "An unsupported declaration should be skipped while later declarations survive.");

        Require(
            RequireNode<ModuleNode>(
                *architecture,
                0,
                "The supported declaration after include should be a module.").Name() == "Kept",
            "The module after an unsupported declaration should be retained.");

        Require(
            diagnostics.Diagnostics().Size() == 1,
            "An unsupported top-level keyword should report one diagnostic.");

        RequireErrorAt(
            diagnostics,
            0,
            0,
            std::string_view("include").size(),
            "The include diagnostic should cover the unsupported keyword");
    }

    void TestMissingProjectIdentifierReportsAtEof()
    {
        const Grammar grammar;
        DiagnosticEngine diagnostics;
        const std::string source = "project";

        Parser parser(grammar, diagnostics, source);
        const auto architecture = parser.Parse();

        Require(
            architecture->ChildCount() == 0,
            "A malformed project should not create an AST declaration.");

        Require(
            diagnostics.Diagnostics().Size() == 1,
            "A project missing its identifier should report one diagnostic.");

        RequireErrorAt(
            diagnostics,
            0,
            source.size(),
            source.size(),
            "A missing identifier at EOF should have a zero-width EOF span");
    }

    void TestMalformedRelationshipReportsMissingArrowAndTarget()
    {
        const Grammar grammar;

        {
            DiagnosticEngine diagnostics;
            const std::string source =
                "relationship Source Target\n"
                "module Later";

            Parser parser(grammar, diagnostics, source);
            const auto architecture = parser.Parse();
            const std::size_t targetOffset = source.find("Target");

            Require(
                architecture->ChildCount() == 1,
                "A relationship missing its arrow should be dropped and later declarations retained.");

            Require(
                RequireNode<ModuleNode>(
                    *architecture,
                    0,
                    "The declaration after the malformed relationship should be a module.").Name() == "Later",
                "The later module should survive a missing relationship arrow.");

            Require(
                diagnostics.Diagnostics().Size() == 1,
                "A missing relationship arrow should report one diagnostic.");

            RequireErrorAt(
                diagnostics,
                0,
                targetOffset,
                targetOffset + std::string_view("Target").size(),
                "The missing-arrow diagnostic should point to the unexpected target");
        }

        {
            DiagnosticEngine diagnostics;
            const std::string source = "relationship Source ->";

            Parser parser(grammar, diagnostics, source);
            const auto architecture = parser.Parse();

            Require(
                architecture->ChildCount() == 0,
                "A relationship missing its target at EOF should be dropped.");

            Require(
                diagnostics.Diagnostics().Size() == 1,
                "A missing relationship target should report one diagnostic.");

            RequireErrorAt(
                diagnostics,
                0,
                source.size(),
                source.size(),
                "A missing relationship target should have a zero-width EOF span");
        }
    }

    void TestMultipleErrorsPreserveValidDeclarationsAndSkipMalformedOnes()
    {
        const Grammar grammar;
        DiagnosticEngine diagnostics;
        const std::string source =
            "module Before\n"
            "project\n"
            "module Middle\n"
            "build\n"
            "file After";

        Parser parser(grammar, diagnostics, source);
        const auto architecture = parser.Parse();

        Require(
            diagnostics.Diagnostics().Size() == 2,
            "Two malformed declarations should produce two diagnostics.");

        const std::size_t middleOffset = source.find("module Middle");
        const std::size_t fileOffset = source.find("file After");

        RequireErrorAt(
            diagnostics,
            0,
            middleOffset,
            middleOffset + std::string_view("module").size(),
            "The missing project identifier should point at the next declaration");

        RequireErrorAt(
            diagnostics,
            1,
            fileOffset,
            fileOffset + std::string_view("file").size(),
            "The missing build target should point at the next declaration");

        Require(
            architecture->ChildCount() == 3,
            "Valid declarations before and after malformed declarations should remain in the AST.");

        Require(
            RequireNode<ModuleNode>(
                *architecture,
                0,
                "The first valid declaration should be a module.").Name() == "Before",
            "The valid declaration before errors should be preserved.");

        Require(
            RequireNode<ModuleNode>(
                *architecture,
                1,
                "The valid declaration after the first error should be a module.").Name() == "Middle",
            "The module following the malformed project should be preserved.");

        const auto& resource =
            RequireNode<ResourceNode>(
                *architecture,
                2,
                "The final valid declaration should be a resource.");

        Require(
            resource.IsFile() && resource.Path() == "After",
            "The file following the malformed build should be preserved.");
    }

    void TestUnterminatedStringReportsOriginalSpanAndRecovers()
    {
        const Grammar grammar;
        DiagnosticEngine diagnostics;
        const std::string source =
            "file \"unfinished\n"
            "module Later";

        Parser parser(grammar, diagnostics, source);
        const auto architecture = parser.Parse();

        const std::size_t literalBegin = source.find('"');
        const std::size_t literalEnd = source.find('\n');

        Require(
            diagnostics.Diagnostics().Size() == 1,
            "An unterminated string should report exactly one diagnostic.");

        RequireErrorAt(
            diagnostics,
            0,
            literalBegin,
            literalEnd,
            "The unterminated string diagnostic should use the literal token span");

        Require(
            architecture->ChildCount() == 1,
            "The malformed file declaration should be dropped while later declarations survive.");

        Require(
            RequireNode<ModuleNode>(
                *architecture,
                0,
                "The declaration after the unterminated string should be a module.").Name() == "Later",
            "The module after an unterminated string should be retained.");
    }

    void TestRecoveryAtEverySupportedDeclarationStart()
    {
        struct RecoveryCase
        {
            std::string_view Declaration;
            std::string_view ExpectedKind;
        };

        constexpr std::array<RecoveryCase, 7> cases{{
            { "project Kept", "Project" },
            { "module Kept", "Module" },
            { "directory path", "Resource" },
            { "file path", "Resource" },
            { "relationship Source -> Target", "Relationship" },
            { "build Release", "Build" },
            { "generator CMake Desktop", "Generator" }
        }};

        const Grammar grammar;

        for (const RecoveryCase& recoveryCase : cases)
        {
            DiagnosticEngine diagnostics;
            const std::string source =
                "project\n" +
                std::string(recoveryCase.Declaration);

            Parser parser(grammar, diagnostics, source);
            const auto architecture = parser.Parse();

            Require(
                diagnostics.Diagnostics().Size() == 1,
                "A malformed project before a declaration start should produce one diagnostic.");

            Require(
                architecture->ChildCount() == 1,
                "Recovery should preserve the declaration at each supported start token.");

            Require(
                std::string_view(architecture->Child(0)->KindName()) ==
                    recoveryCase.ExpectedKind,
                "Recovery should resume with the expected supported declaration kind.");

            const std::size_t declarationOffset =
                source.find(recoveryCase.Declaration);
            const std::size_t tokenEndInDeclaration =
                recoveryCase.Declaration.find(' ');

            RequireErrorAt(
                diagnostics,
                0,
                declarationOffset,
                declarationOffset + tokenEndInDeclaration,
                "Recovery should report at, and preserve, the next declaration start");
        }
    }

    void TestUnsupportedRecognizedKeywordsRemainInvalid()
    {
        constexpr std::array<std::string_view, 9> unsupportedKeywords{{
            "architecture",
            "branch",
            "create",
            "modify",
            "remove",
            "rename",
            "move",
            "copy",
            "include"
        }};

        const Grammar grammar;

        for (const std::string_view keyword : unsupportedKeywords)
        {
            DiagnosticEngine diagnostics;
            const std::string source =
                std::string(keyword) +
                " Name\n"
                "module Kept";

            Parser parser(grammar, diagnostics, source);
            const auto architecture = parser.Parse();

            Require(
                diagnostics.Diagnostics().Size() == 1,
                "An unsupported recognized keyword should report a syntax diagnostic.");

            RequireErrorAt(
                diagnostics,
                0,
                0,
                keyword.size(),
                "An unsupported keyword diagnostic should cover that keyword");

            Require(
                architecture->ChildCount() == 1,
                "An unsupported keyword must not produce an AST node.");

            Require(
                RequireNode<ModuleNode>(
                    *architecture,
                    0,
                    "The supported declaration after an unsupported keyword should be a module.").Name() == "Kept",
                "The later supported declaration should survive unsupported syntax.");
        }
    }

    void TestUnsupportedInputRecoversAtEof()
    {
        const Grammar grammar;
        DiagnosticEngine diagnostics;
        const std::string source = "include";

        Parser parser(grammar, diagnostics, source);
        const auto architecture = parser.Parse();

        Require(
            architecture->ChildCount() == 0,
            "Unsupported input ending at EOF should produce no declaration nodes.");

        Require(
            diagnostics.Diagnostics().Size() == 1,
            "Unsupported input ending at EOF should produce one diagnostic.");

        RequireErrorAt(
            diagnostics,
            0,
            0,
            source.size(),
            "The unsupported declaration diagnostic should span its token");
    }

    void TestDeclarationSourceSpans()
    {
        const Grammar grammar;
        DiagnosticEngine diagnostics;
        Parser parser(
            grammar,
            diagnostics,
            "project Example\nmodule Core");

        const auto architecture = parser.Parse();

        Require(
            architecture->ChildCount() == 2,
            "The source-span test should parse two declarations.");

        const auto& project =
            RequireNode<ProjectNode>(
                *architecture,
                0,
                "The first declaration should be a project.");

        const auto& module =
            RequireNode<ModuleNode>(
                *architecture,
                1,
                "The second declaration should be a module.");

        Require(
            project.Span().Begin().Offset == 0,
            "The project span should begin at offset zero.");

        Require(
            project.Span().End().Offset == 16,
            "The project span should end at the next token's starting offset.");

        Require(
            module.Span().Begin().Offset == 16,
            "The module span should begin at its declaration keyword.");

        Require(
            module.Span().End().Offset == 27,
            "The module span should end at EOF.");

        RequireNoDiagnostics(
            diagnostics,
            "Valid declarations should produce no diagnostics.");
    }

    void RunTest(
        const char* name,
        void (*test)())
    {
        try
        {
            test();
            std::cout << "[PASS] " << name << '\n';
        }
        catch (const std::exception& exception)
        {
            std::cerr
                << "[FAIL] "
                << name
                << ": "
                << exception.what()
                << '\n';

            std::exit(EXIT_FAILURE);
        }
    }
}

int main()
{
    RunTest("Empty input", TestEmptyInput);
    RunTest("Project declaration", TestProjectDeclaration);
    RunTest("Module declaration", TestModuleDeclaration);
    RunTest("Directory declaration", TestDirectoryDeclaration);
    RunTest("File declaration", TestFileDeclaration);
    RunTest("Relationship declaration", TestRelationshipDeclaration);
    RunTest("Build declaration", TestBuildDeclaration);
    RunTest("Generator declaration", TestGeneratorDeclaration);
    RunTest("Multiple declarations", TestMultipleDeclarations);
    RunTest("Line comments", TestLineComments);
    RunTest("Unsupported declaration recovery", TestUnexpectedDeclarationReportsAndContinues);
    RunTest("Missing project identifier", TestMissingProjectIdentifierReportsAtEof);
    RunTest("Malformed relationship recovery", TestMalformedRelationshipReportsMissingArrowAndTarget);
    RunTest("Multiple errors and partial AST", TestMultipleErrorsPreserveValidDeclarationsAndSkipMalformedOnes);
    RunTest("Unterminated string recovery", TestUnterminatedStringReportsOriginalSpanAndRecovers);
    RunTest("Recovery at each declaration start", TestRecoveryAtEverySupportedDeclarationStart);
    RunTest("Unsupported keywords stay invalid", TestUnsupportedRecognizedKeywordsRemainInvalid);
    RunTest("EOF recovery", TestUnsupportedInputRecoversAtEof);
    RunTest("Declaration source spans", TestDeclarationSourceSpans);

    std::cout << "All architecture parser tests passed.\n";

    return EXIT_SUCCESS;
}
