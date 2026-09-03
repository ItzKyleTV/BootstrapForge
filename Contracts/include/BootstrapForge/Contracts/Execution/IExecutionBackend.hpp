#pragma once

#include <BootstrapForge/Domain/Models/Resource.hpp>
#include <BootstrapForge/Domain/Models/Relationship.hpp>

namespace BootstrapForge::Contracts::Execution
{
    class IExecutionBackend
    {
    public:

        virtual ~IExecutionBackend() = default;

        [[nodiscard]]
        virtual bool CreateDirectory(
            const Domain::Models::Resource& resource) = 0;

        [[nodiscard]]
        virtual bool CreateFile(
            const Domain::Models::Resource& resource) = 0;

        [[nodiscard]]
        virtual bool CreateSymbolicLink(
            const Domain::Models::Resource& resource) = 0;

        [[nodiscard]]
        virtual bool UpdateFile(
            const Domain::Models::Resource& resource) = 0;

        [[nodiscard]]
        virtual bool DeleteDirectory(
            const Domain::Models::Resource& resource) = 0;

        [[nodiscard]]
        virtual bool DeleteFile(
            const Domain::Models::Resource& resource) = 0;

        [[nodiscard]]
        virtual bool DeleteSymbolicLink(
            const Domain::Models::Resource& resource) = 0;
        [[nodiscard]]
        virtual bool AddRelationship(
            const Domain::Models::Relationship& relationship) = 0;

        [[nodiscard]]
        virtual bool RemoveRelationship(
            const Domain::Models::Relationship& relationship) = 0;
    };
}