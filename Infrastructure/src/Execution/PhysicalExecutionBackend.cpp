#include <BootstrapForge/Infrastructure/Execution/PhysicalExecutionBackend.hpp>

#include <BootstrapForge/Domain/Types/Hash.hpp>

#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace BootstrapForge::Infrastructure::Execution
{
    PhysicalExecutionBackend::PhysicalExecutionBackend(
        std::filesystem::path workspaceRoot)
        :
        m_workspaceRoot(std::move(workspaceRoot))
    {
    }

    std::filesystem::path
        PhysicalExecutionBackend::Resolve(
            const Domain::Models::Resource& resource) const
    {
        std::filesystem::path logical(
            resource.Path().Value());

        if (logical.has_root_directory())
        {
            logical =
                logical.relative_path();
        }

        return m_workspaceRoot / logical;
    }

    std::filesystem::path
        PhysicalExecutionBackend::RelationshipStore() const
    {
        return
            m_workspaceRoot /
            ".bootstrapforge" /
            "relationships";
    }

    bool PhysicalExecutionBackend::CreateDirectory(
        const Domain::Models::Resource& resource)
    {
        try
        {
            const auto path =
                Resolve(resource);

            std::filesystem::create_directories(
                path);

            return true;
        }
        catch (const std::filesystem::filesystem_error&)
        {
            return false;
        }
    }

    bool PhysicalExecutionBackend::CreateFile(
        const Domain::Models::Resource& resource)
    {
        try
        {
            const auto path =
                Resolve(resource);

            std::filesystem::create_directories(
                path.parent_path());

            std::ofstream stream(
                path,
                std::ios::binary |
                std::ios::trunc);

            if (!stream)
            {
                return false;
            }

            if (resource.Content().has_value())
            {
                const std::string& content =
                    resource.Content()->Value();

                stream.write(
                    content.data(),
                    static_cast<std::streamsize>(
                        content.size()));

                if (!stream)
                {
                    return false;
                }
            }

            stream.close();

            return stream.good();
        }
        catch (const std::filesystem::filesystem_error&)
        {
            return false;
        }
    }

    bool PhysicalExecutionBackend::UpdateFile(
        const Domain::Models::Resource& resource)
    {
        try
        {
            const auto path =
                Resolve(resource);

            std::filesystem::create_directories(
                path.parent_path());

            std::ofstream stream(
                path,
                std::ios::binary |
                std::ios::trunc);

            if (!stream)
            {
                return false;
            }

            if (resource.Content().has_value())
            {
                const std::string& content =
                    resource.Content()->Value();

                stream.write(
                    content.data(),
                    static_cast<std::streamsize>(
                        content.size()));

                if (!stream)
                {
                    return false;
                }
            }

            stream.close();

            return stream.good();
        }
        catch (const std::filesystem::filesystem_error&)
        {
            return false;
        }
    }

    bool PhysicalExecutionBackend::DeleteDirectory(
        const Domain::Models::Resource& resource)
    {
        try
        {
            std::filesystem::remove_all(
                Resolve(resource));

            return true;
        }
        catch (const std::filesystem::filesystem_error&)
        {
            return false;
        }
    }

    bool PhysicalExecutionBackend::DeleteFile(
        const Domain::Models::Resource& resource)
    {
        try
        {
            std::filesystem::remove(
                Resolve(resource));

            return true;
        }
        catch (const std::filesystem::filesystem_error&)
        {
            return false;
        }
    }

    bool PhysicalExecutionBackend::CreateSymbolicLink(
        const Domain::Models::Resource& resource)
    {
        try
        {
            if (!resource.Target().has_value())
            {
                return false;
            }

            const auto linkPath =
                Resolve(resource);

            std::filesystem::path targetPath =
                resource.Target()
                ->Value()
                .Value();

            if (targetPath.has_root_directory())
            {
                targetPath =
                    targetPath.relative_path();

                targetPath =
                    m_workspaceRoot /
                    targetPath;
            }
            else
            {
                targetPath =
                    linkPath.parent_path() /
                    targetPath;
            }

            std::filesystem::create_directories(
                linkPath.parent_path());

            //
            // exists() does not reliably identify a
            // dangling symbolic link because its target
            // does not exist.
            //
            // is_symlink() identifies the link itself.
            //
            if (std::filesystem::exists(linkPath) ||
                std::filesystem::is_symlink(linkPath))
            {
                return false;
            }

            //
            // A directory symbolic link must use
            // create_directory_symlink().
            //
            // A file symbolic link uses create_symlink().
            //
            if (std::filesystem::is_directory(targetPath))
            {
                std::filesystem::create_directory_symlink(
                    targetPath,
                    linkPath);
            }
            else
            {
                std::filesystem::create_symlink(
                    targetPath,
                    linkPath);
            }

            return true;
        }
        catch (const std::filesystem::filesystem_error&)
        {
            return false;
        }
    }

    bool PhysicalExecutionBackend::DeleteSymbolicLink(
        const Domain::Models::Resource& resource)
    {
        try
        {
            const auto linkPath =
                Resolve(resource);

            if (!std::filesystem::exists(
                linkPath) &&
                !std::filesystem::is_symlink(
                    linkPath))
            {
                return true;
            }

            //
            // remove() is intentional here.
            //
            // We must remove the link itself rather than
            // recursively deleting the resource it points to.
            //
            std::filesystem::remove(
                linkPath);

            return true;
        }
        catch (const std::filesystem::filesystem_error&)
        {
            return false;
        }
    }

    bool PhysicalExecutionBackend::AddRelationship(
        const Domain::Models::Relationship& relationship)
    {
        try
        {
            const auto store =
                RelationshipStore();

            std::filesystem::create_directories(
                store.parent_path());

            //
            // Relationship format:
            //
            // relationship-id source-id target-id
            //
            // All three values are Hash::ToHex()
            // representations.
            //

            std::vector<std::string> lines;

            if (std::filesystem::exists(store))
            {
                std::ifstream input(store);

                if (!input)
                {
                    return false;
                }

                std::string line;

                while (std::getline(
                    input,
                    line))
                {
                    if (!line.empty())
                    {
                        lines.emplace_back(
                            std::move(line));
                    }
                }
            }

            const std::string id =
                relationship.Id()
                .Value()
                .ToHex();

            const std::string source =
                relationship.Source()
                .Value()
                .ToHex();

            const std::string target =
                relationship.Target()
                .Value()
                .ToHex();

            const std::string record =
                id + " " +
                source + " " +
                target;

            for (const auto& existing :
                lines)
            {
                if (existing == record)
                {
                    return true;
                }
            }

            lines.emplace_back(
                record);

            std::ofstream output(
                store,
                std::ios::trunc);

            if (!output)
            {
                return false;
            }

            for (const auto& line :
                lines)
            {
                output
                    << line
                    << '\n';
            }

            output.flush();

            return output.good();
        }
        catch (const std::filesystem::filesystem_error&)
        {
            return false;
        }
    }

    bool PhysicalExecutionBackend::RemoveRelationship(
        const Domain::Models::Relationship& relationship)
    {
        try
        {
            const auto store =
                RelationshipStore();

            if (!std::filesystem::exists(store))
            {
                return true;
            }

            std::ifstream input(store);

            if (!input)
            {
                return false;
            }

            const std::string id =
                relationship.Id()
                .Value()
                .ToHex();

            std::vector<std::string> remaining;

            std::string line;

            while (std::getline(
                input,
                line))
            {
                if (line.empty())
                {
                    continue;
                }

                //
                // The first token is the
                // RelationshipId.
                //

                const std::size_t separator =
                    line.find(' ');

                if (separator ==
                    std::string::npos)
                {
                    return false;
                }

                const std::string existingId =
                    line.substr(
                        0,
                        separator);

                if (existingId != id)
                {
                    remaining.emplace_back(
                        std::move(line));
                }
            }

            input.close();

            std::ofstream output(
                store,
                std::ios::trunc);

            if (!output)
            {
                return false;
            }

            for (const auto& remainingLine :
                remaining)
            {
                output
                    << remainingLine
                    << '\n';
            }

            output.flush();

            return output.good();
        }
        catch (const std::filesystem::filesystem_error&)
        {
            return false;
        }
    }

} // namespace BootstrapForge::Infrastructure::Execution