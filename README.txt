CSOPESY MO4 - Desktop-Style OS Mock-up
======================================
(CSOPESY Desktop OS Emulator, v1.0)
Section S04 - Group 10

Group members:
  David, Justin Ice
  Limpin, Kryster Knowell
  Singson, Keith Railey

Version date: October 12, 2026

Repository (SOURCE package + this README):
  https://github.com/snowyter/CSOPESY-MO4-Desktop-OS-Emulator

What this is (MO4 general instructions):
  An OS emulator needs a command interpreter and a display output.
  MO4 is the desktop-style display-output side: a Windows-like desktop
  (BIOS + loading boot screens, wallpaper, clock, taskbar, draggable
  and resizable windows) with three apps - File Explorer, Notepad and
  Task Manager. Input is via mouse and keyboard through the GUI.

Entry point:
  src/main.cpp  (contains the main() function)

Requirements:
  - Windows 10/11
  - VS Code with the "C/C++" extension (Microsoft)
  - WinLibs MinGW-w64 at C:\winlibs (same setup as MO3):
        winget install -e --id BrechtSanders.WinLibs.POSIX.UCRT --scope user
        move "%LOCALAPPDATA%\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64" C:\winlibs
  - Nothing else: Dear ImGui, GLFW and stb_image are included in external/.

How to run:
  1. Open THIS folder (the one containing src/ and .vscode/) in VS Code
     (File > Open Folder), or clone the repository above and open it.
  2. Press F5. The first build takes ~30 seconds (it compiles Dear ImGui);
     later builds only recompile the files you changed.
  3. Shut the OS down with the red PWR button in the taskbar (then confirm).
  Do NOT use the small play button at the top right of the editor
  ("Run C/C++ File"): it only compiles one file and fails with
  "undefined reference" errors. Use Run and Debug -> "Run CSOPESY" (F5).

  From a terminal in this folder instead:
     C:\winlibs\bin\mingw32-make -j4
     csopesy.exe

Using the OS:
  - Startup: BIOS screen -> loading screen -> desktop. Click or press any key
    to skip a screen.
  - Taskbar icons (left to right): File Explorer, Notepad, Task Manager.
    Click to open. Click the app in front again to minimise it; click a
    minimised app to bring it back. The X on a window closes the app.
  - File Explorer: double-click folders to open them, Back/Forward/Up
    buttons, or type a path in the address bar (e.g. C:\Program Files) and
    press Enter. Double-click a .txt file to open it in Notepad.
  - Notepad: type text; File > Save (Ctrl+S) / Save As; File > New (Ctrl+N);
    Edit > Insert date/time (F5); Format > Word wrap. Files are saved in a
    pretend file system in memory, so they are gone after shutdown.
  - Task Manager: Applications tab (open apps: End Task / Switch To),
    Processes tab (click a header to sort, select a row and press End
    Process; system processes can't be ended) and Performance tab (CPU and
    memory graphs). All numbers are dummy data.
  - PWR (bottom right): asks "Are you sure?", then shuts the OS down.

Settings (config.txt, no recompiling needed):
  window_width / window_height   window size (default 1280x720)
  wallpaper_mode                 image or gradient
  wallpaper_path                 image file (default assets/wallpaper.jpg)
  wallpaper_fit                  cover, fit or stretch
  gradient_top / gradient_bottom gradient colours, #RRGGBB
  font_path / font_size          UI font
  clock_24h                      true or false
  vsync                          true or false
  boot_enabled                   true = show BIOS + loading screens
  boot_bios_ms / boot_loading_ms how long each boot screen lasts (0 = skip it)
  shutdown_ms                    how long "Shutting down..." stays on screen
  taskmgr_refresh_ms             how often Task Manager's numbers change
  Bad values are ignored (default kept) and shown as a warning on screen.
  Startup messages are written to csopesy.log.

Code map (src/):
  main.cpp               reads config.txt, runs the 5 phases
  GUIApplication         window + main loop; Booting -> Running -> ShuttingDown
  BootScreen             BIOS, loading and shutdown screens
  Desktop                wallpaper, clock, status line, warnings
  Taskbar                OS logo, app icons, FPS, PWR button + confirmation
  UIManager              window registry (singleton): open/minimise/close apps
  AWindow                base class of every app window
  TaskManagerWindow      app: Task Manager (dummy processes, graphs)
  FileExplorerWindow     app: File Explorer
  NotepadWindow          app: Notepad
  VirtualFS              pretend in-memory file system used by the two apps
  Config, Theme, Icons, Texture, Log   helpers

Folders:
  src/        our code
  assets/     wallpaper.jpg (put the image here) and fonts/ (JetBrains Mono, OFL licence)
  external/   third-party libraries (Dear ImGui, GLFW, stb_image) - do not edit
  build/      compiled object files (created by make)

Third-party licences: external/imgui/LICENSE.txt, external/glfw/LICENSE.md,
assets/fonts/OFL.txt; stb_image is public domain.
