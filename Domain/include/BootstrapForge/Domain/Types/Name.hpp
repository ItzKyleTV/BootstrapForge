#pragma once

#include <compare>
#include <string>
#include <string_view>

namespace BootstrapForge::Domain::Types
{
    class Name
    {
    public:

        Name() = default;

        explicit Name(std::string value);

        [[nodiscard]]
        bool IsEmpty() const noexcept;

        [[nodiscard]]
        std::string_view Value() const noexcept;

        [[nodiscard]]
        const std::string& ToString() const noexcept;

        auto operator<=>(const Name&) const noexcept = default;

    private:

        std::string m_value;
    };

    inline Name::Name(std::string value)
        : m_value(std::move(value))
    {
    }

    inline bool Name::IsEmpty() const noexcept
    {
        return m_value.empty();
    }

    inline std::string_view Name::Value() const noexcept
    {
        return m_value;
    }

    inline const std::string& Name::ToString() const noexcept
    {
        return m_value;
    }

} // namespace BootstrapForge::Domain::Types