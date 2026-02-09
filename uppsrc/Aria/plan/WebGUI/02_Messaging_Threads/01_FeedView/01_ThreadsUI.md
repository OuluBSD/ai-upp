# Task: Threads UI Integration
# Status: IN_PROGRESS

## Objective
Implement a comprehensive tabbed interface for Instagram Threads.

## Requirements
- **Structure**: `TabCtrl` hosting four sub-views:
    1.  **Feed**: Main timeline feed (Read-only for now).
    2.  **Public Messages**: User's own posts and replies.
    3.  **Private Messages**: Direct messages (if accessible).
    4.  **Settings**: User-specific configuration (e.g., scrape depth).

## Widgets
- **Feed/Public/Private**: `ArrayCtrl` with "Author" and "Content" columns.
- **Settings**: `ParentCtrl` with configuration options.