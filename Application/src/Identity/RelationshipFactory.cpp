#include <BootstrapForge/Application/Identity/RelationshipFactory.hpp>

#include <BootstrapForge/Domain/Types/Hash.hpp>
#include <BootstrapForge/Domain/Types/RelationshipId.hpp>

#include <span>
#include <string>

namespace BootstrapForge::Application::Identity
{
    using BootstrapForge::Domain::Models::Relationship;
    using BootstrapForge::Domain::Models::Resource;
    using BootstrapForge::Domain::Types::Hash;
    using BootstrapForge::Domain::Types::RelationshipId;

    RelationshipFactory::RelationshipFactory(
        BootstrapForge::Contracts::Services::Cryptography::IHasher& hasher) noexcept
        :
        m_hasher(hasher)
    {
    }

    Relationship RelationshipFactory::Create(
        const Resource& source,
        const Resource& target) const
    {
        const std::string canonical =
            "source=" + source.Id().ToString() +
            "\ntarget=" + target.Id().ToString();

        const auto bytes =
            std::as_bytes(
                std::span(
                    canonical.data(),
                    canonical.size()));

        const Hash hash = m_hasher.Compute(bytes);

        return Relationship(
            RelationshipId(hash),
            source.Id(),
            target.Id());
    }

} // namespace BootstrapForge::Application::Identity