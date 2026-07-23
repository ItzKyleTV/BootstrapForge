#include <BootstrapForge/Domain/Types/Hash.hpp>

#include <array>
#include <string>

namespace BootstrapForge::Domain::Types
{
    namespace
    {
        constexpr std::array<char, 16> HexDigits{
            '0', '1', '2', '3',
            '4', '5', '6', '7',
            '8', '9', 'a', 'b',
            'c', 'd', 'e', 'f'
        };
    }

    std::string Hash::ToHex() const
    {
        std::string result;
        result.reserve(Size * 2);

        for (const auto byte : m_bytes)
        {
            const auto value = std::to_integer<std::uint8_t>(byte);

            result.push_back(HexDigits[(value >> 4) & 0x0F]);
            result.push_back(HexDigits[value & 0x0F]);
        }

        return result;
    }

} // namespace BootstrapForge::Domain::Types