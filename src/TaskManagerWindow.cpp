// TaskManagerWindow.cpp
#include "TaskManagerWindow.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "Theme.h"
#include "UIManager.h"

namespace {
    // Column IDs, so the sort code knows which column was clicked.
    enum Column { COL_NAME, COL_PID, COL_STATUS, COL_CPU, COL_MEM };

    // Base CPU % / memory MB for each app window while it is open.
    struct AppUsage { const char* windowId; float cpu; float mem; };
    const AppUsage APP_USAGE[] = {
        { "explorer", 1.6f, 86.0f },
        { "notepad",  0.4f, 24.0f },
        { "taskmgr",  2.2f, 41.0f },
    };
}

TaskManagerWindow::TaskManagerWindow(const AppConfig& config)
    : AWindow("taskmgr", "Task Manager", ImVec2(760, 500), ImVec2(0.56f, 0.46f)),
      refreshSeconds_(config.taskmgrRefreshMs / 1000.0) {
    //          name                       PID   status        critical  CPU%   MB
    addDummy("System",                       4,  "Running",    true,     0.9f, 142.0f);
    addDummy("CSOPESY Kernel",              88,  "Running",    true,     2.4f, 310.0f);
    addDummy("Process Scheduler",          112,  "Running",    true,     3.1f,  58.0f);
    addDummy("Memory Manager",             236,  "Running",    true,     1.2f, 204.0f);
    addDummy("Desktop Window Manager",     640,  "Running",    true,     4.6f, 268.0f);
    addDummy("Taskbar Service",            712,  "Running",    true,     0.7f,  46.0f);
    addDummy("Clock Service",              820,  "Background", false,    0.2f,  12.0f);
    addDummy("Audio Service",              904,  "Background", false,    0.8f,  38.0f);
    addDummy("Network Service",           1032,  "Background", false,    1.4f,  64.0f);
    addDummy("Print Spooler",             1188,  "Background", false,    0.1f,  21.0f);
    addDummy("Search Indexer",            1376,  "Background", false,    5.2f, 182.0f);
    addDummy("Antivirus Scanner",         1764,  "Background", false,    6.8f, 236.0f);
    addDummy("Update Service",            1520,  "Suspended",  false,    0.0f,  33.0f);
    addDummy("Background Worker",         2048,  "Background", false,    2.9f,  97.0f);
    // TODO(groupmate): add more dummy processes here if you want a longer list.

    // Fill the graphs with a minute of history so they aren't empty at first.
    for (int i = 0; i < HISTORY; ++i) updateValues();
}

void TaskManagerWindow::addDummy(const char* name, int pid, const char* status, bool critical,
                                 float cpu, float mem) {
    Process p;
    p.name = name;
    p.pid = pid;
    p.status = status;
    p.critical = critical;
    p.baseCpu = cpu;
    p.baseMem = mem;
    p.phase = static_cast<float>(pid % 97) * 0.37f;   // different start for each process
    processes_.push_back(p);
}

// ===========================================================================
// Data
// ===========================================================================

// Every open app window shows up as a process; closed ones disappear.
void TaskManagerWindow::syncAppProcesses() {
    UIManager& ui = UIManager::instance();

    // Remove rows for windows that were closed.
    processes_.erase(std::remove_if(processes_.begin(), processes_.end(),
        [&](const Process& p) { return !p.windowId.empty() && !ui.isShown(p.windowId); }),
        processes_.end());

    // Minimised apps keep running, just like on Windows.
    for (Process& p : processes_) {
        if (!p.windowId.empty()) p.status = ui.isMinimized(p.windowId) ? "Minimized" : "Running";
    }

    // Add rows for windows that were opened.
    for (const auto& window : ui.windows()) {
        if (!window->isShown()) continue;
        const bool listed = std::any_of(processes_.begin(), processes_.end(),
            [&](const Process& p) { return p.windowId == window->id(); });
        if (listed) continue;

        Process p;
        p.name = window->appName();
        p.pid = nextPid_;
        nextPid_ += 4;                 // Windows PIDs are multiples of 4 too
        p.status = "Running";
        p.windowId = window->id();
        p.baseCpu = 1.0f;
        p.baseMem = 30.0f;
        for (const AppUsage& u : APP_USAGE) {
            if (p.windowId == u.windowId) { p.baseCpu = u.cpu; p.baseMem = u.mem; }
        }
        p.phase = static_cast<float>(p.pid % 97) * 0.37f;
        p.cpu = p.baseCpu;
        p.mem = p.baseMem;
        processes_.push_back(p);
    }
}

// One refresh. Each value = base value + two sine waves, so it moves up
// and down smoothly and never goes negative.
void TaskManagerWindow::updateValues() {
    ++tick_;
    const float t = static_cast<float>(tick_);

    totalCpu_ = 0.0f;
    usedMem_ = 1450.0f;   // pretend the OS itself (kernel, drivers, cache) uses this much
    for (Process& p : processes_) {
        if (p.status == "Suspended") {
            p.cpu = 0.0f;
        } else {
            const float wave = 0.55f * std::sin(t * 0.45f + p.phase)
                             + 0.25f * std::sin(t * 1.30f + p.phase * 1.7f);
            p.cpu = std::max(0.0f, p.baseCpu * (1.0f + wave));
        }
        p.mem = p.baseMem * (1.0f + 0.06f * std::sin(t * 0.20f + p.phase));

        totalCpu_ += p.cpu;
        usedMem_ += p.mem;
    }
    totalCpu_ = std::min(totalCpu_, 100.0f);

    // History for the graphs: drop the oldest value, add the newest.
    cpuHistory_.push_back(totalCpu_);
    memHistory_.push_back(usedMem_ / TOTAL_MEM_MB * 100.0f);
    if (static_cast<int>(cpuHistory_.size()) > HISTORY) cpuHistory_.erase(cpuHistory_.begin());
    if (static_cast<int>(memHistory_.size()) > HISTORY) memHistory_.erase(memHistory_.begin());
}

// draw() only runs while the window is open. When it is reopened after a
// while, run the refreshes that were missed (up to a minute's worth) so the
// graph keeps moving as if it never stopped.
void TaskManagerWindow::catchUp() {
    const double now = ImGui::GetTime();
    if (lastUpdate_ < 0.0) lastUpdate_ = now;

    int missed = 0;
    while (now - lastUpdate_ >= refreshSeconds_) {
        lastUpdate_ += refreshSeconds_;
        if (missed++ < HISTORY) updateValues();
    }
}

void TaskManagerWindow::setMessage(const std::string& text, bool error) {
    message_ = text;
    messageIsError_ = error;
    messageTime_ = ImGui::GetTime();
}

void TaskManagerWindow::endSelectedTask() {
    auto it = std::find_if(processes_.begin(), processes_.end(),
        [&](const Process& p) { return p.pid == selectedPid_; });
    if (it == processes_.end()) return;

    if (it->critical) {
        setMessage("Access denied: \"" + it->name + "\" is a critical system process.", true);
        return;
    }
    const std::string name = it->name;
    if (!it->windowId.empty()) {
        UIManager::instance().hide(it->windowId);   // closes the app's window
        processes_.erase(it);                        // (a reopened app gets a new PID)
    } else {
        processes_.erase(it);
    }
    selectedPid_ = -1;
    setMessage("Ended \"" + name + "\".", false);
}

// ===========================================================================
// Drawing -- kept plain on purpose, like the old Windows XP Task Manager:
// three tabs, plain tables, buttons under them, and a status bar.
// ===========================================================================
void TaskManagerWindow::draw() {
    syncAppProcesses();
    catchUp();

    if (!beginWindow()) return;

    // Leave one line at the bottom for the status bar.
    const float statusHeight = ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("##tm_body", ImVec2(0, -statusHeight));
    if (ImGui::BeginTabBar("##tm_tabs")) {
        if (ImGui::BeginTabItem("Applications")) {
            drawApplicationsTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Processes")) {
            drawProcessesTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Performance")) {
            drawPerformanceTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::EndChild();

    drawStatusBar();
    endWindow();
}

// Bottom line: "Processes: 16 | CPU Usage: 37% | Memory: 3.1 GB / 8.0 GB"
void TaskManagerWindow::drawStatusBar() {
    ImGui::Separator();
    ImGui::Text("Processes: %d  |  CPU Usage: %.0f%%  |  Memory: %.1f GB / %.1f GB",
                static_cast<int>(processes_.size()), totalCpu_,
                usedMem_ / 1024.0f, TOTAL_MEM_MB / 1024.0f);
}

// Buttons + last message under a tab's table.
// Returns true if the button was clicked.
bool TaskManagerWindow::footerButton(const char* label, bool enabled) {
    ImGui::BeginDisabled(!enabled);
    const bool clicked = ImGui::Button(label);
    ImGui::EndDisabled();
    return clicked;
}

void TaskManagerWindow::drawMessage() {
    if (ImGui::GetTime() - messageTime_ < 5.0) {
        ImGui::SameLine(0.0f, 16.0f);
        ImGui::TextUnformatted(message_.c_str());
    }
}

// ---- Applications: just the open app windows ----
void TaskManagerWindow::drawApplicationsTab() {
    UIManager& ui = UIManager::instance();
    const float footer = ImGui::GetFrameHeightWithSpacing() + 4.0f;

    if (ImGui::BeginTable("##apps", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY, ImVec2(0, -footer))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Task", ImGuiTableColumnFlags_WidthStretch, 3.0f);
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableHeadersRow();
        for (const auto& w : ui.windows()) {
            if (!w->isShown()) continue;
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            if (ImGui::Selectable(w->title().c_str(), selectedApp_ == w->id(), ImGuiSelectableFlags_SpanAllColumns)) {
                selectedApp_ = w->id();
            }
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(w->isMinimized() ? "Minimized" : "Running");
        }
        ImGui::EndTable();
    }

    const bool valid = ui.isShown(selectedApp_);
    if (footerButton("End Task", valid)) {
        if (AWindow* w = ui.find(selectedApp_)) setMessage("Ended " + w->appName() + ".", false);
        ui.hide(selectedApp_);
        selectedApp_.clear();
    }
    ImGui::SameLine();
    if (footerButton("Switch To", valid)) ui.show(selectedApp_);
    drawMessage();
}

// ---- Processes: every (dummy) process ----
void TaskManagerWindow::drawProcessesTab() {
    const float footer = ImGui::GetFrameHeightWithSpacing() + 4.0f;

    const ImGuiTableFlags flags = ImGuiTableFlags_Sortable | ImGuiTableFlags_Borders |
                                  ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("##processes", 5, flags, ImVec2(0.0f, -footer))) {
        ImGui::TableSetupScrollFreeze(0, 1);   // header row stays visible when scrolling
        ImGui::TableSetupColumn("Image Name", ImGuiTableColumnFlags_WidthStretch, 3.0f, COL_NAME);
        ImGui::TableSetupColumn("PID",        ImGuiTableColumnFlags_WidthStretch, 0.8f, COL_PID);
        ImGui::TableSetupColumn("Status",     ImGuiTableColumnFlags_WidthStretch, 1.3f, COL_STATUS);
        ImGui::TableSetupColumn("CPU",        ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_PreferSortDescending, 0.9f, COL_CPU);
        ImGui::TableSetupColumn("Mem Usage",  ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_PreferSortDescending, 1.2f, COL_MEM);
        ImGui::TableHeadersRow();

        // Sort a list of pointers (not the processes themselves) by the
        // clicked column. Done every frame because the numbers keep changing.
        std::vector<const Process*> rows;
        for (const Process& p : processes_) rows.push_back(&p);
        if (ImGuiTableSortSpecs* specs = ImGui::TableGetSortSpecs()) {
            if (specs->SpecsCount > 0) {
                const ImGuiTableColumnSortSpecs& s = specs->Specs[0];
                const bool asc = s.SortDirection == ImGuiSortDirection_Ascending;
                std::stable_sort(rows.begin(), rows.end(), [&](const Process* a, const Process* b) {
                    switch (s.ColumnUserID) {
                        case COL_NAME:   return asc ? a->name < b->name     : a->name > b->name;
                        case COL_PID:    return asc ? a->pid < b->pid       : a->pid > b->pid;
                        case COL_STATUS: return asc ? a->status < b->status : a->status > b->status;
                        case COL_CPU:    return asc ? a->cpu < b->cpu       : a->cpu > b->cpu;
                        default:         return asc ? a->mem < b->mem       : a->mem > b->mem;
                    }
                });
            }
            specs->SpecsDirty = false;
        }

        for (const Process* p : rows) {
            ImGui::PushID(p->pid);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(COL_NAME);
            if (ImGui::Selectable(p->name.c_str(), p->pid == selectedPid_, ImGuiSelectableFlags_SpanAllColumns)) {
                selectedPid_ = p->pid;
            }
            ImGui::TableSetColumnIndex(COL_PID);
            ImGui::Text("%d", p->pid);
            ImGui::TableSetColumnIndex(COL_STATUS);
            ImGui::TextUnformatted(p->status.c_str());
            ImGui::TableSetColumnIndex(COL_CPU);
            ImGui::Text("%02.0f", p->cpu);              // XP showed whole numbers: "03"
            ImGui::TableSetColumnIndex(COL_MEM);
            ImGui::Text("%d K", static_cast<int>(p->mem * 1024.0f));
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    const bool hasSelection = std::any_of(processes_.begin(), processes_.end(),
        [&](const Process& p) { return p.pid == selectedPid_; });
    if (footerButton("End Process", hasSelection)) endSelectedTask();
    drawMessage();
}

// ---- Performance: two plain graphs ----
void TaskManagerWindow::drawPerformanceTab() {
    const float graphHeight = (ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeightWithSpacing() * 4.0f) * 0.5f;
    char overlay[64];

    // Old-school green-on-black graphs
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 1));
    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.0f, 0.85f, 0.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

    ImGui::Text("CPU Usage: %.0f%%", totalCpu_);
    std::snprintf(overlay, sizeof(overlay), "%.0f%%", totalCpu_);
    ImGui::PlotLines("##cpu", cpuHistory_.data(), static_cast<int>(cpuHistory_.size()), 0,
                     overlay, 0.0f, 100.0f, ImVec2(-1.0f, graphHeight));

    ImGui::Text("Memory Usage: %.0f MB / %.0f MB", usedMem_, TOTAL_MEM_MB);
    std::snprintf(overlay, sizeof(overlay), "%.0f%%", usedMem_ / TOTAL_MEM_MB * 100.0f);
    ImGui::PlotLines("##mem", memHistory_.data(), static_cast<int>(memHistory_.size()), 0,
                     overlay, 0.0f, 100.0f, ImVec2(-1.0f, graphHeight));

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    const int up = static_cast<int>(ImGui::GetTime());
    ImGui::Text("Up Time: %02d:%02d:%02d    Total Memory: %.0f MB    Refresh: every %.1f s",
                up / 3600, (up / 60) % 60, up % 60, TOTAL_MEM_MB, refreshSeconds_);
}
