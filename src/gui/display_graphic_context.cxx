//  $Id: display_graphic_context.cxx,v 1.6 2003/08/16 20:51:28 grumbel Exp $
//
//  Pingus - A free Lemmings clone
//  Copyright (C) 2002 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <iostream>
#include <ClanLib/Display/Display/display.h>
#include <ClanLib/Display/Font/font.h>
#include <config.h>
#include "display_graphic_context.hxx"
#include "../color.hxx"
#include "../math.hxx"
#include "../sprite.hxx"

DisplayGraphicContext::DisplayGraphicContext (int x1_, int y1_, int x2_, int y2_,
					      int /*x_offset*/, int /*y_offset*/)
  : x1(x1_), y1(y1_), x2(x2_), y2(y2_), offset(-(x2_ - x1_) / 2.0f, -(y2_ - x1_) / 2.0f, 1.0f)
{
  center = Vector ((x2 - x1)/2.0f + x1,
		      (y2 - y1)/2.0f + y1);
  //std::cout << "View: " << x1 << ", " << y1 << ", " << x2 << ", " << y2 << std::endl;
}

DisplayGraphicContext::~DisplayGraphicContext ()
{
}

void
DisplayGraphicContext::set_offset (float x, float y)
{
  offset.x = x;
  offset.y = y;
}

CL_Rect
DisplayGraphicContext::get_clip_rect()
{
  // FIXME: Zooming isn't handled
  // FIXME: This might be of-by-one
  Vector pos = -offset - center;
  return CL_Rect (int(pos.x), int(pos.y),
                  int(pos.x + get_width () + 1), int(pos.y + get_height () + 1));
}

Vector
DisplayGraphicContext::get_offset ()
{
  return offset;
}

float
DisplayGraphicContext::get_zoom ()
{
  return offset.z;
}

void
DisplayGraphicContext::set_zoom (float new_zoom)
{
  offset.z = new_zoom;
  //std::cout << "Zoom: " << offset.z << std::endl;
}

void
DisplayGraphicContext::zoom_to (const CL_Rect & arg_rect)
{
  CL_Rect rect;

  rect.x1 = Math::min (arg_rect.x1, arg_rect.x2);
  rect.x2 = Math::max (arg_rect.x1, arg_rect.x2);
  rect.y1 = Math::min (arg_rect.y1, arg_rect.y2);
  rect.y2 = Math::max (arg_rect.y1, arg_rect.y2);

  Vector pos1 = screen_to_world (Vector(rect.x1, rect.y1));
  Vector pos2 = screen_to_world (Vector(rect.x2, rect.y2));

  Vector center_ = (pos2 + pos1) * 0.5f;
  offset = -center_;

  float width  = pos2.x - pos1.x;
  float height = pos2.y - pos1.y;

  if (width < 10 && height < 10)
    return ;

  float screen_relation = float(get_width ()) / float(get_height ());
  float rect_reation = width / height;

  if (rect_reation > screen_relation)
    {
      set_zoom (get_width () / (pos2.x - pos1.x));
    }
  else
    {
      set_zoom (get_height () / (pos2.y - pos1.y));
    }
}

int
DisplayGraphicContext::get_width ()
{
  return x2 - x1;
}

int
DisplayGraphicContext::get_height ()
{
  return y2 - y1;
}

void
DisplayGraphicContext::move (const Vector & delta)
{
  offset += delta;
}

Vector
DisplayGraphicContext::screen_to_world (Vector pos)
{
  return ((pos - center) * (1.0f/offset.z)) - offset;
}

Vector
DisplayGraphicContext::world_to_screen (Vector pos)
{
  return ((pos + offset) * offset.z) + center;
}

float
DisplayGraphicContext::get_x_offset ()
{
  return offset.x;
}

float
DisplayGraphicContext::get_y_offset ()
{
  return offset.y;
}

void
DisplayGraphicContext::clear (float r, float g, float b)
{
  CL_Display::clear_display(r, g, b);
}

void
DisplayGraphicContext::draw (CL_Surface& sur, int x_pos, int y_pos)
{
  if (offset.z == 1.0)
    {
      sur.put_screen(w2s_x(x_pos), w2s_y(y_pos));
    }
  else
    {
      sur.put_screen(w2s_x(x_pos), w2s_y(y_pos),
                     offset.z, offset.z);
    }
}

void
DisplayGraphicContext::draw (CL_Surface& sur, int x_pos, int y_pos, int frame)
{
  if (offset.z == 1.0)
    {
      sur.put_screen (w2s_x(x_pos), w2s_y(y_pos), frame);
    }
  else
    {
      sur.put_screen (w2s_x(x_pos), w2s_y(y_pos),
		      offset.z, offset.z,
		      frame);
    }
}

void
DisplayGraphicContext::draw (CL_Surface& sur, int x_pos, int y_pos,
	    float size_x, float size_y, int frame)
{
  sur.put_screen (w2s_x(x_pos), w2s_y(y_pos),
		  size_x * offset.z,
		  size_y * offset.z, frame);
}

void
DisplayGraphicContext::draw_line (const Vector& pos1, const Vector& pos2,
		       float r, float g, float b, float a)
{
  draw_line(int(pos1.x), int(pos1.y), int(pos2.x), int(pos2.y), r, g, b, a);
}

void
DisplayGraphicContext::draw_line (int x1, int y1, int x2, int y2,
		                  float r, float g, float b, float a)
{
  CL_Display::draw_line(w2s_x(x1), w2s_y(y1),
                        w2s_x(x2), w2s_y(y2),
                        r, g, b, a);
}

void
DisplayGraphicContext::draw_fillrect (int x1, int y1, int x2, int y2,
			              float r, float g, float b, float a)
{
  CL_Display::fill_rect(w2s_x(x1), w2s_y(y1),
                        w2s_x(x2), w2s_y(y2),
                        r, g, b, a);
}

void
DisplayGraphicContext::draw_rect (int x1, int y1, int x2, int y2,
                                  float r, float g, float b, float a)
{
  CL_Display::draw_rect(w2s_x(x1), w2s_y(y1),
                        w2s_x(x2), w2s_y(y2),
                        r, g, b, a);
}

void
DisplayGraphicContext::draw_pixel (int /*x_pos*/, int /*y_pos*/,
		   float /*r*/, float /*g*/, float /*b*/, float /*a*/)
{
  //CL_Display::put_pixel (x1 + get_x_offset (),
  //			 y1 + get_y_offset (), r, g, b, a);
  std::cout << "View::draw_pixel () not implemented" << std::endl;
}

void
DisplayGraphicContext::draw_circle (int x_pos, int y_pos, int radius,
				    float r, float g, float b, float a)
{
  // FIXME: Probally not the fast circle draw algo on this world...
  const float pi = 3.1415927f * 2.0f;
  const float steps = 8;
  Vector current (radius, 0);
  Vector next = current.rotate (pi/steps, Vector (0, 0, 1.0f));

  for (int i = 0; i < steps; ++i)
    {
      draw_line (int(x_pos + current.x), int(y_pos + current.y),
		 int(x_pos + next.x), int(y_pos + next.y),
		 r, g, b, a);
      current = next;
      next = next.rotate (pi/8, Vector (0, 0, 1.0f));
    }
}

void
DisplayGraphicContext::print_left (FontHandle font, int x_pos, int y_pos, const std::string& str)
{
  font->print_left(w2s_x(x_pos), w2s_y(y_pos), str.c_str ());
}

void
DisplayGraphicContext::print_center (FontHandle font, int x_pos, int y_pos, const std::string& str)
{
  font->print_center(w2s_x(x_pos), w2s_y(y_pos), str.c_str ());
}

void
DisplayGraphicContext::print_right (FontHandle font, int x_pos, int y_pos, const std::string& str)
{
  font->print_right(w2s_x(x_pos), w2s_y(y_pos), str.c_str ());
}

void
DisplayGraphicContext::clear(const Color& color)
{
  CL_Display::clear_display (color.red, color.green, color.blue, color.alpha);
}

/* EOF */