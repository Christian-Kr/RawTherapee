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
#include "cursormanager.h"

#include "rtimage.h"

CursorManager mainWindowCursorManager;
CursorManager editWindowCursorManager;

void CursorManager::init (Glib::RefPtr<Gdk::Surface> mainWindow)
{

    display = Gdk::Display::get_default ();
#ifndef NDEBUG

    if (!display) {
        printf("Error: no default display!\n");
    }

#endif

    Glib::RefPtr<Gdk::Pixbuf> add           = RTImage::createPixbufFromFile("crosshair-hicontrast.png");
    Glib::RefPtr<Gdk::Pixbuf> colPick       = RTImage::createPixbufFromFile("color-picker-hicontrast.png");
    Glib::RefPtr<Gdk::Pixbuf> colPickAdd    = RTImage::createPixbufFromFile("color-picker-add-hicontrast.png");
    Glib::RefPtr<Gdk::Pixbuf> cropDraw      = RTImage::createPixbufFromFile("crop-point-hicontrast.png");
    Glib::RefPtr<Gdk::Pixbuf> crosshair     = RTImage::createPixbufFromFile("crosshair-hicontrast.png");
    Glib::RefPtr<Gdk::Pixbuf> empty         = RTImage::createPixbufFromFile("empty.png");
    Glib::RefPtr<Gdk::Pixbuf> handClosed    = RTImage::createPixbufFromFile("hand-closed-hicontrast.png");
    Glib::RefPtr<Gdk::Pixbuf> handOpen      = RTImage::createPixbufFromFile("hand-open-hicontrast.png");
    Glib::RefPtr<Gdk::Pixbuf> moveBL        = RTImage::createPixbufFromFile("node-move-sw-ne-hicontrast.png");
    Glib::RefPtr<Gdk::Pixbuf> moveBR        = RTImage::createPixbufFromFile("node-move-nw-se-hicontrast.png");
    Glib::RefPtr<Gdk::Pixbuf> moveL         = RTImage::createPixbufFromFile("node-move-x-hicontrast.png");
    Glib::RefPtr<Gdk::Pixbuf> moveR         = RTImage::createPixbufFromFile("node-move-x-hicontrast.png");
    Glib::RefPtr<Gdk::Pixbuf> moveTL        = RTImage::createPixbufFromFile("node-move-nw-se-hicontrast.png");
    Glib::RefPtr<Gdk::Pixbuf> moveTR        = RTImage::createPixbufFromFile("node-move-sw-ne-hicontrast.png");
    Glib::RefPtr<Gdk::Pixbuf> moveX         = RTImage::createPixbufFromFile("node-move-x-hicontrast.png");
    Glib::RefPtr<Gdk::Pixbuf> moveXY        = RTImage::createPixbufFromFile("node-move-xy-hicontrast.png");
    Glib::RefPtr<Gdk::Pixbuf> moveY         = RTImage::createPixbufFromFile("node-move-y-hicontrast.png");
    Glib::RefPtr<Gdk::Pixbuf> rotate        = RTImage::createPixbufFromFile("rotate-aroundnode-hicontrast.png");
    Glib::RefPtr<Gdk::Pixbuf> wait          = RTImage::createPixbufFromFile("gears.png"); // Currently unused, create *-hicontrast once used.

    double s = RTScalable::getTweakedDPI() / RTScalable::baseDPI;  // RTScalable::getDPI() might be preferable, however it imply a lot of work to support this option

    cAdd = add                  ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(add), (int)(8.*s), (int)(8.*s))           : Gdk::Cursor::create("copy");
    cAddPicker = colPickAdd     ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(colPickAdd), (int)(4.*s), (int)(21.*s))   : Gdk::Cursor::create("copy");
    cCropDraw = cropDraw        ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(cropDraw), (int)(3.*s), (int)(3.*s))      : Gdk::Cursor::create("all-cross");
    cCrosshair = crosshair      ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(crosshair), (int)(12.*s), (int)(12.*s))   : Gdk::Cursor::create("crosshair");
    cEmpty = empty              ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(empty), 12, 12) /* PNG: do not scale */   : Gdk::Cursor::create("none");
    cHandClosed = handClosed    ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(handClosed), (int)(12.*s), (int)(12.*s))  : Gdk::Cursor::create("pointer");
    cHandOpen = handOpen        ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(handOpen), (int)(12.*s), (int)(12.*s))    : Gdk::Cursor::create("pointer");
    cMoveBL = moveBL            ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(moveBL), (int)(12.*s), (int)(12.*s))      : Gdk::Cursor::create("sw-resize");
    cMoveBR = moveBR            ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(moveBR), (int)(12.*s), (int)(12.*s))      : Gdk::Cursor::create("se-resize");
    cMoveL = moveL              ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(moveL), (int)(12.*s), (int)(12.*s))       : Gdk::Cursor::create("w-resize");
    cMoveR = moveR              ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(moveR), (int)(12.*s), (int)(12.*s))       : Gdk::Cursor::create("e-resize");
    cMoveTL = moveTL            ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(moveTL), (int)(12.*s), (int)(12.*s))      : Gdk::Cursor::create("nw-resize");
    cMoveTR = moveTR            ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(moveTR), (int)(12.*s), (int)(12.*s))      : Gdk::Cursor::create("ne-resize");
    cMoveX = moveX              ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(moveX), (int)(12.*s), (int)(12.*s))       : Gdk::Cursor::create("col-resize");
    cMoveXY = moveXY            ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(moveXY), (int)(12.*s), (int)(12.*s))      : Gdk::Cursor::create("move");
    cMoveY = moveY              ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(moveY), (int)(12.*s), (int)(12.*s))       : Gdk::Cursor::create("row-size");
    cRotate = rotate            ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(rotate), (int)(12.*s), (int)(12.*s))      : Gdk::Cursor::create("default");
    cWB = colPick               ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(colPick), (int)(4.*s), (int)(21.*s))      : Gdk::Cursor::create("default");
    cWait = wait                ? Gdk::Cursor::create(Gdk::Texture::create_for_pixbuf(wait), (int)(12.*s), (int)(12.*s))        : Gdk::Cursor::create("progress");

    surface = mainWindow;
}

void CursorManager::cleanup()
{
    cAdd.reset();
    cAddPicker.reset();
    cCropDraw.reset();
    cCrosshair.reset();
    cHandClosed.reset();
    cHandOpen.reset();
    cEmpty.reset();
    cMoveBL.reset();
    cMoveBR.reset();
    cMoveL.reset();
    cMoveR.reset();
    cMoveTL.reset();
    cMoveTR.reset();
    cMoveX.reset();
    cMoveY.reset();
    cMoveXY.reset();
    cRotate.reset();
    cWB.reset();
    cWait.reset();
}

/* Set the cursor of the given surface */
void CursorManager::setCursor (Glib::RefPtr<Gdk::Surface> surface, CursorShape shape)
{
    switch (shape)
    {
        case CursorShape::CSAddColPicker:
            surface->set_cursor(cAddPicker);
            break;
        case CursorShape::CSArrow:
            surface->set_cursor(); // set_cursor without any arguments to select system default
            break;
        case CursorShape::CSCropSelect:
            surface->set_cursor(cCropDraw);
            break;
        case CursorShape::CSCrosshair:
            surface->set_cursor(cCrosshair);
            break;
        case CursorShape::CSEmpty:
            surface->set_cursor(cEmpty);
            break;
        case CursorShape::CSHandClosed:
            surface->set_cursor(cHandClosed);
            break;
        case CursorShape::CSHandOpen:
            surface->set_cursor(cHandOpen);
            break;
        case CursorShape::CSMove:
            surface->set_cursor(cHandClosed);
            break;
        case CursorShape::CSMove1DH:
            surface->set_cursor(cMoveX);
            break;
        case CursorShape::CSMove1DV:
            surface->set_cursor(cMoveY);
            break;
        case CursorShape::CSMove2D:
            surface->set_cursor(cMoveXY);
            break;
        case CursorShape::CSMoveLeft:
            surface->set_cursor(cMoveL);
            break;
        case CursorShape::CSMoveRight:
            surface->set_cursor(cMoveR);
            break;
        case CursorShape::CSMoveRotate:
            surface->set_cursor(cRotate);
            break;
        case CursorShape::CSPlus:
            surface->set_cursor(cAdd);
            break;
        case CursorShape::CSResizeBottomLeft:
            surface->set_cursor(cMoveBL);
            break;
        case CursorShape::CSResizeBottomRight:
            surface->set_cursor(cMoveBR);
            break;
        case CursorShape::CSResizeDiagonal:
            surface->set_cursor(cMoveXY);
            break;
        case CursorShape::CSResizeHeight:
            surface->set_cursor(cMoveY);
            break;
        case CursorShape::CSResizeTopLeft:
            surface->set_cursor(cMoveTL);
            break;
        case CursorShape::CSResizeTopRight:
            surface->set_cursor(cMoveTR);
            break;
        case CursorShape::CSResizeWidth:
            surface->set_cursor(cMoveX);
            break;
        case CursorShape::CSSpotWB:
            surface->set_cursor(cWB);
            break;
        case CursorShape::CSStraighten:
            surface->set_cursor(cRotate);
            break;
        case CursorShape::CSUndefined:
            break;
        case CursorShape::CSWait:
            surface->set_cursor(cWait);
            break;
        default:
            surface->set_cursor(cCrosshair);
    }
}

void CursorManager::setWidgetCursor (Glib::RefPtr<Gdk::Surface> surface, CursorShape shape)
{
    if (surface->get_display() == mainWindowCursorManager.display) {
        mainWindowCursorManager.setCursor(surface, shape);
    } else if (surface->get_display() == editWindowCursorManager.display) {
        editWindowCursorManager.setCursor(surface, shape);
    }

#ifndef NDEBUG
    else {
        printf("CursorManager::setWidgetCursor  /  Error: Display not found!\n");
    }

#endif
}

void CursorManager::setCursorOfMainWindow (Glib::RefPtr<Gdk::Surface> surface, CursorShape shape)
{
    if (surface->get_display() == mainWindowCursorManager.display) {
        mainWindowCursorManager.setCursor(shape);
    } else if (surface->get_display() == editWindowCursorManager.display) {
        editWindowCursorManager.setCursor(shape);
    }

#ifndef NDEBUG
    else {
        printf("CursorManager::setCursorOfMainWindow  /  Error: Display not found!\n");
    }

#endif
}

/* Set the cursor of the main window */
void CursorManager::setCursor (CursorShape shape)
{
    setCursor (surface, shape);
}

