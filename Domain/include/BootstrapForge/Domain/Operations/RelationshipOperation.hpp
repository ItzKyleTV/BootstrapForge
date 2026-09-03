#pragma once

#include <BootstrapForge/Domain/Models/Relationship.hpp>
#include <BootstrapForge/Domain/Operations/OperationKind.hpp>

#include <compare>
#include <utility>

namespace BootstrapForge::Domain::Operations
{
    class RelationshipOperation
    {
    public:

        RelationshipOperation() = default;

        constexpr RelationshipOperation(
            OperationKind kind,
            Models::Relationship relationship)
            :
            m_kind(kind),
            m_relationship(std::move(relationship))
        {
        }

        [[nodiscard]]
        constexpr OperationKind
            Kind() const noexcept
        {
            return m_kind;
        }

        [[nodiscard]]
        constexpr const Models::Relationship&
            Relationship() const noexcept
        {
            return m_relationship;
        }

        auto operator<=>(
            const RelationshipOperation&) const = default;

        bool operator<(
            const RelationshipOperation& other) const noexcept
        {
            return std::is_lt(
                *this <=> other);
        }

    private:

        OperationKind m_kind =
            OperationKind::AddRelationship;

        Models::Relationship m_relationship;
    };

} // namespace BootstrapForge::Domain::Operations