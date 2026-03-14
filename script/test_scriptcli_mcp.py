#!/usr/bin/env python3
import json
import subprocess
import sys
import tempfile
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent
SCRIPTCLI_BIN = REPO_ROOT / "bin" / "ScriptCLI"


def run_command(cmd, description):
    print(f"--- {description} ---")
    print(f"Executing: {' '.join(str(c) for c in cmd)}")
    result = subprocess.run(cmd, cwd=REPO_ROOT)
    if result.returncode != 0:
        print(f"Error: {description} failed with exit code {result.returncode}")
        sys.exit(result.returncode)


def send_requests(workspace, lines):
    proc = subprocess.Popen(
        [str(SCRIPTCLI_BIN), "mcp", "serve", "--workspace", str(workspace)],
        cwd=REPO_ROOT,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    stdout, stderr = proc.communicate("".join(line + "\n" for line in lines))
    if proc.returncode != 0:
        print("MCP host stderr:")
        print(stderr)
        raise RuntimeError(f"ScriptCLI mcp serve failed with exit code {proc.returncode}")
    out_lines = []
    for line in stdout.splitlines():
        line = line.strip()
        if not line:
            continue
        if line.startswith("{") or line.startswith("["):
            out_lines.append(line)
    return [json.loads(line) for line in out_lines]


def assert_true(condition, message):
    if not condition:
        raise AssertionError(message)


def assert_error(resp, code):
    err = resp.get("error")
    assert_true(isinstance(err, dict), f"Expected error response, got: {resp}")
    assert_true(err.get("code") == code, f"Expected error code {code}, got: {resp}")


def main():
    run_command([sys.executable, "script/build.py", "-mc", "1", "-j12", "ScriptCLI"], "Building ScriptCLI (USEMALLOC)")

    if not SCRIPTCLI_BIN.exists():
        print(f"Error: {SCRIPTCLI_BIN} not found after build")
        sys.exit(1)

    with tempfile.TemporaryDirectory(prefix="scriptcli-mcp-") as tmp:
        workspace = Path(tmp)
        ok_file = workspace / "ok.py"
        bad_file = workspace / "bad.py"
        ok_file.write_text("print('hello from mcp')\n", encoding="utf-8")
        bad_file.write_text("def broken(:\n", encoding="utf-8")

        responses = send_requests(
            workspace,
            [
                '{"jsonrpc":"2.0","id":1,"method":"mcp.ping"}',
                '{"jsonrpc":"2.0","id":2,"method":"mcp.capabilities"}',
                '{"jsonrpc":"2.0","id":3,"method":"workspace.info"}',
                '{"jsonrpc":"2.0","id":4,"method":"script.run","params":{"path":"ok.py"}}',
                '{"jsonrpc":"2.0","id":5,"method":"script.lint","params":{"path":"ok.py"}}',
                '{"jsonrpc":"2.0","id":6,"method":"script.lint","params":{"path":"bad.py"}}',
                '{"jsonrpc":"2.0","id":7,"method":"plugin.list"}',
                '{"jsonrpc":"2.0","id":8,"method":"missing.method"}',
                '{"jsonrpc":"2.0","id":9,"method":"script.run","params":{}}',
                '{"jsonrpc":"2.0","id":10,"method":"plugin.test","params":{"plugin_id":"does-not-exist"}}',
                'not-json',
            ],
        )

        assert_true(len(responses) == 11, f"Expected 11 responses, got {len(responses)}")

        assert_true(responses[0]["result"]["text"] == "pong", f"Unexpected ping response: {responses[0]}")
        caps = responses[1]["result"]
        assert_true(caps["surface"] == "scriptcli-mcp", f"Unexpected capabilities: {responses[1]}")
        assert_true("plugin.test" in caps["methods"], f"Missing plugin.test in capabilities: {responses[1]}")
        assert_true(responses[2]["result"]["workspace"] == str(workspace), f"Unexpected workspace.info: {responses[2]}")

        run_result = responses[3]["result"]
        assert_true(run_result["ok"] is True, f"script.run failed: {responses[3]}")
        assert_true(run_result["kind"] == "python", f"Unexpected script.run kind: {responses[3]}")

        lint_ok = responses[4]["result"]
        assert_true(lint_ok["ok"] is True and lint_ok["issues"] == [], f"Unexpected clean lint result: {responses[4]}")

        lint_bad = responses[5]["result"]
        assert_true(lint_bad["ok"] is False, f"Expected lint failure for bad.py: {responses[5]}")
        assert_true(len(lint_bad["issues"]) >= 1, f"Expected at least one lint issue: {responses[5]}")

        plugins = responses[6]["result"]["plugins"]
        assert_true(any(p["id"] == "GameStatePlugin" for p in plugins), f"Missing GameStatePlugin: {responses[6]}")

        assert_error(responses[7], -32601)
        assert_error(responses[8], -32602)
        assert_error(responses[9], -32602)
        assert_error(responses[10], -32700)

    print("--- MCP smoke/regression summary ---")
    print("ScriptCLI MCP stdio contract looks healthy.")


if __name__ == "__main__":
    main()
