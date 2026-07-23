#pragma once

#include <BootstrapForge/Domain/Types/Hash.hpp>

#include <compare>
#include <string>

namespace BootstrapForge::Domain::Types
{
    class Identifier
    {
    public:

        constexpr Identifier() noexcept = default;

        constexpr explicit Identifier(const Hash& hash) noexcept;

        [[nodiscard]]
        constexpr bool IsEmpty() const noexcept;

        [[nodiscard]]
        constexpr const Hash& Value() const noexcept;

        [[nodiscard]]
        std::string ToString() const;

        auto operator<=>(const Identifier&) const noexcept = default;

    private:

        Hash m_hash{};
    };

    constexpr Identifier::Identifier(const Hash& hash) noexcept
        : m_hash(hash)
    {
    }

    constexpr bool Identifier::IsEmpty() const noexcept
    {
        return m_hash.IsEmpty();
    }

    constexpr const Hash& Identifier::Value() const noexcept
    {
        return m_hash;
    }

} // namespace BootstrapForge::Domain::Types