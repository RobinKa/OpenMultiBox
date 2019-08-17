# OpenMultiBox
Multiboxing software for World of Warcraft (and perhaps other games, if you are willing to modify it slightly).

# Features
- Key broadcasting
- Mouse click broadcasting
- Set focused window as primary window
- Copy various WTF files
- Status UI, Window indicators, Mouse mirroring UI

# Guide
1. Download binaries from [Releases](https://github.com/RobinKa/OpenMultiBox/releases) or build them yourself with CMake (requires Qt5).
2. Edit `settings.json` (mainly setting the WoW path and instance count, the rest is for copying WTF settings)
3. Make sure your WoW uses borderless or windowed mode
4. Start `OpenMultiBox.exe` which will launch the WoW instances and align them
5. Log in and turn on key broadcasting with F10

# Hotkeys
- F2: Broadcast mouse left click
- F7: Toggles movement key (WASD) broadcasting
- F8: Toggles WoW window topmost setting (keeps the windows in front of all other windows)
- F9: Copy WTF files (account-wide settings / macro / config cache, per-character addon settings), requires `/console synchronizeSettings 0` and logging out so that settings don't get overriden from blizzard servers)
- F10: Enables keyboard broadcasting
- F11: Disables keyboard broadcasting

# Known major issues and to do list
- Window placement hardcoded (although still relative to screen size)

# Acknowledgements
- [Qt](https://www.qt.io/)
- [nlohmann json](https://github.com/nlohmann/json)