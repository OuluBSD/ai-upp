# Task: Gemini Session Caching
# Status: DONE

## Objective
Optimize `CliMaestroEngine::ListSessions` for Gemini by implementing persistent caching and background updates.

## Requirements
- Define `GeminiSessionCache` struct with `Visit` serialization.
- Load cache from `ConfigFile("sessions.json")` on startup.
- Serve cached sessions immediately in `ListSessions`.
- Launch a background thread (`Thread::Start`) to run `gemini --list-sessions`.
- Update the cache and file upon completion, using `Mutex` for thread safety.
- Trigger a UI refresh callback when the background update completes.
