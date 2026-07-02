# Resource model

This document describes the editor-side resource model for user-owned project assets.
It is intentionally separate from scripting details: scripts are one resource type, not
the whole resource system.

## Scope

Only files under `Assets/` are user resources.

System project files outside `Assets/`, such as `Configs/project_config.toml` and
`Configs/game_config.toml`, are not asset resources. They are project infrastructure
and are handled by the project service directly.

Folders are not resources either. A folder is only a container and a UI/navigation
structure. Folders do not have GUIDs, do not have an asset type, and should not be
referenced from game or build configuration.

## Asset identity

Any asset that can be referenced by another file or config should have a stable GUID.
The GUID lives in the asset `.meta` file next to the asset's physical files.

Project and game configuration must reference assets by GUID, not by path. Paths can
change when the user renames or moves files; GUIDs are the stable identity.

For the current script implementation, a script asset is represented by:

```text
Player.hpp
Player.cpp
Player.meta
```

The current script meta format is:

```toml
guid = "..."
class_name = "Player"
```

This is considered the first resource-meta format for scripts. Other asset types will
grow their own `.meta` data as the project needs them.

## AssetResourceType

`AssetResourceType` is the enum for user assets in `Assets/`. It should evolve with
the project as new asset kinds become real features.

Examples:

```text
Script
Scene
Texture
Material
Model
Audio
Shader
```

`AssetResourceType` must not be used for system files. System files are already split
into a separate tree by code and do not need asset classification.

`Folder` should not become an asset resource type. Folder behavior belongs to the tree
and filesystem UI, not to the resource identity model.

## Filtering

Asset filters should eventually filter by `AssetResourceType`, not by first-level
folder names.

Current folder-name filtering is only a temporary UI convention. It is not a stable
resource model because the user controls the structure under `Assets/` and may place
scripts, scenes, textures, and other files wherever they want.

Target behavior:

- A file node has an asset type.
- A folder node has no asset type.
- Filtering hides resource nodes by type.
- A folder stays visible if it contains at least one visible child.
- A folder becomes hidden only when none of its descendants are visible.

## Game config references

`Configs/game_config.toml` should store references to assets by GUID.

For game-wide scripts, the current config stores an ordered GUID array:

```toml
global_script_guids = ["..."]
```

The script GUID resolves through `.meta`. The path and class name are derived data.

If a script file is renamed or moved, the reference remains valid because the GUID in
`.meta` moves with it.

If a script is deleted from the project, its GUID should be removed from
`game_config.toml`. On project open, validation should also remove stale GUIDs that no
longer resolve to existing assets.

## Tree highlighting

When a resource is selected from settings, the Assets tree should resolve its GUID,
expand the containing folders, and highlight the resource node.

Resources referenced by `game_config.toml` may also have a persistent visual marker in
the tree. This marker belongs to the file node, not to the containing folders.

## Validation on project open

Opening a project should perform resource validation in this order:

1. Synchronize script `.meta` files under `Assets/`.
2. Build a GUID index for known asset resources.
3. Load `game_config.toml`.
4. Remove stale script/resource references whose GUIDs no longer exist.
5. Report GUID collisions as errors; do not silently choose one asset.
6. Save `game_config.toml` if validation changed it.

This keeps project configs stable across renames and moves, while still cleaning up
references to assets that were actually deleted.

## ProjectFileWatcherService

`ProjectFileWatcherService` owns the single `Assets/` filesystem watcher for the
currently open project.

The watcher is not owned by UI widgets. It publishes raw file events:

```text
Created
Deleted
Renamed
Changed
```

Project services subscribe to those events and decide how to update their own state.
This keeps external changes from Explorer, Git checkout, or another IDE synchronized
with editor caches and configuration.

Current consumers:

- `ScriptAssetIndexService` updates script `.meta` files and rebuilds the script index.
- `AssetsViewModel` refreshes the tree view and schedules script compilation when the
  script index says compilation is affected.

## ScriptAssetIndexService

`ScriptAssetIndexService` is the runtime cache for script assets under `Assets/`.
It is the single lookup source for script GUIDs inside the editor.

It owns:

```text
ByGuid      : guid -> ScriptAssetInfo
ByClassName : class_name -> [ScriptAssetInfo]
```

`ScriptAssetInfo` contains:

```text
Guid
ClassName
HppPath
CppPath
MetaPath
```

The index is rebuilt from `.meta` files and is not saved as a separate project file.
The persisted source of truth remains the script `.meta` file.

On project open, the service performs a full script meta synchronization:

- `.hpp` without `.meta` gets a new `.meta` with a new GUID.
- orphan `.meta` without matching `.hpp` is removed.
- duplicate GUIDs and duplicate class names are reported through the editor log.

During the editor session, the service listens to `ProjectFileWatcherService`:

- created `.hpp` -> generate missing `.meta`;
- deleted `.hpp` -> remove orphan `.meta`;
- renamed `.hpp` -> update the index through delete/create handling;
- changed `.meta` -> rebuild the index;
- changed `.hpp` or `.cpp` -> mark compilation as affected.

Future game/build configuration validation should use this service to resolve GUIDs
before asking the native script registry whether a script is registered as a global
game script.

## Reflected config editing

System config files are edited by the inspector through their registered parser, not
through `ProjectService` cache objects.

The flow is:

```text
ProjectNode.RelativePath
-> ProjectStructure.AllFiles parser
-> IEditorConfigParser.DeserializeForEditor()
-> reflection over editor attributes
-> IEditorConfigParser.SerializeFromEditor()
-> write the same file back
```

Config data classes use editor-only attributes from
`Services/Project/Infrastructure/EditorVisibilityAttribute.cs`:

- `EditorVisibilityAttribute` controls hidden/read-only/editable fields.
- `EditorDisplayNameAttribute` replaces reflected CLR names with human-readable
  inspector labels without changing serialized TOML keys.
- `EditorOptionsAttribute` renders scalar properties as combo boxes.
- `EditorCollectionAttribute` renders list properties and describes sorting,
  duplicate policy, and optional asset type hints.

`Configs/game_config.toml` currently exposes:

```toml
defines = []
log_listener = ""
global_script_guids = []
```

`global_script_guids` is ordered and stores only global game scripts. Future picker
and tree-highlight UI should resolve these GUIDs through `ScriptAssetIndexService`.

Collection fields can choose different editor behaviour by attribute kind.

`defines` is an editable text list. Each item has a remove button, text edits are
validated as C/C++ preprocessor defines, invalid edits are logged and rolled back in
the field. `+` adds an empty row, `Reset` reloads the last applied value, and `Apply`
writes the whole block through undo/redo.

`global_script_guids` is a GUID list rendered as script names. Scripts are added by
dragging script assets from the project tree, removed with the row remove button, and
sorted by drag-and-drop. Script add/remove/sort actions save immediately through
undo/redo.
