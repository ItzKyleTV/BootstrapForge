#pragma once

#include <BootstrapForge/Architecture/IR/IRInstructionKind.hpp>
#include <BootstrapForge/Architecture/IR/IROperand.hpp>

#include <compare>
#include <utility>
#include <vector>

namespace BootstrapForge::Architecture::IR
{
    class IRInstruction final
    {
    public:

        constexpr IRInstruction() = default;

        constexpr IRInstruction(
            IRInstructionKind kind,
            std::vector<IROperand> operands);

        [[nodiscard]]
        constexpr IRInstructionKind Kind() const noexcept;

        [[nodiscard]]
        constexpr const std::vector<IROperand>& Operands() const noexcept;

        auto operator<=>(const IRInstruction&) const noexcept = default;

    private:

        IRInstructionKind m_kind =
            IRInstructionKind::CreateProject;

        std::vector<IROperand> m_operands;
    };

    constexpr IRInstruction::IRInstruction(
        IRInstructionKind kind,
        std::vector<IROperand> operands)
        :
        m_kind(kind),
        m_operands(std::move(operands))
    {
    }

    constexpr IRInstructionKind
        IRInstruction::Kind() const noexcept
    {
        return m_kind;
    }

    constexpr const std::vector<IROperand>&
        IRInstruction::Operands() const noexcept
    {
        return m_operands;
    }

} // namespace BootstrapForge::Architecture::IR