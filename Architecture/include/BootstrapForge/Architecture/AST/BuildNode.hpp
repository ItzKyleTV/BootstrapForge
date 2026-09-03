#pragma once

#include <BootstrapForge/Architecture/AST/Node.hpp>

#include <string>
#include <utility>

namespace BootstrapForge::Architecture::AST
{
    class BuildNode final : public Node
    {
    public:

        BuildNode() = default;

        BuildNode(
            std::string target,
            Parsing::SourceSpan span)
            :
            Node(span),
            m_target(std::move(target))
        {
        }

        [[nodiscard]]
        const std::string& Target() const noexcept
        {
            return m_target;
        }

        [[nodiscard]]
        const char* KindName() const noexcept override
        {
            return "Build";
        }

    private:

        std::string m_target;
    };

} // namespace BootstrapForge::Architecture::AST