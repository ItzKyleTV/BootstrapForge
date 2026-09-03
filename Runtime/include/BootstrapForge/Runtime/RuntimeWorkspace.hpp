#pragma once

#include <filesystem>

namespace BootstrapForge::Runtime
{
    class RuntimeWorkspace final
    {
    public:

        RuntimeWorkspace();

        [[nodiscard]]
        const std::filesystem::path& Root() const noexcept;

        [[nodiscard]]
        const std::filesystem::path& Current() const noexcept;

        [[nodiscard]]
        const std::filesystem::path& Desired() const noexcept;

        [[nodiscard]]
        const std::filesystem::path& Output() const noexcept;

        void EnsureExists() const;

    private:

        std::filesystem::path m_root;
        std::filesystem::path m_current;
        std::filesystem::path m_desired;
        std::filesystem::path m_output;
    };
}