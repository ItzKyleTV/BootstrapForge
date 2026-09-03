#pragma once

#include <filesystem>

namespace BootstrapForge::Runtime
{
    class RuntimeOptions final
    {
    public:

        RuntimeOptions(
            int argc,
            char* argv[]);

        [[nodiscard]]
        const std::filesystem::path& Root() const noexcept;

    private:

        std::filesystem::path m_root;
    };
}