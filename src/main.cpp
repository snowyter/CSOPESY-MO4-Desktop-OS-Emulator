// main.cpp  --  ENTRY POINT of the CSOPESY Desktop OS Emulator (MO4)
// ---------------------------------------------------------------------------
// Reads config.txt, then hands control to GUIApplication, which runs the OS
// through its 5 phases (see GUIApplication.h).
// ---------------------------------------------------------------------------
#include <string>
#include <vector>

#include "Config.h"
#include "GUIApplication.h"
#include "Log.h"

int main() {
    Log::open("csopesy.log");
    Log::line("CSOPESY Desktop OS Emulator starting");

    // Settings: defaults from Config.h, overridden by config.txt.
    AppConfig config;
    std::vector<std::string> warnings;
    if (!loadConfig("config.txt", config, warnings)) {
        warnings.push_back("config.txt not found; using built-in defaults.");
    }

    int exitCode = 0;
    {
        GUIApplication app(config, warnings);
        if (app.initialize()) {   // phases 1-3
            app.run();            // phase 4 (until PWR)
            app.shutdown();       // phase 5
        } else {
            Log::line("Startup failed -- see the errors above.");
            exitCode = 1;
        }
    }

    Log::line("CSOPESY Desktop OS Emulator exited");
    Log::close();
    return exitCode;
}
