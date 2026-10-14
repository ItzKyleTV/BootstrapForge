#include <BootstrapForge/Architecture/Grammar/KeywordTable.hpp>

namespace BootstrapForge::Architecture::Grammar
{
    KeywordTable::KeywordTable()
    {
        m_keywords =
        {
            Keyword{ "architecture", TokenKind::KeywordArchitecture },

            Keyword{ "generate", TokenKind::KeywordGenerate },

            Keyword{ "project", TokenKind::KeywordProject },
            Keyword{ "module", TokenKind::KeywordModule },

            Keyword{ "directory", TokenKind::KeywordDirectory },
            Keyword{ "file", TokenKind::KeywordFile },

            Keyword{ "branch", TokenKind::KeywordBranch },
            Keyword{ "subbranch", TokenKind::KeywordSubbranch },
            Keyword{ "folder", TokenKind::KeywordFolder },
            Keyword{ "relationship", TokenKind::KeywordRelationship },

            Keyword{ "build", TokenKind::KeywordBuild },
            Keyword{ "generator", TokenKind::KeywordGenerator },

            Keyword{ "create", TokenKind::KeywordCreate },
            Keyword{ "modify", TokenKind::KeywordModify },
            Keyword{ "remove", TokenKind::KeywordRemove },
            Keyword{ "rename", TokenKind::KeywordRename },
            Keyword{ "move", TokenKind::KeywordMove },
            Keyword{ "copy", TokenKind::KeywordCopy },

            Keyword{ "include", TokenKind::KeywordInclude }
        };
    }

    std::optional<Keyword>
        KeywordTable::Find(
            std::string_view text) const noexcept
    {
        for (const auto& keyword : m_keywords)
        {
            if (keyword.Text() == text)
            {
                return keyword;
            }
        }

        return std::nullopt;
    }

    const std::vector<Keyword>&
        KeywordTable::Keywords() const noexcept
    {
        return m_keywords;
    }

} // namespace BootstrapForge::Architecture::Grammar
