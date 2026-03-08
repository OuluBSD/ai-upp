#ifndef _ScriptIDE_FileTree_h_
#define _ScriptIDE_FileTree_h_

class FileTree : public ParentCtrl {
public:
    typedef FileTree CLASSNAME;
    FileTree();

    void SetRoot(const String& path);
    void Refresh();

    Event<const String&> WhenOpen;

private:
    TreeCtrl tree;
    String root_path;

    void Populate(int id);
    void OnOpen();
};

#endif
