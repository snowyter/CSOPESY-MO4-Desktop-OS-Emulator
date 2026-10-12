// VirtualFS.cpp
#include "VirtualFS.h"

#include <algorithm>
#include <cctype>
#include <ctime>

namespace {
    std::string lower(std::string s) {
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    std::string extensionOf(const std::string& name) {
        size_t dot = name.rfind('.');
        return dot == std::string::npos ? "" : lower(name.substr(dot + 1));
    }

    // Folders first, then files, each group A-Z (like Windows).
    bool entryOrder(const FsEntry& a, const FsEntry& b) {
        if (a.isFolderLike() != b.isFolderLike()) return a.isFolderLike();
        return lower(a.name) < lower(b.name);
    }
}

// ===========================================================================
// FsEntry
// ===========================================================================
bool FsEntry::isText() const {
    if (kind != Kind::File) return false;
    const std::string ext = extensionOf(name);
    return ext == "txt" || ext == "log" || ext == "cfg" || ext == "ini" ||
           ext == "md"  || ext == "cpp" || ext == "h";
}

std::string FsEntry::typeName() const {
    if (kind == Kind::Folder) return "File folder";
    if (kind == Kind::Drive)  return "Local Disk";
    const std::string ext = extensionOf(name);
    if (ext == "txt")  return "Text Document";
    if (ext == "log")  return "Log File";
    if (ext == "cfg" || ext == "ini") return "Configuration File";
    if (ext == "md")   return "Markdown File";
    if (ext == "cpp")  return "C++ Source File";
    if (ext == "h")    return "C/C++ Header";
    if (ext == "jpg" || ext == "jpeg") return "JPEG Image";
    if (ext == "png")  return "PNG Image";
    if (ext == "zip")  return "Compressed (zipped) Folder";
    if (ext == "docx") return "Microsoft Word Document";
    if (ext == "pptx") return "Microsoft PowerPoint Presentation";
    if (ext == "exe")  return "Application";
    if (ext == "mp4")  return "MP4 Video";
    if (ext.empty()) return "File";
    std::string upper = ext;
    for (char& c : upper) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return upper + " File";                       // "XYZ File", like Windows
}

// ===========================================================================
// Starting contents
// ===========================================================================
VirtualFS& VirtualFS::instance() {
    static VirtualFS fs;
    return fs;
}

VirtualFS::VirtualFS() {
    // This PC -> the C: drive
    FsEntry drive;
    drive.name = "Local Disk (C:)";
    drive.kind = FsEntry::Kind::Drive;
    drive.target = "C:";
    folders_[ROOT].push_back(drive);
    folders_["C:"];

    // C:
    addFolder("C:", "CSOPESY", "09/14/2026 08:00 AM");
    addFolder("C:", "Program Files", "09/14/2026 08:02 AM");
    addFolder("C:", "Users", "09/14/2026 08:05 AM");

    addFile("C:\\CSOPESY", "boot.log", "10/11/2026 09:00 AM",
        "[BIOS]   POST complete, memory test OK\n"
        "[BOOT]   Loading CSOPESY kernel...\n"
        "[KERNEL] Phase 1: bootstrapping (GLFW, OpenGL, Dear ImGui)\n"
        "[KERNEL] Phase 2: kernel init (style, window registry, wallpaper)\n"
        "[KERNEL] Phase 3: starting services (desktop, taskbar, apps)\n"
        "[KERNEL] Phase 4: main loop running\n");
    addFile("C:\\CSOPESY", "system.cfg", "09/14/2026 08:00 AM",
        "# Not the real config -- the real one is config.txt next to csopesy.exe\n"
        "os_name=CSOPESY OS\n"
        "version=1.0\n"
        "scheduler=round-robin\n"
        "quantum=5\n");
    addFile("C:\\CSOPESY", "kernel.sys", "09/14/2026 08:00 AM", "", 2'457'600);

    addFolder("C:\\Program Files", "Task Manager", "09/14/2026 08:02 AM");
    addFolder("C:\\Program Files", "Notepad", "09/14/2026 08:02 AM");
    addFolder("C:\\Program Files", "File Explorer", "09/14/2026 08:02 AM");
    addFile("C:\\Program Files\\Task Manager", "taskmgr.exe", "09/14/2026 08:02 AM", "", 1'203'712);
    addFile("C:\\Program Files\\Notepad", "notepad.exe", "09/14/2026 08:02 AM", "", 360'448);
    addFile("C:\\Program Files\\File Explorer", "explorer.exe", "09/14/2026 08:02 AM", "", 4'882'432);

    addFolder("C:\\Users", "CSOPESY", "09/14/2026 08:05 AM");

    // The user's home folder: C:\Users\CSOPESY
    const std::string home = HOME;
    addFolder(home, "Desktop", "10/11/2026 09:01 AM");
    addFolder(home, "Documents", "10/10/2026 07:45 PM");
    addFolder(home, "Downloads", "10/09/2026 03:12 PM");
    addFolder(home, "Pictures", "10/08/2026 11:20 AM");

    addFile(home + "\\Desktop", "README.txt", "10/11/2026 09:01 AM",
        "Welcome to CSOPESY OS!\n"
        "\n"
        "- Click the icons on the taskbar to open File Explorer, Notepad and Task Manager.\n"
        "- Click an open app's icon again to minimise it.\n"
        "- Double-click a .txt file in File Explorer to open it in Notepad.\n"
        "- Press PWR (bottom right) to shut down.\n");

    const std::string docs = home + "\\Documents";
    addFolder(docs, "Projects", "10/10/2026 07:40 PM");
    addFile(docs, "notes.txt", "10/10/2026 07:45 PM",
        "CSOPESY notes\n"
        "=============\n"
        "\n"
        "Immediate mode GUI: the UI is rebuilt from scratch every frame.\n"
        "Retained mode GUI: widgets are objects that stay alive between frames.\n"
        "\n"
        "Main loop: poll input -> new frame -> build UI -> render -> swap buffers.\n");
    addFile(docs, "todo.txt", "10/10/2026 06:30 PM",
        "[x] Desktop + wallpaper\n"
        "[x] Taskbar with clock\n"
        "[x] Task Manager\n"
        "[x] File Explorer + Notepad\n"
        "[ ] Record the demo video\n"
        "[ ] Finish the slides\n");
    addFile(docs, "MO4 Specs.docx", "10/02/2026 10:15 AM", "", 48'213);

    const std::string projects = docs + "\\Projects";
    addFile(projects, "MO3 Marquee.cpp", "09/28/2026 11:58 PM",
        "// Marquee console from MO3 (excerpt)\n"
        "while (running) {\n"
        "    handleInput();\n"
        "    moveMarquee();\n"
        "    redraw();\n"
        "    sleepFor(refreshMs);\n"
        "}\n");
    addFile(projects, "group10.md", "09/20/2026 04:00 PM",
        "# Group 10\n"
        "- Justin David\n"
        "- Kryster Limpin\n"
        "- Keith Singson\n");

    const std::string downloads = home + "\\Downloads";
    addFile(downloads, "imgui-1.92.5.zip", "10/03/2026 02:41 PM", "", 1'932'510);
    addFile(downloads, "glfw-3.4.bin.WIN64.zip", "10/03/2026 02:44 PM", "", 3'318'096);
    addFile(downloads, "setup notes.txt", "10/03/2026 03:12 PM",
        "1. Unzip WinLibs to C:\\winlibs\n"
        "2. Open the project folder in VS Code\n"
        "3. Run and Debug -> \"Run CSOPESY\" -> F5\n");

    const std::string pictures = home + "\\Pictures";
    addFile(pictures, "wallpaper.jpg", "10/08/2026 11:20 AM", "", 1'465'232);
    addFile(pictures, "desktop screenshot.png", "10/11/2026 09:05 AM", "", 734'018);

    // TODO(groupmate): add more folders/files here, e.g.
    //   addFolder(docs, "School", "10/11/2026 08:00 AM");
    //   addFile(docs + "\\School", "schedule.txt", "10/11/2026 08:00 AM", "Mon: CSOPESY\n");

    for (auto& folder : folders_) std::sort(folder.second.begin(), folder.second.end(), entryOrder);
}

void VirtualFS::addFolder(const std::string& parent, const std::string& name, const std::string& modified) {
    FsEntry e;
    e.name = name;
    e.kind = FsEntry::Kind::Folder;
    e.modified = modified;
    folders_[parent].push_back(e);
    folders_[parent + "\\" + name];   // create the (empty) folder itself
}

void VirtualFS::addFile(const std::string& folder, const std::string& name, const std::string& modified,
                        const std::string& text, size_t fakeBytes) {
    FsEntry e;
    e.name = name;
    e.kind = FsEntry::Kind::File;
    e.modified = modified;
    e.content = text;
    e.fakeBytes = fakeBytes;
    folders_[folder].push_back(e);
}

// ===========================================================================
// Lookups
// ===========================================================================
const std::vector<FsEntry>* VirtualFS::list(const std::string& folder) const {
    auto it = folders_.find(folder);
    return it == folders_.end() ? nullptr : &it->second;
}

std::string VirtualFS::resolve(const std::string& typed) const {
    // Tidy up what the user typed: trim spaces, / -> \, no trailing backslash.
    std::string path = typed;
    std::replace(path.begin(), path.end(), '/', '\\');
    while (!path.empty() && std::isspace(static_cast<unsigned char>(path.back()))) path.pop_back();
    while (!path.empty() && std::isspace(static_cast<unsigned char>(path.front()))) path.erase(path.begin());
    while (path.size() > 2 && path.back() == '\\') path.pop_back();
    if (path == "C:\\" || path == "c:\\") path = "C:";

    // Paths are not case-sensitive (like Windows).
    for (const auto& folder : folders_) {
        if (lower(folder.first) == lower(path)) return folder.first;
    }
    return "";
}

std::string VirtualFS::childPath(const std::string& folder, const FsEntry& entry) {
    if (entry.kind == FsEntry::Kind::Drive) return entry.target;
    return folder + "\\" + entry.name;
}

std::string VirtualFS::parentPath(const std::string& path) {
    if (path == ROOT) return "";
    size_t slash = path.rfind('\\');
    if (slash == std::string::npos) return ROOT;   // "C:" -> "This PC"
    return path.substr(0, slash);
}

std::string VirtualFS::baseName(const std::string& path) {
    if (path == "C:") return "Local Disk (C:)";
    size_t slash = path.rfind('\\');
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

const FsEntry* VirtualFS::findFile(const std::string& folder, const std::string& name) const {
    const std::vector<FsEntry>* entries = list(folder);
    if (!entries) return nullptr;
    for (const FsEntry& e : *entries) {
        if (e.kind == FsEntry::Kind::File && lower(e.name) == lower(name)) return &e;
    }
    return nullptr;
}

bool VirtualFS::validFileName(const std::string& name) {
    if (name.empty() || name.size() > 64 || name == "." || name == "..") return false;
    if (name.back() == ' ' || name.back() == '.') return false;
    for (char c : name) {
        if (std::string("\\/:*?\"<>|").find(c) != std::string::npos) return false;
        if (static_cast<unsigned char>(c) < 32) return false;
    }
    return true;
}

bool VirtualFS::writeFile(const std::string& folder, const std::string& name, const std::string& text) {
    auto it = folders_.find(folder);
    if (it == folders_.end() || folder == ROOT || !validFileName(name)) return false;

    std::vector<FsEntry>& entries = it->second;
    for (FsEntry& e : entries) {
        if (lower(e.name) != lower(name)) continue;
        if (e.kind != FsEntry::Kind::File) return false;   // a folder already has that name
        e.content = text;
        e.modified = now();
        return true;
    }
    FsEntry e;
    e.name = name;
    e.kind = FsEntry::Kind::File;
    e.modified = now();
    e.content = text;
    entries.push_back(e);
    std::sort(entries.begin(), entries.end(), entryOrder);
    return true;
}

std::string VirtualFS::now() {
    std::time_t t = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &t);
#else
    localtime_r(&t, &local);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%m/%d/%Y %I:%M %p", &local);
    return buf;
}
