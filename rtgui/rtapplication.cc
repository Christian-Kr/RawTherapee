/*
 *  This file is part of RawTherapee.
 *
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

#include "rtapplication.h"
#include "rtoptions.h"
#include "extprog.h"
#include "cachemanager.h"
#include "version.h"

#include <glib/gstdio.h>
#include <tiffio.h>

#include <iostream>

RTApplication::RTApplication()
    : Gtk::Application(
            "com.rawtherapee.application",
            Gio::Application::Flags::HANDLES_OPEN)
{
    init_actions();
    init_main_options();
}

RTApplication::~RTApplication() = default;

void RTApplication::init_actions()
{
    // Create custom action to handle multiple windows in primary application instance.
    auto action_create_window = Gio::SimpleAction::create("action-create-window");
    action_create_window->signal_activate().connect(
            sigc::mem_fun(*this, &RTApplication::on_create_window));
    add_action(action_create_window);
}

void RTApplication::init_main_options()
{
    // Option for not starting a new window if one already exist.
    add_main_option_entry(
            Gio::Application::OptionType::BOOL, "remote", 'r',
            "Raise an already running RawTherapee instance (if available)", "",
            Glib::OptionEntry::Flags::NONE);

    // Show version of RawTherapee.
    add_main_option_entry(
            Gio::Application::OptionType::BOOL, "version", 'v',
            "Print RawTherapee version number and exit", "",
            Glib::OptionEntry::Flags::NONE);

    // Show detailed information like about in GUI application.
    add_main_option_entry(
            Gio::Application::OptionType::BOOL, "about", 'a',
            "Display about information", "",
            Glib::OptionEntry::Flags::NONE);
}

void RTApplication::create_window()
{
    auto window = new Gtk::Window();
    add_window(*window);
    window->set_visible(true);
}

void RTApplication::on_activate()
{
    auto windows = get_windows();

    if (windows.empty())
    {
        // Create a new window, if there is no.
        create_window();
    }
    else
    {
        // If the is one window at minimum, bring the first window in front of the application.
        windows.at(0)->present();
    }
}

void RTApplication::on_create_window(const Glib::VariantBase&)
{
    create_window();
}

void RTApplication::on_open(
        const Gio::Application::type_vec_files& files,
        const Glib::ustring& hint)
{
    create_window();

    // TODO - CK: Open a specific file.

    // TODO - CK: Rewrite tu be used here.
//    if (!remote && Glib::file_test(argv1, Glib::FileTest::EXISTS) &&
//               !Glib::file_test(argv1, Glib::FileTest::IS_DIR)) {
//        simpleEditor = true;
//    }
}

int RTApplication::on_handle_local_options(const Glib::RefPtr<Glib::VariantDict>& options)
{
    // If about (question mark) has been selected as an option, show detailed information.
    if (options->contains("about"))
    {
        printf("  An advanced, cross-platform program for developing raw photos.\n\n");
        printf("  Website: http://www.rawtherapee.com/\n");
        printf("  Documentation: http://rawpedia.rawtherapee.com/\n");
        printf("  Forum: https://discuss.pixls.us/c/software/rawtherapee\n");
        printf("  Code and bug reports: https://github.com/Beep6581/RawTherapee\n\n");

        return 0;
    }

    // If user wants to show the current version. Show it and exit application.
    if (options->contains("version"))
    {
        printf("RawTherapee, version %s\n", RTVERSION);

        return 0;
    }

    if (options->contains("remote"))
    {
        // Calling with option "remote" active. Try to bring a primary window in front or if no
        // window exist, create one. Therefore exit method with negative value indicating, that the
        // "normal" process (on_activate or on_open) should be followed.
        return -1;
    }
    else
    {
        // Calling without the "remote" option. Always create a new window. Therefore activate
        // custom action and return non-negative value to indicate, that the following "normal"
        // process of Application should be stopped. (No call of on_activate or on_open.)
        activate_action("action-create-window");

        return 0;
    }
}

void RTApplication::on_startup()
{
    Gtk::Application::on_startup();

    // TODO - CK: Finish initialization part.

    extProgStore->init();
    //SoundManager::init();

    if (!rtengine::settings->verbose)
    {
        TIFFSetWarningHandler(nullptr);
    }

#ifndef _WIN32
    // Move the old path to the new one if the new does not exist.
    auto rtdirCache = Glib::build_filename(RTOptions::rtdir, "cache");

    auto rtdirCacheIsDir = Glib::file_test(rtdirCache, Glib::FileTest::IS_DIR);
    auto cacheBaseDirIsDir = Glib::file_test(RTOptions::cacheBaseDir, Glib::FileTest::IS_DIR);

    if (rtdirCacheIsDir && !cacheBaseDirIsDir)
    {
        g_rename(rtdirCache.c_str(), RTOptions::cacheBaseDir.c_str());
    }
#endif

}
