/*
 *  This file is part of RawTherapee.
 *
 *  Copyright (c) 2004-2010 Gabor Horvath <hgabor@rawtherapee.com>
 *  Copyright (c) 2018 Jean-Christophe FRISCH <natureh.510@gmail.com>
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

#include "rtimage.h"

#include <cassert>
#include <iostream>
#include <gtkmm/icontheme.h>
#include <unordered_map>

#include "../rtengine/settings.h"

namespace
{

struct GIconKey {
    Glib::RefPtr<const Gio::Icon> icon;
    /**
     * Icon size in pixels.
     */
    int icon_size;

    GIconKey()
        : icon_size(0)
    {}

    GIconKey(const Glib::RefPtr<const Gio::Icon> &icon, int icon_size): icon(icon), icon_size(icon_size) {}

    bool operator==(const GIconKey &other) const
    {
        bool icons_match =
                (icon.get() == nullptr && other.icon.get() == nullptr) ||
                (icon.get() != nullptr && icon->equal(
                        std::const_pointer_cast<Gio::Icon>(other.icon)));
        return icons_match && icon_size == other.icon_size;
    }
};

struct GIconKeyHash {
    size_t operator()(const GIconKey &key) const
    {
        const size_t icon_hash = key.icon ? key.icon->hash() : 0;
        return icon_hash ^ std::hash<int>()(key.icon_size);
    }
};

std::unordered_map<GIconKey, Glib::RefPtr<Gdk::Pixbuf>, GIconKeyHash> gIconPixbufCache;
std::map<std::string, Glib::RefPtr<Gdk::Pixbuf> > pixbufCache;
std::map<std::string, Cairo::RefPtr<Cairo::ImageSurface> > surfaceCache;

}

double RTImage::dpiBack = 0.;
int RTImage::scaleBack = 0;

RTImage::RTImage ()
    : gIconSize(0)
{}

RTImage::RTImage (RTImage &other)
    : surface(other.surface)
    , pixbuf(other.pixbuf)
    , gIconSize(0)
{
    if (pixbuf) {
        set(pixbuf);
    } else if (surface) {
        set(Gdk::Pixbuf::create(surface, 0, 0, surface->get_width(), surface->get_height()));
    } else if (other.gIcon) {
        changeImage(other.gIcon, other.gIconSize);
    }
}

RTImage::RTImage (const Glib::ustring& fileName, const Glib::ustring& rtlFileName)
    : Gtk::Image()
    , gIconSize(0)
{
    setImage (fileName, rtlFileName);
}

RTImage::RTImage (Glib::RefPtr<Gdk::Pixbuf> &pbuf)
    : gIconSize(0)
{
    if (surface) {
        surface.reset();
    }
    if (pbuf) {
        set(pbuf);
        this->pixbuf = pbuf;
    }
}

RTImage::RTImage (Cairo::RefPtr<Cairo::ImageSurface> &surf)
    : gIconSize(0)
{
    if (pixbuf) {
        pixbuf.reset();
    }
    if (surf) {
        surface = surf;
        set(Gdk::Pixbuf::create(surface, 0, 0, surface->get_width(), surface->get_height()));
    }
}

RTImage::RTImage (Glib::RefPtr<RTImage> &other)
    : gIconSize(0)
{
    if (other) {
        if (other->get_surface()) {
            surface = other->get_surface();
            set(Gdk::Pixbuf::create(surface, 0, 0, surface->get_width(), surface->get_height()));
        } else if (other->pixbuf) {
            pixbuf = other->get_pixbuf_from_internal_surface();
            set(pixbuf);
        } else if (other->gIcon) {
            changeImage(other->gIcon, other->gIconSize);
        }
    }
}

RTImage::RTImage(const Glib::RefPtr<const Gio::Icon> &gIcon, Gtk::IconSize size)
    : gIconSize(0)
{
    changeImage(gIcon, size);
}

int RTImage::iconSizeToPixels(Gtk::IconSize size) const
{
    int width, height;

    // TODO: Can't find a direct replacement, so I just set it to fixed values for now.
    // Gtk::IconSize::lookup(size, width, height);
    width = height = 32;

    return (int) std::round(getTweakedDPI() / baseDPI * std::max(width, height));
}

void RTImage::setImage (const Glib::ustring& fileName, const Glib::ustring& rtlFileName)
{
    Glib::ustring imageName;

    if (!rtlFileName.empty() && getDirection() == Gtk::TextDirection::RTL) {
        imageName = rtlFileName;
    } else {
        imageName = fileName;
    }

    changeImage (imageName);
}

/*
 * On windows, if scale = 2, the dpi is non significant, i.e. should be considered = 192
 */
void RTImage::setDPInScale (const double newDPI, const int newScale)
{
    if (scaleBack != newScale || (scaleBack == 1 && dpiBack != newDPI)) {
        RTScalable::setDPInScale(newDPI, newScale);
        dpiBack = getDPI();
        scaleBack = getScale();
        updateImages();
    }
}

void RTImage::changeImage(const Glib::RefPtr<const Gio::Icon> &gIcon, int size)
{
    clear();

    pixbuf.reset();
    surface.reset();
    this->gIcon = gIcon;

    if (!gIcon) {
        return;
    }

    gIconSize = size;
    GIconKey key(gIcon, gIconSize);
    auto iterator = gIconPixbufCache.find(key);

    if (iterator == gIconPixbufCache.end()) {
        auto icon_pixbuf = createPixbufFromGIcon(gIcon, gIconSize);
        iterator = gIconPixbufCache.emplace(key, icon_pixbuf).first;
    }

    set(iterator->second);
}

void RTImage::changeImage(const Glib::RefPtr<const Gio::Icon> &gIcon, Gtk::IconSize size)
{
    changeImage(gIcon, iconSizeToPixels(size));
}

void RTImage::changeImage (const Glib::ustring& imageName)
{
    clear ();

    gIcon.reset();

    if (imageName.empty()) {
        return;
    }

    if (pixbuf) {
        auto iterator = pixbufCache.find (imageName);
        assert(iterator != pixbufCache.end ());
        pixbuf = iterator->second;
        set(iterator->second);
    } else  {  // if no Pixbuf is set, we update or create a Cairo::ImageSurface
        auto iterator = surfaceCache.find (imageName);
        if (iterator == surfaceCache.end ()) {
            auto surf = createImgSurfFromFile(imageName);
            iterator = surfaceCache.emplace (imageName, surf).first;
        }
        surface = iterator->second;
        set(Gdk::Pixbuf::create(surface, 0, 0, surface->get_width(), surface->get_height()));
    }
}

Cairo::RefPtr<Cairo::ImageSurface> RTImage::get_surface()
{
    return surface;
}

Cairo::RefPtr<Gdk::Pixbuf> RTImage::get_pixbuf_from_internal_surface() const
{
    return Gdk::Pixbuf::create(surface, 0, 0, surface->get_width(), surface->get_height());
}

int RTImage::get_width()
{
    if (surface) {
        return surface->get_width();
    }
    if (pixbuf) {
        return pixbuf->get_width();
    }

    if (gIcon) {
        return this->get_pixbuf_from_internal_surface()->get_width();
    }

    return -1;
}

int RTImage::get_height()
{
    if (surface) {
        return surface->get_height();
    }
    if (pixbuf) {
        return pixbuf->get_height();
    }

    if (gIcon) {
        return this->get_pixbuf_from_internal_surface()->get_height();
    }

    return -1;
}

void RTImage::init()
{
    dpiBack = RTScalable::getDPI();
    scaleBack = RTScalable::getScale();
}

void RTImage::cleanup(bool all)
{
    for (auto& entry : pixbufCache) {
        entry.second.reset();
    }
    for (auto& entry : surfaceCache) {
        entry.second.reset();
    }

    for (auto& entry : gIconPixbufCache) {
        entry.second.reset();
    }

    RTScalable::cleanup(all);
}

void RTImage::updateImages()
{
    for (auto& entry : pixbufCache) {
        entry.second = createPixbufFromFile(entry.first);
    }
    for (auto& entry : surfaceCache) {
        entry.second = createImgSurfFromFile(entry.first);
    }

    for (auto& entry : gIconPixbufCache) {
        entry.second = createPixbufFromGIcon(entry.first.icon, entry.first.icon_size);
    }
}

Glib::RefPtr<Gdk::Pixbuf> RTImage::createPixbufFromFile (const Glib::ustring& fileName)
{
    Cairo::RefPtr<Cairo::ImageSurface> imgSurf = createImgSurfFromFile(fileName);
    return Gdk::Pixbuf::create(imgSurf, 0, 0, imgSurf->get_width(), imgSurf->get_height());
}

Glib::RefPtr<Gdk::Pixbuf> RTImage::createPixbufFromGIcon(
        const Glib::RefPtr<const Gio::Icon> &icon,
        int size)
{
    // TODO: Listen for theme changes and update icon, remove from cache.
    auto iconPaintable = Gtk::IconTheme::get_for_display(
            Gdk::Display::get_default())->lookup_icon(
            icon, size, 1, Gtk::TextDirection::LTR, Gtk::IconLookupFlags::FORCE_REGULAR);
    try {
        auto iconFile = iconPaintable->get_file();
        if (iconFile == nullptr) {
            return {};
        }
        return Gdk::Pixbuf::create_from_file(iconFile->get_path());
    } catch (std::exception &e) {
        return {};
    }
}

Cairo::RefPtr<Cairo::ImageSurface> RTImage::createImgSurfFromFile (const Glib::ustring& fileName)
{
    Cairo::RefPtr<Cairo::ImageSurface> surf;

    try {
        surf = loadImage(fileName, getTweakedDPI());

        // HOMBRE: As of now, GDK_SCALE is forced to 1, so setting the Cairo::ImageSurface scale is not required
        /*
        double x=0., y=0.;
        cairo_surface_get_device_scale(surf->cobj(), &x, &y);
        if (getScale() == 2) {
            cairo_surface_set_device_scale(surf->cobj(), 0.5, 0.5);
            cairo_surface_get_device_scale(surf->cobj(), &x, &y);
            surf->flush();
        }
        */
    } catch (const std::exception& exception) {
        if (rtengine::settings->verbose) {
            std::cerr << "Failed to load image \"" << fileName << "\": " << exception.what() << std::endl;
        }
    }

    return surf;
}
