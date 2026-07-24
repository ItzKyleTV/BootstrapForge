#pragma once

#include <BootstrapForge/Domain/Models/Resource.hpp>
#include <BootstrapForge/Domain/Operations/OperationKind.hpp>

namespace BootstrapForge::Domain::Operations
{
    class Operation
    {
    public:

        constexpr Operation(
            OperationKind kind,
            Models::Resource resource)
            :
            m_kind(kind),
            m_resource(std::move(resource))
        {
        }

        [[nodiscard]]
        constexpr OperationKind Kind() const noexcept
        {
            return m_kind;
        }

        [[nodiscard]]
        constexpr const Models::Resource& Resource() const noexcept
        {
            return m_resource;
        }

    private:

        OperationKind m_kind;

        Models::Resource m_resource;
    };
}