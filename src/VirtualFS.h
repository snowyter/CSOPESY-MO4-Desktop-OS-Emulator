#pragma once
// VirtualFS.h
// ---------------------------------------------------------------------------
// A pretend file system that only exists in memory (nothing touches the real
// disk). File Explorer browses it and Notepad opens/saves text files in it.
//
// Folders are stored in a map: full folder path -> list of entries inside.
//   "C:\Users\CSOPESY\Documents" -> [ Projects (folder), notes.txt, ... ]
// "This PC" is the top level; it holds the C: drive.
//
// Like UIManager it is a singleton: VirtualFS::instance().
// Everything resets when the program closes.
// ---------------------------------------------------------------------------
#include <map>
#include <string>
#include <vector>

struct FsEntry {
    enum class Kind { Folder, File, Drive };

    std::string name;
    Kind kind = Kind::File;
    std::string modified;     // "10/11/2026 02:14 PM"
    std::string content;      // text files only
    size_t fakeBytes = 0;     // size shown for non-text files (images, zips...)
    std::string target;       // Drive only: folder it opens ("C:")

    bool isFolderLike() const { return kind != Kind::File; }
    bool isText() const;      // .txt .log .cfg .cpp ... -> opens in Notepad
    size_t sizeBytes() const { return isText() ? content.size() : fakeBytes; }
    std::string typeName() const;   // "File folder", "Text Document", ...
};

class VirtualFS {
public:
    static VirtualFS& instance();

    static constexpr const char* ROOT = "This PC";
    static constexpr const char* HOME = "C:\\Users\\CSOPESY";

    // nullptr if `folder` doesn't exist
    const std::vector<FsEntry>* list(const std::string& folder) const;
    bool folderExists(const std::string& folder) const { return list(folder) != nullptr; }

    // Typed path -> real path ("c:\users\" -> "C:\Users"), or "" if not found.
    std::string resolve(const std::string& typed) const;

    // Path you end up in after double-clicking `entry` inside `folder`.
    static std::string childPath(const std::string& folder, const FsEntry& entry);
    // One level up: "C:\Users" -> "C:", "C:" -> "This PC", "This PC" -> ""
    static std::string parentPath(const std::string& path);
    // Last part of a path: "C:\Users\CSOPESY" -> "CSOPESY"
    static std::string baseName(const std::string& path);

    const FsEntry* findFile(const std::string& folder, const std::string& name) const;

    // Creates the file, or replaces its text if it already exists.
    // Returns false if the folder doesn't exist or the name is not allowed.
    bool writeFile(const std::string& folder, const std::string& name, const std::string& text);

    static bool validFileName(const std::string& name);
    static std::string now();   // current date/time in the "Date modified" format

private:
    VirtualFS();   // builds the starting folders and files
    VirtualFS(const VirtualFS&) = delete;
    VirtualFS& operator=(const VirtualFS&) = delete;

    void addFolder(const std::string& parent, const std::string& name, const std::string& modified);
    void addFile(const std::string& folder, const std::string& name, const std::string& modified,
                 const std::string& text, size_t fakeBytes = 0);

    std::map<std::string, std::vector<FsEntry>> folders_;
};
