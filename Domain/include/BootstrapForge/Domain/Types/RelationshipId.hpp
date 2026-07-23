#pragma once

#include <BootstrapForge/Domain/Types/Identifier.hpp>

namespace BootstrapForge::Domain::Types
{
    class RelationshipId final : public Identifier
    {
    public:

        constexpr RelationshipId() noexcept = default;

        constexpr explicit RelationshipId(const Hash& hash) noexcept
            : Identifier(hash)
        {
        }

        constexpr explicit RelationshipId(const Identifier& identifier) noexcept
            : Identifier(identifier)
        {
        }
    };

} // namespace BootstrapForge::Domain::Types