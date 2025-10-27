// U++-compatible ArrayCtrl wrapper for table-like controls
// This header is aggregated and wrapped into namespace Upp by CtrlLib.h

class ArrayCtrl : public CtrlBase {
private:
    struct Column {
        std::string title;
        int width;
        int align;  // 0=left, 1=center, 2=right
        bool visible;
        
        Column(const std::string& t = "", int w = 100, int a = 0, bool v = true)
            : title(t), width(w), align(a), visible(v) {}
    };
    
    std::vector<Column> columns;
    std::vector<std::vector<std::string>> data;  // Rows of columns
    std::vector<int> row_heights;  // Height of each row
    int header_height;
    int row_height;
    int top_row;      // First visible row
    int visible_rows; // Number of visible rows
    int selection;    // Selected row index (-1 if none)
    int sort_column;  // Column to sort by (-1 if none)
    bool ascending;   // Sort direction

public:
    // Constructors
    ArrayCtrl() : CtrlBase(), header_height(24), row_height(20), 
                  top_row(0), visible_rows(10), selection(-1), 
                  sort_column(-1), ascending(true) {
        // Default size for an array control
        SetSize(400, 200);
    }

    // U++-style static constructors
    static ArrayCtrl* Create() { return new ArrayCtrl(); }

    // U++-style column operations
    ArrayCtrl& AddColumn(const std::string& title, int width = 100) { 
        columns.emplace_back(title, width);
        // Ensure all rows have this column
        for (auto& row : data) {
            if (row.size() < columns.size()) {
                row.resize(columns.size());
            }
        }
        return *this; 
    }

    ArrayCtrl& ColumnWidth(int column, int width) {
        if (column >= 0 && column < static_cast<int>(columns.size())) {
            columns[column].width = width;
        }
        return *this;
    }

    ArrayCtrl& ColumnAlign(int column, int align) {
        if (column >= 0 && column < static_cast<int>(columns.size())) {
            columns[column].align = align;
        }
        return *this;
    }

    // U++-style row operations
    int Add(const std::vector<std::string>& row_data) {
        std::vector<std::string> row = row_data;
        // Ensure row has enough columns
        if (row.size() < columns.size()) {
            row.resize(columns.size());
        }
        data.push_back(row);
        row_heights.push_back(row_height);
        return static_cast<int>(data.size() - 1);
    }

    int Add() {  // Add empty row
        std::vector<std::string> empty_row(columns.size());
        return Add(empty_row);
    }

    ArrayCtrl& Set(int row, int column, const std::string& value) {
        if (row >= 0 && row < static_cast<int>(data.size()) && 
            column >= 0 && column < static_cast<int>(columns.size())) {
            data[row][column] = value;
        }
        return *this;
    }

    ArrayCtrl& Set(int row, const std::vector<std::string>& values) {
        if (row >= 0 && row < static_cast<int>(data.size())) {
            // Ensure we don't exceed column count
            int cols = static_cast<int>(std::min(values.size(), columns.size()));
            for (int i = 0; i < cols; i++) {
                data[row][i] = values[i];
            }
        }
        return *this;
    }

    std::string Get(int row, int column) const {
        if (row >= 0 && row < static_cast<int>(data.size()) && 
            column >= 0 && column < static_cast<int>(columns.size())) {
            return data[row][column];
        }
        return "";
    }

    // U++-style selection operations
    ArrayCtrl& Set(int index) {  // Select row
        if (index >= 0 && index < static_cast<int>(data.size())) {
            selection = index;
        }
        return *this;
    }
    
    int Get() const { return selection; }  // Get selected row
    
    ArrayCtrl& ClearSelection() {
        selection = -1;
        return *this;
    }

    // U++-style painting
    void Paint(Draw& draw) const override {
        if (!IsVisible()) return;

        Rect r = GetRect();
        
        // Draw background
        draw.DrawRect(r, GetBackgroundColor());

        // Calculate visible area
        int current_y = r.GetTop() + header_height;
        int rows_to_draw = std::min(visible_rows, 
                                   static_cast<int>(data.size()) - top_row);
        
        // Draw header
        int current_x = r.GetLeft();
        for (const auto& col : columns) {
            if (col.visible) {
                Rect header_rect(current_x, r.GetTop(), 
                                current_x + col.width, 
                                r.GetTop() + header_height);
                
                // Draw header background
                draw.DrawRect(header_rect, Color(200, 200, 200), Color(200, 200, 200));
                
                // Draw header text
                // In a real implementation, this would draw the column title
                // draw.DrawText(header_rect.GetTopLeft() + Point(5, 3), 
                //               col.title, Font::Arial(10), Color::Black());
                
                // Draw header border
                draw.DrawRect(header_rect, Color::Black());
                
                current_x += col.width;
            }
        }

        // Draw rows
        for (int i = 0; i < rows_to_draw; i++) {
            int row_idx = top_row + i;
            if (row_idx >= static_cast<int>(data.size())) break;
            
            bool is_selected = (selection == row_idx);
            Color row_color = is_selected ? Color(180, 180, 255) : 
                             (i % 2 == 0 ? GetBackgroundColor() : Color(240, 240, 240));
            
            Rect row_rect(r.GetLeft(), current_y, r.GetRight(), 
                         current_y + row_heights[row_idx]);
            
            // Draw row background
            draw.DrawRect(row_rect, row_color);
            
            // Draw row content
            current_x = r.GetLeft();
            for (int j = 0; j < static_cast<int>(columns.size()) && j < static_cast<int>(data[row_idx].size()); j++) {
                if (columns[j].visible) {
                    Rect cell_rect(current_x, current_y, 
                                  current_x + columns[j].width, 
                                  current_y + row_heights[row_idx]);
                    
                    // Draw cell content
                    if (!data[row_idx][j].empty()) {
                        // In a real implementation, this would draw the cell content
                        // draw.DrawText(cell_rect.GetTopLeft() + Point(5, 2), 
                        //               data[row_idx][j], Font(), Color::Black());
                    }
                    
                    // Draw cell border
                    draw.DrawRect(cell_rect, Color(200, 200, 200));
                    
                    current_x += columns[j].width;
                }
            }
            
            current_y += row_heights[row_idx];
        }
        
        // Draw border around the entire control
        draw.DrawRect(r, Color::Black());
    }

    // U++-style operations
    int GetCount() const { return static_cast<int>(data.size()); }
    int GetColumnCount() const { return static_cast<int>(columns.size()); }
    
    ArrayCtrl& Clear() {
        data.clear();
        row_heights.clear();
        selection = -1;
        return *this;
    }
    
    ArrayCtrl& NoHeader() {
        header_height = 0;
        return *this;
    }
    
    ArrayCtrl& NoGrid() {
        // In a real implementation, this would hide grid lines
        return *this;
    }
    
    ArrayCtrl& NoHScroll() {
        // In a real implementation, this would disable horizontal scrolling
        return *this;
    }
    
    ArrayCtrl& NoVScroll() {
        // In a real implementation, this would disable vertical scrolling
        return *this;
    }
    
    ArrayCtrl& SingleSelect() {
        // In a real implementation, this would configure for single selection
        return *this;
    }
    
    ArrayCtrl& MultiSelect() {
        // In a real implementation, this would configure for multiple selection
        return *this;
    }

    // U++-style sorting
    ArrayCtrl& SortBy(int column, bool ascending = true) {
        if (column >= 0 && column < static_cast<int>(columns.size())) {
            sort_column = column;
            this->ascending = ascending;
            // In a real implementation, this would perform the actual sort
        }
        return *this;
    }

    // U++-style size adjustment
    ArrayCtrl& SizePos() {
        // In a real implementation, this would size based on content
        return *this;
    }
    
    ArrayCtrl& SetRow(int row, int height) {
        if (row >= 0 && row < static_cast<int>(row_heights.size())) {
            row_heights[row] = height;
        } else if (row >= 0 && row < static_cast<int>(data.size())) {
            // Extend row_heights if needed
            row_heights.resize(data.size(), row_height);
            row_heights[row] = height;
        }
        return *this;
    }

    // U++-style data operations
    ArrayCtrl& SetData(const std::vector<std::vector<std::string>>& new_data) {
        data = new_data;
        row_heights.resize(data.size(), row_height);
        return *this;
    }
    
    std::vector<std::vector<std::string>> GetData() const {
        return data;
    }

    // U++-style methods for identifying control types
    const char* GetClassName() const override { return "ArrayCtrl"; }
};