#include <BootstrapForge/Infrastructure/Generation/GenerationExecutor.hpp>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cwctype>
#include <fstream>
#include <map>
#include <sstream>
#include <system_error>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace BootstrapForge::Infrastructure::Generation
{
    namespace
    {
        std::atomic<unsigned long long> temporaryCounter{0};

        struct CollisionApproval
        {
            std::string ExistingContent;
        };

        std::string ErrorText(const std::error_code& error)
        {
            return error.message();
        }

#ifdef _WIN32
        std::string WindowsErrorText(DWORD error)
        {
            return std::system_category().message(
                static_cast<int>(error));
        }
#endif

        bool EqualName(
            const std::filesystem::path& left,
            const std::filesystem::path& right)
        {
#ifdef _WIN32
            std::wstring leftName = left.filename().wstring();
            std::wstring rightName = right.filename().wstring();
            std::transform(leftName.begin(), leftName.end(), leftName.begin(), towlower);
            std::transform(rightName.begin(), rightName.end(), rightName.begin(), towlower);
            return leftName == rightName;
#else
            std::string leftName = left.filename().generic_string();
            std::string rightName = right.filename().generic_string();
            std::transform(leftName.begin(), leftName.end(), leftName.begin(),
                [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
            std::transform(rightName.begin(), rightName.end(), rightName.begin(),
                [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
            return leftName == rightName;
#endif
        }

        std::filesystem::path FindCaseInsensitiveChild(
            const std::filesystem::path& parent,
            const std::filesystem::path& requested,
            std::error_code& error)
        {
            std::filesystem::directory_iterator iterator(parent, error);
            if (error)
            {
                return {};
            }

            const std::filesystem::directory_iterator end;
            while (iterator != end)
            {
                const auto& entry = *iterator;
                if (EqualName(entry.path(), requested))
                {
                    return entry.path();
                }

                iterator.increment(error);
                if (error)
                {
                    return {};
                }
            }
            return {};
        }

        bool IsSafeRelativePath(const std::filesystem::path& path)
        {
            if (path.empty() || path.is_absolute() || path.has_root_name() ||
                path.has_root_directory())
            {
                return false;
            }

            for (const auto& component : path)
            {
                if (component.empty() || component == "." || component == "..")
                {
                    return false;
                }
            }
            return true;
        }

        std::string PathKey(const std::filesystem::path& path)
        {
            std::string key = path.generic_string();
            std::transform(key.begin(), key.end(), key.begin(),
                [](unsigned char value)
                {
                    return static_cast<char>(std::tolower(value));
                });
            return key;
        }

        bool LocateExistingFile(
            const std::filesystem::path& root,
            const std::filesystem::path& relative,
            std::filesystem::path& existing,
            std::string& errorMessage)
        {
            if (!IsSafeRelativePath(relative) || relative.filename().empty())
            {
                errorMessage = "Unsafe relative file path.";
                return false;
            }

            auto parent = root;
            for (const auto& component : relative.parent_path())
            {
                std::error_code error;
                auto child = FindCaseInsensitiveChild(parent, component, error);
                if (error)
                {
                    errorMessage = "Cannot inspect directory '" + parent.string() +
                        "': " + ErrorText(error);
                    return false;
                }
                if (child.empty())
                {
                    return true;
                }

                const auto status = std::filesystem::symlink_status(child, error);
                if (error)
                {
                    errorMessage = "Cannot inspect path '" + child.string() +
                        "': " + ErrorText(error);
                    return false;
                }
                if (std::filesystem::is_symlink(status))
                {
                    errorMessage = "Refusing to traverse symbolic link '" +
                        child.string() + "'.";
                    return false;
                }
                if (!std::filesystem::is_directory(status))
                {
                    errorMessage = "A file blocks the generated file's parent path: '" +
                        child.string() + "'.";
                    return false;
                }
                parent = std::move(child);
            }

            std::error_code error;
            existing = FindCaseInsensitiveChild(
                parent, relative.filename(), error);
            if (error)
            {
                errorMessage = "Cannot inspect target directory '" +
                    parent.string() + "': " + ErrorText(error);
                return false;
            }
            return true;
        }

        bool EnsureDirectoryPath(
            const std::filesystem::path& root,
            const std::filesystem::path& relative,
            std::filesystem::path& resolved,
            std::string& errorMessage)
        {
            if (!IsSafeRelativePath(relative))
            {
                errorMessage = "Unsafe relative directory path.";
                return false;
            }

            resolved = root;
            for (const auto& component : relative)
            {
                std::error_code error;
                auto existing = FindCaseInsensitiveChild(resolved, component, error);
                if (error)
                {
                    errorMessage = "Cannot inspect directory '" +
                        resolved.string() + "': " + ErrorText(error);
                    return false;
                }

                if (!existing.empty())
                {
                    const auto status = std::filesystem::symlink_status(existing, error);
                    if (error)
                    {
                        errorMessage = "Cannot inspect path '" + existing.string() +
                            "': " + ErrorText(error);
                        return false;
                    }
                    if (std::filesystem::is_symlink(status))
                    {
                        errorMessage = "Refusing to traverse symbolic link '" +
                            existing.string() + "'.";
                        return false;
                    }
                    if (!std::filesystem::is_directory(status))
                    {
                        errorMessage = "A file already occupies directory path '" +
                            existing.string() + "'.";
                        return false;
                    }
                    resolved = std::move(existing);
                    continue;
                }

                const auto createdPath = resolved / component;
                if (!std::filesystem::create_directory(createdPath, error) && error)
                {
                    errorMessage = "Cannot create directory '" +
                        createdPath.string() + "': " + ErrorText(error);
                    return false;
                }

                if (error)
                {
                    errorMessage = "Cannot create directory '" +
                        createdPath.string() + "': " + ErrorText(error);
                    return false;
                }

                resolved = createdPath;
            }
            return true;
        }

        bool ReadFile(
            const std::filesystem::path& path,
            std::string& content,
            std::string& errorMessage)
        {
            std::ifstream input(path, std::ios::binary);
            if (!input)
            {
                errorMessage = "Cannot open existing file for preview: '" +
                    path.string() + "'.";
                return false;
            }
            std::ostringstream buffer;
            buffer << input.rdbuf();
            if (input.bad())
            {
                errorMessage = "Cannot read existing file for preview: '" +
                    path.string() + "'.";
                return false;
            }
            content = buffer.str();
            return true;
        }

        bool WriteExclusive(
            const std::filesystem::path& path,
            const std::string& contents,
            std::string& errorMessage)
        {
#ifdef _WIN32
            HANDLE file = CreateFileW(
                path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
                FILE_ATTRIBUTE_NORMAL, nullptr);
            if (file == INVALID_HANDLE_VALUE)
            {
                errorMessage = WindowsErrorText(GetLastError());
                return false;
            }

            std::size_t offset = 0;
            while (offset < contents.size())
            {
                const DWORD chunk = static_cast<DWORD>(std::min<std::size_t>(
                    contents.size() - offset, 1024 * 1024));
                DWORD written = 0;
                if (!WriteFile(file, contents.data() + offset, chunk, &written, nullptr) ||
                    written != chunk)
                {
                    const DWORD error = GetLastError();
                    CloseHandle(file);
                    std::error_code ignored;
                    std::filesystem::remove(path, ignored);
                    errorMessage = WindowsErrorText(error);
                    return false;
                }
                offset += written;
            }

            const bool flushed = FlushFileBuffers(file) != 0;
            const DWORD flushError = flushed ? ERROR_SUCCESS : GetLastError();
            CloseHandle(file);
            if (!flushed)
            {
                std::error_code ignored;
                std::filesystem::remove(path, ignored);
                errorMessage = WindowsErrorText(flushError);
                return false;
            }
            return true;
#else
            const int descriptor = ::open(
                path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0666);
            if (descriptor < 0)
            {
                errorMessage = std::error_code(errno, std::system_category()).message();
                return false;
            }

            std::size_t offset = 0;
            while (offset < contents.size())
            {
                const ssize_t written = ::write(
                    descriptor, contents.data() + offset, contents.size() - offset);
                if (written <= 0)
                {
                    const auto error = std::error_code(errno, std::system_category());
                    ::close(descriptor);
                    std::error_code ignored;
                    std::filesystem::remove(path, ignored);
                    errorMessage = error.message();
                    return false;
                }
                offset += static_cast<std::size_t>(written);
            }

            if (::fsync(descriptor) != 0)
            {
                const auto error = std::error_code(errno, std::system_category());
                ::close(descriptor);
                std::error_code ignored;
                std::filesystem::remove(path, ignored);
                errorMessage = error.message();
                return false;
            }
            ::close(descriptor);
            return true;
#endif
        }

        bool ReplaceFileAtomically(
            const std::filesystem::path& path,
            const std::string& contents,
            std::string& errorMessage)
        {
            std::filesystem::path temporary;
            bool temporaryCreated = false;
            for (int attempt = 0; attempt < 32; ++attempt)
            {
                temporary = path;
                temporary += ".bootstrapforge-" +
                    std::to_string(temporaryCounter.fetch_add(1)) + ".tmp";
                if (WriteExclusive(temporary, contents, errorMessage))
                {
                    temporaryCreated = true;
                    break;
                }
            }

            if (!temporaryCreated)
            {
                errorMessage = "Cannot stage replacement file: " + errorMessage;
                return false;
            }

#ifdef _WIN32
            if (!MoveFileExW(
                temporary.c_str(),
                path.c_str(),
                MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            {
                errorMessage = WindowsErrorText(GetLastError());
                std::error_code ignored;
                std::filesystem::remove(temporary, ignored);
                return false;
            }
#else
            std::error_code error;
            std::filesystem::rename(temporary, path, error);
            if (error)
            {
                errorMessage = ErrorText(error);
                std::filesystem::remove(temporary, error);
                return false;
            }
#endif
            return true;
        }

        bool ExecuteDirectory(
            const std::filesystem::path& root,
            const std::filesystem::path& relative,
            std::string& errorMessage)
        {
            std::filesystem::path resolved;
            return EnsureDirectoryPath(root, relative, resolved, errorMessage);
        }

        bool ExecuteFile(
            const std::filesystem::path& root,
            const Application::Generation::GenerationPlanEntry& entry,
            const GenerationExecutor::CollisionHandler& onCollision,
            const CollisionApproval* approval,
            bool& skipped,
            bool& stopRequested,
            std::string& errorMessage)
        {
            if (!IsSafeRelativePath(entry.RelativePath) ||
                entry.RelativePath.filename().empty())
            {
                errorMessage = "Unsafe relative file path.";
                return false;
            }

            const auto parentRelative = entry.RelativePath.parent_path();
            std::filesystem::path parent;
            if (parentRelative.empty())
            {
                parent = root;
            }
            else if (!EnsureDirectoryPath(root, parentRelative, parent, errorMessage))
            {
                return false;
            }

            std::error_code error;
            auto existing = FindCaseInsensitiveChild(
                parent, entry.RelativePath.filename(), error);
            if (error)
            {
                errorMessage = "Cannot inspect target directory '" +
                    parent.string() + "': " + ErrorText(error);
                return false;
            }

            const std::string desired = entry.Content.value_or(std::string{});
            if (existing.empty())
            {
                if (approval != nullptr)
                {
                    errorMessage = "The existing file disappeared after preview; refusing to apply a stale approval.";
                    return false;
                }
                return WriteExclusive(parent / entry.RelativePath.filename(),
                    desired, errorMessage);
            }

            const auto status = std::filesystem::symlink_status(existing, error);
            if (error)
            {
                errorMessage = "Cannot inspect existing target '" +
                    existing.string() + "': " + ErrorText(error);
                return false;
            }
            if (std::filesystem::is_symlink(status) ||
                !std::filesystem::is_regular_file(status))
            {
                errorMessage = "Existing target is not a regular file: '" +
                    existing.string() + "'.";
                return false;
            }

            std::string preview;
            if (!ReadFile(existing, preview, errorMessage))
            {
                return false;
            }

            const CollisionDecision decision = approval
                ? CollisionDecision::Replace
                : (onCollision
                    ? onCollision(entry.RelativePath, preview, desired)
                    : CollisionDecision::Stop);

            if (decision == CollisionDecision::Skip)
            {
                skipped = true;
                return true;
            }
            if (decision == CollisionDecision::Stop)
            {
                stopRequested = true;
                errorMessage = "File collision was declined by the user.";
                return false;
            }

            std::string current;
            if (!ReadFile(existing, current, errorMessage))
            {
                return false;
            }
            const std::string& approvedContent = approval
                ? approval->ExistingContent
                : preview;
            if (current != approvedContent)
            {
                errorMessage = "The existing file changed after preview; refusing to replace it.";
                return false;
            }

            return ReplaceFileAtomically(existing, desired, errorMessage);
        }
    }

    GenerationExecutionReport GenerationExecutor::Execute(
        const Application::Generation::GenerationPlan& plan,
        const std::filesystem::path& workspaceRoot,
        const CollisionHandler& onCollision,
        const FailureHandler& onFailure) const
    {
        GenerationExecutionReport report;
        std::error_code rootError;
        const auto rootStatus = std::filesystem::status(workspaceRoot, rootError);
        if (rootError || !std::filesystem::is_directory(rootStatus))
        {
            report.Stopped = true;
            report.Steps.push_back({
                {}, false, false,
                rootError
                    ? "Workspace root is unavailable: " + ErrorText(rootError)
                    : "Workspace root is not a directory."});
            return report;
        }

        std::error_code canonicalError;
        const auto root = std::filesystem::canonical(workspaceRoot, canonicalError);
        if (canonicalError)
        {
            report.Stopped = true;
            report.Steps.push_back({
                {}, false, false,
                "Cannot resolve workspace root: " + ErrorText(canonicalError)});
            return report;
        }

        std::map<std::string, CollisionApproval> approvedReplacements;
        std::map<std::string, std::string> preflightSkips;

        // Resolve every existing-file collision before the first filesystem
        // mutation. A user who declines a later collision should not be left
        // with earlier directories created by the same command.
        for (const auto& entry : plan.Entries())
        {
            if (entry.Kind != Application::Generation::GenerationEntryKind::File)
            {
                continue;
            }

            const std::string key = PathKey(entry.RelativePath);
            while (true)
            {
                std::filesystem::path existing;
                std::string errorMessage;
                if (!LocateExistingFile(root, entry.RelativePath, existing, errorMessage))
                {
                    const FailureDecision decision = onFailure
                        ? onFailure(entry.RelativePath, errorMessage)
                        : FailureDecision::Stop;
                    if (decision == FailureDecision::Retry)
                    {
                        continue;
                    }
                    if (decision == FailureDecision::Skip)
                    {
                        preflightSkips[key] = errorMessage;
                        break;
                    }
                    report.Steps.push_back({
                        entry.RelativePath, false, false, errorMessage});
                    report.Stopped = true;
                    return report;
                }

                if (existing.empty())
                {
                    break;
                }

                std::error_code statusError;
                const auto status = std::filesystem::symlink_status(existing, statusError);
                if (statusError || std::filesystem::is_symlink(status) ||
                    !std::filesystem::is_regular_file(status))
                {
                    errorMessage = statusError
                        ? "Cannot inspect existing target '" + existing.string() +
                            "': " + ErrorText(statusError)
                        : "Existing target is not a regular file: '" +
                            existing.string() + "'.";
                    const FailureDecision decision = onFailure
                        ? onFailure(entry.RelativePath, errorMessage)
                        : FailureDecision::Stop;
                    if (decision == FailureDecision::Retry)
                    {
                        continue;
                    }
                    if (decision == FailureDecision::Skip)
                    {
                        preflightSkips[key] = errorMessage;
                        break;
                    }
                    report.Steps.push_back({
                        entry.RelativePath, false, false, errorMessage});
                    report.Stopped = true;
                    return report;
                }

                std::string existingContent;
                if (!ReadFile(existing, existingContent, errorMessage))
                {
                    const FailureDecision decision = onFailure
                        ? onFailure(entry.RelativePath, errorMessage)
                        : FailureDecision::Stop;
                    if (decision == FailureDecision::Retry)
                    {
                        continue;
                    }
                    if (decision == FailureDecision::Skip)
                    {
                        preflightSkips[key] = errorMessage;
                        break;
                    }
                    report.Steps.push_back({
                        entry.RelativePath, false, false, errorMessage});
                    report.Stopped = true;
                    return report;
                }

                const CollisionDecision decision = onCollision
                    ? onCollision(
                        entry.RelativePath,
                        existingContent,
                        entry.Content.value_or(std::string{}))
                    : CollisionDecision::Stop;

                if (decision == CollisionDecision::Skip)
                {
                    preflightSkips[key] = "Existing file kept after preview.";
                    break;
                }
                if (decision == CollisionDecision::Stop)
                {
                    report.Steps.push_back({
                        entry.RelativePath,
                        false,
                        false,
                        "File collision was declined by the user."});
                    report.Stopped = true;
                    return report;
                }

                approvedReplacements.emplace(
                    key,
                    CollisionApproval{std::move(existingContent)});
                break;
            }
        }

        for (const auto& entry : plan.Entries())
        {
            const std::string key = PathKey(entry.RelativePath);
            if (const auto skippedEntry = preflightSkips.find(key);
                skippedEntry != preflightSkips.end())
            {
                report.Steps.push_back({
                    entry.RelativePath,
                    false,
                    true,
                    skippedEntry->second});
                continue;
            }

            while (true)
            {
                std::string errorMessage;
                bool succeeded = false;
                bool skipped = false;
                bool stopRequested = false;

                if (entry.Kind == Application::Generation::GenerationEntryKind::Directory)
                {
                    succeeded = ExecuteDirectory(root, entry.RelativePath, errorMessage);
                }
                else
                {
                    const auto approval = approvedReplacements.find(key);
                    succeeded = ExecuteFile(
                        root,
                        entry,
                        onCollision,
                        approval == approvedReplacements.end() ? nullptr : &approval->second,
                        skipped,
                        stopRequested,
                        errorMessage);
                }

                if (succeeded)
                {
                    report.Steps.push_back({entry.RelativePath, true, skipped, {}});
                    break;
                }

                if (stopRequested)
                {
                    report.Steps.push_back({
                        entry.RelativePath, false, false, errorMessage});
                    report.Stopped = true;
                    return report;
                }

                const FailureDecision decision = onFailure
                    ? onFailure(entry.RelativePath, errorMessage)
                    : FailureDecision::Stop;

                if (decision == FailureDecision::Retry)
                {
                    continue;
                }
                if (decision == FailureDecision::Skip)
                {
                    report.Steps.push_back({
                        entry.RelativePath, false, true, errorMessage});
                    break;
                }

                report.Steps.push_back({
                    entry.RelativePath, false, false, errorMessage});
                report.Stopped = true;
                return report;
            }
        }

        return report;
    }
}
