# Task: Implement Plots Pane

## Goal
Implement the `PlotsPane` to display graphical output from the Python runtime, fulfilling the specification in `spyder/GUI.md`. It must handle multiple figures and allow saving/copying images.

## U++ Widget Mapping
- **Container**: `DockableCtrl`.
- **Display**: `ImageCtrl` or `ScrollMirror` for large images.
- **Navigation**: `ToolBar` for switching between multiple plot figures.

## Interface Definition (PlotsPane.h)

```cpp
class PlotsPane : public ParentCtrl {
public:
    typedef PlotsPane CLASSNAME;
    PlotsPane();

    void AddPlot(const Image& img);
    void Clear();

private:
    ToolBar toolbar;
    ImageCtrl display;
    Array<Image> plots;
    int current_index = -1;

    void UpdateDisplay();
    void SaveSelected();
    void CopySelected();
    void PrevPlot();
    void NextPlot();
    void LayoutMenu(Bar& bar);
};
```

## Implementation Steps

### 1. UI Layout
- Create a `ToolBar` at the top.
- Add "Previous", "Next", "Save", "Clear" buttons.
- Add the `ImageCtrl` to the remaining area using `SizePos()`.
- Set `ImageCtrl` to "Keep Aspect Ratio".

### 2. Plot Storage
- Use `Array<Image>` to store the history of plots generated during the session.
- When `AddPlot` is called, append to the array and switch `current_index` to the new plot.

### 3. Runtime Export (Internals)
- The `ByteVM` needs a way to "emit" an image.
- Since we are not using a full Matplotlib yet, create a bridge where a script can call a builtin function `gui_plot(data)` which transfers an image buffer to the UI.

### 4. Image Operations
- **Save**: Open `FileSel` and save the `Image` as PNG.
- **Copy**: Use `WriteClipboardImage(plots[current_index])`.

## Success Criteria
- [ ] A toolbar is visible with navigation buttons.
- [ ] Images sent from the runtime appear in the pane.
- [ ] Can cycle through multiple plots using "Prev/Next".
- [ ] "Save" button successfully exports the image to disk.
- [ ] "Clear" removes all stored plots.
