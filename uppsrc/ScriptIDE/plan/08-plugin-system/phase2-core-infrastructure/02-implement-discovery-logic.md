# Task: Implement Discovery Logic

## Goal
Automate the process of finding and registering plugins.

## Strategy
1. **Directory Scanning**: Scan a designated `plugins/` directory for metadata.
2. **Dynamic Loading (Future)**: While initially supporting compiled-in plugins, design the system to eventually support DLL/shared library loading.
3. **Internal Registration**: Provide a macro or function for internal (static) plugins to register during application startup.

## Success Criteria
- [ ] Plugins in the search path are automatically identified.
- [ ] Registered plugins appear in the `PluginRegistry`.
- [ ] System handles missing or malformed plugins gracefully.
