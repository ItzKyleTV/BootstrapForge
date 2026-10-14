#pragma once

#include <BootstrapForge/Application/Generation/GenerationPlan.hpp>

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace BootstrapForge::Infrastructure::Generation
{
    enum class CollisionDecision
    {
        Replace,
        Skip,
        Stop
    };

    enum class FailureDecision
    {
        Retry,
        Skip,
        Stop
    };

    struct GenerationStepResult
    {
        std::filesystem::path RelativePath;
        bool Succeeded = false;
        bool Skipped = false;
        std::string Detail;
    };

    struct GenerationExecutionReport
    {
        std::vector<GenerationStepResult> Steps;
        bool Stopped = false;
    };

    class GenerationExecutor final
    {
    public:

        using CollisionHandler = std::function<CollisionDecision(
            const std::filesystem::path&,
            const std::string&,
            const std::string&)>;

        using FailureHandler = std::function<FailureDecision(
            const std::filesystem::path&,
            const std::string&)>;

        [[nodiscard]]
        GenerationExecutionReport Execute(
            const Application::Generation::GenerationPlan& plan,
            const std::filesystem::path& workspaceRoot,
            const CollisionHandler& onCollision,
            const FailureHandler& onFailure) const;
    };
}
