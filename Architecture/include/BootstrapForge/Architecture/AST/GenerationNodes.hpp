#pragma once

#include <BootstrapForge/Architecture/AST/Node.hpp>

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace BootstrapForge::Architecture::AST
{
    class GenerationContainerNode : public Node
    {
    public:

        using NodePtr = std::unique_ptr<Node>;
        using NodeChildren = std::vector<NodePtr>;

        GenerationContainerNode(
            std::string name,
            Parsing::SourceSpan span)
            :
            Node(span),
            m_name(std::move(name))
        {
        }

        void AddChild(NodePtr child)
        {
            if (child)
            {
                m_children.emplace_back(std::move(child));
            }
        }

        [[nodiscard]]
        const std::string& Name() const noexcept
        {
            return m_name;
        }

        [[nodiscard]]
        const NodeChildren& Children() const noexcept
        {
            return m_children;
        }

        void SetEndLocation(Parsing::SourceLocation end) noexcept
        {
            SetSpan(Parsing::SourceSpan(Span().Begin(), end));
        }

    private:

        std::string m_name;
        NodeChildren m_children;
    };

    class GenerateNode final : public GenerationContainerNode
    {
    public:

        explicit GenerateNode(Parsing::SourceSpan span)
            :
            GenerationContainerNode("", span)
        {
        }

        [[nodiscard]]
        const char* KindName() const noexcept override
        {
            return "Generate";
        }
    };

    class BranchNode final : public GenerationContainerNode
    {
    public:

        using GenerationContainerNode::GenerationContainerNode;

        [[nodiscard]]
        const char* KindName() const noexcept override
        {
            return "Branch";
        }
    };

    class SubbranchNode final : public GenerationContainerNode
    {
    public:

        using GenerationContainerNode::GenerationContainerNode;

        [[nodiscard]]
        const char* KindName() const noexcept override
        {
            return "Subbranch";
        }
    };

    class FolderNode final : public GenerationContainerNode
    {
    public:

        using GenerationContainerNode::GenerationContainerNode;

        [[nodiscard]]
        const char* KindName() const noexcept override
        {
            return "Folder";
        }
    };

    class FileNode final : public Node
    {
    public:

        FileNode(
            std::string name,
            std::optional<std::string> content,
            Parsing::SourceSpan span)
            :
            Node(span),
            m_name(std::move(name)),
            m_content(std::move(content))
        {
        }

        [[nodiscard]]
        const std::string& Name() const noexcept
        {
            return m_name;
        }

        [[nodiscard]]
        const std::optional<std::string>& Content() const noexcept
        {
            return m_content;
        }

        [[nodiscard]]
        const char* KindName() const noexcept override
        {
            return "GeneratedFile";
        }

    private:

        std::string m_name;
        std::optional<std::string> m_content;
    };

} // namespace BootstrapForge::Architecture::AST
