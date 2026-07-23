#pragma once

#include <BootstrapForge/Domain/Operations/OperationKind.hpp>
#include <BootstrapForge/Domain/Types/Name.hpp>
#include <BootstrapForge/Domain/Types/ResourceId.hpp>

namespace BootstrapForge::Domain::Operations
{

    class Operation
    {
    public:

        Operation(
            OperationKind kind,
            Types::ResourceId resourceId,
            Types::Name name);

        [[nodiscard]]
        OperationKind Kind() const noexcept;

        [[nodiscard]]
        const Types::ResourceId& ResourceId() const noexcept;

        [[nodiscard]]
        const Types::Name& Name() const noexcept;

    private:

        OperationKind m_kind;

        Types::ResourceId m_resourceId;

        Types::Name m_name;
    };

}