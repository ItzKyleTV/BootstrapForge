#pragma once

#include <BootstrapForge/Architecture/Grammar/GrammarVersion.hpp>
#include <BootstrapForge/Architecture/Grammar/Keyword.hpp>
#include <BootstrapForge/Architecture/Grammar/KeywordTable.hpp>
#include <BootstrapForge/Architecture/Grammar/Operator.hpp>
#include <BootstrapForge/Architecture/Grammar/OperatorTable.hpp>

#include <optional>
#include <string_view>

namespace BootstrapForge::Architecture::Grammar
{
    class Grammar final
    {
    public:

        explicit Grammar(
            GrammarVersion version =
            GrammarVersion::Version1);

        [[nodiscard]]
        GrammarVersion Version() const noexcept;

        [[nodiscard]]
        const KeywordTable& Keywords() const noexcept;

        [[nodiscard]]
        const OperatorTable& Operators() const noexcept;

        [[nodiscard]]
        bool IsKeyword(
            std::string_view text) const noexcept;

        [[nodiscard]]
        bool IsOperator(
            std::string_view text) const noexcept;

        [[nodiscard]]
        std::optional<Keyword> FindKeyword(
            std::string_view text) const noexcept;

        [[nodiscard]]
        std::optional<Operator> FindOperator(
            std::string_view text) const noexcept;

    private:

        GrammarVersion m_version;

        KeywordTable m_keywords;

        OperatorTable m_operators;
    };

} // namespace BootstrapForge::Architecture::Grammar