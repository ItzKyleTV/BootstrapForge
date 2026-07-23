#pragma once

#include <BootstrapForge/Domain/Types/Name.hpp>
#include <BootstrapForge/Domain/Types/Path.hpp>
#include <BootstrapForge/Domain/Types/ResourceId.hpp>

#include <compare>

namespace BootstrapForge::Domain::Models
{
    class Resource
    {
    public:

        Resource() = default;

        constexpr Resource(
            Types::ResourceId id,
            Types::Name name,
            Types::Path path);

        [[nodiscard]]
        constexpr const Types::ResourceId& Id() const noexcept;

        [[nodiscard]]
        constexpr const Types::Name& Name() const noexcept;

        [[nodiscard]]
        constexpr const Types::Path& Path() const noexcept;

        auto operator<=>(const Resource&) const noexcept = default;

    private:

        Types::ResourceId m_id;
        Types::Name m_name;
        Types::Path m_path;
    };

    constexpr Resource::Resource(
        Types::ResourceId id,
        Types::Name name,
        Types::Path path)
        : m_id(std::move(id))
        , m_name(std::move(name))
        , m_path(std::move(path))
    {
    }

    constexpr const Types::ResourceId& Resource::Id() const noexcept
    {
        return m_id;
    }

    constexpr const Types::Name& Resource::Name() const noexcept
    {
        return m_name;
    }

    constexpr const Types::Path& Resource::Path() const noexcept
    {
        return m_path;
    }

} // namespace BootstrapForge::Domain::Models