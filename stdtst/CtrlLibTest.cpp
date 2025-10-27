// Test for stdsrc CtrlLib functionality

#include "CtrlLib/CtrlLib.h"

using namespace Upp;

CONSOLE_APP_MAIN {
    // Test Button functionality
    {
        Button btn;
        
        // Test initial state
        ASSERT(btn.GetLabel() == "Button");
        ASSERT(btn.GetText() == "Button");
        ASSERT(!btn.IsPressed());
        ASSERT(!btn.IsPush());
        
        // Test label setting
        btn.Label("Click Me");
        ASSERT(btn.GetLabel() == "Click Me");
        
        btn.SetLabel("Submit");
        ASSERT(btn.GetLabel() == "Submit");
        
        // Test pressed state
        btn.SetPressed(true);
        ASSERT(btn.IsPressed());
        ASSERT(btn.IsPush());
        
        btn.SetPressed(false);
        ASSERT(!btn.IsPressed());
        ASSERT(!btn.IsPush());
        
        // Test toggle functionality
        btn.Toggle(true);
        ASSERT(btn.IsPressed());
        
        // Test standard button types
        Button okBtn;
        okBtn.OK();
        ASSERT(okBtn.GetLabel() == "OK");
        
        Button cancelBtn;
        cancelBtn.Cancel();
        ASSERT(cancelBtn.GetLabel() == "Cancel");
        
        Button yesBtn;
        yesBtn.Yes();
        ASSERT(yesBtn.GetLabel() == "Yes");
        
        Button noBtn;
        noBtn.No();
        ASSERT(noBtn.GetLabel() == "No");
        
        // Test class name
        ASSERT(std::string(btn.GetClassName()) == "Button");
        
        RLOG("Button tests passed!");
    }
    
    // Test Label functionality
    {
        Label lbl;
        
        // Test initial state
        ASSERT(lbl.GetLabel() == "Label");
        ASSERT(lbl.GetText() == "Label");
        ASSERT(lbl.GetAlign() == 0);  // Left aligned by default
        
        // Test label setting
        lbl.Label("Sample Label");
        ASSERT(lbl.GetLabel() == "Sample Label");
        
        lbl.SetLabel("Another Label");
        ASSERT(lbl.GetLabel() == "Another Label");
        
        // Test font
        Font initialFont = lbl.GetFont();
        ASSERT(initialFont.GetFace() == "Arial");
        ASSERT(initialFont.GetHeight() == 12);
        
        Font newFont = Font::Arial(16);
        lbl.SetFont(newFont);
        ASSERT(lbl.GetFont().GetHeight() == 16);
        
        lbl.Font(Font::Arial(14));
        ASSERT(lbl.GetFont().GetHeight() == 14);
        
        // Test text color
        Color black = Color::Black();
        Color initialColor = lbl.GetTextColor();
        ASSERT(initialColor.GetR() == black.GetR());
        ASSERT(initialColor.GetG() == black.GetG());
        ASSERT(initialColor.GetB() == black.GetB());
        
        Color red(255, 0, 0);
        lbl.SetTextColor(red);
        ASSERT(lbl.GetTextColor().GetR() == 255);
        ASSERT(lbl.GetTextColor().GetG() == 0);
        ASSERT(lbl.GetTextColor().GetB() == 0);
        
        lbl.TextColor(black);
        ASSERT(lbl.GetTextColor().GetR() == black.GetR());
        
        // Test alignment
        lbl.SetAlign(1);  // Center
        ASSERT(lbl.GetAlign() == 1);
        
        lbl.Left();
        ASSERT(lbl.GetAlign() == 0);
        
        lbl.Center();
        ASSERT(lbl.GetAlign() == 1);
        
        lbl.Right();
        ASSERT(lbl.GetAlign() == 2);
        
        // Test class name
        ASSERT(std::string(lbl.GetClassName()) == "Label");
        
        // Test data operations
        lbl.SetData("Data Label");
        ASSERT(lbl.GetData() == "Data Label");
        
        RLOG("Label tests passed!");
    }
    
    // Test EditField functionality
    {
        EditField edit;
        
        // Test initial state
        ASSERT(edit.GetText() == "");
        ASSERT(edit.GetData() == "");
        ASSERT(!edit.IsPassword());
        ASSERT(edit.GetPrompt() == "");
        
        // Test text operations
        edit.Text("Hello World");
        ASSERT(edit.GetText() == "Hello World");
        
        edit.SetText("New Text");
        ASSERT(edit.GetText() == "New Text");
        
        edit.Clear();
        ASSERT(edit.GetText() == "");
        ASSERT(edit.IsEmpty());
        
        // Test prompt
        edit.Prompt("Enter text here");
        ASSERT(edit.GetPrompt() == "Enter text here");
        
        // Test password mode
        edit.Password();
        ASSERT(edit.IsPassword());
        
        edit.SetText("secret");
        ASSERT(edit.GetText() == "secret");  // But would render as asterisks
        
        // Test font
        ASSERT(edit.GetFont().GetFace() == "Arial");
        Font newFont = Font::Arial(15);
        edit.SetFont(newFont);
        ASSERT(edit.GetFont().GetHeight() == 15);
        
        edit.Font(Font::Arial(10));
        ASSERT(edit.GetFont().GetHeight() == 10);
        
        // Test text color
        Color black = Color::Black();
        ASSERT(edit.GetTextColor().GetR() == black.GetR());
        
        Color blue(0, 0, 255);
        edit.SetTextColor(blue);
        ASSERT(edit.GetTextColor().GetB() == 255);
        
        // Test cursor operations
        edit.SetText("Testing");
        ASSERT(edit.GetLength() == 7);
        ASSERT(edit.GetCursor() == 0);  // Should be at start after SetText
        
        edit.SetCursor(5);
        ASSERT(edit.GetCursor() == 5);
        
        // Test text insertion
        edit.InsertText("123");
        ASSERT(edit.GetText() == "Test123ing");
        
        // Test selection operations
        edit.SelectAll();
        // Selection is set but actual implementation would need more complex handling
        
        // Test class name
        ASSERT(std::string(edit.GetClassName()) == "EditField");
        
        // Test data operations
        edit.SetData("Data Input");
        ASSERT(edit.GetData() == "Data Input");
        
        RLOG("EditField tests passed!");
    }
    
    // Test ArrayCtrl functionality
    {
        ArrayCtrl array;
        
        // Test initial state
        ASSERT(array.GetCount() == 0);
        ASSERT(array.GetColumnCount() == 0);
        
        // Test column operations
        array.AddColumn("Name", 100);
        array.AddColumn("Age", 50);
        array.AddColumn("City", 150);
        
        ASSERT(array.GetColumnCount() == 3);
        ASSERT(array.GetCount() == 0);
        
        // Test adding rows
        int row1 = array.Add();
        ASSERT(row1 == 0);
        ASSERT(array.GetCount() == 1);
        
        std::vector<std::string> rowData = {"John Doe", "30", "New York"};
        int row2 = array.Add(rowData);
        ASSERT(row2 == 1);
        ASSERT(array.GetCount() == 2);
        
        // Test setting values
        array.Set(0, 0, "Jane Smith");
        array.Set(0, 1, "25");
        array.Set(0, 2, "Los Angeles");
        
        ASSERT(array.Get(0, 0) == "Jane Smith");
        ASSERT(array.Get(0, 1) == "25");
        ASSERT(array.Get(0, 2) == "Los Angeles");
        
        // Set entire row
        std::vector<std::string> newRow = {"Bob Johnson", "35", "Chicago"};
        array.Set(2, newRow);  // This will create row 2 if it doesn't exist
        ASSERT(array.Get(2, 0) == "Bob Johnson");
        ASSERT(array.Get(2, 1) == "35");
        ASSERT(array.Get(2, 2) == "Chicago");
        
        // Test selection
        array.Set(1);  // Select row 1
        ASSERT(array.Get() == 1);  // Get selected row
        
        array.ClearSelection();
        ASSERT(array.Get() == -1);
        
        // Test column operations
        array.ColumnWidth(1, 75);  // Set Age column width to 75
        // Width is stored but not easily tested without getter
        
        array.ColumnAlign(0, 1);  // Center-align Name column
        // Alignment is stored but not easily tested without getter
        
        // Test data operations
        std::vector<std::vector<std::string>> new_data = {
            {"Alice", "28", "Miami"},
            {"Charlie", "45", "Seattle"}
        };
        array.SetData(new_data);
        ASSERT(array.GetCount() == 2);
        ASSERT(array.Get(0, 0) == "Alice");
        ASSERT(array.Get(1, 2) == "Seattle");
        
        std::vector<std::vector<std::string>> retrieved_data = array.GetData();
        ASSERT(retrieved_data.size() == 2);
        ASSERT(retrieved_data[0][1] == "28");
        
        // Test clearing
        array.Clear();
        ASSERT(array.GetCount() == 0);
        ASSERT(array.Get() == -1);  // No selection after clear
        
        // Add some data again
        array.AddColumn("ID", 50);
        array.AddColumn("Value", 100);
        array.Add({"1", "First Value"});
        array.Add({"2", "Second Value"});
        array.Add({"3", "Third Value"});
        
        ASSERT(array.GetCount() == 3);
        ASSERT(array.GetColumnCount() == 2);
        ASSERT(array.Get(1, 0) == "2");
        ASSERT(array.Get(2, 1) == "Third Value");
        
        // Test class name
        ASSERT(std::string(array.GetClassName()) == "ArrayCtrl");
        
        RLOG("ArrayCtrl tests passed!");
    }
    
    RLOG("All CtrlLib tests passed successfully!");
}