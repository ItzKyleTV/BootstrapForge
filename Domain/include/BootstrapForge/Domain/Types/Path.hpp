#pragma once

#include <compare>
#include <filesystem>
#include <string>

namespace BootstrapForge::Domain::Types
{
    class Path
    {
    public:

        Path() = default;

        explicit Path(std::filesystem::path value);

        [[nodiscard]]
        bool IsEmpty() const noexcept;

        [[nodiscard]]
        const std::filesystem::path& Value() const noexcept;

        [[nodiscard]]
        std::string ToString() const;

        auto operator<=>(const Path&) const noexcept = default;

    private:

        std::filesystem::path m_value;
    };

    inline Path::Path(std::filesystem::path value)
        : m_value(std::move(value))
    {
    }

    inline bool Path::IsEmpty() const noexcept
    {
        return m_value.empty();
    }

    inline const std::filesystem::path& Path::Value() const noexcept
    {
        return m_value;
    }

    inline std::string Path::ToString() const
    {
        return m_value.generic_string();
    }

} // namespace BootstrapForge::Domain::Types