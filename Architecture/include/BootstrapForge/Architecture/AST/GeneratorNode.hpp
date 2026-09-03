#pragma once

#include <BootstrapForge/Architecture/AST/Node.hpp>

#include <string>
#include <utility>

namespace BootstrapForge::Architecture::AST
{
    class GeneratorNode final : public Node
    {
    public:

        GeneratorNode() = default;

        GeneratorNode(
            std::string generator,
            std::string target,
            Parsing::SourceSpan span)
            :
            Node(span),
            m_generator(std::move(generator)),
            m_target(std::move(target))
        {
        }

        [[nodiscard]]
        const std::string& Generator() const noexcept
        {
            return m_generator;
        }

        [[nodiscard]]
        const std::string& Target() const noexcept
        {
            return m_target;
        }

        [[nodiscard]]
        const char* KindName() const noexcept override
        {
            return "Generator";
        }

    private:

        std::string m_generator;

        std::string m_target;
    };

} // namespace BootstrapForge::Architecture::AST