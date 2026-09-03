#pragma once

#include <BootstrapForge/Contracts/Services/Cryptography/IHasher.hpp>

#include <BootstrapForge/Domain/Models/Relationship.hpp>
#include <BootstrapForge/Domain/Types/ResourceId.hpp>

#include <string_view>

namespace BootstrapForge::Application::Identity
{
    class RelationshipFactory final
    {
    public:

        explicit RelationshipFactory(
            BootstrapForge::Contracts::Services::Cryptography::IHasher& hasher)
            noexcept;

        [[nodiscard]]
        BootstrapForge::Domain::Models::Relationship Create(
            std::string_view manifestId,
            const BootstrapForge::Domain::Types::ResourceId& source,
            const BootstrapForge::Domain::Types::ResourceId& target) const;

    private:

        BootstrapForge::Contracts::Services::Cryptography::IHasher&
            m_hasher;
    };

} // namespace BootstrapForge::Application::Identity