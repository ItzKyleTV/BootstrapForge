#pragma once

namespace BootstrapForge::Domain::Operations
{
    enum class OperationKind
    {
        Create,
        Delete,
        Update,
        Move,
        Rename,

        AddRelationship,
        RemoveRelationship
    };

} // namespace BootstrapForge::Domain::Operations