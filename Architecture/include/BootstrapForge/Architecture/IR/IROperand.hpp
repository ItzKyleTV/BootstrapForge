#pragma once

#include <BootstrapForge/Architecture/IR/IRValue.hpp>

#include <compare>
#include <string>
#include <string_view>
#include <utility>

namespace BootstrapForge::Architecture::IR
{
    class IROperand final
    {
    public:

        constexpr IROperand() = default;

        constexpr IROperand(
            std::string name,
            IRValue value);

        [[nodiscard]]
        constexpr std::string_view Name() const noexcept;

        [[nodiscard]]
        constexpr const IRValue& Value() const noexcept;

        auto operator<=>(const IROperand&) const noexcept = default;

    private:

        std::string m_name;

        IRValue m_value;
    };

    constexpr IROperand::IROperand(
        std::string name,
        IRValue value)
        :
        m_name(std::move(name)),
        m_value(std::move(value))
    {
    }

    constexpr std::string_view
        IROperand::Name() const noexcept
    {
        return m_name;
    }

    constexpr const IRValue&
        IROperand::Value() const noexcept
    {
        return m_value;
    }

} // namespace BootstrapForge::Architecture::IR