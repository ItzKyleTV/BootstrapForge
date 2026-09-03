#include <BootstrapForge/Application/Identity/ResourceFactory.hpp>

#include <BootstrapForge/Domain/Models/Resource.hpp>
#include <BootstrapForge/Domain/Models/ResourceKind.hpp>

#include <BootstrapForge/Domain/Types/Hash.hpp>
#include <BootstrapForge/Domain/Types/Name.hpp>
#include <BootstrapForge/Domain/Types/Path.hpp>
#include <BootstrapForge/Domain/Types/ResourceId.hpp>

#include <BootstrapForge/Domain/Values/FileContent.hpp>
#include <BootstrapForge/Domain/Values/SymbolicLinkTarget.hpp>

#include <span>
#include <string>
#include <utility>

namespace BootstrapForge::Application::Identity
{
    using BootstrapForge::Domain::Models::Resource;
    using BootstrapForge::Domain::Models::ResourceKind;

    using BootstrapForge::Domain::Types::Hash;
    using BootstrapForge::Domain::Types::Name;
    using BootstrapForge::Domain::Types::Path;
    using BootstrapForge::Domain::Types::ResourceId;

    using BootstrapForge::Domain::Values::FileContent;
    using BootstrapForge::Domain::Values::SymbolicLinkTarget;

    ResourceFactory::ResourceFactory(
        BootstrapForge::Contracts::Services::Cryptography::IHasher& hasher)
        noexcept
        :
        m_hasher(hasher)
    {
    }

    Resource ResourceFactory::Create(
        const std::filesystem::path& path,
        ResourceKind kind,
        std::optional<FileContent> content,
        std::optional<SymbolicLinkTarget> symbolicLinkTarget) const
    {
        Resource resource(
            ResourceId(Hash{}),
            Name(path.filename().string()),
            Path(path),
            kind,
            std::move(content),
            std::move(symbolicLinkTarget));

        const std::string identity =
            resource.IdentityCanonical();

        const auto bytes =
            std::as_bytes(
                std::span(
                    identity.data(),
                    identity.size()));

        const Hash hash =
            m_hasher.Compute(bytes);

        return Resource(
            ResourceId(hash),
            Name(path.filename().string()),
            Path(path),
            kind,
            resource.Content(),
            resource.SymbolicLinkTarget());
    }

    Resource ResourceFactory::Create(
        std::string_view manifestId,
        const std::filesystem::path& path,
        ResourceKind kind,
        std::optional<FileContent> content,
        std::optional<SymbolicLinkTarget> symbolicLinkTarget) const
    {
        //
        // Manifest IDs are the stable logical identity.
        //
        // Path, content, and symbolic-link target are deliberately
        // excluded from this hash.
        //
        // Therefore:
        //
        //     id = "hello-file"
        //
        // remains the same ResourceId even if the resource state
        // changes.
        //
        const auto bytes =
            std::as_bytes(
                std::span(
                    manifestId.data(),
                    manifestId.size()));

        const Hash hash =
            m_hasher.Compute(bytes);

        return Resource(
            ResourceId(hash),
            Name(path.filename().string()),
            Path(path),
            kind,
            std::move(content),
            std::move(symbolicLinkTarget));
    }

} // namespace BootstrapForge::Application::Identity