// NotepadWindow.cpp
#include "NotepadWindow.h"

#include <algorithm>
#include <cstring>
#include <ctime>

#include "imgui_internal.h"   // ImGui::ClearActiveID()

#include "FileExplorerWindow.h"
#include "Theme.h"
#include "UIManager.h"
#include "VirtualFS.h"

NotepadWindow::NotepadWindow()
    : AWindow("notepad", "Notepad", ImVec2(640, 440), ImVec2(0.60f, 0.42f)),
      text_(BUFFER_SIZE, '\0') {
    updateTitle();
}

// ===========================================================================
// Documents
// ===========================================================================
void NotepadWindow::setText(const std::string& s) {
    // If the user is typing in the box, ImGui holds its own copy of the text.
    // Deactivating the box makes it read our buffer again next frame.
    if (editorActive_) ImGui::ClearActiveID();

    const size_t len = std::min(s.size(), BUFFER_SIZE - 1);
    std::memcpy(text_.data(), s.data(), len);
    text_[len] = '\0';
    cursor_ = 0;
    focusEditor_ = true;
}

void NotepadWindow::newDocument() {
    setText("");
    folder_.clear();
    fileName_.clear();
    modified_ = false;
}

void NotepadWindow::loadFile(const std::string& folder, const std::string& name) {
    const FsEntry* file = VirtualFS::instance().findFile(folder, name);
    if (!file) {
        setStatus("Could not find " + name);
        return;
    }
    setText(file->content);
    folder_ = folder;
    fileName_ = file->name;
    modified_ = false;
    if (file->content.size() >= BUFFER_SIZE) setStatus("File was too big; only the first 64 KB were opened.");
}

void NotepadWindow::openFile(const std::string& folder, const std::string& name) {
    show();
    // Same file already open? Just bring the window to the front.
    if (folder == folder_ && name == fileName_) return;
    pendingFolder_ = folder;
    pendingName_ = name;
    askBeforeDiscarding(Pending::OpenFile);
}

void NotepadWindow::askBeforeDiscarding(Pending action) {
    pending_ = action;
    if (modified_) openDialog(Dialog::Unsaved);   // ask "Save changes?" first
    else runPending();
}

void NotepadWindow::runPending() {
    const Pending action = pending_;
    pending_ = Pending::None;
    if (action == Pending::NewDocument) newDocument();
    if (action == Pending::OpenFile)    loadFile(pendingFolder_, pendingName_);
}

bool NotepadWindow::save() {
    if (fileName_.empty()) {               // never saved -> ask for a name
        std::strncpy(saveAsName_, "Untitled.txt", sizeof(saveAsName_) - 1);
        saveAsError_.clear();
        openDialog(Dialog::SaveAs);
        return false;
    }
    if (!VirtualFS::instance().writeFile(folder_, fileName_, text_.data())) {
        setStatus("Could not save " + fileName_);
        return false;
    }
    modified_ = false;
    setStatus("Saved " + fileName_);
    return true;
}

bool NotepadWindow::saveAs(std::string name) {
    // trim spaces; no extension -> add .txt
    while (!name.empty() && name.back() == ' ') name.pop_back();
    while (!name.empty() && name.front() == ' ') name.erase(name.begin());
    if (name.find('.') == std::string::npos && !name.empty()) name += ".txt";

    if (!VirtualFS::validFileName(name)) {
        saveAsError_ = "Invalid name. Don't use \\ / : * ? \" < > |";
        return false;
    }
    const std::string folder = folder_.empty() ? std::string(VirtualFS::HOME) + "\\Documents" : folder_;
    if (!VirtualFS::instance().writeFile(folder, name, text_.data())) {
        saveAsError_ = "A folder with that name already exists.";
        return false;
    }
    folder_ = folder;
    fileName_ = name;
    modified_ = false;
    setStatus("Saved " + fileName_);
    return true;
}

// Inserts text at the cursor (used by Edit > Insert date/time).
void NotepadWindow::insertText(const std::string& s) {
    if (editorActive_) {                   // typing right now: let the callback do it
        insertWhileActive_ = s;
        return;
    }
    const size_t len = std::strlen(text_.data());
    if (len + s.size() >= BUFFER_SIZE) { setStatus("Not enough room (64 KB limit)."); return; }
    const size_t at = std::min(static_cast<size_t>(cursor_), len);
    std::memmove(text_.data() + at + s.size(), text_.data() + at, len - at + 1);   // +1: the '\0'
    std::memcpy(text_.data() + at, s.data(), s.size());
    cursor_ = static_cast<int>(at + s.size());
    modified_ = true;
    focusEditor_ = true;
}

void NotepadWindow::setStatus(const std::string& s) {
    status_ = s;
    statusTime_ = ImGui::GetTime();
}

void NotepadWindow::updateTitle() {
    title_ = (modified_ ? "*" : "") + (fileName_.empty() ? std::string("Untitled") : fileName_) + " - Notepad";
}

// Called by ImGui every frame while the text box is active.
int NotepadWindow::editorCallback(ImGuiInputTextCallbackData* data) {
    auto* self = static_cast<NotepadWindow*>(data->UserData);
    if (!self->insertWhileActive_.empty()) {
        const std::string& s = self->insertWhileActive_;
        if (data->BufTextLen + static_cast<int>(s.size()) < data->BufSize) {
            data->InsertChars(data->CursorPos, s.c_str());
            self->modified_ = true;
        }
        self->insertWhileActive_.clear();
    }
    self->cursor_ = data->CursorPos;
    return 0;
}

// ===========================================================================
// Drawing
// ===========================================================================
void NotepadWindow::draw() {
    updateTitle();
    if (!beginWindow(ImGuiWindowFlags_MenuBar)) return;

    // While a dialog is open, the menu, shortcuts and text box are disabled.
    const bool dialogOpen = dialog_ != Dialog::None;
    if (!dialogOpen) handleShortcuts();
    ImGui::BeginDisabled(dialogOpen);
    drawMenuBar();
    drawEditor();
    ImGui::EndDisabled();
    drawStatusBar();
    drawDialog();

    endWindow();
}

void NotepadWindow::handleShortcuts() {
    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) return;
    const ImGuiIO& io = ImGui::GetIO();
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N, false)) askBeforeDiscarding(Pending::NewDocument);
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) save();
    if (ImGui::IsKeyPressed(ImGuiKey_F5, false)) {
        char stamp[32];
        std::time_t t = std::time(nullptr);
        std::strftime(stamp, sizeof(stamp), "%I:%M %p %m/%d/%Y", std::localtime(&t));
        insertText(stamp);
    }
}

void NotepadWindow::drawMenuBar() {
    if (!ImGui::BeginMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New", "Ctrl+N")) askBeforeDiscarding(Pending::NewDocument);
        if (ImGui::MenuItem("Open...")) {
            // There is no file dialog: open File Explorer at Documents instead.
            if (auto* explorer = dynamic_cast<FileExplorerWindow*>(UIManager::instance().find("explorer"))) {
                explorer->navigateTo(std::string(VirtualFS::HOME) + "\\Documents");
                explorer->show();
            }
        }
        if (ImGui::MenuItem("Save", "Ctrl+S")) save();
        if (ImGui::MenuItem("Save As...")) {
            std::strncpy(saveAsName_, fileName_.empty() ? "Untitled.txt" : fileName_.c_str(), sizeof(saveAsName_) - 1);
            saveAsError_.clear();
            openDialog(Dialog::SaveAs);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Close")) hide();   // the text stays until you reopen it
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Insert date/time", "F5")) {
            char stamp[32];
            std::time_t t = std::time(nullptr);
            std::strftime(stamp, sizeof(stamp), "%I:%M %p %m/%d/%Y", std::localtime(&t));
            insertText(stamp);
        }
        if (ImGui::MenuItem("Clear all")) {
            setText("");
            modified_ = true;
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Format")) {
        ImGui::MenuItem("Word wrap", nullptr, &wordWrap_);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("About Notepad")) openDialog(Dialog::About);
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
}

void NotepadWindow::drawEditor() {
    const float statusHeight = ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y * 2.0f;

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackAlways;
    if (wordWrap_) flags |= ImGuiInputTextFlags_WordWrap;

    editorTop_ = ImGui::GetCursorPosY();   // the dialogs are placed relative to this
    if (focusEditor_ && dialog_ == Dialog::None) {
        ImGui::SetKeyboardFocusHere();
        focusEditor_ = false;
    }
    if (ImGui::InputTextMultiline("##text", text_.data(), text_.size(),
                                  ImVec2(-1.0f, -statusHeight), flags, editorCallback, this)) {
        modified_ = true;
    }
    editorActive_ = ImGui::IsItemActive();
}

void NotepadWindow::drawStatusBar() {
    // Line and column of the cursor: count the line breaks before it.
    const char* text = text_.data();
    const int len = static_cast<int>(std::strlen(text));
    const int cur = std::min(cursor_, len);
    int line = 1, lineStart = 0;
    for (int i = 0; i < cur; ++i) {
        if (text[i] == '\n') { ++line; lineStart = i + 1; }
    }
    // Count characters, not bytes (UTF-8 continuation bytes start with 10xxxxxx).
    int chars = 0;
    for (int i = 0; i < len; ++i) {
        if ((static_cast<unsigned char>(text[i]) & 0xC0) != 0x80) ++chars;
    }

    ImGui::Separator();
    ImGui::TextDisabled("Ln %d, Col %d  |  %d characters  |  %s  |  UTF-8",
                        line, cur - lineStart + 1, chars, wordWrap_ ? "Word wrap" : "No wrap");

    // Last message on the right ("Saved to ..."), fades after a few seconds.
    const double age = ImGui::GetTime() - statusTime_;
    if (age < 4.0) {
        const float w = ImGui::CalcTextSize(status_.c_str()).x;
        ImGui::SameLine();
        ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - w));
        ImVec4 c = ImGui::ColorConvertU32ToFloat4(Theme::Green);
        c.w = age < 3.0 ? 1.0f : static_cast<float>(4.0 - age);
        ImGui::TextColored(c, "%s", status_.c_str());
    }
}

void NotepadWindow::openDialog(Dialog dialog) {
    dialog_ = dialog;
    dialogJustOpened_ = true;
}

// The dialogs are drawn INSIDE the Notepad window (a box over the text),
// not as ImGui modal popups: a modal would freeze the whole desktop,
// including the taskbar and the PWR button.
void NotepadWindow::drawDialog() {
    if (dialog_ == Dialog::None) return;

    const bool focusedHere = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    const bool escape = focusedHere && ImGui::IsKeyPressed(ImGuiKey_Escape, false);
    const bool firstFrame = dialogJustOpened_;
    dialogJustOpened_ = false;

    // Centre a 440px-wide box near the top of the text area.
    const float width = std::min(440.0f, ImGui::GetWindowWidth() - 32.0f);
    ImGui::SetCursorPos(ImVec2((ImGui::GetWindowWidth() - width) * 0.5f, editorTop_ + 30.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_PopupBg));
    ImGui::PushStyleColor(ImGuiCol_Border, ImGui::ColorConvertU32ToFloat4(Theme::Cyan));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
    ImGui::BeginChild("##dialog", ImVec2(width, 0.0f),
                      ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysUseWindowPadding);
    if (firstFrame) ImGui::SetWindowFocus();   // so keys (Enter, Esc) go to the dialog

    const ImVec2 buttonSize(110.0f, 0.0f);
    ImGui::PushFont(Theme::BoldFont, 0.0f);
    ImGui::TextUnformatted(dialog_ == Dialog::SaveAs ? "Save As" : dialog_ == Dialog::About ? "About Notepad" : "Notepad");
    ImGui::PopFont();
    ImGui::Separator();

    if (dialog_ == Dialog::Unsaved) {
        // ---- "Do you want to save changes?" ----
        ImGui::TextWrapped("Do you want to save changes to %s?", fileName_.empty() ? "Untitled" : fileName_.c_str());
        ImGui::Spacing();
        if (ImGui::Button("Save", buttonSize)) {
            dialog_ = Dialog::None;
            if (save()) runPending();       // untitled: Save As opens, pending runs after it
        }
        ImGui::SameLine();
        if (ImGui::Button("Don't Save", buttonSize)) {
            dialog_ = Dialog::None;
            runPending();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", buttonSize) || escape) {
            dialog_ = Dialog::None;
            pending_ = Pending::None;
        }
    } else if (dialog_ == Dialog::SaveAs) {
        // ---- Save As: type a file name ----
        const std::string folder = folder_.empty() ? std::string(VirtualFS::HOME) + "\\Documents" : folder_;
        ImGui::TextDisabled("Save in: %s", folder.c_str());
        ImGui::TextUnformatted("File name:");
        if (firstFrame) ImGui::SetKeyboardFocusHere();
        ImGui::SetNextItemWidth(-1.0f);
        const bool enter = ImGui::InputText("##name", saveAsName_, sizeof(saveAsName_),
                                            ImGuiInputTextFlags_EnterReturnsTrue);
        if (!saveAsError_.empty()) ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(Theme::Red), "%s", saveAsError_.c_str());
        ImGui::Spacing();
        if (ImGui::Button("Save", buttonSize) || enter) {
            if (saveAs(saveAsName_)) {
                dialog_ = Dialog::None;
                runPending();               // e.g. "New" that was waiting for this save
            } else {
                dialogJustOpened_ = true;   // keep typing in the box
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", buttonSize) || escape) {
            dialog_ = Dialog::None;
            pending_ = Pending::None;
        }
    } else if (dialog_ == Dialog::About) {
        // ---- About ----
        ImGui::TextUnformatted("CSOPESY Notepad, version 1.0");
        ImGui::TextUnformatted("Part of CSOPESY OS (Group 10)");
        ImGui::TextDisabled("Built with Dear ImGui %s", IMGUI_VERSION);
        ImGui::Spacing();
        if (ImGui::Button("OK", buttonSize) || escape) dialog_ = Dialog::None;
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    if (dialog_ == Dialog::None) focusEditor_ = true;   // back to typing
}
