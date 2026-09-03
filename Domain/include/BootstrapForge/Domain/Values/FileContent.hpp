#pragma once

#include <compare>
#include <string>
#include <utility>

namespace BootstrapForge::Domain::Values
{
    class FileContent final
    {
    public:

        FileContent() = default;

        explicit FileContent(
            std::string value)
            :
            m_value(std::move(value))
        {
        }

        [[nodiscard]]
        const std::string& Value() const noexcept
        {
            return m_value;
        }

        [[nodiscard]]
        bool Empty() const noexcept
        {
            return m_value.empty();
        }

        friend bool operator==(
            const FileContent&,
            const FileContent&) = default;

        auto operator<=>(
            const FileContent&) const noexcept = default;

    private:

        std::string m_value;
    };

} // namespace BootstrapForge::Domain::Values