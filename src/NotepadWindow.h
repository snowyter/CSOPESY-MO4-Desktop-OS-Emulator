#pragma once
// NotepadWindow.h
// ---------------------------------------------------------------------------
// App 3: a simple text editor, like Windows Notepad.
//
//  - File:   New, Open... (jumps to File Explorer), Save, Save As..., Close
//  - Edit:   Insert date/time, Clear all
//  - Format: Word wrap on/off
//  - Help:   About
//  - Status bar: line, column and character count
//
// Files are opened from / saved to the in-memory VirtualFS, so File Explorer
// sees the changes right away. Nothing is written to the real disk.
// The title shows "*" when there are unsaved changes, e.g. "*notes.txt - Notepad".
// ---------------------------------------------------------------------------
#include <string>
#include <vector>

#include "AWindow.h"

struct ImGuiInputTextCallbackData;

class NotepadWindow : public AWindow {
public:
    NotepadWindow();
    void draw() override;

    // Opens a text file from VirtualFS (File Explorer calls this on double-click).
    // If the current document has unsaved changes, the user is asked first.
    void openFile(const std::string& folder, const std::string& name);

private:
    // What to do after the "unsaved changes" question is answered.
    enum class Pending { None, NewDocument, OpenFile };
    // The small box shown over the text (only one at a time).
    enum class Dialog { None, Unsaved, SaveAs, About };

    void handleShortcuts();
    void drawMenuBar();
    void drawEditor();
    void drawStatusBar();
    void drawDialog();

    void openDialog(Dialog dialog);
    void newDocument();
    void loadFile(const std::string& folder, const std::string& name);
    void askBeforeDiscarding(Pending action);   // runs the action now if nothing is unsaved
    void runPending();
    bool save();                                // false -> opened "Save As" instead
    bool saveAs(std::string name);
    void insertText(const std::string& s);
    void setText(const std::string& s);
    void setStatus(const std::string& s);
    void updateTitle();

    static int editorCallback(ImGuiInputTextCallbackData* data);

    static constexpr size_t BUFFER_SIZE = 64 * 1024;   // max text size: 64 KB
    std::vector<char> text_;           // the text itself (ImGui edits this buffer)

    std::string folder_;               // where the file lives ("" = never saved)
    std::string fileName_;             // "notes.txt" ("" = Untitled)
    bool modified_ = false;
    bool wordWrap_ = true;

    int cursor_ = 0;                   // cursor position (byte offset), for Ln/Col
    bool editorActive_ = false;        // is the user typing in the text box right now?
    bool focusEditor_ = false;         // put the cursor back in the text box next frame
    std::string insertWhileActive_;    // text for the callback to insert (F5 while typing)

    Pending pending_ = Pending::None;
    std::string pendingFolder_, pendingName_;

    Dialog dialog_ = Dialog::None;
    bool dialogJustOpened_ = false;    // focus the dialog on its first frame
    float editorTop_ = 0.0f;           // y position of the text box in the window
    char saveAsName_[64] = {};
    std::string saveAsError_;

    std::string status_;               // short message in the status bar ("Saved")
    double statusTime_ = -100.0;
};
