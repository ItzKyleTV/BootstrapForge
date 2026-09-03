#pragma once

#include <BootstrapForge/Architecture/AST/Node.hpp>

#include <BootstrapForge/Architecture/Grammar/TokenKind.hpp>

#include <string>
#include <utility>

namespace BootstrapForge::Architecture::AST
{
    class ResourceNode final : public Node
    {
    public:

        ResourceNode() = default;

        ResourceNode(
            Grammar::TokenKind kind,
            std::string path,
            Parsing::SourceSpan span)
            :
            Node(span),
            m_kind(kind),
            m_path(std::move(path))
        {
        }

        [[nodiscard]]
        Grammar::TokenKind ResourceTokenKind() const noexcept
        {
            return m_kind;
        }

        [[nodiscard]]
        const std::string& Path() const noexcept
        {
            return m_path;
        }

        [[nodiscard]]
        bool IsDirectory() const noexcept
        {
            return m_kind ==
                Grammar::TokenKind::KeywordDirectory;
        }

        [[nodiscard]]
        bool IsFile() const noexcept
        {
            return m_kind ==
                Grammar::TokenKind::KeywordFile;
        }

        [[nodiscard]]
        const char* KindName() const noexcept override
        {
            return "Resource";
        }

    private:

        Grammar::TokenKind m_kind =
            Grammar::TokenKind::Unknown;

        std::string m_path;
    };

} // namespace BootstrapForge::Architecture::AST