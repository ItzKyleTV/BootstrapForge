#pragma once

namespace BootstrapForge::Domain::Operations
{

    enum class OperationKind
    {
        Create,
        Delete,
        Update,
        Move,
        Rename
    };

} // namespace BootstrapForge::Domain::Operations