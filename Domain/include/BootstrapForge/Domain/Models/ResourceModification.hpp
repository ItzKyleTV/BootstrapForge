#pragma once

#include <BootstrapForge/Domain/Models/Resource.hpp>

#include <compare>
#include <utility>

namespace BootstrapForge::Domain::Models
{
    class ResourceModification
    {
    public:

        ResourceModification() = default;

        ResourceModification(
            Resource current,
            Resource desired)
            :
            m_current(std::move(current)),
            m_desired(std::move(desired))
        {
        }

        [[nodiscard]]
        const Resource& Current() const noexcept
        {
            return m_current;
        }

        [[nodiscard]]
        const Resource& Desired() const noexcept
        {
            return m_desired;
        }

        auto operator<=>(
            const ResourceModification&) const = default;

    private:

        Resource m_current;
        Resource m_desired;
    };

} // namespace BootstrapForge::Domain::Models