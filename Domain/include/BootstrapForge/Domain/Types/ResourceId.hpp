#pragma once

#include <BootstrapForge/Domain/Types/Identifier.hpp>

#include <functional>

namespace BootstrapForge::Domain::Types
{
    class ResourceId final : public Identifier
    {
    public:

        constexpr ResourceId() noexcept = default;

        constexpr explicit ResourceId(
            const Hash& hash) noexcept
            : Identifier(hash)
        {
        }

        constexpr explicit ResourceId(
            const Identifier& identifier) noexcept
            : Identifier(identifier)
        {
        }
    };

} // namespace BootstrapForge::Domain::Types


namespace std
{
    template<>
    struct hash<
        BootstrapForge::Domain::Types::ResourceId>
    {
        [[nodiscard]]
        std::size_t operator()(
            const BootstrapForge::Domain::Types::ResourceId& value) const noexcept
        {
            return std::hash<
                BootstrapForge::Domain::Types::Hash>{}(
                    value.Value());
        }
    };
}