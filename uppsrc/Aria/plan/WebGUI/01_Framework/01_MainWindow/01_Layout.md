# Task: Main Window Container
# Status: PENDING

## Objective
Create the primary application window hosting the tabbed interface for different services.

## Requirements
- **TopWindow**: Derive `AriaMainWindow` from `TopWindow`.
- **Navigation**: Implement `TabCtrl` as the central widget to switch between services (Discord, WhatsApp, etc.).
- **Toolbar**: Add a global toolbar for "Refresh All", "Settings", and "Stop Scrapers".
- **Status Bar**: Display global connection status and AI provider latency.
- **Service Registration**: Mechanism to dynamically add tabs based on enabled plugins/scrapers.
