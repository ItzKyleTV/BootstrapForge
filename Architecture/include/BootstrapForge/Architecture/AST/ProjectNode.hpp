#pragma once

#include <BootstrapForge/Architecture/AST/Node.hpp>

#include <string>
#include <utility>

namespace BootstrapForge::Architecture::AST
{
    class ProjectNode final : public Node
    {
    public:

        ProjectNode() = default;

        ProjectNode(
            std::string name,
            Parsing::SourceSpan span)
            :
            Node(span),
            m_name(std::move(name))
        {
        }

        [[nodiscard]]
        const std::string& Name() const noexcept
        {
            return m_name;
        }

        [[nodiscard]]
        const char* KindName() const noexcept override
        {
            return "Project";
        }

    private:

        std::string m_name;
    };

} // namespace BootstrapForge::Architecture::AST