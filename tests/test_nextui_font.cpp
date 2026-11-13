#include "../src/nextuiFont.h"
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

static void write_file(const std::string &path, const std::string &body)
{
    std::ofstream out(path.c_str());
    out << body;
}

static void ensure_dir(const std::string &path)
{
    mkdir(path.c_str(), 0755);
}

int main()
{
    char tmpl[] = "/tmp/nextui-font-test-XXXXXX";
    const char *root = mkdtemp(tmpl);
    assert(root);
    setenv("SDCARD_PATH", root, 1);

    const std::string r(root);
    ensure_dir(r + "/.system");
    ensure_dir(r + "/.system/res");
    ensure_dir(r + "/.userdata");
    ensure_dir(r + "/.userdata/shared");
    ensure_dir(r + "/Fonts");
    write_file(r + "/.system/res/font1.ttf", "font1");
    write_file(r + "/.system/res/font2.ttf", "font2");
    write_file(r + "/.system/res/font3.ttf", "font3");
    write_file(r + "/Fonts/MyFont.ttf", "custom");

    // default (no settings) -> font1
    assert(nextui::ResolveSystemFontPath() == r + "/.system/res/font1.ttf");

    write_file(r + "/.userdata/shared/minuisettings.txt", "font=font2.ttf\n");
    assert(nextui::ResolveSystemFontPath() == r + "/.system/res/font2.ttf");

    write_file(r + "/.userdata/shared/minuisettings.txt", "font=1\n");
    assert(nextui::ResolveSystemFontPath() == r + "/.system/res/font1.ttf");

    write_file(r + "/.userdata/shared/minuisettings.txt", "fontOrder=2\nfont=0\n");
    assert(nextui::ResolveSystemFontPath() == r + "/.system/res/font1.ttf");

    write_file(r + "/.userdata/shared/minuisettings.txt", "fontOrder=2\nfont=1\n");
    assert(nextui::ResolveSystemFontPath() == r + "/.system/res/font2.ttf");

    write_file(r + "/.userdata/shared/minuisettings.txt", "font=2\n");
    assert(nextui::ResolveSystemFontPath() == r + "/.system/res/font3.ttf");

    write_file(r + "/.userdata/shared/minuisettings.txt", "font=custom:MyFont.ttf\n");
    assert(nextui::ResolveSystemFontPath() == r + "/Fonts/MyFont.ttf");

    // missing custom file falls back to font1
    write_file(r + "/.userdata/shared/minuisettings.txt", "font=custom:Missing.ttf\n");
    assert(nextui::ResolveSystemFontPath() == r + "/.system/res/font1.ttf");

    // rejected traversal falls back
    write_file(r + "/.userdata/shared/minuisettings.txt", "font=custom:../evil.ttf\n");
    assert(nextui::ResolveSystemFontPath() == r + "/.system/res/font1.ttf");

    write_file(r + "/.userdata/shared/minuisettings.txt", "fontStyle=1\nfont=font2.ttf\n");
    assert(nextui::ResolveSystemFontPath() == r + "/.system/res/font2.ttf");
    assert(nextui::ResolveSystemFontStyle() == 1);

    std::cout << "nextuiFont tests passed\n";
    return 0;
}
