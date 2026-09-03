#pragma once

#include <BootstrapForge/Architecture/AST/Node.hpp>
#include <BootstrapForge/Architecture/Parsing/SourceSpan.hpp>

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace BootstrapForge::Architecture::AST
{
    class ArchitectureNode final : public Node
    {
    public:

        using NodePtr = std::unique_ptr<Node>;
        using NodeChildren = std::vector<NodePtr>;

        ArchitectureNode() = default;

        explicit ArchitectureNode(
            Parsing::SourceSpan span)
            :
            Node(span)
        {
        }

        void AddChild(
            NodePtr child)
        {
            if (child)
            {
                m_children.emplace_back(
                    std::move(child));
            }
        }

        [[nodiscard]]
        const NodeChildren& Children() const noexcept
        {
            return m_children;
        }

        [[nodiscard]]
        std::size_t ChildCount() const noexcept
        {
            return m_children.size();
        }

        [[nodiscard]]
        const Node* Child(
            std::size_t index) const noexcept
        {
            if (index >= m_children.size())
            {
                return nullptr;
            }

            return m_children[index].get();
        }

        [[nodiscard]]
        const char* KindName() const noexcept override
        {
            return "Architecture";
        }

    private:

        NodeChildren m_children;
    };

} // namespace BootstrapForge::Architecture::AST