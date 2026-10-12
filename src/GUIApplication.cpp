// GUIApplication.cpp
#include "GUIApplication.h"

#include <algorithm>
#include <cfloat>
#include <fstream>
#include <utility>

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "BootScreen.h"
#include "Desktop.h"
#include "FileExplorerWindow.h"
#include "Log.h"
#include "NotepadWindow.h"
#include "Taskbar.h"
#include "TaskManagerWindow.h"
#include "Theme.h"
#include "UIManager.h"

namespace {
    void glfwErrorCallback(int code, const char* description) {
        Log::line("GLFW error " + std::to_string(code) + ": " + description);
    }

    bool fileExists(const std::string& path) {
        std::ifstream f(path);
        return f.good();
    }

    // Loads a .ttf font, or returns nullptr if the file is missing.
    // (ImGui would stop the program with an assert on a missing file.)
    ImFont* loadFont(const std::string& path, float size) {
        if (!fileExists(path)) return nullptr;
        return ImGui::GetIO().Fonts->AddFontFromFileTTF(path.c_str(), size);
    }

    // "assets/fonts/JetBrainsMono-Regular.ttf" -> ".../JetBrainsMono-Bold.ttf"
    std::string boldVariant(const std::string& path) {
        const std::string tag = "Regular";
        size_t pos = path.rfind(tag);
        if (pos == std::string::npos) return "";
        return path.substr(0, pos) + "Bold" + path.substr(pos + tag.size());
    }
}

GUIApplication::GUIApplication(AppConfig config, std::vector<std::string> configWarnings)
    : config_(std::move(config)), warnings_(std::move(configWarnings)) {}

GUIApplication::~GUIApplication() {
    shutdown();   // safe to call twice
}

// ===========================================================================
// Phases 1-3
// ===========================================================================
bool GUIApplication::initialize() {
    if (!bootstrap()) return false;
    initKernel();
    startServices();
    return true;
}

// Phase 1: Bootstrapping -- "hardware" setup: window, graphics, UI library.
bool GUIApplication::bootstrap() {
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        Log::line("Phase 1 failed: glfwInit()");
        return false;
    }

    // Ask for an OpenGL 3.3 context (what the ImGui OpenGL3 backend expects).
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   // fixed size (window_width/height)

    window_ = glfwCreateWindow(config_.windowWidth, config_.windowHeight,
                               config_.windowTitle.c_str(), nullptr, nullptr);
    if (!window_) {
        Log::line("Phase 1 failed: glfwCreateWindow()");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(config_.vsync ? 1 : 0);   // 1 = wait for the monitor (no tearing)

    // Dear ImGui: create its context and connect it to GLFW (input) + OpenGL (drawing).
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;   // don't save window positions to imgui.ini
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    imguiReady_ = true;

    // Fonts
    Theme::RegularFont = loadFont(config_.fontPath, config_.fontSize);
    if (!Theme::RegularFont) {
        warnings_.push_back("Font '" + config_.fontPath + "' not found; using ImGui's built-in font.");
        Theme::RegularFont = io.Fonts->AddFontDefault();
    }
    Theme::BoldFont = loadFont(boldVariant(config_.fontPath), config_.fontSize);
    if (!Theme::BoldFont) Theme::BoldFont = Theme::RegularFont;
    io.FontDefault = Theme::RegularFont;

    // Real details for the BIOS screen.
    auto glString = [](GLenum name) {
        const GLubyte* s = glGetString(name);
        return s ? std::string(reinterpret_cast<const char*>(s)) : std::string("unknown");
    };
    int major = 0, minor = 0, rev = 0;
    glfwGetVersion(&major, &minor, &rev);
    systemInfo_.glVersion = glString(GL_VERSION);
    systemInfo_.glRenderer = glString(GL_RENDERER);
    systemInfo_.glfwVersion = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(rev);

    Log::line("Phase 1 done: window " + std::to_string(config_.windowWidth) + "x" +
              std::to_string(config_.windowHeight) + ", OpenGL " + systemInfo_.glVersion +
              " (" + systemInfo_.glRenderer + ")");
    return true;
}

// Phase 2: Kernel initialisation -- data structures and resources.
void GUIApplication::initKernel() {
    Theme::applyStyle();
    UIManager::instance();                         // create the window registry

    desktop_ = std::make_unique<Desktop>(config_);
    desktop_->loadWallpaper(warnings_);            // needs OpenGL, so after phase 1
    Log::line("Phase 2 done: style, window registry, wallpaper");
}

// Phase 3: Start system services -- the always-running parts of the OS.
void GUIApplication::startServices() {
    taskbar_ = std::make_unique<Taskbar>(config_);

    // The app windows. They start hidden; the taskbar icons open them.
    UIManager& ui = UIManager::instance();
    ui.registerWindow(std::make_shared<FileExplorerWindow>());
    ui.registerWindow(std::make_shared<NotepadWindow>());
    ui.registerWindow(std::make_shared<TaskManagerWindow>(config_));

    // Boot screen (skipped entirely if boot_enabled=false or both times are 0)
    if (config_.bootEnabled && (config_.bootBiosMs > 0 || config_.bootLoadingMs > 0)) {
        bootScreen_ = std::make_unique<BootScreen>(config_, systemInfo_);
        state_ = State::Booting;
    } else {
        state_ = State::Running;
    }

    for (const auto& w : warnings_) Log::line("Warning: " + w);
    desktop_->setNotices(warnings_);
    Log::line("Phase 3 done: desktop, taskbar and " + std::to_string(ui.windows().size()) + " app windows ready");
}

// ===========================================================================
// Phase 4: the main loop
// ===========================================================================
void GUIApplication::run() {
    Log::line("Phase 4: entering main loop");
    while (!glfwWindowShouldClose(window_)) {
        // 1. Poll input: keyboard, mouse, window events. Never waits.
        glfwPollEvents();

        // Minimised? Nothing to draw -- sleep a little instead of spinning.
        if (glfwGetWindowAttrib(window_, GLFW_ICONIFIED)) {
            glfwWaitEventsTimeout(0.05);
            continue;
        }

        // 2. Start a new ImGui frame.
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 3. Update state, 4. declare everything on screen for this frame.
        updateLogic();
        renderFrame();

        // 5. Turn this frame's widgets into draw data.
        ImGui::Render();

        // 6. Clear the screen and draw it with OpenGL.
        int fbWidth = 0, fbHeight = 0;
        glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // 7. Show the finished frame (double buffering: no half-drawn frames).
        glfwSwapBuffers(window_);
    }
    Log::line("Phase 4: main loop ended");
}

void GUIApplication::updateLogic() {
    // Boot screen finished (or skipped): show the desktop.
    if (state_ == State::Booting && (!bootScreen_ || bootScreen_->finished())) {
        state_ = State::Running;
        bootScreen_.reset();
        desktop_->setNotices(warnings_);   // restart the notice timer now that they're visible
        Log::line("Boot complete: desktop running");
    }
    // PWR was clicked: start the shutdown screen (once).
    if (state_ == State::Running && UIManager::instance().powerOffRequested()) {
        state_ = State::ShuttingDown;
        shutdownStart_ = ImGui::GetTime();
        Log::line("PWR pressed: shutting down");
    }
    // Shutdown screen finished: end the main loop.
    if (state_ == State::ShuttingDown && (ImGui::GetTime() - shutdownStart_) * 1000.0 >= config_.shutdownMs) {
        glfwSetWindowShouldClose(window_, GLFW_TRUE);
    }
}

void GUIApplication::renderFrame() {
    if (state_ == State::Booting) {
        bootScreen_->draw();
        return;
    }
    if (state_ == State::ShuttingDown) {
        drawShutdownScreen();
        return;
    }
    // Back to front: desktop (bottom), app windows, taskbar (always on top).
    desktop_->draw();
    UIManager::instance().renderAllWindows();
    taskbar_->draw();
}

void GUIApplication::drawShutdownScreen() {
    // Same look as the loading screen, with "Shutting down..." instead.
    BootScreen::drawSplash(config_, "Shutting down");
}

// ===========================================================================
// Phase 5: shutdown -- reverse order of creation
// ===========================================================================
void GUIApplication::shutdown() {
    if (!window_) return;   // already done (or never started)

    bootScreen_.reset();
    UIManager::instance().clear();     // app windows
    taskbar_.reset();
    if (desktop_) desktop_->release(); // wallpaper texture (needs OpenGL still alive)
    desktop_.reset();

    if (imguiReady_) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        imguiReady_ = false;
    }
    glfwDestroyWindow(window_);
    window_ = nullptr;
    glfwTerminate();
    Log::line("Phase 5 done: shut down cleanly");
}
