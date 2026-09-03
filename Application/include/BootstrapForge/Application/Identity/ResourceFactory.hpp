#pragma once

#include <BootstrapForge/Contracts/Services/Cryptography/IHasher.hpp>

#include <BootstrapForge/Domain/Models/Resource.hpp>
#include <BootstrapForge/Domain/Models/ResourceKind.hpp>

#include <BootstrapForge/Domain/Values/FileContent.hpp>
#include <BootstrapForge/Domain/Values/SymbolicLinkTarget.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace BootstrapForge::Application::Identity
{
    class ResourceFactory final
    {
    public:

        explicit ResourceFactory(
            BootstrapForge::Contracts::Services::Cryptography::IHasher& hasher)
            noexcept;

        [[nodiscard]]
        BootstrapForge::Domain::Models::Resource Create(
            const std::filesystem::path& path,
            BootstrapForge::Domain::Models::ResourceKind kind =
            BootstrapForge::Domain::Models::ResourceKind::Directory,
            std::optional<
            BootstrapForge::Domain::Values::FileContent> content =
            std::nullopt,
            std::optional<
            BootstrapForge::Domain::Values::SymbolicLinkTarget>
            symbolicLinkTarget =
            std::nullopt) const;

        [[nodiscard]]
        BootstrapForge::Domain::Models::Resource Create(
            std::string_view manifestId,
            const std::filesystem::path& path,
            BootstrapForge::Domain::Models::ResourceKind kind =
            BootstrapForge::Domain::Models::ResourceKind::Directory,
            std::optional<
            BootstrapForge::Domain::Values::FileContent> content =
            std::nullopt,
            std::optional<
            BootstrapForge::Domain::Values::SymbolicLinkTarget>
            symbolicLinkTarget =
            std::nullopt) const;

    private:

        BootstrapForge::Contracts::Services::Cryptography::IHasher&
            m_hasher;
    };

} // namespace BootstrapForge::Application::Identity