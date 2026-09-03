#pragma once

#include <BootstrapForge/Domain/Models/ResourceKind.hpp>

#include <BootstrapForge/Domain/Types/Name.hpp>
#include <BootstrapForge/Domain/Types/Path.hpp>
#include <BootstrapForge/Domain/Types/ResourceId.hpp>

#include <BootstrapForge/Domain/Values/FileContent.hpp>
#include <BootstrapForge/Domain/Values/SymbolicLinkTarget.hpp>

#include <compare>
#include <optional>
#include <string>
#include <utility>

namespace BootstrapForge::Domain::Models
{
    class Resource
    {
    public:

        Resource() = default;

        constexpr Resource(
            Types::ResourceId id,
            Types::Name name,
            Types::Path path,
            ResourceKind kind,
            std::optional<Values::FileContent> content =
            std::nullopt,
            std::optional<Values::SymbolicLinkTarget>
            symbolicLinkTarget =
            std::nullopt)
            :
            m_id(std::move(id)),
            m_name(std::move(name)),
            m_path(std::move(path)),
            m_kind(kind),
            m_content(std::move(content)),
            m_symbolicLinkTarget(
                std::move(symbolicLinkTarget))
        {
        }

        [[nodiscard]]
        constexpr const Types::ResourceId&
            Id() const noexcept
        {
            return m_id;
        }

        [[nodiscard]]
        constexpr const Types::Name&
            Name() const noexcept
        {
            return m_name;
        }

        [[nodiscard]]
        constexpr const Types::Path&
            Path() const noexcept
        {
            return m_path;
        }

        [[nodiscard]]
        constexpr ResourceKind
            Kind() const noexcept
        {
            return m_kind;
        }

        [[nodiscard]]
        constexpr const std::optional<Values::FileContent>&
            Content() const noexcept
        {
            return m_content;
        }

        [[nodiscard]]
        constexpr const std::optional<
            Values::SymbolicLinkTarget>&
            SymbolicLinkTarget() const noexcept
        {
            return m_symbolicLinkTarget;
        }

        //
        // Determines whether two resources represent the same
        // physical/logical state, excluding their identity.
        //
        // Resource IDs may originate from different identity
        // sources:
        //
        //   Desired model -> manifest ID
        //   Current model -> physical/path identity
        //
        // Therefore ID must not participate in state comparison.
        //
        [[nodiscard]]
        bool SameState(
            const Resource& other) const noexcept
        {
            return
                m_name ==
                other.m_name &&
                m_path ==
                other.m_path &&
                m_kind ==
                other.m_kind &&
                m_content ==
                other.m_content &&
                m_symbolicLinkTarget ==
                other.m_symbolicLinkTarget;
        }

        //
        // Identity only.
        //
        // Content and symbolic-link target are deliberately
        // excluded because they describe mutable resource state.
        //
        [[nodiscard]]
        std::string IdentityCanonical() const
        {
            std::string canonical;

            canonical.reserve(128);

            canonical += "resource\n";

            canonical += "path=";
            canonical += m_path.ToString();

            canonical += "\nname=";
            canonical += std::string(
                m_name.Value());

            canonical += "\nkind=";

            switch (m_kind)
            {
            case ResourceKind::Directory:
                canonical += "directory";
                break;

            case ResourceKind::File:
                canonical += "file";
                break;

            case ResourceKind::SymbolicLink:
                canonical += "symbolic-link";
                break;
            }

            return canonical;
        }

        //
        // Full canonical representation.
        //
        // This represents the complete state of the resource.
        //
        [[nodiscard]]
        std::string Canonical() const
        {
            std::string canonical =
                IdentityCanonical();

            if (m_content.has_value())
            {
                canonical += "\ncontent=";
                canonical +=
                    m_content->Value();
            }

            if (m_symbolicLinkTarget.has_value())
            {
                canonical +=
                    "\nsymbolic-link-target=";

                canonical +=
                    m_symbolicLinkTarget->ToString();
            }

            return canonical;
        }

        //
        // Structural comparison.
        //
        // Reconciliation uses SameState() because desired and
        // current resource identities may originate from different
        // identity sources.
        //
        auto operator<=>(const Resource&) const noexcept = default;

    private:

        Types::ResourceId m_id;
        Types::Name m_name;
        Types::Path m_path;

        ResourceKind m_kind =
            ResourceKind::Directory;

        std::optional<
            Values::FileContent>
            m_content;

        std::optional<
            Values::SymbolicLinkTarget>
            m_symbolicLinkTarget;
    };

} // namespace BootstrapForge::Domain::Models