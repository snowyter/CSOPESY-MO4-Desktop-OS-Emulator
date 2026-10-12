#pragma once
// FileExplorerWindow.h
// ---------------------------------------------------------------------------
// App 2: a file browser for the in-memory VirtualFS (see VirtualFS.h).
//
//  - Left: Places (Desktop, Documents, Downloads, Pictures) and This PC
//  - Top: Back / Forward / Up buttons and an address bar you can type into
//  - Middle: the folder's contents (Name, Size, Type, Date Modified)
//  - Double-click a folder to open it, or a text file to open it in Notepad
// ---------------------------------------------------------------------------
#include <string>
#include <vector>

#include "AWindow.h"

class FileExplorerWindow : public AWindow {
public:
    FileExplorerWindow();
    void draw() override;

    // Go to a folder (adds it to the Back history). Ignored if it doesn't exist.
    void navigateTo(const std::string& path);

private:
    void drawToolbar();
    void drawSidebar();
    void drawFileList();
    void drawStatusBar();
    void sidebarItem(const char* label, const std::string& path);
    void goBack();
    void goForward();
    void goToAddress();
    void openEntry(const std::string& name);   // double-click
    void setAddressText();
    void setMessage(const std::string& text, bool error);

    std::string current_;                 // folder being shown
    std::vector<std::string> back_;       // Back history
    std::vector<std::string> forward_;    // Forward history
    std::string selected_;                // name of the selected entry ("" = none)
    char address_[260] = {};              // address bar text

    std::string message_;
    bool messageIsError_ = false;
    double messageTime_ = -100.0;
};
