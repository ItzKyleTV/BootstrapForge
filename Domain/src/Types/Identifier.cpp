#include <BootstrapForge/Domain/Types/Identifier.hpp>

namespace BootstrapForge::Domain::Types
{
    std::string Identifier::ToString() const
    {
        return m_hash.ToHex();
    }

} // namespace BootstrapForge::Domain::Types