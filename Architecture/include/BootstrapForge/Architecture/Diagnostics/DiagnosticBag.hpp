#pragma once

#include <BootstrapForge/Architecture/Diagnostics/Diagnostic.hpp>

#include <cstddef>
#include <vector>

namespace BootstrapForge::Architecture::Diagnostics
{
    class DiagnosticBag final
    {
    public:

        DiagnosticBag() = default;

        void Add(
            Diagnostic diagnostic);

        [[nodiscard]]
        const std::vector<Diagnostic>& Diagnostics() const noexcept;

        [[nodiscard]]
        std::size_t Size() const noexcept;

        [[nodiscard]]
        bool Empty() const noexcept;

        [[nodiscard]]
        bool HasErrors() const noexcept;

        [[nodiscard]]
        bool HasFatalErrors() const noexcept;

        void Clear() noexcept;

    private:

        std::vector<Diagnostic> m_diagnostics;
    };

} // namespace BootstrapForge::Architecture::Diagnostics