#pragma once

#include <BootstrapForge/Contracts/Execution/IExecutionBackend.hpp>

#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>
#include <BootstrapForge/Domain/Models/Resource.hpp>
#include <BootstrapForge/Domain/Models/ResourceKind.hpp>
#include <BootstrapForge/Domain/Models/Relationship.hpp>

#include <BootstrapForge/Domain/Operations/ExecutionPlan.hpp>
#include <BootstrapForge/Domain/Operations/Operation.hpp>
#include <BootstrapForge/Domain/Operations/PlannedOperation.hpp>
#include <BootstrapForge/Domain/Operations/RelationshipOperation.hpp>
#include <BootstrapForge/Domain/Operations/ScheduledExecutionPlan.hpp>

#include <BootstrapForge/Domain/Types/Hash.hpp>
#include <BootstrapForge/Domain/Types/Name.hpp>
#include <BootstrapForge/Domain/Types/Path.hpp>
#include <BootstrapForge/Domain/Types/RelationshipId.hpp>
#include <BootstrapForge/Domain/Types/ResourceId.hpp>

#include <BootstrapForge/Domain/Values/FileContent.hpp>
#include <BootstrapForge/Domain/Values/SymbolicLinkTarget.hpp>

#include <array>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace BootstrapForge::Tests
{
    using Contracts::Execution::IExecutionBackend;

    using Domain::Models::ArchitectureModel;
    using Domain::Models::Resource;
    using Domain::Models::ResourceKind;
    using Domain::Models::Relationship;

    using Domain::Operations::ExecutionPlan;
    using Domain::Operations::Operation;
    using Domain::Operations::OperationKind;
    using Domain::Operations::PlannedOperation;
    using Domain::Operations::RelationshipOperation;
    using Domain::Operations::ScheduledExecutionPlan;

    using Domain::Types::Hash;
    using Domain::Types::Name;
    using Domain::Types::Path;
    using Domain::Types::RelationshipId;
    using Domain::Types::ResourceId;

    using Domain::Values::FileContent;
    using Domain::Values::SymbolicLinkTarget;

    [[nodiscard]]
    inline Hash MakeHash(
        unsigned char value)
    {
        std::array<
            std::byte,
            Hash::Size>
            bytes{};

        bytes.back() =
            std::byte{ value };

        return Hash(bytes);
    }

    [[nodiscard]]
    inline Resource MakeResource(
        unsigned char id,
        std::string name,
        std::string path,
        ResourceKind kind)
    {
        return Resource(
            ResourceId(
                MakeHash(id)),
            Name(
                std::move(name)),
            Path(
                std::filesystem::path(
                    path)),
            kind);
    }

    [[nodiscard]]
    inline Resource MakeFile(
        unsigned char id,
        std::string path,
        std::string content)
    {
        return Resource(
            ResourceId(
                MakeHash(id)),
            Name(
                std::filesystem::path(path)
                .filename()
                .string()),
            Path(
                std::filesystem::path(path)),
            ResourceKind::File,
            FileContent(
                std::move(content)));
    }

    [[nodiscard]]
    inline Resource MakeSymbolicLink(
        unsigned char id,
        std::string path,
        std::string target)
    {
        return Resource(
            ResourceId(
                MakeHash(id)),
            Name(
                std::filesystem::path(path)
                .filename()
                .string()),
            Path(
                std::filesystem::path(path)),
            ResourceKind::SymbolicLink,
            std::nullopt,
            SymbolicLinkTarget(
                std::filesystem::path(
                    target)));
    }

    [[nodiscard]]
    inline Relationship MakeRelationship(
        unsigned char id,
        unsigned char source,
        unsigned char target)
    {
        return Relationship(
            RelationshipId(
                MakeHash(id)),
            ResourceId(
                MakeHash(source)),
            ResourceId(
                MakeHash(target)));
    }

    [[nodiscard]]
    inline ArchitectureModel MakeModel(
        std::vector<Resource> resources)
    {
        return ArchitectureModel(
            std::move(resources),
            {});
    }

    [[nodiscard]]
    inline ExecutionPlan MakeExecutionPlan(
        std::vector<PlannedOperation> operations)
    {
        return ExecutionPlan(
            std::move(operations));
    }

    [[nodiscard]]
    inline ScheduledExecutionPlan MakeScheduledExecutionPlan(
        std::vector<PlannedOperation> operations)
    {
        return ScheduledExecutionPlan(
            std::move(operations));
    }

    inline void Require(
        bool condition,
        const std::string& message)
    {
        if (!condition)
        {
            std::cerr
                << "[FAIL] "
                << message
                << '\n';

            std::exit(
                EXIT_FAILURE);
        }
    }

    [[nodiscard]]
    inline const Operation& RequireResourceOperation(
        const PlannedOperation& operation,
        const std::string& message)
    {
        Require(
            std::holds_alternative<Operation>(
                operation),
            message);

        return std::get<Operation>(
            operation);
    }

    [[nodiscard]]
    inline const RelationshipOperation&
        RequireRelationshipOperation(
            const PlannedOperation& operation,
            const std::string& message)
    {
        Require(
            std::holds_alternative<
            RelationshipOperation>(
                operation),
            message);

        return std::get<RelationshipOperation>(
            operation);
    }

    class RecordingExecutionBackend final
        : public IExecutionBackend
    {
    public:

        bool CreateDirectory(
            const Resource& resource) override
        {
            m_calls.push_back(
                "CreateDirectory:" +
                resource.Path().ToString());

            return CreateDirectoryResult;
        }

        bool CreateFile(
            const Resource& resource) override
        {
            m_calls.push_back(
                "CreateFile:" +
                resource.Path().ToString());

            return CreateFileResult;
        }

        bool UpdateFile(
            const Resource& resource) override
        {
            m_calls.push_back(
                "UpdateFile:" +
                resource.Path().ToString());

            return UpdateFileResult;
        }

        bool DeleteDirectory(
            const Resource& resource) override
        {
            m_calls.push_back(
                "DeleteDirectory:" +
                resource.Path().ToString());

            return DeleteDirectoryResult;
        }

        bool DeleteFile(
            const Resource& resource) override
        {
            m_calls.push_back(
                "DeleteFile:" +
                resource.Path().ToString());

            return DeleteFileResult;
        }

        bool CreateSymbolicLink(
            const Resource& resource) override
        {
            m_calls.push_back(
                "CreateSymbolicLink:" +
                resource.Path().ToString());

            return CreateSymbolicLinkResult;
        }

        bool DeleteSymbolicLink(
            const Resource& resource) override
        {
            m_calls.push_back(
                "DeleteSymbolicLink:" +
                resource.Path().ToString());

            return DeleteSymbolicLinkResult;
        }

        bool AddRelationship(
            const Relationship& relationship) override
        {
            m_calls.push_back(
                "AddRelationship:" +
                relationship.Id().Value().ToHex());

            return AddRelationshipResult;
        }

        bool RemoveRelationship(
            const Relationship& relationship) override
        {
            m_calls.push_back(
                "RemoveRelationship:" +
                relationship.Id().Value().ToHex());

            return RemoveRelationshipResult;
        }

        [[nodiscard]]
        const std::vector<std::string>&
            Calls() const noexcept
        {
            return m_calls;
        }

        bool CreateDirectoryResult = true;
        bool CreateFileResult = true;
        bool UpdateFileResult = true;
        bool DeleteDirectoryResult = true;
        bool DeleteFileResult = true;
        bool CreateSymbolicLinkResult = true;
        bool DeleteSymbolicLinkResult = true;
        bool AddRelationshipResult = true;
        bool RemoveRelationshipResult = true;

    private:

        std::vector<std::string> m_calls;
    };

} // namespace BootstrapForge::Tests