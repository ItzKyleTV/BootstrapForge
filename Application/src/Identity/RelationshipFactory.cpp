#include <BootstrapForge/Application/Identity/RelationshipFactory.hpp>

#include <BootstrapForge/Domain/Types/Hash.hpp>
#include <BootstrapForge/Domain/Types/RelationshipId.hpp>

#include <span>
#include <string_view>

namespace BootstrapForge::Application::Identity
{
    RelationshipFactory::RelationshipFactory(
        BootstrapForge::Contracts::Services::Cryptography::IHasher& hasher)
        noexcept
        :
        m_hasher(hasher)
    {
    }

    BootstrapForge::Domain::Models::Relationship
        RelationshipFactory::Create(
            std::string_view manifestId,
            const BootstrapForge::Domain::Types::ResourceId& source,
            const BootstrapForge::Domain::Types::ResourceId& target) const
    {
        const auto bytes =
            std::as_bytes(
                std::span(
                    manifestId.data(),
                    manifestId.size()));

        const BootstrapForge::Domain::Types::Hash hash =
            m_hasher.Compute(bytes);

        return BootstrapForge::Domain::Models::Relationship(
            BootstrapForge::Domain::Types::RelationshipId(hash),
            source,
            target);
    }

} // namespace BootstrapForge::Application::Identity