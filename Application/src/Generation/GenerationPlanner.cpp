#include <BootstrapForge/Application/Generation/GenerationPlanner.hpp>

#include <algorithm>
#include <cctype>
#include <iterator>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace BootstrapForge::Application::Generation
{
    namespace
    {
        std::string FoldCase(std::string value)
        {
            std::transform(
                value.begin(),
                value.end(),
                value.begin(),
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

        bool IsReservedWindowsName(const std::string& name)
        {
            std::string stem = name.substr(0, name.find('.'));
            stem = FoldCase(std::move(stem));

            if (stem == "con" || stem == "prn" || stem == "aux" || stem == "nul")
            {
                return true;
            }

            return stem.size() == 4 &&
                (stem.starts_with("com") || stem.starts_with("lpt")) &&
                stem[3] >= '1' && stem[3] <= '9';
        }

        bool IsSafeSegment(const std::string& name)
        {
            if (name.empty() || name == "." || name == ".." ||
                name.back() == '.' || name.back() == ' ' ||
                IsReservedWindowsName(name))
            {
                return false;
            }

            for (const unsigned char character : name)
            {
                if (character < 32 || character == '<' || character == '>' ||
                    character == ':' || character == '"' || character == '/' ||
                    character == '\\' || character == '|' || character == '?' ||
                    character == '*')
                {
                    return false;
                }
            }

            const std::filesystem::path segment = PathFromUtf8(name);
            return !segment.is_absolute() &&
                !segment.has_root_name() &&
                !segment.has_root_directory() &&
                segment.filename() == segment;
        }

        bool AddContainer(
            const Architecture::AST::GenerationContainerNode& container,
            const std::filesystem::path& parent,
            std::vector<GenerationPlanEntry>& entries,
            std::map<std::string, GenerationEntryKind>& destinations,
            Architecture::Diagnostics::DiagnosticEngine& diagnostics)
        {
            const std::string& name = container.Name();
            if (!IsSafeSegment(name))
            {
                diagnostics.Error(
                    container.Span(),
                    "Invalid or unsafe generation container name: '" + name + "'.");
                return false;
            }

            const std::filesystem::path relative = parent / PathFromUtf8(name);
            std::string key = relative.generic_string();
            std::transform(key.begin(), key.end(), key.begin(),
                [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
            const auto [found, inserted] = destinations.emplace(
                key, GenerationEntryKind::Directory);
            static_cast<void>(found);

            if (!inserted)
            {
                diagnostics.Error(
                    container.Span(),
                    "Duplicate case-insensitive generation destination: '" +
                        relative.generic_string() + "'.");
                return false;
            }

            entries.push_back(GenerationPlanEntry{
                GenerationEntryKind::Directory,
                relative,
                std::nullopt,
                container.Span()});

            for (const auto& child : container.Children())
            {
                if (const auto* nested =
                    dynamic_cast<const Architecture::AST::GenerationContainerNode*>(child.get()))
                {
                    if (!AddContainer(
                        *nested, relative, entries, destinations, diagnostics))
                    {
                        return false;
                    }
                    continue;
                }

                const auto* file =
                    dynamic_cast<const Architecture::AST::FileNode*>(child.get());
                if (file == nullptr)
                {
                    diagnostics.Error(
                        child->Span(),
                        "Unsupported node in generation command.");
                    return false;
                }

                const std::string& fileName = file->Name();
                if (!IsSafeSegment(fileName))
                {
                    diagnostics.Error(
                        file->Span(),
                        "Invalid or unsafe generated file name: '" + fileName + "'.");
                    return false;
                }

                const std::filesystem::path filePath = relative / PathFromUtf8(fileName);
                std::string fileKey = FoldCase(filePath.generic_string());
                const auto [fileFound, fileInserted] = destinations.emplace(
                    fileKey, GenerationEntryKind::File);
                static_cast<void>(fileFound);
                if (!fileInserted)
                {
                    diagnostics.Error(
                        file->Span(),
                        "Duplicate case-insensitive generation destination: '" +
                            filePath.generic_string() + "'.");
                    return false;
                }

                entries.push_back(GenerationPlanEntry{
                    GenerationEntryKind::File,
                    filePath,
                    file->Content().value_or(std::string{}),
                    file->Span()});
            }

            return true;
        }
    }

    std::optional<GenerationPlan> GenerationPlanner::Build(
        const Architecture::AST::GenerateNode& command,
        Architecture::Diagnostics::DiagnosticEngine& diagnostics) const
    {
        std::vector<GenerationPlanEntry> entries;
        std::map<std::string, GenerationEntryKind> destinations;

        for (const auto& child : command.Children())
        {
            if (const auto* file =
                dynamic_cast<const Architecture::AST::FileNode*>(child.get()))
            {
                if (!IsSafeSegment(file->Name()))
                {
                    diagnostics.Error(
                        file->Span(),
                        "Invalid or unsafe generated file name: '" + file->Name() + "'.");
                    return std::nullopt;
                }

                const std::filesystem::path filePath = PathFromUtf8(file->Name());
                const std::string fileKey = FoldCase(filePath.generic_string());
                if (!destinations.emplace(fileKey, GenerationEntryKind::File).second)
                {
                    diagnostics.Error(
                        file->Span(),
                        "Duplicate case-insensitive generation destination: '" +
                            filePath.generic_string() + "'.");
                    return std::nullopt;
                }

                entries.push_back(GenerationPlanEntry{
                    GenerationEntryKind::File,
                    filePath,
                    file->Content().value_or(std::string{}),
                    file->Span()});
                continue;
            }

            const auto* branch =
                dynamic_cast<const Architecture::AST::BranchNode*>(child.get());
            const auto* folder =
                dynamic_cast<const Architecture::AST::FolderNode*>(child.get());
            const auto* container = branch != nullptr
                ? static_cast<const Architecture::AST::GenerationContainerNode*>(branch)
                : static_cast<const Architecture::AST::GenerationContainerNode*>(folder);
            if (container == nullptr)
            {
                diagnostics.Error(
                    child->Span(),
                    "Only branch, folder, or file declarations are allowed at the root of generate.");
                return std::nullopt;
            }

            if (!AddContainer(
                *container, {}, entries, destinations, diagnostics))
            {
                return std::nullopt;
            }
        }

        std::sort(
            entries.begin(),
            entries.end(),
            [](const GenerationPlanEntry& left, const GenerationPlanEntry& right)
            {
                const auto leftDepth =
                    std::distance(left.RelativePath.begin(), left.RelativePath.end());
                const auto rightDepth =
                    std::distance(right.RelativePath.begin(), right.RelativePath.end());
                if (leftDepth != rightDepth)
                {
                    return leftDepth < rightDepth;
                }

                std::string leftKey = left.RelativePath.generic_string();
                std::string rightKey = right.RelativePath.generic_string();
                std::transform(leftKey.begin(), leftKey.end(), leftKey.begin(),
                    [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
                std::transform(rightKey.begin(), rightKey.end(), rightKey.begin(),
                    [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
                if (leftKey != rightKey)
                {
                    return leftKey < rightKey;
                }

                return left.Kind < right.Kind;
            });

        return GenerationPlan(std::move(entries));
    }
}
