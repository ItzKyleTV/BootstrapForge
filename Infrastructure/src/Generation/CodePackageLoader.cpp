#include <BootstrapForge/Infrastructure/Generation/CodePackageLoader.hpp>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string_view>
#include <system_error>
#include <utility>

namespace BootstrapForge::Infrastructure::Generation
{
    namespace
    {
        using Json = nlohmann::json;
        using EntryKind = Application::Generation::GenerationEntryKind;
        using PlanEntry = Application::Generation::GenerationPlanEntry;

        std::string FoldCase(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(),
                [](unsigned char character)
                {
                    return static_cast<char>(std::tolower(character));
                });
            return value;
        }

        std::filesystem::path PathFromUtf8(std::string_view value)
        {
            const auto* begin = reinterpret_cast<const char8_t*>(value.data());
            return std::filesystem::path(std::u8string(begin, value.size()));
        }

        bool IsSafeSegment(const std::string& segment)
        {
            if (segment.empty() || segment == "." || segment == ".." ||
                segment.back() == '.' || segment.back() == ' ')
            {
                return false;
            }

            std::string device = segment.substr(0, segment.find('.'));
            device = FoldCase(std::move(device));
            if (device == "con" || device == "prn" || device == "aux" ||
                device == "nul" ||
                (device.size() == 4 &&
                    (device.starts_with("com") || device.starts_with("lpt")) &&
                    device[3] >= '1' && device[3] <= '9'))
            {
                return false;
            }

            return std::none_of(segment.begin(), segment.end(),
                [](unsigned char character)
                {
                    return character < 32 || character == '<' || character == '>' ||
                        character == ':' || character == '"' || character == '/' ||
                        character == '\\' || character == '|' || character == '?' ||
                        character == '*';
                });
        }

        bool SplitSafePath(
            const std::string& value,
            std::vector<std::string>& segments)
        {
            if (value.empty() || value.front() == '/' || value.front() == '\\' ||
                (value.size() >= 2 && value[1] == ':'))
            {
                return false;
            }

            std::size_t begin = 0;
            while (begin <= value.size())
            {
                const std::size_t end = value.find('/', begin);
                const std::string segment = value.substr(
                    begin,
                    end == std::string::npos ? std::string::npos : end - begin);
                if (!IsSafeSegment(segment))
                {
                    return false;
                }
                segments.push_back(segment);
                if (end == std::string::npos)
                {
                    break;
                }
                begin = end + 1;
            }
            return !segments.empty();
        }

        bool IsWithinRoot(
            const std::filesystem::path& candidate,
            const std::filesystem::path& root)
        {
            const std::filesystem::path relative = candidate.lexically_relative(root);
            if (relative.empty() || relative.is_absolute() || relative.has_root_name())
            {
                return false;
            }

            const auto first = relative.begin();
            return first != relative.end() && *first != "..";
        }

        bool ReadString(
            const Json& object,
            const char* key,
            bool required,
            std::string& output,
            std::string& errorMessage)
        {
            const auto found = object.find(key);
            if (found == object.end())
            {
                if (required)
                {
                    errorMessage = std::string("Package manifest is missing '") + key + "'.";
                    return false;
                }
                output.clear();
                return true;
            }

            if (!found->is_string())
            {
                errorMessage = std::string("Package field '") + key + "' must be a string.";
                return false;
            }

            output = found->get<std::string>();
            if (required && output.empty())
            {
                errorMessage = std::string("Package field '") + key + "' cannot be empty.";
                return false;
            }
            return true;
        }

        bool ReadPackageInfo(
            const Json& document,
            CodePackageInfo& info,
            std::string& errorMessage)
        {
            if (!document.is_object())
            {
                errorMessage = "Package manifest root must be a JSON object.";
                return false;
            }

            const auto schema = document.find("schemaVersion");
            if (schema == document.end() || !schema->is_number_integer() ||
                schema->get<int>() != 1)
            {
                errorMessage = "Package manifest schemaVersion must be 1.";
                return false;
            }

            if (!ReadString(document, "id", true, info.Id, errorMessage) ||
                !ReadString(document, "name", true, info.Name, errorMessage) ||
                !ReadString(document, "version", true, info.Version, errorMessage) ||
                !ReadString(document, "kind", true, info.Kind, errorMessage) ||
                !ReadString(document, "description", false, info.Description, errorMessage))
            {
                return false;
            }

            if (info.Kind != "package" && info.Kind != "component")
            {
                errorMessage = "Package kind must be either 'package' or 'component'.";
                return false;
            }

            const auto dependencies = document.find("dependencies");
            if (dependencies != document.end())
            {
                if (!dependencies->is_array())
                {
                    errorMessage = "Package dependencies must be an array of package IDs.";
                    return false;
                }

                std::set<std::string> seenDependencies;
                for (const auto& dependency : *dependencies)
                {
                    if (!dependency.is_string())
                    {
                        errorMessage = "Every package dependency must be a string ID.";
                        return false;
                    }
                    std::string id = dependency.get<std::string>();
                    if (id.empty() || !seenDependencies.insert(FoldCase(id)).second)
                    {
                        errorMessage = "Package dependencies must be non-empty and unique.";
                        return false;
                    }
                    info.Dependencies.push_back(std::move(id));
                }
            }

            return true;
        }

        bool AddParentDirectories(
            const std::filesystem::path& filePath,
            std::map<std::string, std::filesystem::path>& directories,
            const std::map<std::string, EntryKind>& destinations,
            std::string& errorMessage)
        {
            std::filesystem::path parent;
            for (auto iterator = filePath.begin();
                iterator != filePath.end(); ++iterator)
            {
                auto next = iterator;
                ++next;
                if (next == filePath.end())
                {
                    break;
                }
                parent /= *iterator;
                const std::string key = FoldCase(parent.generic_string());
                const auto destination = destinations.find(key);
                if (destination != destinations.end() &&
                    destination->second == EntryKind::File)
                {
                    errorMessage = "A package file is also used as a parent directory: '" +
                        parent.generic_string() + "'.";
                    return false;
                }

                const auto [found, inserted] = directories.emplace(key, parent);
                if (!inserted && found->second.generic_string() != parent.generic_string())
                {
                    errorMessage = "Package paths use different casing for the same directory: '" +
                        parent.generic_string() + "'.";
                    return false;
                }
            }
            return true;
        }

        std::optional<LoadedCodePackage> LoadPackage(
            const std::filesystem::path& manifestPath,
            std::string& errorMessage)
        {
            std::error_code filesystemError;
            const std::filesystem::path absoluteManifest =
                std::filesystem::canonical(manifestPath, filesystemError);
            if (filesystemError || !std::filesystem::is_regular_file(absoluteManifest))
            {
                errorMessage = "The selected package manifest does not exist or is not a file.";
                return std::nullopt;
            }

            std::ifstream manifestStream(absoluteManifest, std::ios::binary);
            if (!manifestStream)
            {
                errorMessage = "Could not open the package manifest for reading.";
                return std::nullopt;
            }

            Json document;
            try
            {
                manifestStream >> document;
            }
            catch (const Json::exception& exception)
            {
                errorMessage = std::string("Invalid package manifest JSON: ") + exception.what();
                return std::nullopt;
            }

            LoadedCodePackage package;
            if (!ReadPackageInfo(document, package.Info, errorMessage))
            {
                return std::nullopt;
            }

            const auto files = document.find("files");
            if (files == document.end() || !files->is_array() || files->empty())
            {
                errorMessage = "Package manifest must contain a non-empty 'files' array.";
                return std::nullopt;
            }

            const std::filesystem::path packageRoot = absoluteManifest.parent_path();
            const std::filesystem::path canonicalRoot =
                std::filesystem::canonical(packageRoot, filesystemError);
            if (filesystemError)
            {
                errorMessage = "Could not resolve the package directory.";
                return std::nullopt;
            }

            std::map<std::string, EntryKind> destinations;
            std::map<std::string, std::filesystem::path> directories;
            std::vector<PlanEntry> entries;

            for (const auto& file : *files)
            {
                if (!file.is_object())
                {
                    errorMessage = "Every package file entry must be an object.";
                    return std::nullopt;
                }

                std::string sourceText;
                std::string destinationText;
                if (!ReadString(file, "source", true, sourceText, errorMessage) ||
                    !ReadString(file, "destination", true, destinationText, errorMessage))
                {
                    return std::nullopt;
                }

                std::vector<std::string> sourceSegments;
                std::vector<std::string> destinationSegments;
                if (!SplitSafePath(sourceText, sourceSegments))
                {
                    errorMessage = "Package source paths must be safe relative paths: '" +
                        sourceText + "'.";
                    return std::nullopt;
                }
                if (!SplitSafePath(destinationText, destinationSegments))
                {
                    errorMessage = "Package destinations must be safe workspace-relative paths: '" +
                        destinationText + "'.";
                    return std::nullopt;
                }

                std::filesystem::path sourceRelative;
                for (const std::string& segment : sourceSegments)
                {
                    sourceRelative /= PathFromUtf8(segment);
                }
                const std::filesystem::path sourcePath =
                    std::filesystem::canonical(packageRoot / sourceRelative, filesystemError);
                if (filesystemError || !std::filesystem::is_regular_file(sourcePath) ||
                    !IsWithinRoot(sourcePath, canonicalRoot))
                {
                    errorMessage = "Package source is missing, not a regular file, or escapes the package directory: '" +
                        sourceText + "'.";
                    return std::nullopt;
                }

                std::filesystem::path destinationPath;
                for (const std::string& segment : destinationSegments)
                {
                    destinationPath /= PathFromUtf8(segment);
                }
                const std::string destinationKey =
                    FoldCase(destinationPath.generic_string());
                if (destinations.contains(destinationKey))
                {
                    errorMessage = "Package has a duplicate case-insensitive destination: '" +
                        destinationText + "'.";
                    return std::nullopt;
                }
                if (directories.contains(destinationKey))
                {
                    errorMessage = "A package destination is both a file and a directory: '" +
                        destinationText + "'.";
                    return std::nullopt;
                }

                std::ifstream sourceStream(sourcePath, std::ios::binary);
                if (!sourceStream)
                {
                    errorMessage = "Could not read package source file: '" + sourceText + "'.";
                    return std::nullopt;
                }
                const std::istreambuf_iterator<char> streamEnd{};
                std::string contents(
                    std::istreambuf_iterator<char>{sourceStream},
                    streamEnd);
                if (sourceStream.bad())
                {
                    errorMessage = "A read error occurred for package source file: '" +
                        sourceText + "'.";
                    return std::nullopt;
                }

                destinations.emplace(destinationKey, EntryKind::File);
                if (!AddParentDirectories(
                    destinationPath, directories, destinations, errorMessage))
                {
                    return std::nullopt;
                }
                entries.push_back(PlanEntry{
                    EntryKind::File,
                    destinationPath,
                    std::move(contents),
                    Architecture::Parsing::SourceSpan{}});
            }

            const auto declaredDirectories = document.find("directories");
            if (declaredDirectories != document.end())
            {
                if (!declaredDirectories->is_array())
                {
                    errorMessage = "Package directories must be an array of workspace-relative paths.";
                    return std::nullopt;
                }
                for (const auto& directory : *declaredDirectories)
                {
                    if (!directory.is_string())
                    {
                        errorMessage = "Every package directory must be a string path.";
                        return std::nullopt;
                    }
                    const std::string directoryText = directory.get<std::string>();
                    std::vector<std::string> directorySegments;
                    if (!SplitSafePath(directoryText, directorySegments))
                    {
                        errorMessage = "Package directories must be safe workspace-relative paths: '" +
                            directoryText + "'.";
                        return std::nullopt;
                    }

                    std::filesystem::path directoryPath;
                    for (const std::string& segment : directorySegments)
                    {
                        directoryPath /= PathFromUtf8(segment);
                    }
                    const std::string directoryKey =
                        FoldCase(directoryPath.generic_string());
                    if (destinations.contains(directoryKey))
                    {
                        errorMessage = "A package destination is both a file and a directory: '" +
                            directoryText + "'.";
                        return std::nullopt;
                    }
                    if (!AddParentDirectories(
                        directoryPath, directories, destinations, errorMessage))
                    {
                        return std::nullopt;
                    }
                    const auto [found, inserted] = directories.emplace(
                        directoryKey, directoryPath);
                    if (!inserted &&
                        found->second.generic_string() != directoryPath.generic_string())
                    {
                        errorMessage = "Package paths use different casing for the same directory: '" +
                            directoryText + "'.";
                        return std::nullopt;
                    }
                }
            }

            for (const auto& [key, path] : directories)
            {
                static_cast<void>(key);
                entries.push_back(PlanEntry{
                    EntryKind::Directory,
                    path,
                    std::nullopt,
                    Architecture::Parsing::SourceSpan{}});
            }

            std::sort(entries.begin(), entries.end(),
                [](const PlanEntry& left, const PlanEntry& right)
                {
                    const auto leftDepth = std::distance(
                        left.RelativePath.begin(), left.RelativePath.end());
                    const auto rightDepth = std::distance(
                        right.RelativePath.begin(), right.RelativePath.end());
                    if (leftDepth != rightDepth)
                    {
                        return leftDepth < rightDepth;
                    }
                    return FoldCase(left.RelativePath.generic_string()) <
                        FoldCase(right.RelativePath.generic_string());
                });

            package.Plan = Application::Generation::GenerationPlan(
                std::move(entries));
            errorMessage.clear();
            return package;
        }
    }

    std::optional<LoadedCodePackage> CodePackageLoader::Load(
        const std::filesystem::path& manifestPath,
        std::string& errorMessage) const
    {
        try
        {
            return LoadPackage(manifestPath, errorMessage);
        }
        catch (const std::filesystem::filesystem_error& exception)
        {
            errorMessage = std::string("Package filesystem error: ") + exception.what();
        }
        catch (const Json::exception& exception)
        {
            errorMessage = std::string("Invalid package manifest: ") + exception.what();
        }
        return std::nullopt;
    }

} // namespace BootstrapForge::Infrastructure::Generation
