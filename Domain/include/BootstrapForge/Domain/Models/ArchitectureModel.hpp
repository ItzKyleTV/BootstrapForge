#pragma once

#include <BootstrapForge/Domain/Models/Relationship.hpp>
#include <BootstrapForge/Domain/Models/Resource.hpp>

#include <compare>
#include <vector>

namespace BootstrapForge::Domain::Models
{
    class ArchitectureModel
    {
    public:

        ArchitectureModel() = default;

        ArchitectureModel(
            std::vector<Resource> resources,
            std::vector<Relationship> relationships);

        [[nodiscard]]
        const std::vector<Resource>& Resources() const noexcept;

        [[nodiscard]]
        const std::vector<Relationship>& Relationships() const noexcept;

        auto operator<=>(const ArchitectureModel&) const = default;

    private:

        std::vector<Resource> m_resources;
        std::vector<Relationship> m_relationships;
    };

    inline ArchitectureModel::ArchitectureModel(
        std::vector<Resource> resources,
        std::vector<Relationship> relationships)
        : m_resources(std::move(resources))
        , m_relationships(std::move(relationships))
    {
    }

    inline const std::vector<Resource>& ArchitectureModel::Resources() const noexcept
    {
        return m_resources;
    }

    inline const std::vector<Relationship>& ArchitectureModel::Relationships() const noexcept
    {
        return m_relationships;
    }

} // namespace BootstrapForge::Domain::Models