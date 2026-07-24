#pragma once

#include <BootstrapForge/Domain/Models/Relationship.hpp>
#include <BootstrapForge/Domain/Models/Resource.hpp>

#include <compare>
#include <vector>

namespace BootstrapForge::Domain::Models
{
    class ArchitectureDelta
    {
    public:

        ArchitectureDelta() = default;

        ArchitectureDelta(
            std::vector<Resource> addedResources,
            std::vector<Resource> removedResources,
            std::vector<Resource> modifiedResources,
            std::vector<Relationship> addedRelationships,
            std::vector<Relationship> removedRelationships);

        [[nodiscard]]
        const std::vector<Resource>& AddedResources() const noexcept;

        [[nodiscard]]
        const std::vector<Resource>& RemovedResources() const noexcept;

        [[nodiscard]]
        const std::vector<Resource>& ModifiedResources() const noexcept;

        [[nodiscard]]
        const std::vector<Relationship>& AddedRelationships() const noexcept;

        [[nodiscard]]
        const std::vector<Relationship>& RemovedRelationships() const noexcept;

        auto operator<=>(const ArchitectureDelta&) const = default;

    private:

        std::vector<Resource> m_addedResources;
        std::vector<Resource> m_removedResources;
        std::vector<Resource> m_modifiedResources;

        std::vector<Relationship> m_addedRelationships;
        std::vector<Relationship> m_removedRelationships;
    };

    inline ArchitectureDelta::ArchitectureDelta(
        std::vector<Resource> addedResources,
        std::vector<Resource> removedResources,
        std::vector<Resource> modifiedResources,
        std::vector<Relationship> addedRelationships,
        std::vector<Relationship> removedRelationships)
        :
        m_addedResources(std::move(addedResources)),
        m_removedResources(std::move(removedResources)),
        m_modifiedResources(std::move(modifiedResources)),
        m_addedRelationships(std::move(addedRelationships)),
        m_removedRelationships(std::move(removedRelationships))
    {
    }

    inline const std::vector<Resource>&
        ArchitectureDelta::AddedResources() const noexcept
    {
        return m_addedResources;
    }

    inline const std::vector<Resource>&
        ArchitectureDelta::RemovedResources() const noexcept
    {
        return m_removedResources;
    }

    inline const std::vector<Resource>&
        ArchitectureDelta::ModifiedResources() const noexcept
    {
        return m_modifiedResources;
    }

    inline const std::vector<Relationship>&
        ArchitectureDelta::AddedRelationships() const noexcept
    {
        return m_addedRelationships;
    }

    inline const std::vector<Relationship>&
        ArchitectureDelta::RemovedRelationships() const noexcept
    {
        return m_removedRelationships;
    }

} // namespace BootstrapForge::Domain::Models