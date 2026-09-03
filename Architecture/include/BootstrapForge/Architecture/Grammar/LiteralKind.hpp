#pragma once

#include <cstdint>

namespace BootstrapForge::Architecture::Grammar
{
    enum class LiteralKind : std::uint8_t
    {
        None = 0,

        Integer,
        FloatingPoint,
        Boolean,
        String,
        Character,

        Identifier,

        Path,

        ResourceId
    };

} // namespace BootstrapForge::Architecture::Grammar