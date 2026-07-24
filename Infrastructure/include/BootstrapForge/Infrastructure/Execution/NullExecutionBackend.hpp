#pragma once

#include <BootstrapForge/Contracts/Execution/IExecutionBackend.hpp>

#include <BootstrapForge/Domain/Models/Resource.hpp>

#include <vector>

namespace BootstrapForge::Infrastructure::Execution
{
    class NullExecutionBackend final
        : public Contracts::Execution::IExecutionBackend
    {
    public:

        [[nodiscard]]
        bool Create(
            const Domain::Models::Resource& resource) override;

        [[nodiscard]]
        bool Update(
            const Domain::Models::Resource& resource) override;

        [[nodiscard]]
        bool Delete(
            const Domain::Models::Resource& resource) override;

        [[nodiscard]]
        const std::vector<Domain::Models::Resource>& Created() const noexcept;

        [[nodiscard]]
        const std::vector<Domain::Models::Resource>& Updated() const noexcept;

        [[nodiscard]]
        const std::vector<Domain::Models::Resource>& Deleted() const noexcept;

    private:

        std::vector<Domain::Models::Resource> m_created;
        std::vector<Domain::Models::Resource> m_updated;
        std::vector<Domain::Models::Resource> m_deleted;
    };
}