#pragma once

#include <cstddef>
#include <span>

#include <BootstrapForge/Contracts/Services/Cryptography/IHasher.hpp>

namespace BootstrapForge::Infrastructure::Cryptography::Hashing
{
    class Blake3Hasher final
        : public BootstrapForge::Contracts::Services::Cryptography::IHasher
    {
    public:

        Blake3Hasher() = default;
        ~Blake3Hasher() override = default;

        [[nodiscard]]
        BootstrapForge::Domain::Types::Hash Compute(
            std::span<const std::byte> data) const override;
    };
}