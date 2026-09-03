#include <BootstrapForge/Architecture/IR/IRBuilder.hpp>

#include <stdexcept>

namespace BootstrapForge::Architecture::IR
{
    namespace
    {
        IRValue PathValue(
            const std::filesystem::path& path)
        {
            return IRValue(
                IRValueType::Path,
                path.generic_string());
        }

        IRValue Identifier(
            const std::string& value)
        {
            return IRValue(
                IRValueType::Identifier,
                value);
        }
    }

    IRBuilder::IRBuilder()
    {
        BeginBlock();
    }

    void IRBuilder::BeginBlock()
    {
        m_blocks.emplace_back();
    }

    void IRBuilder::EndBlock()
    {
    }

    IRBlock&
        IRBuilder::CurrentBlock()
    {
        if (m_blocks.empty())
        {
            throw std::logic_error(
                "IRBuilder has no active block.");
        }

        return m_blocks.back();
    }

    IRInstruction&
        IRBuilder::CurrentInstruction()
    {
        auto& instructions =
            const_cast<std::vector<IRInstruction>&>(
                CurrentBlock().Instructions());

        if (instructions.empty())
        {
            throw std::logic_error(
                "IRBuilder has no active instruction.");
        }

        return instructions.back();
    }

    void IRBuilder::AddInstruction(
        IRInstructionKind kind)
    {
        auto& instructions =
            const_cast<std::vector<IRInstruction>&>(
                CurrentBlock().Instructions());

        instructions.emplace_back(
            kind,
            std::vector<IROperand>{});
    }

    void IRBuilder::AddOperand(
        std::string name,
        IRValue value)
    {
        auto& operands =
            const_cast<std::vector<IROperand>&>(
                CurrentInstruction().Operands());

        operands.emplace_back(
            std::move(name),
            std::move(value));
    }

    void IRBuilder::CreateProject(
        const std::string& name)
    {
        AddInstruction(IRInstructionKind::CreateProject);
        AddOperand("Project", Identifier(name));
    }

    void IRBuilder::DeleteProject(
        const std::string& name)
    {
        AddInstruction(IRInstructionKind::DeleteProject);
        AddOperand("Project", Identifier(name));
    }

    void IRBuilder::RenameProject(
        const std::string& oldName,
        const std::string& newName)
    {
        AddInstruction(IRInstructionKind::RenameProject);
        AddOperand("OldName", Identifier(oldName));
        AddOperand("NewName", Identifier(newName));
    }

    void IRBuilder::CreateModule(
        const std::string& name)
    {
        AddInstruction(IRInstructionKind::CreateModule);
        AddOperand("Module", Identifier(name));
    }

    void IRBuilder::DeleteModule(
        const std::string& name)
    {
        AddInstruction(IRInstructionKind::DeleteModule);
        AddOperand("Module", Identifier(name));
    }

    void IRBuilder::RenameModule(
        const std::string& oldName,
        const std::string& newName)
    {
        AddInstruction(IRInstructionKind::RenameModule);
        AddOperand("OldName", Identifier(oldName));
        AddOperand("NewName", Identifier(newName));
    }

    void IRBuilder::CreateDirectory(
        const std::filesystem::path& path)
    {
        AddInstruction(IRInstructionKind::CreateDirectory);
        AddOperand("Path", PathValue(path));
    }

    void IRBuilder::DeleteDirectory(
        const std::filesystem::path& path)
    {
        AddInstruction(IRInstructionKind::DeleteDirectory);
        AddOperand("Path", PathValue(path));
    }

    void IRBuilder::CreateFile(
        const std::filesystem::path& path)
    {
        AddInstruction(IRInstructionKind::CreateFile);
        AddOperand("Path", PathValue(path));
    }

    void IRBuilder::DeleteFile(
        const std::filesystem::path& path)
    {
        AddInstruction(IRInstructionKind::DeleteFile);
        AddOperand("Path", PathValue(path));
    }

    void IRBuilder::ModifyFile(
        const std::filesystem::path& path)
    {
        AddInstruction(IRInstructionKind::ModifyFile);
        AddOperand("Path", PathValue(path));
    }

    void IRBuilder::GenerateCMake()
    {
        AddInstruction(
            IRInstructionKind::GenerateCMake);
    }

    void IRBuilder::GenerateWorkspace()
    {
        AddInstruction(
            IRInstructionKind::GenerateWorkspace);
    }

    void IRBuilder::GenerateSolution()
    {
        AddInstruction(
            IRInstructionKind::GenerateSolution);
    }

    void IRBuilder::ExecuteTemplate(
        const std::string& templateName)
    {
        AddInstruction(
            IRInstructionKind::ExecuteTemplate);

        AddOperand(
            "Template",
            Identifier(templateName));
    }

    IRDocument
        IRBuilder::Build() const
    {
        return IRDocument(
            m_blocks);
    }

}