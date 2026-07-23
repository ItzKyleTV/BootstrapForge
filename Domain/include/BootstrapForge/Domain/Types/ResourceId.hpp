#pragma once

#include <BootstrapForge/Domain/Types/Identifier.hpp>

namespace BootstrapForge::Domain::Types
{
    class ResourceId final : public Identifier
    {
    public:

        constexpr ResourceId() noexcept = default;

        constexpr explicit ResourceId(const Hash& hash) noexcept
            : Identifier(hash)
        {
        }

        constexpr explicit ResourceId(const Identifier& identifier) noexcept
            : Identifier(identifier)
        {
        }
    };

} // namespace BootstrapForge::Domain::Types