# Aria Package AGENTS Guide

## Purpose
This package contains the C++ conversion of the Python Aria software for browser automation.

## Structure
The package follows the structure of the original Python Aria software with adaptations for the U++ framework.

## Key Components
- Aria: Main application class
- CredentialManager: Handles credential management
- Logger: Logging functionality
- Navigator: Navigation and browsing functionality
- PluginManager: Plugin management
- ReportManager: Reporting functionality
- SafetyManager: Safety and security features
- ScriptManager: Script execution
- SiteManager: Site-specific functionality
- UndetectedAria/UndetectedFirefox: Anti-detection features

## Sites
- Calendar: Google Calendar integration
- Discord: Discord messaging platform
- GoogleMessages: Google Messages integration
- Threads: Threads messaging platform
- Whatsapp: WhatsApp integration
- YoutubeStudio: YouTube Studio management

## Conventions
- All classes are wrapped in the UPP namespace
- Follow U++ naming conventions
- Use U++ types where appropriate

## Dependencies
- Core: Basic U++ functionality
- WebDriver: Browser automation capabilities