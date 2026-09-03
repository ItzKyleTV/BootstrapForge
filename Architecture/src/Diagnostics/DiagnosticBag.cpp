#include <BootstrapForge/Architecture/Diagnostics/DiagnosticBag.hpp>

#include <utility>

namespace BootstrapForge::Architecture::Diagnostics
{
    void DiagnosticBag::Add(
        Diagnostic diagnostic)
    {
        m_diagnostics.emplace_back(
            std::move(diagnostic));
    }

    const std::vector<Diagnostic>&
        DiagnosticBag::Diagnostics() const noexcept
    {
        return m_diagnostics;
    }

    std::size_t DiagnosticBag::Size() const noexcept
    {
        return m_diagnostics.size();
    }

    bool DiagnosticBag::Empty() const noexcept
    {
        return m_diagnostics.empty();
    }

    bool DiagnosticBag::HasErrors() const noexcept
    {
        for (const Diagnostic& diagnostic :
            m_diagnostics)
        {
            if (diagnostic.IsError() ||
                diagnostic.IsFatal())
            {
                return true;
            }
        }

        return false;
    }

    bool DiagnosticBag::HasFatalErrors() const noexcept
    {
        for (const Diagnostic& diagnostic :
            m_diagnostics)
        {
            if (diagnostic.IsFatal())
            {
                return true;
            }
        }

        return false;
    }

    void DiagnosticBag::Clear() noexcept
    {
        m_diagnostics.clear();
    }

} // namespace BootstrapForge::Architecture::Diagnostics