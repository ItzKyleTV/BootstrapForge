#include <BootstrapForge/Engine/Differ/ArchitectureDiffer.hpp>

#include <BootstrapForge/Domain/Models/ResourceKind.hpp>

#include <algorithm>
#include <filesystem>
#include <utility>
#include <vector>

namespace BootstrapForge::Engine::Differ
{
    using namespace BootstrapForge::Domain;

    namespace
    {
        [[nodiscard]]
        bool IsAncestorPath(
            const std::filesystem::path& ancestor,
            const std::filesystem::path& descendant)
        {
            if (ancestor.empty() ||
                descendant.empty() ||
                ancestor == descendant)
            {
                return false;
            }

            const auto relative =
                descendant.lexically_relative(
                    ancestor);

            if (relative.empty())
            {
                return false;
            }

            const auto first =
                relative.begin();

            if (first == relative.end())
            {
                return false;
            }

            return
                *first != "." &&
                *first != "..";
        }
    }

    Models::ArchitectureDelta ArchitectureDiffer::Compare(
        const Models::ArchitectureModel& desired,
        const Models::ArchitectureModel& current) const
    {
        std::vector<Models::Resource> addedResources;
        std::vector<Models::Resource> removedResources;
        std::vector<Models::Resource> modifiedResources;

        std::vector<Models::Relationship> addedRelationships;
        std::vector<Models::Relationship> removedRelationships;

        //
        // Added and modified resources.
        //
        // First try to match by ResourceId.
        //
        // If that fails, match by canonical path.
        //
        // This is important because desired resources can receive
        // stable manifest-based IDs while physical resources receive
        // path-based IDs.
        //
        for (const auto& desiredResource : desired.Resources())
        {
            const auto currentIt =
                std::ranges::find_if(
                    current.Resources(),
                    [&](const auto& currentResource)
                    {
                        if (currentResource.Id() ==
                            desiredResource.Id())
                        {
                            return true;
                        }

                        return currentResource.Path() ==
                            desiredResource.Path();
                    });

            if (currentIt == current.Resources().end())
            {
                addedResources.emplace_back(
                    desiredResource);

                continue;
            }

            //
            // The resource exists.
            //
            // Compare its actual state rather than its identity.
            //
            if (!desiredResource.SameState(*currentIt))
            {
                modifiedResources.emplace_back(
                    desiredResource);
            }
        }

        //
        // Removed resources.
        //
        for (const auto& currentResource : current.Resources())
        {
            const bool exists =
                std::ranges::any_of(
                    desired.Resources(),
                    [&](const auto& desiredResource)
                    {
                        if (desiredResource.Id() ==
                            currentResource.Id())
                        {
                            return true;
                        }

                        return desiredResource.Path() ==
                            currentResource.Path();
                    });

            if (exists)
            {
                continue;
            }

            //
            // A directory that is an ancestor of a desired resource
            // is structural infrastructure for that desired resource.
            //
            // It must remain available even when the directory itself
            // is not explicitly declared in the desired architecture.
            //
            // For example:
            //
            //     current:
            //         /src
            //
            //     desired:
            //         /src/target.txt
            //
            // /src must not be classified as removed.
            //
            if (currentResource.Kind() ==
                Models::ResourceKind::Directory)
            {
                const bool isRequiredParent =
                    std::ranges::any_of(
                        desired.Resources(),
                        [&](const auto& desiredResource)
                        {
                            return IsAncestorPath(
                                currentResource.Path().Value(),
                                desiredResource.Path().Value());
                        });

                if (isRequiredParent)
                {
                    continue;
                }
            }

            removedResources.emplace_back(
                currentResource);
        }

        //
        // Added relationships.
        //
        for (const auto& desiredRelationship :
            desired.Relationships())
        {
            const bool exists =
                std::ranges::any_of(
                    current.Relationships(),
                    [&](const auto& currentRelationship)
                    {
                        return currentRelationship.Id() ==
                            desiredRelationship.Id();
                    });

            if (!exists)
            {
                addedRelationships.emplace_back(
                    desiredRelationship);
            }
        }

        //
        // Removed relationships.
        //
        for (const auto& currentRelationship :
            current.Relationships())
        {
            const bool exists =
                std::ranges::any_of(
                    desired.Relationships(),
                    [&](const auto& desiredRelationship)
                    {
                        return desiredRelationship.Id() ==
                            currentRelationship.Id();
                    });

            if (!exists)
            {
                removedRelationships.emplace_back(
                    currentRelationship);
            }
        }

        return Models::ArchitectureDelta(
            std::move(addedResources),
            std::move(removedResources),
            std::move(modifiedResources),
            std::move(addedRelationships),
            std::move(removedRelationships));
    }

} // namespace BootstrapForge::Engine::Differ