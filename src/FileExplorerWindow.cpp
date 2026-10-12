// FileExplorerWindow.cpp
#include "FileExplorerWindow.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "NotepadWindow.h"
#include "Theme.h"
#include "UIManager.h"
#include "VirtualFS.h"

namespace {
    // 1234567 bytes -> "1,206 KB" (Windows always shows KB in this column)
    std::string formatSize(size_t bytes) {
        const size_t kb = (bytes + 1023) / 1024;   // round up, so 1 byte = 1 KB
        std::string digits = std::to_string(kb);
        for (int i = static_cast<int>(digits.size()) - 3; i > 0; i -= 3) digits.insert(i, ",");
        return digits + " KB";
    }
}

FileExplorerWindow::FileExplorerWindow()
    : AWindow("explorer", "File Explorer", ImVec2(820, 470), ImVec2(0.42f, 0.44f)) {
    current_ = std::string(VirtualFS::HOME) + "\\Documents";
    setAddressText();
}

// ===========================================================================
// Navigation
// ===========================================================================
void FileExplorerWindow::navigateTo(const std::string& path) {
    if (path.empty() || path == current_ || !VirtualFS::instance().folderExists(path)) return;
    back_.push_back(current_);
    forward_.clear();
    current_ = path;
    selected_.clear();
    setAddressText();
}

void FileExplorerWindow::setAddressText() {
    std::snprintf(address_, sizeof(address_), "%s", current_.c_str());
}

void FileExplorerWindow::setMessage(const std::string& text, bool error) {
    message_ = text;
    messageIsError_ = error;
    messageTime_ = ImGui::GetTime();
}

void FileExplorerWindow::openEntry(const std::string& name) {
    const std::vector<FsEntry>* entries = VirtualFS::instance().list(current_);
    if (!entries) return;
    auto it = std::find_if(entries->begin(), entries->end(), [&](const FsEntry& e) { return e.name == name; });
    if (it == entries->end()) return;

    if (it->isFolderLike()) {
        navigateTo(VirtualFS::childPath(current_, *it));
    } else if (it->isText()) {
        if (auto* notepad = dynamic_cast<NotepadWindow*>(UIManager::instance().find("notepad"))) {
            notepad->openFile(current_, it->name);
        }
    } else {
        setMessage("Can't open \"" + it->name + "\": no app in CSOPESY OS opens " + it->typeName() + " files.", true);
    }
}

// ===========================================================================
// Drawing -- kept plain on purpose: text buttons, a list of places on the
// left, and a plain "details" table (no icons).
// ===========================================================================
void FileExplorerWindow::draw() {
    // Title shows the folder name, like "Documents - File Explorer".
    title_ = VirtualFS::baseName(current_) + " - File Explorer";
    if (!beginWindow()) return;

    drawToolbar();
    ImGui::Separator();

    const float statusHeight = ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
    ImGui::BeginChild("##sidebar", ImVec2(170, -statusHeight), ImGuiChildFlags_Borders);
    drawSidebar();
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("##files", ImVec2(0, -statusHeight), ImGuiChildFlags_Borders);
    drawFileList();
    ImGui::EndChild();

    drawStatusBar();
    endWindow();
}

void FileExplorerWindow::goBack() {
    forward_.push_back(current_);
    current_ = back_.back();
    back_.pop_back();
    selected_.clear();
    setAddressText();
}

void FileExplorerWindow::goForward() {
    back_.push_back(current_);
    current_ = forward_.back();
    forward_.pop_back();
    selected_.clear();
    setAddressText();
}

void FileExplorerWindow::goToAddress() {
    const std::string path = VirtualFS::instance().resolve(address_);
    if (path.empty()) {
        setMessage(std::string("Can't find \"") + address_ + "\".", true);
    } else {
        navigateTo(path);
    }
    setAddressText();   // show the properly-capitalised path
}

void FileExplorerWindow::drawToolbar() {
    ImGui::BeginDisabled(back_.empty());
    if (ImGui::Button("Back")) goBack();
    ImGui::EndDisabled();
    ImGui::SameLine();

    ImGui::BeginDisabled(forward_.empty());
    if (ImGui::Button("Forward")) goForward();
    ImGui::EndDisabled();
    ImGui::SameLine();

    const std::string parent = VirtualFS::parentPath(current_);
    ImGui::BeginDisabled(parent.empty());
    if (ImGui::Button("Up")) navigateTo(parent);
    ImGui::EndDisabled();
    ImGui::SameLine(0.0f, 16.0f);

    // Address bar: type a path and press Enter (or click Go).
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Address");
    ImGui::SameLine();
    const float goWidth = ImGui::CalcTextSize("Go").x + ImGui::GetStyle().FramePadding.x * 2.0f;
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - goWidth - ImGui::GetStyle().ItemSpacing.x);
    const bool enter = ImGui::InputText("##address", address_, sizeof(address_), ImGuiInputTextFlags_EnterReturnsTrue);
    const bool typing = ImGui::IsItemActive();
    ImGui::SameLine();
    if (ImGui::Button("Go") || enter) goToAddress();

    // Keep the box in sync with the folder unless the user is typing in it.
    if (!typing && std::strcmp(address_, current_.c_str()) != 0) setAddressText();
}

void FileExplorerWindow::sidebarItem(const char* label, const std::string& path) {
    if (ImGui::Selectable(label, current_ == path)) navigateTo(path);
}

void FileExplorerWindow::drawSidebar() {
    const std::string home = VirtualFS::HOME;

    ImGui::TextDisabled("Places");
    sidebarItem("Desktop",   home + "\\Desktop");
    sidebarItem("Documents", home + "\\Documents");
    sidebarItem("Downloads", home + "\\Downloads");
    sidebarItem("Pictures",  home + "\\Pictures");
    ImGui::Spacing();
    ImGui::TextDisabled("Computer");
    sidebarItem("This PC", VirtualFS::ROOT);
    sidebarItem("Local Disk (C:)", "C:");
}

void FileExplorerWindow::drawFileList() {
    const std::vector<FsEntry>* entries = VirtualFS::instance().list(current_);
    if (!entries) {
        ImGui::TextUnformatted("This folder doesn't exist.");
        return;
    }
    if (entries->empty()) {
        ImGui::TextUnformatted("This folder is empty.");
        return;
    }

    std::string toOpen;   // double-clicked entry, opened after the loop
    const ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("##entries", 4, flags)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 2.4f);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthStretch, 0.8f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 1.6f);
        ImGui::TableSetupColumn("Date Modified", ImGuiTableColumnFlags_WidthStretch, 1.6f);
        ImGui::TableHeadersRow();

        for (const FsEntry& e : *entries) {
            ImGui::PushID(e.name.c_str());
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            if (ImGui::Selectable(e.name.c_str(), selected_ == e.name,
                                  ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
                selected_ = e.name;
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) toOpen = e.name;
            }

            ImGui::TableSetColumnIndex(1);
            if (e.kind == FsEntry::Kind::File)        ImGui::TextUnformatted(formatSize(e.sizeBytes()).c_str());
            else if (e.kind == FsEntry::Kind::Folder) ImGui::TextUnformatted("<DIR>");
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(e.typeName().c_str());
            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(e.modified.c_str());

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (!toOpen.empty()) openEntry(toOpen);
}

void FileExplorerWindow::drawStatusBar() {
    const std::vector<FsEntry>* entries = VirtualFS::instance().list(current_);
    const int count = entries ? static_cast<int>(entries->size()) : 0;
    ImGui::Text("%d object(s)", count);
    if (!selected_.empty()) {
        ImGui::SameLine(0.0f, 16.0f);
        ImGui::Text("Selected: %s", selected_.c_str());
    }
    if (ImGui::GetTime() - messageTime_ < 5.0) {
        ImGui::SameLine(0.0f, 16.0f);
        ImGui::TextUnformatted(message_.c_str());
    }
}
