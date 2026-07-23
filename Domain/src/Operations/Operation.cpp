#include <BootstrapForge/Domain/Operations/Operation.hpp>

namespace BootstrapForge::Domain::Operations
{

    Operation::Operation(
        OperationKind kind,
        Types::ResourceId resourceId,
        Types::Name name)
        :
        m_kind(kind),
        m_resourceId(std::move(resourceId)),
        m_name(std::move(name))
    {
    }

    OperationKind Operation::Kind() const noexcept
    {
        return m_kind;
    }

    const Types::ResourceId& Operation::ResourceId() const noexcept
    {
        return m_resourceId;
    }

    const Types::Name& Operation::Name() const noexcept
    {
        return m_name;
    }

}