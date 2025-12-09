#!/bin/bash
# Test script to verify the new editor commands and script mode

echo "Testing new editor commands and script mode..."

# Create a test file
echo "Creating test file test.cpp..."
echo "// Original content" > test.cpp
echo "int main() {" >> test.cpp
echo "    // TODO: Implement function" >> test.cpp
echo "    return 0;" >> test.cpp
echo "}" >> test.cpp

# Create a script file to test
echo "Creating test script commands.txt..."
cat > commands.txt << EOF
# Test script for editor commands
open_file --path test.cpp
insert_text --path test.cpp --pos 0 --text "// Auto-generated header\n"
replace_all --path test.cpp --pattern TODO --replacement DONE --case_sensitive false
goto_line --file test.cpp --line 3
save_file --path test.cpp
EOF

echo "Script file created with the following content:"
cat commands.txt

echo ""
echo "To test the implementation, run the following command from the project root:"
echo "theide-cli --script commands.txt"

echo ""
echo "Expected behavior:"
echo "1. The script will execute each command in sequence"
echo "2. It will add a header to test.cpp"
echo "3. It will replace 'TODO' with 'DONE' in test.cpp"
echo "4. It will navigate to line 3"
echo "5. It will save the file"
echo "6. If any command fails, it should stop execution"

echo ""
echo "Test files created successfully!"