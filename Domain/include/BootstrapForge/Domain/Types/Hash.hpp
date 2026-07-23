#pragma once

#include <array>
#include <cstddef>
#include <compare>
#include <span>
#include <string>

namespace BootstrapForge::Domain::Types
{
    class Hash
    {
    public:

        static constexpr std::size_t Size = 32;

        constexpr Hash() noexcept = default;

        constexpr explicit Hash(
            const std::array<std::byte, Size>& bytes) noexcept;

        [[nodiscard]]
        constexpr bool IsEmpty() const noexcept;

        [[nodiscard]]
        std::string ToHex() const;

        [[nodiscard]]
        constexpr std::span<const std::byte, Size> Bytes() const noexcept;

        auto operator<=>(const Hash&) const noexcept = default;

    private:

        std::array<std::byte, Size> m_bytes{};
    };

    constexpr Hash::Hash(
        const std::array<std::byte, Size>& bytes) noexcept
        : m_bytes(bytes)
    {
    }

    constexpr bool Hash::IsEmpty() const noexcept
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

    constexpr std::span<const std::byte, Hash::Size> Hash::Bytes() const noexcept
    {
        return std::span<const std::byte, Size>(m_bytes);
    }

} // namespace BootstrapForge::Domain::Types