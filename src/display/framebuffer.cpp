//  Pingus - A free Lemmings clone
//  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "../math.hpp"
#include "display.hpp"
#include "framebuffer.hpp"

namespace {

void draw_pixel16(int x, int y, const Color& c)
{
  Uint32 color = SDL_MapRGBA(Display::get_screen()->format, c.r, c.g, c.b, c.a);

  if (c.a < 255) {
    Uint16 *p;
    unsigned long dp;
    unsigned char alpha;

    // Loses precision for speed
    alpha = (255 - c.a) >> 3;

    p = &((Uint16 *)Display::get_screen()->pixels)[x + y * Display::get_screen()->w];
    color = (((color << 16) | color) & 0x07E0F81F);
    dp = *p;
    dp = ((dp << 16) | dp) & 0x07E0F81F;
    dp = ((((dp - color) * alpha) >> 5) + color) & 0x07E0F81F;
    *p = (Uint16)((dp >> 16) | dp);
  } else {
    ((Uint16 *)Display::get_screen()->pixels)[x + y * Display::get_screen()->w] = color;
  }
}

void draw_pixel32(int x, int y, const Color& c)
{
  Uint32 color = SDL_MapRGBA(Display::get_screen()->format, c.r, c.g, c.b, c.a);

  if (c.a < 255) {
    Uint32 *p;
    unsigned long sp2;
    unsigned long dp1;
    unsigned long dp2;
    unsigned char alpha;

    alpha = 255 - c.a;

    p = &((Uint32*)Display::get_screen()->pixels)[x + y * Display::get_screen()->w];

    sp2 = (color & 0xFF00FF00) >> 8;
    color &= 0x00FF00FF;

    dp1 = *p;
    dp2 = (dp1 & 0xFF00FF00) >> 8;
    dp1 &= 0x00FF00FF;

    dp1 = ((((dp1 - color) * alpha) >> 8) + color) & 0x00FF00FF;
    dp2 = ((((dp2 - sp2) * alpha) >> 8) + sp2) & 0x00FF00FF;
    *p = (dp1 | (dp2 << 8));
  } else {
    ((Uint32 *)Display::get_screen()->pixels)[x + y * Display::get_screen()->w] = color;
  }
}

typedef void (*draw_pixel_func)(int, int, const Color&);
draw_pixel_func get_draw_pixel()
{
  switch (Display::get_screen()->format->BitsPerPixel)
    {
      case 16:
        return draw_pixel16;
      case 32:
        return draw_pixel32;
    }
  return NULL;
}

void draw_vline(int x, int y, int length, const Color& color)
{
  draw_pixel_func draw_pixel = get_draw_pixel();
  if (!draw_pixel)
    return;

  SDL_LockSurface(Display::get_screen());
  for (int i = 0; i < length; ++i) {
    draw_pixel(x, y + i, color);
  }
  SDL_UnlockSurface(Display::get_screen());
}

void draw_hline(int x, int y, int length, const Color& color)
{
  draw_pixel_func draw_pixel = get_draw_pixel();
  if (!draw_pixel)
    return;

  SDL_LockSurface(Display::get_screen());
  for (int i = 0; i < length; ++i) {
    draw_pixel(x + i, y, color);
  }
  SDL_UnlockSurface(Display::get_screen());
}

SDL_Rect Intersection(SDL_Rect* r1, SDL_Rect* r2)
{
  SDL_Rect rect;
  rect.x = Math::max(r1->x, r2->x);
  rect.y = Math::max(r1->y, r2->y);
  int endx = Math::min(r1->x + r1->w, r2->x + r2->w);
  rect.w = Math::max(endx - rect.x, 0);
  int endy = Math::min(r1->y + r1->h, r2->y + r2->h);
  rect.h = Math::max(endy - rect.y, 0);
  return rect;
}

void clip(int& i, int min, int max)
{
  if (i < min)
    i = min;
  else if (i > max)
    i = max;
}

} // namespace

Framebuffer::Framebuffer(SDL_Surface* screen_)
  : screen(screen_)
{
}

Framebuffer::~Framebuffer()
{
}

void
Framebuffer::draw_surface(SDL_Surface* sur, const Vector2i& pos)
{
}

void
Framebuffer::draw_surface(SDL_Surface* sur, const Vector2i& pos, const Rect& rect)
{
}

void
Framebuffer::draw_line(const Vector2i& pos1, const Vector2i& pos2, const Color& color)
{
  int x, y, xlen, ylen, incr;
  int sx = pos1.x;
  int sy = pos1.y;
  int dx = pos2.x;
  int dy = pos2.y;
  void (*draw_pixel)(int x, int y, const Color& color);
  int clipx1, clipx2, clipy1, clipy2;
  SDL_Rect rect;

  SDL_GetClipRect(Display::get_screen(), &rect);
  clipx1 = rect.x;
  clipx2 = rect.x + rect.w - 1;
  clipy1 = rect.y;
  clipy2 = rect.y + rect.h - 1;

  // vertical line
  if (sx == dx) {
    if (sx < clipx1 || sx > clipx2 || (sy < clipy1 && dy < clipy1) || (sy > clipy2 && dy > clipy2)) {
      return;
    }
    clip(sy, clipy1, clipy2);
    clip(dy, clipy1, clipy2);
    if (sy < dy) {
      draw_vline(sx, sy, dy - sy + 1, color);
    } else {
      draw_vline(dx, dy, sy - dy + 1, color);
    }
    return;
  }

  // horizontal
  if (sy == dy) {
    if (sy < clipy1 || sy > clipy2 || (sx < clipx1 && dx < clipx1) || (sx > clipx2 && dx > clipx2)) {
      return;
    }
    clip(sx, clipx1, clipx2);
    clip(dx, clipx1, clipx2);
    if (sx < dx) {
      draw_hline(sx, sy, dx - sx + 1, color);
    } else {
      draw_hline(dx, dy, sx - dx + 1, color);
    }
    return;
  }

  draw_pixel = get_draw_pixel();
  if (!draw_pixel) {
    return;
  }

  // exchange coordinates
  if (sy > dy) {
    int t = dx;
    dx = sx;
    sx = t;
    t = dy;
    dy = sy;
    sy = t;
  }
  ylen = dy - sy;

  if (sx > dx) {
    xlen = sx - dx;
    incr = -1;
  } else {
    xlen = dx - sx;
    incr = 1;
  }

  y = sy;
  x = sx;

  if (xlen > ylen) {
    if (sx > dx) {
      int t = sx;
      sx = dx;
      dx = t;
      y = dy;
    }

    int p = (ylen << 1) - xlen;

    SDL_LockSurface(screen);
    for (x = sx; x < dx; ++x) {
      if (x >= clipx1 && x <= clipx2 && y >= clipy1 && y <= clipy2) {
        draw_pixel(x, y, color);
      }
      if (p >= 0) {
	y += incr;
	p += (ylen - xlen) << 1;
      } else {
	p += (ylen << 1);
      }
    }
    SDL_UnlockSurface(screen);
    return;
  }

  if (ylen > xlen) {
    int p = (xlen << 1) - ylen;

    SDL_LockSurface(screen);
    for (y = sy; y < dy; ++y) {
      if (x >= clipx1 && x <= clipx2 && y >= clipy1 && y <= clipy2) {
        draw_pixel(x, y, color);
      }
      if (p >= 0) {
	x += incr;
	p += (xlen - ylen) << 1;
      } else {
	p += (xlen << 1);
      }
    }
    SDL_UnlockSurface(screen);
    return;
  }

  // Draw a diagonal line
  if (ylen == xlen) {
    SDL_LockSurface(screen);
    while (y != dy) {
      if (x >= clipx1 && x <= clipx2 && y >= clipy1 && y <= clipy2) {
        draw_pixel(x, y, color);
      }
      x += incr;
      ++y;
    }
    SDL_UnlockSurface(screen);
  }
}

void
Framebuffer::draw_rect(const Rect& rect, const Color& color)
{
  draw_line(Vector2i(rect.left,    rect.top),      Vector2i(rect.right-1, rect.top),      color);
  draw_line(Vector2i(rect.left,    rect.bottom-1), Vector2i(rect.right-1, rect.bottom-1), color);
  draw_line(Vector2i(rect.left,    rect.top),      Vector2i(rect.left,    rect.bottom-1), color);
  draw_line(Vector2i(rect.right-1, rect.top),      Vector2i(rect.right-1, rect.bottom-1), color);
}

void
Framebuffer::fill_rect(const Rect& rect_, const Color& color)
{
  Rect rect = rect_;
  rect.normalize();

  if (color.a == 255)
    {
      SDL_Rect srcrect;

      srcrect.x = rect.left;
      srcrect.y = rect.top;
      srcrect.w = rect.get_width();
      srcrect.h = rect.get_height();

      SDL_FillRect(screen, &srcrect, SDL_MapRGB(screen->format, color.r, color.g, color.b));
    }
  else if (color.a != 0)
    {
      int top, bottom, left, right;
      int clipx1, clipx2, clipy1, clipy2;
      SDL_Rect cliprect;

      SDL_GetClipRect(screen, &cliprect);
      clipx1 = cliprect.x;
      clipx2 = cliprect.x + cliprect.w - 1;
      clipy1 = cliprect.y;
      clipy2 = cliprect.y + cliprect.h - 1;

      if (rect.right < clipx1 || rect.left > clipx2 || rect.bottom < clipy1 || rect.top > clipy2)
        return;

      top = rect.top < clipy1 ? clipy1 : rect.top;
      bottom = rect.bottom > clipy2 ? clipy2 : rect.bottom;
      left = rect.left < clipx1 ? clipx1 : rect.left;
      right = rect.right > clipx2 ? clipx2 : rect.right;

      draw_pixel_func draw_pixel = get_draw_pixel();
      if (!draw_pixel)
        return;

      SDL_LockSurface(screen);
      for (int j = top; j <= bottom; ++j) {
        for (int i = left; i <= right; ++i) {
          draw_pixel(i, j, color);
        }
      }
      SDL_UnlockSurface(screen);
    }
}

void
Framebuffer::flip()
{
  SDL_Flip(screen);
}

Size
Framebuffer::get_size()
{
  return Size(screen->w, screen->h);
}

void
Framebuffer::set_video_mode(int width, int height)
{
}
  
void
Framebuffer::clear()
{
  SDL_FillRect(screen, NULL, SDL_MapRGB(Display::get_screen()->format, 0, 0, 0));
}

void
Framebuffer::push_cliprect(const Rect& rect)
{
  SDL_Rect sdl_rect;
  sdl_rect.x = rect.left;
  sdl_rect.y = rect.top;
  sdl_rect.w = rect.get_width();
  sdl_rect.h = rect.get_height();

  if (!cliprect_stack.empty())
    {
      sdl_rect = Intersection(&cliprect_stack.back(), &sdl_rect);
    }
  
  cliprect_stack.push_back(sdl_rect);
  SDL_SetClipRect(screen, &cliprect_stack.back());
}

void
Framebuffer::pop_cliprect()
{
  cliprect_stack.pop_back();
  if (cliprect_stack.empty())
    SDL_SetClipRect(screen, NULL);
  else
    SDL_SetClipRect(screen, &cliprect_stack.back());
}

/* EOF */
