# Core Command Registry

`CoreCommandRegistry` is the generic bottom-up command registry for headless
actions. Packages register callable command objects during initialization, and a
small shared console main exposes those commands for listing, description,
execution, JSON automation, and pipeline composition.

## Purpose

- Keep headless and GUI behavior wired to the same command implementation.
- Let packages register small CLI-style actions without each package inventing a
  new command-line protocol.
- Let pipelines be normal commands that call other registered commands through
  `CoreCommandContext::Run`.
- Let GUI tools browse and execute any compatible registry application by
  spawning the target process and exchanging JSON through stdout.

## Registration Pattern

Register commands in `.icpp` files with `INITBLOCK`:

```cpp
struct EchoCommand {
	static void Setup(CoreCommand& command)
	{
		command.description = "Echoes the text argument.";
		command.category = "sample";
		command.tags << "sample" << "json";
		command.args_schema.Add("text", "string");
		command.handler = [](CoreCommandContext& context) {
			CoreCommandResult result;
			ValueMap out;
			out.Add("text", context.Get("text", ""));
			result.value = out;
			result.message = "echo completed";
			return result;
		};
	}
};

INITBLOCK {
	CoreCommandRegistry::Register<EchoCommand>("sample.echo");
}
```

Use a tiny console main for registry applications:

```cpp
CONSOLE_APP_MAIN
{
	CoreCommandRegistryMain();
}
```

## Naming

- Use `CoreCommand*` names for the generic Core feature because `uppsrc/cli`
  already has `Upp::CommandRegistry` and `Upp::Command`.
- Use dotted command names such as `sample.echo` or `vsm.logic-self-test`.
- Use category and tags for browsing and filtering; do not encode all metadata in
  the command name.

## Console Protocol

Any application using `CoreCommandRegistryMain()` supports:

```text
<app> --list [--json]
<app> --describe <name> [--json]
<app> --run <name> [--args-json <json>] [--arg key=value]... [--json]
```

Argument rules:

- Pass one `--arg key=value` pair per argument.
- `--arg "a=1 b=2"` is malformed and must return exit code `2`, not crash.
- Integers and booleans are parsed into typed `Value` entries; other values stay
  strings.
- `--args-json` accepts a JSON object and can be combined with `--arg`, where
  later key/value arguments override earlier values.

JSON run results use the `CoreCommandResult` shape:

```json
{
  "ok": true,
  "code": 0,
  "message": "human-readable summary",
  "value": {}
}
```

## Pipeline Commands

Pipelines are ordinary commands. A pipeline handler can call child commands with
`CoreCommandContext::Run`, collect each child `CoreCommandResult`, and return a
machine-readable step list in `value`.

Use direct in-process child calls when all commands are registered in the same
application. Use process-wrapper commands when the authoritative behavior already
lives in another executable and sharing a library entrypoint would create
duplication or premature coupling.

## Reference Packages

- `reference/CommandRegistry` demonstrates sample commands, typed arguments,
  JSON list/describe/run, and an in-process pipeline.
- `reference/CommandRegistryGui` is a generic GUI wrapper. It receives the target
  registry application path as an argument and uses the same process/JSON helper
  in GUI and headless smoke modes.
- `reference/VisualStateCommandRegistry` registers VisualStateModel pilot
  commands as process-backed wrappers over existing VSM reference executables.

## Verification

Representative commands:

```text
bin\build.exe -m MSVS22x64 -j12 .\reference\CommandRegistry\CommandRegistry.upp
bin\build.exe -m MSVS22x64 -j12 .\reference\CommandRegistryGui\CommandRegistryGui.upp
bin\build.exe -m MSVS22x64 -j12 .\reference\VisualStateCommandRegistry\VisualStateCommandRegistry.upp
bin\CommandRegistry.exe --list --json
bin\CommandRegistry.exe --run sample.pipeline --arg text=hello --arg a=4 --arg b=5 --json
bin\CommandRegistry.exe --run sample.add --arg "a=1 b=2" --json
bin\CommandRegistryGui.exe --headless-smoke bin\CommandRegistry.exe
bin\VisualStateCommandRegistry.exe --run vsm.logic-self-test --json
bin\VisualStateCommandRegistry.exe --run vsm.m07-smoke-pipeline --json
bin\CommandRegistryGui.exe --headless-smoke bin\VisualStateCommandRegistry.exe --smoke-command vsm.logic-self-test
```

