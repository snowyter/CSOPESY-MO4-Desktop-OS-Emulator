#pragma once
// GUIApplication.h
// ---------------------------------------------------------------------------
// Owns the window and runs the OS through the 5 phases from the Week 2 lecture:
//
//   1. Bootstrapping        GLFW opens the window, OpenGL + Dear ImGui start, fonts load
//   2. Kernel init          style, UIManager, wallpaper texture
//   3. Start services       Desktop, Taskbar and app windows are created
//   4. Main loop            poll input -> build the frame -> draw -> show it, ~60x/s
//   5. Shutdown & cleanup   everything is released in REVERSE order of creation
//
// initialize() runs phases 1-3, run() is phase 4, shutdown() is phase 5.
// ---------------------------------------------------------------------------
#include <memory>
#include <string>
#include <vector>

#include "BootScreen.h"
#include "Config.h"

struct GLFWwindow;
class Desktop;
class Taskbar;

class GUIApplication {
public:
    GUIApplication(AppConfig config, std::vector<std::string> configWarnings);
    ~GUIApplication();

    bool initialize();   // phases 1-3; false if the window couldn't be created
    void run();          // phase 4: returns when the OS has shut down
    void shutdown();     // phase 5

private:
    bool bootstrap();          // phase 1
    void initKernel();         // phase 2
    void startServices();      // phase 3

    void updateLogic();        // per-frame state changes (e.g. power off)
    void renderFrame();        // per-frame drawing
    void drawShutdownScreen();

    // Booting -> Running -> ShuttingDown
    enum class State { Booting, Running, ShuttingDown };

    AppConfig config_;
    std::vector<std::string> warnings_;
    GLFWwindow* window_ = nullptr;
    SystemInfo systemInfo_;           // OpenGL/GLFW details shown on the BIOS screen
    bool imguiReady_ = false;

    std::unique_ptr<BootScreen> bootScreen_;
    std::unique_ptr<Desktop> desktop_;
    std::unique_ptr<Taskbar> taskbar_;

    State state_ = State::Booting;
    double shutdownStart_ = 0.0;
};
