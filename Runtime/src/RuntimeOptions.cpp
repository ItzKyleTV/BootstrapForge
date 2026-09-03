#include <BootstrapForge/Runtime/RuntimeOptions.hpp>

namespace BootstrapForge::Runtime
{
    RuntimeOptions::RuntimeOptions(
        int argc,
        char* argv[])
    {
        if (argc > 1)
        {
            m_root = argv[1];
        }
        else
        {
            m_root = std::filesystem::current_path();
        }
    }

    const std::filesystem::path&
        RuntimeOptions::Root() const noexcept
    {
        return m_root;
    }

} // namespace BootstrapForge::Runtime