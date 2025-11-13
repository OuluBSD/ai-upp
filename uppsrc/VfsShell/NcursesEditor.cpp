#include "NcursesEditor.h"
#include <ncurses.h>
#include <panel.h>
#include <form.h>
#include <menu.h>
#include <string.h>
#include <stdlib.h>

NAMESPACE_UPP

bool NcursesEditor::fileModified = false;
String NcursesEditor::currentPath = "";

// Save file function implementation
static bool SaveFile(const String& path, const String& content) {
    Upp::FileOut out(path);
    if (out.IsOpen()) {
        out.Put(content);
        return !out.IsError();
    }
    return false;
}

// A simple ncurses-based text editor for VfsShell
bool NcursesEditor::RunEditor(const String& vfsPath, const String& initialContent) {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, FALSE);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // Colors (if supported)
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_BLUE, COLOR_BLACK);   // Title bar
        init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Status bar
        init_pair(3, COLOR_CYAN, COLOR_BLACK);   // Line numbers
        init_pair(4, COLOR_RED, COLOR_BLACK);    // Modified indicator
    }

    // Convert initial content to lines
    Vector<String> lines;
    if (!initialContent.IsEmpty()) {
        lines = Split(initialContent, "\n", false); // Don't omit empty lines
    } else {
        lines.Add(""); // Start with one empty line
    }

    // Ensure at least one line exists
    if (lines.IsEmpty()) {
        lines.Add("");
    }

    int current_line = 0;
    int top_line = 0;
    int cursor_x = 0;
    bool file_exists = !initialContent.IsEmpty();
    bool editor_active = true;
    int content_height = rows - 4;  // Leave space for status bars

    // Main editor loop
    while (editor_active) {
        // Clear screen
        clear();

        // Draw title bar
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(0, 0, "VfsShell Text Editor - %s", vfsPath.ToStd().c_str());
        attroff(COLOR_PAIR(1) | A_BOLD);

        // Draw separator
        mvhline(1, 0, '-', cols);

        // Draw content area
        for (int i = 0; i < content_height && (top_line + i) < lines.GetCount(); ++i) {
            int line_idx = top_line + i;
            int screen_row = i + 2;

            // Line number
            attron(COLOR_PAIR(3));
            mvprintw(screen_row, 0, "%3d:", line_idx + 1);
            attroff(COLOR_PAIR(3));

            // Line content
            if (line_idx < lines.GetCount()) {
                String display_line = lines[line_idx];
                // Truncate if too long
                if (display_line.GetCount() > cols - 6) {
                    display_line = display_line.Mid(0, cols - 9);
                    display_line << "...";
                }
                mvprintw(screen_row, 5, "%s", display_line.ToStd().c_str());
            }
        }

        // Fill remaining lines with tildes
        for (int i = lines.GetCount() - top_line; i < content_height; ++i) {
            int screen_row = i + 2;
            if (screen_row < rows - 2) {
                mvprintw(screen_row, 0, "~");
            }
        }

        // Draw separator
        mvhline(rows - 2, 0, '-', cols);

        // Draw status bar
        attron(COLOR_PAIR(2));
        mvprintw(rows - 1, 0, "Line:%d/%d Col:%d | %s%s | :w (save) :q (quit) :wq (save&quit)",
                current_line + 1, lines.GetCount(), cursor_x,
                fileModified ? "[Modified] " : "",
                !file_exists ? "[New File] " : "");
        attroff(COLOR_PAIR(2));

        // Position cursor
        if (current_line >= top_line && current_line < top_line + content_height) {
            move(current_line - top_line + 2, min(cursor_x + 5, cols - 1));
        }

        // Refresh screen
        refresh();

        // Get user input
        int ch = getch();

        // Process input
        switch (ch) {
            case KEY_UP:
                if (current_line > 0) {
                    current_line--;
                    if (current_line < top_line) {
                        top_line = current_line;
                    }
                    // Adjust cursor_x if needed
                    if (cursor_x > lines[current_line].GetCount()) {
                        cursor_x = lines[current_line].GetCount();
                    }
                }
                break;

            case KEY_DOWN:
                if (current_line < lines.GetCount() - 1) {
                    current_line++;
                    if (current_line >= top_line + content_height) {
                        top_line = current_line - content_height + 1;
                    }
                    // Adjust cursor_x if needed
                    if (cursor_x > lines[current_line].GetCount()) {
                        cursor_x = lines[current_line].GetCount();
                    }
                }
                break;

            case KEY_LEFT:
                if (cursor_x > 0) {
                    cursor_x--;
                } else if (current_line > 0) {
                    // Move to end of previous line
                    current_line--;
                    if (current_line < top_line) {
                        top_line = current_line;
                    }
                    cursor_x = lines[current_line].GetCount();
                }
                break;

            case KEY_RIGHT:
                if (cursor_x < lines[current_line].GetCount()) {
                    cursor_x++;
                } else if (current_line < lines.GetCount() - 1) {
                    // Move to beginning of next line
                    current_line++;
                    if (current_line >= top_line + content_height) {
                        top_line = current_line - content_height + 1;
                    }
                    cursor_x = 0;
                }
                break;

            case KEY_BACKSPACE:
            case 127:  // Delete key
            case '\b':
                if (cursor_x > 0) {
                    // Remove character at cursor_x - 1
                    lines[current_line].Remove(cursor_x - 1, 1);
                    cursor_x--;
                    fileModified = true;
                } else if (current_line > 0) {
                    // Join with previous line
                    String current_content = lines[current_line];
                    lines.Remove(current_line);  // Remove current line
                    current_line--;
                    cursor_x = lines[current_line].GetCount();
                    lines[current_line] << current_content;
                    fileModified = true;

                    if (current_line < top_line) {
                        top_line = current_line;
                    }
                }
                break;

            case KEY_DC:  // Delete key
                if (cursor_x < lines[current_line].GetCount()) {
                    lines[current_line].Remove(cursor_x, 1);
                    fileModified = true;
                } else if (current_line < lines.GetCount() - 1) {
                    // Join with next line
                    String next_content = lines[current_line + 1];
                    lines[current_line] << next_content;
                    lines.Remove(current_line + 1);
                    fileModified = true;
                }
                break;

            case KEY_ENTER:
            case '\n':
            case '\r': {
                // Split line at cursor position
                String new_line = lines[current_line].Mid(cursor_x);
                lines[current_line] = lines[current_line].Mid(0, cursor_x);
                lines.Insert(current_line + 1, new_line);
                current_line++;
                cursor_x = 0;
                fileModified = true;

                if (current_line >= top_line + content_height) {
                    top_line = current_line - content_height + 1;
                }
                break;
            }

            case 27:  // ESC key - command mode
                {
                    // Show command prompt at bottom
                    move(rows - 1, 0);
                    clrtoeol();
                    attron(COLOR_PAIR(2));
                    printw(":");
                    attroff(COLOR_PAIR(2));
                    refresh();

                    // Get command - simple implementation to read a line
                    echo();
                    
                    char cmd[256];
                    getnstr(cmd, sizeof(cmd) - 1);
                    noecho();

                    String command = String(cmd);

                    if (command == "q") {
                        if (fileModified) {
                            // Show warning
                            move(rows - 1, 0);
                            clrtoeol();
                            attron(COLOR_PAIR(2) | A_BOLD);
                            printw("File modified. Use :wq to save or :q! to quit without saving.");
                            attroff(COLOR_PAIR(2) | A_BOLD);
                            refresh();
                            getch();  // Wait for keypress
                        } else {
                            editor_active = false;
                        }
                    } else if (command == "q!") {
                        editor_active = false;
                    } else if (command == "w") {
                        // Save file
                        String newContent;
                        for (int i = 0; i < lines.GetCount(); ++i) {
                            newContent << lines[i];
                            if (i < lines.GetCount() - 1) newContent << "\n";
                        }

                        // Write to file
                        if (SaveFile(vfsPath, newContent)) {
                            fileModified = false;
                            // Show confirmation
                            move(rows - 1, 0);
                            clrtoeol();
                            attron(COLOR_PAIR(2));
                            printw("[Saved %d lines to %s]", lines.GetCount(), vfsPath.ToStd().c_str());
                            attroff(COLOR_PAIR(2));
                            refresh();
                            getch(); // Wait for key press
                        } else {
                            // Error occurred during write
                            move(rows - 1, 0);
                            clrtoeol();
                            attron(COLOR_PAIR(2) | A_BOLD);
                            printw("Error: Could not write file: %s", vfsPath.ToStd().c_str());
                            attroff(COLOR_PAIR(2) | A_BOLD);
                            refresh();
                            getch(); // Wait for key press
                        }
                    } else if (command == "wq" || command == "x") {
                        // Save and quit
                        String newContent;
                        for (int i = 0; i < lines.GetCount(); ++i) {
                            newContent << lines[i];
                            if (i < lines.GetCount() - 1) newContent << "\n";
                        }

                        // Write to file
                        SaveFile(vfsPath, newContent);
                        editor_active = false;
                    } else if (command == "help") {
                        // Show help screen
                        clear();
                        mvprintw(0, 0, "VfsShell Editor Help");
                        mvprintw(1, 0, "=====================");
                        mvprintw(2, 0, "Navigation:");
                        mvprintw(3, 2, "Arrow Keys - Move cursor");
                        mvprintw(4, 2, "ESC        - Enter command mode");
                        mvprintw(5, 0, "Editing:");
                        mvprintw(6, 2, "Type       - Insert text");
                        mvprintw(7, 2, "Backspace  - Delete character before cursor");
                        mvprintw(8, 2, "Delete     - Delete character at cursor");
                        mvprintw(9, 2, "Enter      - Insert new line");
                        mvprintw(10, 0, "Commands (in command mode):");
                        mvprintw(11, 2, ":w         - Save file");
                        mvprintw(12, 2, ":q         - Quit");
                        mvprintw(13, 2, ":q!        - Quit without saving");
                        mvprintw(14, 2, ":wq or :x  - Save and quit");
                        mvprintw(15, 2, ":help      - Show this help");
                        mvprintw(17, 0, "Press any key to continue...");
                        refresh();
                        getch();
                    } else if (!command.IsEmpty()) {
                        // Unknown command
                        move(rows - 1, 0);
                        clrtoeol();
                        attron(COLOR_PAIR(2) | A_BOLD);
                        printw("Unknown command: %s", command.ToStd().c_str());
                        attroff(COLOR_PAIR(2) | A_BOLD);
                        refresh();
                        getch(); // Wait for key press
                    }
                }
                break;

            default:
                // Regular character input
                if (ch >= 32 && ch <= 126) {  // Printable ASCII
                    lines[current_line].Insert(cursor_x, String((char)ch, 1));
                    cursor_x++;
                    fileModified = true;
                }
                break;
        }
    }

    // Clean up ncurses
    endwin();

    return true;
}

END_UPP_NAMESPACE