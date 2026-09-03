#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace BootstrapForge::Domain::Types
{
    class Hash
    {
    public:

        static constexpr std::size_t Size = 32;

        constexpr Hash() noexcept = default;

        constexpr explicit Hash(
            const std::array<std::byte, Size>& bytes) noexcept
            : m_bytes(bytes)
        {
        }

        [[nodiscard]]
        constexpr bool IsEmpty() const noexcept
        {
            for (const auto byte : m_bytes)
            {
                if (byte != std::byte{ 0 })
                {
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]]
        std::string ToHex() const;

        [[nodiscard]]
        static std::optional<Hash> FromHex(
            std::string_view value);

        [[nodiscard]]
        constexpr std::span<
            const std::byte,
            Size>
            Bytes() const noexcept
        {
            return std::span<
                const std::byte,
                Size>(
                    m_bytes);
        }

        auto operator<=>(const Hash&) const noexcept = default;

    private:

        std::array<std::byte, Size> m_bytes{};
    };

} // namespace BootstrapForge::Domain::Types


namespace std
{
    template<>
    struct hash<
        BootstrapForge::Domain::Types::Hash>
    {
        [[nodiscard]]
        std::size_t operator()(
            const BootstrapForge::Domain::Types::Hash& value) const noexcept
        {
            constexpr std::size_t OffsetBasis =
                sizeof(std::size_t) == 8
                ? 14695981039346656037ull
                : 2166136261u;

            constexpr std::size_t Prime =
                sizeof(std::size_t) == 8
                ? 1099511628211ull
                : 16777619u;

            std::size_t result = OffsetBasis;

            for (const std::byte byte :
            value.Bytes())
            {
                result ^=
                    static_cast<std::size_t>(
                        std::to_integer<unsigned int>(byte));

                result *= Prime;
            }

            return result;
        }
    };
}