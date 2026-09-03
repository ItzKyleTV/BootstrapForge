#include <BootstrapForge/Domain/Types/Hash.hpp>

#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace BootstrapForge::Domain::Types
{
    namespace
    {
        [[nodiscard]]
        int HexValue(
            const char character) noexcept
        {
            if (character >= '0' &&
                character <= '9')
            {
                return character - '0';
            }

            if (character >= 'a' &&
                character <= 'f')
            {
                return character - 'a' + 10;
            }

            if (character >= 'A' &&
                character <= 'F')
            {
                return character - 'A' + 10;
            }

            return -1;
        }
    }

    std::string Hash::ToHex() const
    {
        static constexpr char Digits[] =
            "0123456789abcdef";

        std::string result;

        result.reserve(
            Size * 2);

        for (const std::byte byte :
        m_bytes)
        {
            const auto value =
                std::to_integer<unsigned int>(
                    byte);

            result +=
                Digits[(value >> 4) & 0x0F];

            result +=
                Digits[value & 0x0F];
        }

        return result;
    }

    std::optional<Hash>
        Hash::FromHex(
            std::string_view value)
    {
        if (value.size() != Size * 2)
        {
            return std::nullopt;
        }

        std::array<std::byte, Size> bytes{};

        for (std::size_t index = 0;
            index < Size;
            ++index)
        {
            const int high =
                HexValue(
                    value[index * 2]);

            const int low =
                HexValue(
                    value[index * 2 + 1]);

            if (high < 0 || low < 0)
            {
                return std::nullopt;
            }

            const unsigned int combined =
                static_cast<unsigned int>(
                    (high << 4) | low);

            bytes[index] =
                static_cast<std::byte>(
                    combined);
        }

        return Hash(bytes);
    }

} // namespace BootstrapForge::Domain::Types