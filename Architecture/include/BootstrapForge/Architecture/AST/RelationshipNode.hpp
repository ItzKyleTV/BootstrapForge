#pragma once

#include <BootstrapForge/Architecture/AST/Node.hpp>

#include <string>
#include <utility>

namespace BootstrapForge::Architecture::AST
{
    class RelationshipNode final : public Node
    {
    public:

        RelationshipNode() = default;

        RelationshipNode(
            std::string source,
            std::string target,
            Parsing::SourceSpan span)
            :
            Node(span),
            m_source(std::move(source)),
            m_target(std::move(target))
        {
        }

        [[nodiscard]]
        const std::string& Source() const noexcept
        {
            return m_source;
        }

        [[nodiscard]]
        const std::string& Target() const noexcept
        {
            return m_target;
        }

        [[nodiscard]]
        const char* KindName() const noexcept override
        {
            return "Relationship";
        }

    private:

        std::string m_source;
        std::string m_target;
    };

} // namespace BootstrapForge::Architecture::AST