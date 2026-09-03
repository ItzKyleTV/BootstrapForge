#pragma once

#include <compare>
#include <string>
#include <string_view>
#include <utility>

namespace BootstrapForge::Architecture::IR
{
    enum class IRValueType
    {
        String,
        Integer,
        Boolean,
        Path,
        Identifier
    };

    class IRValue final
    {
    public:

        constexpr IRValue() = default;

        constexpr IRValue(
            IRValueType type,
            std::string value);

        [[nodiscard]]
        constexpr IRValueType Type() const noexcept;

        [[nodiscard]]
        constexpr std::string_view Value() const noexcept;

        [[nodiscard]]
        std::string ToString() const;

        auto operator<=>(const IRValue&) const noexcept = default;

    private:

        IRValueType m_type = IRValueType::String;

        std::string m_value;
    };

    constexpr IRValue::IRValue(
        IRValueType type,
        std::string value)
        :
        m_type(type),
        m_value(std::move(value))
    {
    }

    constexpr IRValueType
        IRValue::Type() const noexcept
    {
        return m_type;
    }

    constexpr std::string_view
        IRValue::Value() const noexcept
    {
        return m_value;
    }

    inline std::string
        IRValue::ToString() const
    {
        return m_value;
    }

} // namespace BootstrapForge::Architecture::IR