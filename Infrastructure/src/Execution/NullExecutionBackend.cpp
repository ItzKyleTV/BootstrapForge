#include <BootstrapForge/Infrastructure/Execution/NullExecutionBackend.hpp>

namespace BootstrapForge::Infrastructure::Execution
{
    bool NullExecutionBackend::Create(
        const Domain::Models::Resource& resource)
    {
        m_created.push_back(resource);
        return true;
    }

    bool NullExecutionBackend::Update(
        const Domain::Models::Resource& resource)
    {
        m_updated.push_back(resource);
        return true;
    }

    bool NullExecutionBackend::Delete(
        const Domain::Models::Resource& resource)
    {
        m_deleted.push_back(resource);
        return true;
    }

    const std::vector<Domain::Models::Resource>&
        NullExecutionBackend::Created() const noexcept
    {
        return m_created;
    }

    const std::vector<Domain::Models::Resource>&
        NullExecutionBackend::Updated() const noexcept
    {
        return m_updated;
    }

    const std::vector<Domain::Models::Resource>&
        NullExecutionBackend::Deleted() const noexcept
    {
        return m_deleted;
    }
}