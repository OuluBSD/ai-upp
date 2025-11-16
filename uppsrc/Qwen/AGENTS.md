# Qwen Agents Documentation

## Overview

The Qwen system implements a powerful AI-assisted development workflow tool that is adapted from the Google Gemini CLI, and specifically optimized for Qwen3-Coder models. It provides advanced code understanding, automated tasks, and intelligent assistance for developers.

## Architecture Components

### Core Files
- `QwenClient.h` - Client interface for communicating with qwen-code executable
- `QwenClient.cpp` - Implementation of client communication with qwen-code process
- `QwenProtocol.h/cpp` - Structured data protocol for qwen-code integration
- `QwenStateManager.h/cpp` - Session/state management for conversations
- `QwenManager.h/cpp` - Multi-session management and TCP connections
- `QwenTCPServer.h/cpp` - TCP server that spawns qwen-code and proxies connections
- `CmdQwen.h/cpp` - Command-line interface implementation with ncurses UI
- `QwenLogger.h` - Fine-grained timestamp logger for debugging
- `VfsBootQwenTCPServer.h/cpp` - VFS-integrated TCP server wrapper

### Key Features
- Interactive AI assistant powered by qwen-code
- NCurses-based terminal interface with permission system
- Support for multiple AI providers (qwen-openai, qwen-auth)
- Session management with save/load capabilities
- TCP client/server mode for remote connections
- Tool approval workflow with permission modes

## Permission System

The Qwen system implements a sophisticated permission system with the following modes:
- PLAN - Plan before executing
- NORMAL - Ask for approval on every tool
- AUTO-EDIT - Auto-approve Edit/Write tools only
- YOLO - Approve anything, no sandbox restrictions

## Manager Mode

Advanced manager mode enables:
- Multi-repository management
- WORKER→MANAGER escalation workflows
- Automatic test-driven workflows
- Session management with snapshots and grouping

## Communication Protocol

The system uses a structured data protocol that provides a thick client interface for receiving semantic data from qwen-code, defined in qwenStateSerializer.ts.

### TCP Server Mode

The QwenTCPServer provides a network-accessible interface to qwen-code:

1. **Process Management**: Spawns and manages the qwen-code subprocess
   - Communicates via stdin/stdout pipes with qwen-code
   - Handles process lifecycle (start, stop, restart)

2. **Network Interface**: Accepts TCP connections from clients
   - Listens on configurable port (default: 7774)
   - Routes user input from TCP clients to qwen-code subprocess
   - Broadcasts AI responses from qwen-code to connected clients

3. **Message Flow**:
   ```
   TCP Client → QwenTCPServer → qwen-code subprocess
                                    ↓
   TCP Client ← QwenTCPServer ← AI Response
   ```

4. **qwen-code Integration**:
   - Uses wrapper script at `script/qwen-code` to launch the Node.js qwen-code server
   - The wrapper script locates the qwen-code bundle (typically `~/Dev/qwen-code/dist/src/gemini.js`)
   - Communicates using JSON-based protocol over stdin/stdout

### Server Scripts

- `script/run_qwen_server.sh` - Start TCP server on port 7774
- `script/run_qwen_client.sh` - Connect to TCP server
- `script/qwen-code` - Wrapper to launch qwen-code Node.js bundle

## Command-Line Interface

The system supports various command-line options:
- `--attach <id>` - Attach to existing session
- `--list-sessions` - List all sessions
- `--simple` - Force stdio mode instead of ncurses
- `--openai` - Use OpenAI provider instead of default
- `--manager, -m` - Enable manager mode

## VFS Integration

The system integrates with Virtual File System (VFS) for:
- Persistent session storage
- Conversation history management
- File context sharing with AI
- Workspace management

## Dependencies

- libncurses - For terminal user interface
- VfsShell - For file system integration
- OpenSSL - For SSL/TLS support in some configurations

## Console Application Support

The system can be compiled as a console application with CONSOLE_APP_MAIN when the flagMAIN flag is defined, allowing for headless operation without the ncurses interface.

## Authentication Options

The system supports multiple authentication methods:
- Qwen OAuth (recommended) - 2,000 requests per day
- OpenAI-compatible API providers
- Regional free tiers (ModelScope, OpenRouter)

## Development Notes

Based on the grep results in the codebase, the system is designed as a workflow automation tool that can handle complex development tasks, code refactoring, and project analysis through intelligent AI interaction.