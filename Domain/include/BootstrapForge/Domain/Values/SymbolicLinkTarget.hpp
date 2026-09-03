#pragma once

#include <BootstrapForge/Domain/Types/Path.hpp>

#include <compare>
#include <filesystem>
#include <utility>

namespace BootstrapForge::Domain::Values
{
    class SymbolicLinkTarget final
    {
    public:

        SymbolicLinkTarget() = default;

        explicit SymbolicLinkTarget(
            std::filesystem::path value)
            :
            m_value(std::move(value))
        {
        }

        [[nodiscard]]
        const Types::Path&
            Value() const noexcept
        {
            return m_value;
        }

        [[nodiscard]]
        bool Empty() const noexcept
        {
            return m_value.IsEmpty();
        }

        [[nodiscard]]
        std::string ToString() const
        {
            return m_value.ToString();
        }

        auto operator<=>(
            const SymbolicLinkTarget&) const = default;

    private:

        Types::Path m_value;
    };

} // namespace BootstrapForge::Domain::Values