#pragma once

#include <BootstrapForge/Architecture/IR/IRBlock.hpp>

#include <compare>
#include <utility>
#include <vector>

namespace BootstrapForge::Architecture::IR
{
    class IRDocument final
    {
    public:

        constexpr IRDocument() = default;

        constexpr explicit IRDocument(
            std::vector<IRBlock> blocks);

        [[nodiscard]]
        constexpr const std::vector<IRBlock>&
            Blocks() const noexcept;

        auto operator<=>(const IRDocument&) const noexcept = default;

    private:

        std::vector<IRBlock> m_blocks;
    };

    constexpr IRDocument::IRDocument(
        std::vector<IRBlock> blocks)
        :
        m_blocks(std::move(blocks))
    {
    }

    constexpr const std::vector<IRBlock>&
        IRDocument::Blocks() const noexcept
    {
        return m_blocks;
    }

} // namespace BootstrapForge::Architecture::IR