#pragma once

#include <BootstrapForge/Domain/Types/Identifier.hpp>

namespace BootstrapForge::Domain::Types
{
    class OperationId final : public Identifier
    {
    public:

        constexpr OperationId() noexcept = default;

        constexpr explicit OperationId(const Hash& hash) noexcept
            : Identifier(hash)
        {
        }

        constexpr explicit OperationId(const Identifier& identifier) noexcept
            : Identifier(identifier)
        {
        }
    };

} // namespace BootstrapForge::Domain::Types