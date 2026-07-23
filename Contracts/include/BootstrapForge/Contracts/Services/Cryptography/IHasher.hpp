#pragma once

#include <BootstrapForge/Domain/Types/Hash.hpp>

#include <cstddef>
#include <span>

namespace BootstrapForge::Contracts::Services::Cryptography
{
    class IHasher
    {
    public:

        virtual ~IHasher() = default;

        [[nodiscard]]
        virtual BootstrapForge::Domain::Types::Hash Compute(
            std::span<const std::byte> data) const = 0;
    };

} // namespace BootstrapForge::Contracts::Services::Cryptographyge::Contracts::Services::Cryptographyta) const = 0;