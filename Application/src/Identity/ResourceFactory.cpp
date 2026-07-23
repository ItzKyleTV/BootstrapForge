#include <BootstrapForge/Application/Identity/ResourceFactory.hpp>

#include <BootstrapForge/Domain/Types/Hash.hpp>
#include <BootstrapForge/Domain/Types/Name.hpp>
#include <BootstrapForge/Domain/Types/Path.hpp>
#include <BootstrapForge/Domain/Types/ResourceId.hpp>

#include <span>
#include <string>

namespace BootstrapForge::Application::Identity
{
    using BootstrapForge::Domain::Models::Resource;
    using BootstrapForge::Domain::Types::Hash;
    using BootstrapForge::Domain::Types::Name;
    using BootstrapForge::Domain::Types::Path;
    using BootstrapForge::Domain::Types::ResourceId;

    ResourceFactory::ResourceFactory(
        BootstrapForge::Contracts::Services::Cryptography::IHasher& hasher) noexcept
        :
        m_hasher(hasher)
    {
    }

    Resource ResourceFactory::Create(
        const std::filesystem::path& path) const
    {
        const std::string canonicalPath = path.generic_string();

        const auto bytes =
            std::as_bytes(
                std::span(
                    canonicalPath.data(),
                    canonicalPath.size()));

        const Hash hash = m_hasher.Compute(bytes);

        return Resource(
            ResourceId(hash),
            Name(path.filename().string()),
            Path(path));
    }
}