#pragma once

#include <BootstrapForge/Contracts/Execution/IExecutionBackend.hpp>

#include <BootstrapForge/Domain/Models/Relationship.hpp>

#include <filesystem>

namespace BootstrapForge::Infrastructure::Execution
{
    class PhysicalExecutionBackend final
        : public Contracts::Execution::IExecutionBackend
    {
    public:

        explicit PhysicalExecutionBackend(
            std::filesystem::path workspaceRoot);

        bool CreateDirectory(
            const Domain::Models::Resource& resource) override;

        bool CreateFile(
            const Domain::Models::Resource& resource) override;

        bool UpdateFile(
            const Domain::Models::Resource& resource) override;

        bool DeleteDirectory(
            const Domain::Models::Resource& resource) override;

        bool DeleteFile(
            const Domain::Models::Resource& resource) override;

        bool CreateSymbolicLink(
            const Domain::Models::Resource& resource) override;

        bool DeleteSymbolicLink(
            const Domain::Models::Resource& resource) override;

        bool AddRelationship(
            const Domain::Models::Relationship& relationship) override;

        bool RemoveRelationship(
            const Domain::Models::Relationship& relationship) override;

    private:

        [[nodiscard]]
        std::filesystem::path Resolve(
            const Domain::Models::Resource& resource) const;

        [[nodiscard]]
        std::filesystem::path RelationshipStore() const;

        std::filesystem::path m_workspaceRoot;
    };

} // namespace BootstrapForge::Infrastructure::Execution