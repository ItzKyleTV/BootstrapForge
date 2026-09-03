#include <BootstrapForge/Infrastructure/Loading/JsonDesiredArchitectureLoader.hpp>

#include <BootstrapForge/Application/Mapping/ManifestMapper.hpp>

#include <BootstrapForge/Infrastructure/Deserialization/JsonManifestDeserializer.hpp>
#include <BootstrapForge/Infrastructure/Filesystem/ManifestFileReader.hpp>

namespace BootstrapForge::Infrastructure::Loading
{

    JsonDesiredArchitectureLoader::JsonDesiredArchitectureLoader(
        const Filesystem::ManifestFileReader& fileReader,
        const Deserialization::JsonManifestDeserializer& deserializer,
        const Application::Mapping::ManifestMapper& mapper) noexcept
        :
        m_fileReader(fileReader),
        m_deserializer(deserializer),
        m_mapper(mapper)
    {
    }

    Domain::Models::ArchitectureModel
        JsonDesiredArchitectureLoader::Load(
            const std::filesystem::path& manifestPath) const
    {
        const std::string json =
            m_fileReader.Read(manifestPath);

        const auto manifest =
            m_deserializer.Deserialize(json);

        return m_mapper.Map(manifest);
    }

}