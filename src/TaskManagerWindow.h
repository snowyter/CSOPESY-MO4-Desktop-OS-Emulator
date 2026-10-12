#pragma once
// TaskManagerWindow.h
// ---------------------------------------------------------------------------
// App 1 (required by the specs): a Task Manager with DUMMY data, laid out
// like the classic Windows XP one:
//
//  - "Applications" tab: the open app windows (End Task / Switch To).
//  - "Processes" tab: a sortable table (click a column header to sort) of
//    fake system processes plus every app window that is currently open,
//    with their CPU and memory. Select a row and press "End Process".
//  - "Performance" tab: graphs of the last 60 CPU / memory readings.
//  - Status bar: process count, CPU usage and memory in use.
//
// The numbers are not real. Each process has a base value and wobbles
// around it with a sine wave, updated every taskmgr_refresh_ms (config.txt).
// ---------------------------------------------------------------------------
#include <string>
#include <vector>

#include "AWindow.h"
#include "Config.h"

class TaskManagerWindow : public AWindow {
public:
    explicit TaskManagerWindow(const AppConfig& config);
    void draw() override;

private:
    struct Process {
        std::string name;
        int pid = 0;
        std::string status;      // "Running", "Background", "Suspended", "Minimized"
        bool critical = false;   // can't be ended (like Windows' "System")
        std::string windowId;    // set for app windows ("notepad"), empty for dummies
        float baseCpu = 0.0f;    // % the value wobbles around
        float baseMem = 0.0f;    // MB the value wobbles around
        float phase = 0.0f;      // where on the sine wave it starts
        float cpu = 0.0f;        // current values shown in the table
        float mem = 0.0f;
    };

    void addDummy(const char* name, int pid, const char* status, bool critical,
                  float cpu, float mem);
    void syncAppProcesses();     // add/remove rows for app windows that opened/closed
    void updateValues();         // one "refresh": new CPU/memory numbers + history
    void catchUp();              // runs updateValues() for the time that has passed

    void drawApplicationsTab();
    void drawProcessesTab();
    void drawPerformanceTab();
    void drawStatusBar();
    bool footerButton(const char* label, bool enabled);
    void drawMessage();
    void endSelectedTask();
    void setMessage(const std::string& text, bool error);

    double refreshSeconds_;
    double lastUpdate_ = -1.0;
    long long tick_ = 0;                 // how many refreshes have happened

    std::vector<Process> processes_;
    int nextPid_ = 3004;                 // PIDs handed to app windows as they open
    int selectedPid_ = -1;
    std::string selectedApp_;            // window id selected in the Applications tab

    float totalCpu_ = 0.0f;              // %
    float usedMem_ = 0.0f;               // MB
    static constexpr float TOTAL_MEM_MB = 8192.0f;   // pretend the PC has 8 GB

    static constexpr int HISTORY = 60;
    std::vector<float> cpuHistory_;
    std::vector<float> memHistory_;      // % of TOTAL_MEM_MB

    std::string message_;
    bool messageIsError_ = false;
    double messageTime_ = -100.0;
};
