#include <BootstrapForge/Architecture/AST/ArchitectureNode.hpp>
#include <BootstrapForge/Architecture/AST/BuildNode.hpp>
#include <BootstrapForge/Architecture/AST/GeneratorNode.hpp>
#include <BootstrapForge/Architecture/AST/ModuleNode.hpp>
#include <BootstrapForge/Architecture/AST/ProjectNode.hpp>
#include <BootstrapForge/Architecture/AST/RelationshipNode.hpp>
#include <BootstrapForge/Architecture/AST/ResourceNode.hpp>

#include <BootstrapForge/Architecture/Grammar/Grammar.hpp>
#include <BootstrapForge/Architecture/Grammar/TokenKind.hpp>

#include <BootstrapForge/Architecture/Parsing/Parser.hpp>

#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
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

    void TestEmptyInput()
    {
        const Grammar grammar;
        Parser parser(grammar, "");

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
    }

    void TestProjectDeclaration()
    {
        const Grammar grammar;
        Parser parser(grammar, "project Example");

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
    }

    void TestModuleDeclaration()
    {
        const Grammar grammar;
        Parser parser(grammar, "module Core");

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
    }

    void TestDirectoryDeclaration()
    {
        const Grammar grammar;
        Parser parser(grammar, R"(directory "src/core")");

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
    }

    void TestFileDeclaration()
    {
        const Grammar grammar;
        Parser parser(grammar, R"(file "src/main.cpp")");

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
    }

    void TestRelationshipDeclaration()
    {
        const Grammar grammar;
        Parser parser(grammar, "relationship Core -> Runtime");

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
    }

    void TestBuildDeclaration()
    {
        const Grammar grammar;
        Parser parser(grammar, "build Release");

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
    }

    void TestGeneratorDeclaration()
    {
        const Grammar grammar;
        Parser parser(grammar, "generator CMake Desktop");

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
    }

    void TestMultipleDeclarations()
    {
        const Grammar grammar;

        Parser parser(
            grammar,
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
    }

    void TestLineComments()
    {
        const Grammar grammar;

        Parser parser(
            grammar,
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
    }

    void TestUnexpectedDeclarationThrows()
    {
        const Grammar grammar;
        Parser parser(grammar, "include common.arch");

        bool threw = false;

        try
        {
            static_cast<void>(parser.Parse());
        }
        catch (const std::runtime_error&)
        {
            threw = true;
        }

        Require(
            threw,
            "An unsupported top-level declaration should throw std::runtime_error.");
    }

    void TestMissingIdentifierThrows()
    {
        const Grammar grammar;
        Parser parser(grammar, "project");

        bool threw = false;

        try
        {
            static_cast<void>(parser.Parse());
        }
        catch (const std::runtime_error&)
        {
            threw = true;
        }

        Require(
            threw,
            "A project declaration without an identifier should throw std::runtime_error.");
    }

    void TestDeclarationSourceSpans()
    {
        const Grammar grammar;
        Parser parser(grammar, "project Example\nmodule Core");

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
    RunTest("Unexpected declaration", TestUnexpectedDeclarationThrows);
    RunTest("Missing identifier", TestMissingIdentifierThrows);
    RunTest("Declaration source spans", TestDeclarationSourceSpans);

    std::cout << "All 13 architecture parser tests passed.\n";

    return EXIT_SUCCESS;
}