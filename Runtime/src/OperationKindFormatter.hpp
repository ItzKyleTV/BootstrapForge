#pragma once

#include <BootstrapForge/Domain/Operations/OperationKind.hpp>

#include <string_view>

[[nodiscard]]
constexpr std::string_view ToString(
    BootstrapForge::Domain::Operations::OperationKind kind) noexcept
{
    using BootstrapForge::Domain::Operations::OperationKind;

    switch (kind)
    {
    case OperationKind::Create:
        return "Create";

    case OperationKind::Delete:
        return "Delete";

    case OperationKind::Update:
        return "Update";

    case OperationKind::Move:
        return "Move";

    case OperationKind::Rename:
        return "Rename";
    }

    return "Unknown";
}