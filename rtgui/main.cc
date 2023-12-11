/*
 *  This file is part of RawTherapee.
 *
 *  Copyright (c) 2004-2010 Gabor Horvath <hgabor@rawtherapee.com>
 *
 *  RawTherapee is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  RawTherapee is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with RawTherapee.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifdef __GNUC__
#if defined(__FAST_MATH__)
#error Using the -ffast-math CFLAG is known to lead to problems. Disable it to compile RawTherapee.
#endif
#endif

#include "config.h"
#include "rtapplication.h"
#include "options.h"
#include "rtimage.h"
#include "version.h"
#include "pathutils.h"

#include <gtkmm.h>
#include <giomm.h>
#include <iostream>
#include <tiffio.h>
#include <clocale>

#ifndef _WIN32
#include <glibmm/fileutils.h>
#include <glib.h>
#else
#include "conio.h"
#include "windows.h"
#endif

// Set this to 1 to make RT work when started with Eclipse and arguments, at least on Windows
// platform.
#define ECLIPSE_ARGS 0

// Stores path to data files.
Glib::ustring argv0;
Glib::ustring argv1;
Glib::ustring argv2;
Glib::ustring creditsPath;
Glib::ustring licensePath;

bool simpleEditor = false;
bool remote = false;
unsigned char initialGdkScale = 1;

// TODO - CK: Why is there a namespace here without a given name like 'RT'?
namespace {

/**
 * Process line command options.
 *
 * @param argc The number of arguments.
 * @param argv The given argument.
 * @returns 0 if process in batch has executed
 *          1 to start GUI (with a dir or file option)
 *         -1 if there is an error in parameters
 */
int processLineParams(int argc, char **argv)
{
    // Set return value to default, which is: Start GUI (with a dir or file option).
    auto ret = 1;

    // Go through every argument and process it.
    for (auto iArg = 1; iArg < argc; iArg++) {

        // Get a string from the argument to handle it more easily.
        std::string currParam(argv[iArg]);
        if (currParam.empty()) {
            continue;
        }

#if ECLIPSE_ARGS
        currParam = currParam.substr(1, currParam.length() - 2);
#endif

        if (currParam.at(0) == '-' && currParam.size() > 1) {
            switch (currParam.at(1)) {
                case '-':
                    // GTK --argument, we're skipping it
                    break;

#ifdef _WIN32
                case 'w': // This case is handled outside this function
                    break;
#endif

                case 'v':
                    printf("RawTherapee, version %s\n", RTVERSION);
                    ret = 0;
                    break;

#ifndef __APPLE__
                case 'R':
                    remote = true;

                    break;
#endif

                // No break here on purpose.

                case 'h':
                case '?':
                default: {
                    printf("  An advanced, cross-platform program for developing raw photos.\n\n");
                    printf("  Website: http://www.rawtherapee.com/\n");
                    printf("  Documentation: http://rawpedia.rawtherapee.com/\n");
                    printf("  Forum: https://discuss.pixls.us/c/software/rawtherapee\n");
                    printf("  Code and bug reports: https://github.com/Beep6581/RawTherapee\n\n");
                    printf("Symbols:\n");
                    printf("  <Chevrons> indicate parameters you can change.\n\n");
                    printf("Usage:\n");
                    printf("  %s <folder>           Start File Browser inside folder.\n",
                           Glib::path_get_basename(argv[0]).c_str());
                    printf("  %s <file>             Start Image Editor with file.\n\n",
                           Glib::path_get_basename(argv[0]).c_str());
                    std::cout << std::endl;
                    printf("Options:\n");

#ifdef _WIN32
                    printf("  -w Do not open the Windows console\n");
#endif

                    printf("  -v Print RawTherapee version number and exit\n");

#ifndef __APPLE__
                    printf("  -R Raise an already running RawTherapee instance (if available)\n");
#endif

                    printf("  -h -? Display this help message\n");

                    ret = -1;
                    break;
                }
            }
        } else {
            // First char of argument is NOT "-" or it has just the size of 1.

            // If the first argument has not been set for now. Set it with the given argv.
            if (argv1.empty()) {
                argv1 = Glib::ustring(fname_to_utf8(argv[iArg]));

#if ECLIPSE_ARGS
                argv1 = argv1.substr(1, argv1.length() - 2);
#endif

            }
            break;
        }
    }

    return ret;
}

} // namespace


int main (int argc, char **argv)
{
    setlocale (LC_ALL, "");
    setlocale (LC_NUMERIC, "C"); // to set decimal point to "."

    simpleEditor = false;
    remote = false;
    argv0 = "";
    argv1 = "";
    argv2 = "";

    Glib::init();  // called by Gtk::Main, but this may be important for thread handling, so we call it ourselves now
    Gio::init ();

#ifdef _WIN32
    if (GetFileType (GetStdHandle (STD_OUTPUT_HANDLE)) == 0x0003) {
        // started from msys2 console => do not buffer stdout
        setbuf(stdout, NULL);
    }
#endif

#ifdef BUILD_BUNDLE
    char exname[512] = {0};
    Glib::ustring exePath;
    // get the path where the rawtherapee executable is stored
#ifdef _WIN32
    WCHAR exnameU[512] = {0};
    GetModuleFileNameW (NULL, exnameU, 511);
    WideCharToMultiByte (CP_UTF8, 0, exnameU, -1, exname, 511, 0, 0 );
#else

    if (readlink ("/proc/self/exe", exname, 511) < 0) {
        strncpy (exname, argv[0], 511);
    }

#endif
    exePath = Glib::path_get_dirname (exname);

    // set paths
    if (Glib::path_is_absolute (DATA_SEARCH_PATH)) {
        argv0 = DATA_SEARCH_PATH;
    } else {
        argv0 = Glib::build_filename (exePath, DATA_SEARCH_PATH);
    }

    if (Glib::path_is_absolute (CREDITS_SEARCH_PATH)) {
        creditsPath = CREDITS_SEARCH_PATH;
    } else {
        creditsPath = Glib::build_filename (exePath, CREDITS_SEARCH_PATH);
    }

    if (Glib::path_is_absolute (LICENCE_SEARCH_PATH)) {
        licensePath = LICENCE_SEARCH_PATH;
    } else {
        licensePath = Glib::build_filename (exePath, LICENCE_SEARCH_PATH);
    }

    options.rtSettings.lensfunDbDirectory = LENSFUN_DB_PATH;
    options.rtSettings.lensfunDbBundleDirectory = LENSFUN_DB_PATH;
#else
    argv0 = DATA_SEARCH_PATH;
    creditsPath = CREDITS_SEARCH_PATH;
    licensePath = LICENCE_SEARCH_PATH;
    options.rtSettings.lensfunDbDirectory = LENSFUN_DB_PATH;
    options.rtSettings.lensfunDbBundleDirectory = LENSFUN_DB_PATH;
#endif

    Glib::ustring fatalError;

    try {
        Options::load();
    } catch (Options::Error &e) {
        fatalError = e.get_msg();
    }

    if (!remote && Glib::file_test(argv1, Glib::FileTest::EXISTS) &&
               !Glib::file_test(argv1, Glib::FileTest::IS_DIR)) {
        simpleEditor = true;
    }

    int ret = 0;

    if (options.pseudoHiDPISupport) {
        // Reading/updating GDK_SCALE early if it exists
        const gchar *gscale = g_getenv("GDK_SCALE");
        if (gscale && gscale[0] == '2') {
            initialGdkScale = 2;
        }
        // HOMBRE: On Windows, if resolution is set to 200%, Gtk internal variables are SCALE=2
        // and DPI=96
        g_setenv("GDK_SCALE", "1", true);
    }

    gtk_init ();

    RTApplication app;
    app.register_application();
    RTApplication::init();

    if (fatalError.empty() && remote) {
        // Start the remote version; Just open an existing instance if it exist.

        ret = app.run(argc, argv);
    } else {
        // Start a new GUI instance.

        // Add additional icon path.
        auto iconPath = Glib::build_filename(argv0, "images");
        auto defaultIconTheme = Gtk::IconTheme::get_for_display(Gdk::Display::get_default());

        defaultIconTheme->add_search_path(iconPath);

        if (fatalError.empty()) {
            app.run(argc, argv);
        } else {
            std::cout << Glib::ustring::compose("FATAL ERROR!\n\n%1", fatalError) << std::endl;
            ret = -2;
        }
    }

#ifdef _WIN32
    if (consoleOpened) {
        printf ("Press any key to exit RawTherapee\n");
        fflush(stdout);
        FlushConsoleInputBuffer (GetStdHandle (STD_INPUT_HANDLE));
        getch();
    }
#endif

    return ret;
}
