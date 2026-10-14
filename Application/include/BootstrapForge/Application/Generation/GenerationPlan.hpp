#pragma once

#include <BootstrapForge/Architecture/Parsing/SourceSpan.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace BootstrapForge::Application::Generation
{
    enum class GenerationEntryKind
    {
        Directory,
        File
    };

    struct GenerationPlanEntry
    {
        GenerationEntryKind Kind = GenerationEntryKind::Directory;
        std::filesystem::path RelativePath;
        std::optional<std::string> Content;
        Architecture::Parsing::SourceSpan Span;
    };

    class GenerationPlan final
    {
    public:

        GenerationPlan() = default;

        explicit GenerationPlan(
            std::vector<GenerationPlanEntry> entries)
            :
            m_entries(std::move(entries))
        {
        }

        [[nodiscard]]
        const std::vector<GenerationPlanEntry>& Entries() const noexcept
        {
            return m_entries;
        }

    private:

        std::vector<GenerationPlanEntry> m_entries;
    };
}
