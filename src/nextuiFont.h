#ifndef NEXTUI_FONT_H_
#define NEXTUI_FONT_H_

// Resolve the font selected in NextUI-CN Settings (minuisettings.txt).
// Mirrors workspace/all/common/config.c: CFG_resolveFontPath / CFG_setFontFile
// (feature/custom-font):
//   font=custom:<filename>  ->  $SDCARD_PATH/Fonts/<filename>
//   font=<id> / font=fontN.ttf / other basename  ->  $SDCARD_PATH/.system/res/<file>
//   legacy fontOrder=2 maps 0/1/2 to font1/font2/font3
// Falls back to $SDCARD_PATH/.system/res/font1.ttf when the chosen file is
// missing or the value is rejected (path separators, empty name).

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

#include <unistd.h>

namespace nextui {

inline const char *DefaultFontFile() { return "font1.ttf"; }
inline const char *CustomPrefix() { return "custom:"; }
inline const char *CustomDir() { return "Fonts"; }

inline std::string SdcardRoot()
{
    const char *env = std::getenv("SDCARD_PATH");
    if (env && env[0]) {
        return env;
    }
    if (access("/mnt/SDCARD", F_OK) == 0) {
        return "/mnt/SDCARD";
    }
    return "";
}

inline std::string MapFontValue(const std::string &value, int font_order)
{
    if (font_order >= 2 && value == "0") return "font1.ttf";
    if (font_order >= 2 && value == "1") return "font2.ttf";
    if (value == "2") return "font3.ttf";
    // backward compat: "0" -> font2.ttf, "1" -> font1.ttf
    if (value == "0") return "font2.ttf";
    if (value == "1") return "font1.ttf";
    return value;
}

inline std::string ResolveFontPath(const std::string &font_file, const std::string &root)
{
    if (font_file.empty() || root.empty()) {
        return "";
    }
    const char *prefix = CustomPrefix();
    const size_t plen = std::strlen(prefix);
    if (font_file.compare(0, plen, prefix) == 0) {
        const std::string name = font_file.substr(plen);
        if (name.empty() || name.find('/') != std::string::npos
            || name == "." || name == "..") {
            return "";
        }
        return root + "/" + CustomDir() + "/" + name;
    }
    if (font_file.find('/') != std::string::npos) {
        return "";
    }
    return root + "/.system/res/" + font_file;
}

inline std::string FallbackFontPath(const std::string &root)
{
    if (root.empty()) return "";
    return root + "/.system/res/" + DefaultFontFile();
}

// Returns a readable path, or empty if nothing is available.
inline std::string ResolveSystemFontPath()
{
    const std::string root = SdcardRoot();
    if (root.empty()) {
        return "";
    }

    std::string loaded;
    int font_order = 0;
    const std::string settings = root + "/.userdata/shared/minuisettings.txt";
    std::ifstream in(settings.c_str());
    if (in) {
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (line.compare(0, 5, "font=") == 0 && line.compare(0, 10, "fontOrder=") != 0
                && line.compare(0, 10, "fontStyle=") != 0) {
                loaded = line.substr(5);
            } else if (line.compare(0, 10, "fontOrder=") == 0) {
                font_order = std::atoi(line.c_str() + 10);
            }
        }
    }

    std::string mapped = loaded.empty() ? DefaultFontFile() : MapFontValue(loaded, font_order);
    std::string path = ResolveFontPath(mapped, root);
    if (path.empty() || access(path.c_str(), R_OK) != 0) {
        path = FallbackFontPath(root);
    }
    if (path.empty() || access(path.c_str(), R_OK) != 0) {
        return "";
    }
    return path;
}

inline int ResolveSystemFontStyle()
{
    const std::string root = SdcardRoot();
    if (root.empty()) return 0;
    const std::string settings = root + "/.userdata/shared/minuisettings.txt";
    std::ifstream in(settings.c_str());
    if (!in) return 0;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.compare(0, 10, "fontStyle=") == 0) {
            return std::atoi(line.c_str() + 10);
        }
    }
    return 0;
}

} // namespace nextui

#endif
