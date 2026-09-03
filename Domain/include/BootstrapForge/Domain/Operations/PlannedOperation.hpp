#pragma once

#include <BootstrapForge/Domain/Operations/Operation.hpp>
#include <BootstrapForge/Domain/Operations/RelationshipOperation.hpp>

#include <compare>
#include <variant>

namespace BootstrapForge::Domain::Operations
{
    using PlannedOperation =
        std::variant<
        Operation,
        RelationshipOperation>;

} // namespace BootstrapForge::Domain::Operations