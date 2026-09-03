#pragma once

#include <BootstrapForge/Architecture/Parsing/SourceSpan.hpp>

#include <compare>

namespace BootstrapForge::Architecture::AST
{
    class Node
    {
    public:

        Node() = default;

        explicit Node(
            Parsing::SourceSpan span) noexcept
            :
            m_span(span)
        {
        }

        virtual ~Node() = default;

        [[nodiscard]]
        const Parsing::SourceSpan& Span() const noexcept
        {
            return m_span;
        }

        [[nodiscard]]
        virtual const char* KindName() const noexcept = 0;

    protected:

        void SetSpan(
            Parsing::SourceSpan span) noexcept
        {
            m_span = span;
        }

    private:

        Parsing::SourceSpan m_span;
    };

} // namespace BootstrapForge::Architecture::AST