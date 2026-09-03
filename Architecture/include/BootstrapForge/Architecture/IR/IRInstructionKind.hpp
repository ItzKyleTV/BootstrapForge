#pragma once

namespace BootstrapForge::Architecture::IR
{
    enum class IRInstructionKind
    {
        CreateProject,
        DeleteProject,
        RenameProject,

        CreateModule,
        DeleteModule,
        RenameModule,

        CreateDirectory,
        DeleteDirectory,

        CreateFile,
        DeleteFile,
        ModifyFile,

        CopyFile,
        MoveFile,

        GenerateCMake,
        GenerateWorkspace,
        GenerateSolution,

        ExecuteTemplate,

        AddRelationship,
        RemoveRelationship
    };

} // namespace BootstrapForge::Architecture::IR