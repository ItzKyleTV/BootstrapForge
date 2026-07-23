#pragma once

#include <BootstrapForge/Domain/Types/RelationshipId.hpp>
#include <BootstrapForge/Domain/Types/ResourceId.hpp>

#include <compare>

namespace BootstrapForge::Domain::Models
{
    class Relationship
    {
    public:

        constexpr Relationship() noexcept = default;

        constexpr Relationship(
            Types::RelationshipId id,
            Types::ResourceId source,
            Types::ResourceId target) noexcept;

        [[nodiscard]]
        constexpr const Types::RelationshipId& Id() const noexcept;

        [[nodiscard]]
        constexpr const Types::ResourceId& Source() const noexcept;

        [[nodiscard]]
        constexpr const Types::ResourceId& Target() const noexcept;

        auto operator<=>(const Relationship&) const noexcept = default;

    private:

        Types::RelationshipId m_id{};
        Types::ResourceId m_source{};
        Types::ResourceId m_target{};
    };

    constexpr Relationship::Relationship(
        Types::RelationshipId id,
        Types::ResourceId source,
        Types::ResourceId target) noexcept
        : m_id(std::move(id))
        , m_source(std::move(source))
        , m_target(std::move(target))
    {
    }

    constexpr const Types::RelationshipId& Relationship::Id() const noexcept
    {
        return m_id;
    }

    constexpr const Types::ResourceId& Relationship::Source() const noexcept
    {
        return m_source;
    }

    constexpr const Types::ResourceId& Relationship::Target() const noexcept
    {
        return m_target;
    }

} // namespace BootstrapForge::Domain::Models