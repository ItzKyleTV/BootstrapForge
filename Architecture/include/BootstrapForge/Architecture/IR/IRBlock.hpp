#pragma once

#include <BootstrapForge/Architecture/IR/IRInstruction.hpp>

#include <compare>
#include <utility>
#include <vector>

namespace BootstrapForge::Architecture::IR
{
    class IRBlock final
    {
    public:

        constexpr IRBlock() = default;

        constexpr explicit IRBlock(
            std::vector<IRInstruction> instructions);

        [[nodiscard]]
        constexpr const std::vector<IRInstruction>&
            Instructions() const noexcept;

        auto operator<=>(const IRBlock&) const noexcept = default;

    private:

        std::vector<IRInstruction> m_instructions;
    };

    constexpr IRBlock::IRBlock(
        std::vector<IRInstruction> instructions)
        :
        m_instructions(std::move(instructions))
    {
    }

    constexpr const std::vector<IRInstruction>&
        IRBlock::Instructions() const noexcept
    {
        return m_instructions;
    }

} // namespace BootstrapForge::Architecture::IR