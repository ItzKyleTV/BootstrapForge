#include <BootstrapForge/Runtime/RuntimeWorkspace.hpp>

#include <filesystem>
#include <stdexcept>

namespace BootstrapForge::Runtime
{
    namespace
    {
        [[nodiscard]]
        std::filesystem::path FindRepositoryRoot()
        {
            std::filesystem::path current =
                std::filesystem::current_path();

            while (!current.empty())
            {
                if (std::filesystem::exists(current / "CMakeLists.txt") &&
                    std::filesystem::exists(current / "Application") &&
                    std::filesystem::exists(current / "Infrastructure") &&
                    std::filesystem::exists(current / "Runtime"))
                {
                    return current;
                }

                const auto parent = current.parent_path();

                if (parent == current)
                {
                    break;
                }

                current = parent;
            }

            throw std::runtime_error(
                "Unable to locate the BootstrapForge repository root.");
        }
    }

    RuntimeWorkspace::RuntimeWorkspace()
        :
        m_root(FindRepositoryRoot() / "Workspace"),
        m_current(m_root / "current"),
        m_desired(m_root / "desired"),
        m_output(m_root / "output")
    {
    }

    const std::filesystem::path&
        RuntimeWorkspace::Root() const noexcept
    {
        return m_root;
    }

    const std::filesystem::path&
        RuntimeWorkspace::Current() const noexcept
    {
        return m_current;
    }

    const std::filesystem::path&
        RuntimeWorkspace::Desired() const noexcept
    {
        return m_desired;
    }

    const std::filesystem::path&
        RuntimeWorkspace::Output() const noexcept
    {
        return m_output;
    }

    void RuntimeWorkspace::EnsureExists() const
    {
        std::filesystem::create_directories(m_current);
        std::filesystem::create_directories(m_desired);
        std::filesystem::create_directories(m_output);
    }

} // namespace BootstrapForge::Runtime