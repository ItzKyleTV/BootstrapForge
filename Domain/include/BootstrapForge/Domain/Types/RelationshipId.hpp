#pragma once

#include <BootstrapForge/Domain/Types/Identifier.hpp>

#include <compare>
#include <functional>

namespace BootstrapForge::Domain::Types
{
    class RelationshipId final : public Identifier
    {
    public:

        constexpr RelationshipId() noexcept = default;

        constexpr explicit RelationshipId(
            const Hash& hash) noexcept
            : Identifier(hash)
        {
        }

        constexpr explicit RelationshipId(
            const Identifier& identifier) noexcept
            : Identifier(identifier)
        {
        }

        auto operator<=>(
            const RelationshipId&) const noexcept = default;
    };

} // namespace BootstrapForge::Domain::Types


namespace std
{
    template<>
    struct hash<
        BootstrapForge::Domain::Types::RelationshipId>
    {
        [[nodiscard]]
        std::size_t operator()(
            const BootstrapForge::Domain::Types::RelationshipId& value) const noexcept
        {
            return std::hash<
                BootstrapForge::Domain::Types::Hash>{}(
                    value.Value());
        }
    };
}