// Config.cpp
#include "Config.h"

#include <cctype>
#include <fstream>
#include <initializer_list>

namespace {

std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

std::string toLower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

// Whole number made only of digits, within [min, max]. Rejects "-5", "12.5",
// "abc" and anything with more than 9 digits (so it can't overflow an int).
bool parseInt(const std::string& text, int min, int max, int& out) {
    if (text.empty() || text.size() > 9) return false;
    for (char c : text) if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    int value = std::stoi(text);
    if (value < min || value > max) return false;
    out = value;
    return true;
}

bool parseBool(const std::string& text, bool& out) {
    std::string s = toLower(text);
    if (s == "true" || s == "yes" || s == "on" || s == "1")  { out = true;  return true; }
    if (s == "false" || s == "no" || s == "off" || s == "0") { out = false; return true; }
    return false;
}

// "#1E3C72" or "1E3C72" -> Color
bool parseColor(const std::string& text, Color& out) {
    std::string s = text;
    if (!s.empty() && s[0] == '#') s = s.substr(1);
    if (s.size() != 6) return false;
    for (char c : s) if (!std::isxdigit(static_cast<unsigned char>(c))) return false;
    out.r = std::stoi(s.substr(0, 2), nullptr, 16);
    out.g = std::stoi(s.substr(2, 2), nullptr, 16);
    out.b = std::stoi(s.substr(4, 2), nullptr, 16);
    return true;
}

bool oneOf(const std::string& value, std::initializer_list<const char*> options) {
    for (const char* o : options) if (value == o) return true;
    return false;
}

} // namespace

bool loadConfig(const std::string& path, AppConfig& config, std::vector<std::string>& warnings) {
    std::ifstream file(path);
    if (!file) return false;

    std::string line;
    int lineNo = 0;
    while (std::getline(file, line)) {
        ++lineNo;
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;          // blank or comment

        const std::string where = "config.txt line " + std::to_string(lineNo) + ": ";
        size_t eq = line.find('=');
        if (eq == std::string::npos) {
            warnings.push_back(where + "expected key=value.");
            continue;
        }
        const std::string key   = toLower(trim(line.substr(0, eq)));
        const std::string value = trim(line.substr(eq + 1));    // keep case (paths, titles)
        const std::string lower = toLower(value);
        auto bad = [&](const std::string& expected) {
            warnings.push_back(where + key + " '" + value + "' is not valid (" + expected + "); using default.");
        };

        if (key == "window_width") {
            if (!parseInt(value, 640, 3840, config.windowWidth)) bad("whole number 640-3840");
        } else if (key == "window_height") {
            if (!parseInt(value, 480, 2160, config.windowHeight)) bad("whole number 480-2160");
        } else if (key == "window_title") {
            if (value.empty()) bad("cannot be empty"); else config.windowTitle = value;
        } else if (key == "vsync") {
            if (!parseBool(value, config.vsync)) bad("true or false");
        } else if (key == "font_path") {
            if (value.empty()) bad("cannot be empty"); else config.fontPath = value;
        } else if (key == "font_size") {
            int size = 0;
            if (!parseInt(value, 10, 40, size)) bad("whole number 10-40"); else config.fontSize = static_cast<float>(size);
        } else if (key == "wallpaper_mode") {
            if (!oneOf(lower, {"image", "gradient"})) bad("image or gradient"); else config.wallpaperMode = lower;
        } else if (key == "wallpaper_path") {
            if (value.empty()) bad("cannot be empty"); else config.wallpaperPath = value;
        } else if (key == "wallpaper_fit") {
            if (!oneOf(lower, {"cover", "fit", "stretch"})) bad("cover, fit or stretch"); else config.wallpaperFit = lower;
        } else if (key == "gradient_top") {
            if (!parseColor(value, config.gradientTop)) bad("#RRGGBB");
        } else if (key == "gradient_bottom") {
            if (!parseColor(value, config.gradientBottom)) bad("#RRGGBB");
        } else if (key == "clock_24h") {
            if (!parseBool(value, config.clock24h)) bad("true or false");
        } else if (key == "boot_enabled") {
            if (!parseBool(value, config.bootEnabled)) bad("true or false");
        } else if (key == "boot_bios_ms") {
            if (!parseInt(value, 0, 60000, config.bootBiosMs)) bad("whole number 0-60000");
        } else if (key == "boot_loading_ms") {
            if (!parseInt(value, 0, 60000, config.bootLoadingMs)) bad("whole number 0-60000");
        } else if (key == "shutdown_ms") {
            if (!parseInt(value, 0, 60000, config.shutdownMs)) bad("whole number 0-60000");
        } else if (key == "taskmgr_refresh_ms") {
            if (!parseInt(value, 100, 60000, config.taskmgrRefreshMs)) bad("whole number 100-60000");
        } else {
            warnings.push_back(where + "unknown setting '" + key + "'.");
        }
    }
    return true;
}
