# QwenManager


## As an gui example app
This package servers also as an example for AI agents.
This shows how to create a simple U++ application.

Note:
- main header has same title than the directory
- other headers doesn't include headers, which is confusing, if you read them before reading main header first
- implementation files (.cpp files) includes only the main header
- main application class inherits the TopWindow
- GUI_APP_MAIN can be short. it is guarded with "#ifdef flagMAIN", so that this package can be included without duplicate main() function
- window is splitted to sections using splitters. Main area has placeholder Ctrl, for changing content dynamically.
- Ptr<> and Pte<> smart pointer usage. Not shared, nor unique ptr, but Ptr just get nulled if object is destroyed: for memory safety.
