#pragma once

#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>

#include <filesystem>

namespace BootstrapForge::Application::Mapping
{
    class ManifestMapper;
}

namespace BootstrapForge::Infrastructure::Filesystem
{
    class ManifestFileReader;
}

namespace BootstrapForge::Infrastructure::Deserialization
{
    class JsonManifestDeserializer;
}

namespace BootstrapForge::Infrastructure::Loading
{
    class JsonDesiredArchitectureLoader final
    {
    public:

        JsonDesiredArchitectureLoader(
            const Filesystem::ManifestFileReader& fileReader,
            const Deserialization::JsonManifestDeserializer& deserializer,
            const Application::Mapping::ManifestMapper& mapper) noexcept;

        [[nodiscard]]
        Domain::Models::ArchitectureModel Load(
            const std::filesystem::path& manifestPath) const;

    private:

        const Filesystem::ManifestFileReader& m_fileReader;
        const Deserialization::JsonManifestDeserializer& m_deserializer;
        const Application::Mapping::ManifestMapper& m_mapper;
    };
}