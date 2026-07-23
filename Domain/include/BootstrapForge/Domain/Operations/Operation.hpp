#pragma once

#include <BootstrapForge/Domain/Types/OperationId.hpp>

#include <compare>

namespace BootstrapForge::Domain::Operations
{
    class Operation
    {
    public:

        constexpr Operation() noexcept = default;

        constexpr explicit Operation(
            Types::OperationId id) noexcept;

        [[nodiscard]]
        constexpr const Types::OperationId& Id() const noexcept;

        auto operator<=>(const Operation&) const noexcept = default;

    private:

        Types::OperationId m_id{};
    };

    constexpr Operation::Operation(
        Types::OperationId id) noexcept
        : m_id(std::move(id))
    {
    }

    constexpr const Types::OperationId& Operation::Id() const noexcept
    {
        return m_id;
    }

} // namespace BootstrapForge::Domain::Operations