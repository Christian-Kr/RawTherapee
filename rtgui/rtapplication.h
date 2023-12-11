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
#pragma once

#include <gtkmm.h>

// Forward declarations.
class RTWindow;

/**
 * Main application class for RawTherapee.
 */
class RTApplication : public Gtk::Application
{

public:
    /**
     * Constructor
     */
    RTApplication();

    /**
     * Destructor
     */
    ~RTApplication() override;

    /**
     * RawTherapee initialization.
     */
    static void init();

protected:
    /**
     * Override gefault signal handler.
     */
    void on_activate() override;

    /**
     * Override default signal handler.
     */
    void on_startup() override;

    /**
     * Override default signal handler.
     *
     * @param files
     * @param hint
     */
    void on_open(
            const Gio::Application::type_vec_files& files,
            const Glib::ustring& hint) override;

    /**
     * Override default signal handler.
     *
     * @param options
     * @return
     */
    int on_handle_local_options(const Glib::RefPtr<Glib::VariantDict>& options) override;

    /**
     * Will be called, when a new window should be created.
     *
     * @param parameter Parameter given to the action or nullptr if empty.
     */
    void on_create_window(const Glib::VariantBase& parameter);

private:
    /**
     * Create the application window and make it visible.
     */
    void create_window();
};
