#include <BootstrapForge/Infrastructure/Cryptography/Hashing/Blake3Hasher.hpp>

#include <blake3.h>

#include <array>
#include <cstring>

namespace BootstrapForge::Infrastructure::Cryptography::Hashing
{
    using BootstrapForge::Domain::Types::Hash;

    Hash Blake3Hasher::Compute(std::span<const std::byte> data) const
    {
        blake3_hasher hasher{};
        blake3_hasher_init(&hasher);

        blake3_hasher_update(
            &hasher,
            data.data(),
            data.size());

        std::array<std::byte, Hash::Size> bytes{};

        blake3_hasher_finalize(
            &hasher,
            reinterpret_cast<std::uint8_t*>(bytes.data()),
            bytes.size());

        return Hash(bytes);
    }

} // namespace BootstrapForge::Infrastructure::Cryptography::Hashing