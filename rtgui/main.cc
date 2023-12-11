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
#include "rtoptions.h"
#include "rtimage.h"
#include "pathutils.h"

#include <gtkmm.h>
#include <giomm.h>
#include <iostream>
#include <clocale>

#ifndef _WIN32
#include <glibmm/fileutils.h>
#include <glib.h>
#else
#include "conio.h"
#include "windows.h"
#endif

// Stores path to data files.

// Search path for data files.
Glib::ustring argv0;

// TODO - CK: This will be handled by RTApplication in future.
// Single files from the command line to be opened.
Glib::ustring argv1;

Glib::ustring creditsPath;
Glib::ustring licensePath;

bool simpleEditor = false;
bool remote = false;
unsigned char initialGdkScale = 1;

int main (int argc, char **argv)
{
    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC, "C"); // to set decimal point to "."

    simpleEditor = false;
    remote = false;

    argv0 = "";
    argv1 = "";

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
        RTOptions::load();
    } catch (RTOptions::Error &e) {
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
