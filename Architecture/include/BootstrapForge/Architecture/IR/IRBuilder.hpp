#pragma once

#include <BootstrapForge/Architecture/IR/IRBlock.hpp>
#include <BootstrapForge/Architecture/IR/IRDocument.hpp>
#include <BootstrapForge/Architecture/IR/IRInstruction.hpp>
#include <BootstrapForge/Architecture/IR/IRInstructionKind.hpp>
#include <BootstrapForge/Architecture/IR/IROperand.hpp>
#include <BootstrapForge/Architecture/IR/IRValue.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace BootstrapForge::Architecture::IR
{
    class IRBuilder final
    {
    public:

        IRBuilder();

        //
        // Block Management
        //

        void BeginBlock();

        void EndBlock();

        //
        // Generic Instruction API
        //

        void AddInstruction(
            IRInstructionKind kind);

        void AddOperand(
            std::string name,
            IRValue value);

        //
        // High-Level Helpers
        //

        void CreateProject(
            const std::string& name);

        void DeleteProject(
            const std::string& name);

        void RenameProject(
            const std::string& oldName,
            const std::string& newName);

        void CreateModule(
            const std::string& name);

        void DeleteModule(
            const std::string& name);

        void RenameModule(
            const std::string& oldName,
            const std::string& newName);

        void CreateDirectory(
            const std::filesystem::path& path);

        void DeleteDirectory(
            const std::filesystem::path& path);

        void CreateFile(
            const std::filesystem::path& path);

        void DeleteFile(
            const std::filesystem::path& path);

        void ModifyFile(
            const std::filesystem::path& path);

        void GenerateCMake();

        void GenerateWorkspace();

        void GenerateSolution();

        void ExecuteTemplate(
            const std::string& templateName);

        [[nodiscard]]
        IRDocument Build() const;

    private:

        IRInstruction& CurrentInstruction();

        IRBlock& CurrentBlock();

    private:

        std::vector<IRBlock> m_blocks;
    };

}