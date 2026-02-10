# Task: Abstract Service Controller
# Status: DONE

## Objective
Define the base interface for all service UI implementations to ensure consistency.

## Requirements
- **Base Class**: Create `ServiceCtrl` inheriting from `ParentCtrl`.
- **Virtual Interface**:
    - `virtual void RefreshData()`: Triggers data reload from the scraper.
    - `virtual String GetTitle()`: Returns tab title (e.g., "Discord (3)").
    - `virtual Image GetIcon()`: Returns service icon.
    - `virtual void UpdateUI()`: Called on timer to refresh visual state without scraping.
- **Data Binding**: Standardize how controllers access `SiteManager` data.
