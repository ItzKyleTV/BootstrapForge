#pragma once

#include <BootstrapForge/Domain/Models/Resource.hpp>
#include <BootstrapForge/Domain/Operations/OperationKind.hpp>

#include <compare>
#include <utility>

namespace BootstrapForge::Domain::Operations
{
    class Operation
    {
    public:

        constexpr Operation() = default;

        constexpr Operation(
            OperationKind kind,
            Models::Resource resource)
            :
            m_kind(kind),
            m_resource(std::move(resource))
        {
        }

        [[nodiscard]]
        constexpr OperationKind
            Kind() const noexcept
        {
            return m_kind;
        }

        [[nodiscard]]
        constexpr const Models::Resource&
            Resource() const noexcept
        {
            return m_resource;
        }

        auto operator<=>(
            const Operation&) const noexcept = default;

        bool operator<(
            const Operation& other) const noexcept;

    private:

        OperationKind m_kind =
            OperationKind::Create;

        Models::Resource m_resource;
    };

    inline bool Operation::operator<(
        const Operation& other) const noexcept
    {
        return std::is_lt(
            *this <=> other);
    }

} // namespace BootstrapForge::Domain::Operations