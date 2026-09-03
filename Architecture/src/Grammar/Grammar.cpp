#include <BootstrapForge/Architecture/Grammar/Grammar.hpp>

namespace BootstrapForge::Architecture::Grammar
{
    Grammar::Grammar(
        GrammarVersion version)
        :
        m_version(version),
        m_keywords(),
        m_operators()
    {
    }

    GrammarVersion Grammar::Version() const noexcept
    {
        return m_version;
    }

    const KeywordTable& Grammar::Keywords() const noexcept
    {
        return m_keywords;
    }

    const OperatorTable& Grammar::Operators() const noexcept
    {
        return m_operators;
    }

    bool Grammar::IsKeyword(
        std::string_view text) const noexcept
    {
        return m_keywords.Find(text).has_value();
    }

    bool Grammar::IsOperator(
        std::string_view text) const noexcept
    {
        return m_operators.Find(text).has_value();
    }

    std::optional<Keyword> Grammar::FindKeyword(
        std::string_view text) const noexcept
    {
        return m_keywords.Find(text);
    }

    std::optional<Operator> Grammar::FindOperator(
        std::string_view text) const noexcept
    {
        return m_operators.Find(text);
    }

} // namespace BootstrapForge::Architecture::Grammar