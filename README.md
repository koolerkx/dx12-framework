# Game with engine

## Build and Run

Please use MSVC to compile.

### Required: Shader Code Generation

Before configuring or building the project, generate shader CB headers first:

```bash
tools\shader-codegen.exe generate --config codegen.yaml
```

This generates required headers under `src/generated/shader/`.

The build path is expected in `{ProjectDir}/out/build`.

Executable output is typically located in: `{ProjectDir}/out/build/<preset>/<Config>/<target>.exe`

(Note: x86/32-bit builds are intentionally not supported, focusing only on x64 as the standard for modern Windows development.)

### Targets

| Target       | Description                           | CMake Option       |
| ------------ | ------------------------------------- | ------------------ |
| `app`        | Main application                      | —                  |
| `app_editor` | Application with ImGui editor overlay | `ENABLE_EDITOR=ON` |

### Generator: `Ninja` (For development)

| Preset                | Config                               | Build                                         |
| --------------------- | ------------------------------------ | --------------------------------------------- |
| x64 Debug             | `cmake --preset x64-debug`           | `cmake --build out/build/x64-debug`           |
| x64 Release           | `cmake --preset x64-release`         | `cmake --build out/build/x64-release`         |
| x64 Debug (Editor)    | `cmake --preset x64-debug-editor`    | `cmake --build out/build/x64-debug-editor`    |
| x64 Release (Editor)  | `cmake --preset x64-release-editor`  | `cmake --build out/build/x64-release-editor`  |
| x64 Debug (RenderDoc) | `cmake --preset x64-debug-renderdoc` | `cmake --build out/build/x64-debug-renderdoc` |

Note: The x64 Debug (RenderDoc) built with DLL.

### Generator: `Visual Studio` (For the solution file)

The config generates the solution, only needs to be executed once.

| Preset               | Config                         | Build                                          |
| -------------------- | ------------------------------ | ---------------------------------------------- |
| x64 Debug            | `cmake --preset vs-x64`        | `cmake --build --preset vs-x64-debug`          |
| x64 Release          | `cmake --preset vs-x64`        | `cmake --build --preset vs-x64-release`        |
| x64 Debug (Editor)   | `cmake --preset vs-x64-editor` | `cmake --build --preset vs-x64-editor-debug`   |
| x64 Release (Editor) | `cmake --preset vs-x64-editor` | `cmake --build --preset vs-x64-editor-release` |
