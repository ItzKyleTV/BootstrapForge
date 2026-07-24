#pragma once

#include <BootstrapForge/Domain/Models/Resource.hpp>

namespace BootstrapForge::Contracts::Execution
{
    class IExecutionBackend
    {
    public:

        virtual ~IExecutionBackend() = default;

        [[nodiscard]]
        virtual bool Create(
            const Domain::Models::Resource& resource) = 0;

        [[nodiscard]]
        virtual bool Update(
            const Domain::Models::Resource& resource) = 0;

        [[nodiscard]]
        virtual bool Delete(
            const Domain::Models::Resource& resource) = 0;
    };
}