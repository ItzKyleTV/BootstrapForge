#pragma once

#include <compare>
#include <filesystem>
#include <string>
#include <utility>
#pragma once

#include <compare>
#include <filesystem>
#include <string>
#include <utility>

namespace BootstrapForge::Domain::Types
{
    class Path
    {
    public:

        Path() = default;

        explicit Path(std::filesystem::path value)
            : m_value(std::move(value))
        {
        }

        [[nodiscard]]
        bool IsEmpty() const noexcept
        {
            return m_value.empty();
        }

        [[nodiscard]]
        const std::filesystem::path& Value() const noexcept
        {
            return m_value;
        }

        [[nodiscard]]
        std::string ToString() const
        {
            return m_value.generic_string();
        }

        auto operator<=>(const Path&) const = default;

    private:

        std::filesystem::path m_value;
    };

} // namespace BootstrapForge::Domain::Types