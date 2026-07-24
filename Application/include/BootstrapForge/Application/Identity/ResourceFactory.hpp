#pragma once

#include <BootstrapForge/Contracts/Services/Cryptography/IHasher.hpp>

#include <BootstrapForge/Domain/Models/Resource.hpp>

#include <filesystem>

namespace BootstrapForge::Application::Identity
{
    class ResourceFactory final
    {
    public:

        explicit ResourceFactory(
            BootstrapForge::Contracts::Services::Cryptography::IHasher& hasher) noexcept;

        [[nodiscard]]
        BootstrapForge::Domain::Models::Resource Create(
            const std::filesystem::path& path) const;

    private:

        BootstrapForge::Contracts::Services::Cryptography::IHasher& m_hasher;
    };

} // namespace BootstrapForge::Application::Identity