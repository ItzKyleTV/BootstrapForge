# BootstrapForge code packages and components

BootstrapForge imports a local code package from a JSON manifest. In the **Generate from Command** dialog, choose **Import Code Package / Component**, select the manifest, and preview the complete workspace-relative plan before applying it.

The manifest is named `*.bfpackage.json` by convention. Its schema version 1 format is:

```json
{
  "schemaVersion": 1,
  "id": "example.widget",
  "name": "Widget Component",
  "version": "1.0.0",
  "kind": "component",
  "description": "A reusable widget implementation.",
  "dependencies": ["example.core"],
  "directories": ["assets/empty"],
  "files": [
    {
      "source": "templates/include/Widget.hpp",
      "destination": "include/Widget.hpp"
    },
    {
      "source": "templates/src/Widget.cpp",
      "destination": "src/Widget.cpp"
    },
    {
      "source": "templates/CMakeLists.txt",
      "destination": "CMakeLists.txt"
    }
  ]
}
```

`kind` must be `package` or `component`. `id`, `name`, and `version` are required. `description`, `dependencies`, and `directories` are optional. Each file source is relative to the manifest's directory; each destination and declared directory is relative to the open project. Directories are also planned automatically for file destinations. Files may contain source code, build configuration, tests, or other text content; BootstrapForge previews their contents before the existing generation apply flow handles collisions.

The importer rejects absolute paths, traversal segments, Windows reserved names, missing sources, source symlinks that escape the package directory, case-insensitive duplicate destinations, and file/directory collisions before the user can apply anything. Existing project files use the regular side-by-side collision preview. Existing directories are reused.

Package contents are treated as data. The importer does not run package scripts, resolve or install declared dependencies, or configure/build/test the imported project automatically. Dependencies are shown as metadata so the user can review them. Include any required `CMakeLists.txt` and test-registration changes as ordinary files in the package when the user requests them.
