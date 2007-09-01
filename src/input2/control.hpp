/*  $Id$
**
**  Pingus - A free Lemmings clone
**  Copyright (C) 2007 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
** 
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
**  02111-1307, USA.
*/

#ifndef HEADER_INPUT_CONTROL_HPP
#define HEADER_INPUT_CONTROL_HPP

#include <iostream>
#include <vector>

#include "math.hpp"
#include "math/vector2f.hpp"
#include "event.hpp"

namespace Input {

class Control {
private:
  Control* parent;
  
public: 
  Control(Control* parent_) 
    : parent(parent_)
  {}

  virtual ~Control() {
  }

  virtual void notify_parent() 
  {
    if (parent)
      {
        parent->update(this);
      }
    else
      {
        std::cout << "Input: Control: Error: parent missing! " << std::endl;
      }
  }

  virtual void update(Control* ctrl) {
    std::cout << "Warning: Control:update() not handled" << std::endl;
  }
};

class Button : public Control 
{
protected:
  ButtonState state;

public:
  Button(Control* parent)
    : Control(parent),
      state(BUTTON_RELEASED)
  {}  

  bool get_state() const { return state; }

  virtual void set_state(ButtonState new_state) 
  {
    if (new_state != state) 
      {
        state = new_state;
        notify_parent();
      }
  }
};

class ButtonGroup : public Button 
{
private:
  std::vector<Button*> buttons;
  
public: 
  ButtonGroup(Control* parent)
    : Button(parent)
  {}

  void add_button(Button* button) {
    buttons.push_back(button);
  }

  virtual void update(Control* ctrl)
  {
    ButtonState new_state = BUTTON_RELEASED;

    for(std::vector<Button*>::iterator i = buttons.begin(); 
        i != buttons.end(); ++i)
      {
        if ((*i)->get_state() == BUTTON_PRESSED)
          new_state = BUTTON_PRESSED;
      }

    if (new_state != state)
      {
        state = new_state;
        notify_parent();
      }
  }
};

class ControllerButton : public ButtonGroup
{
private:
  int id;

public:
  ControllerButton(int id_)
    : ButtonGroup(0),
      id(id_)
  {}

  virtual void notify_parent() {
    std::cout << "ControlButton: id=" << id << " was " << (state == BUTTON_PRESSED
                                                ? "pressed" : "released") << std::endl;    
  }
};

class Axis : public Control 
{
protected:
  float pos;

public:
  Axis(Control* parent)
    : Control(parent),
      pos(0.0f)
  {}

  float get_pos() const { return pos; }

  virtual void set_state(float new_pos) {
    if (new_pos != pos)
      {
        pos = new_pos;
        notify_parent();
      }
  }
};

class Pointer : public Control 
{
protected:
  Vector2f pos;

public:
  Pointer(Control* parent)
    : Control(parent)
  {}

  Vector2f get_pos() const { return pos; }

  void set_pos(const Vector2f& new_pos) {
    if (pos != new_pos) 
      {
        pos = new_pos;
        notify_parent();
      }
  }
};

class Scroller : public Control 
{
protected:
  Vector2f delta;
  
public:
  Scroller(Control* parent)
    : Control(parent),
      delta(0.0f, 0.0f)
  {}

  Vector2f get_delta() const { return delta; }

  void set_delta(const Vector2f& new_delta) {
    if (delta != new_delta) 
      {
        delta = new_delta;
        notify_parent();
      }
  }
};

class AxisGroup : public Axis {
private:
  std::vector<Axis*> axes;

public:
  AxisGroup(Control* parent)
    : Axis(parent)
  {}

  void add_axis(Axis* axis) {
    axes.push_back(axis);
  }

  void update(Control* ctrl) 
  {
    float new_pos = 0;
    
    for(std::vector<Axis*>::iterator i = axes.begin(); i != axes.end(); ++i)
      {
        new_pos += (*i)->get_pos();
      }

    new_pos = Math::clamp(-1.0f, new_pos, 1.0f);
    
    if (pos != new_pos)
      {
        pos = new_pos;
        notify_parent();
      }
  }
};

class ControllerAxis : public AxisGroup 
{
private:
  int id;

public:
  ControllerAxis(int id_) 
    : AxisGroup(0),
      id(id_)
  {}

  virtual void notify_parent() {
    std::cout << "ControlAxis moved id=" << id << " " << pos  << std::endl;
  }
};

class PointerGroup : public Pointer 
{
private:
  std::vector<Pointer*> pointer;

public:
  PointerGroup(Control* parent)
    : Pointer(parent)
  {}

  void update(Control* p) {
    Pointer* pointer = dynamic_cast<Pointer*>(p);
    assert(pointer);
    Vector2f new_pos = pointer->get_pos();
    if (new_pos != pos)
      {
        pos = new_pos;
        notify_parent();
      }
  }

  void add_pointer(Pointer* p) {
    pointer.push_back(p);
  }
};

class ControllerPointer : public PointerGroup
{
private:
  int id;

public:
  ControllerPointer(int id_)
    : PointerGroup(0),
      id(id_)
  {}

  virtual void notify_parent() {
    std::cout << "ControlPointer moved id=" << id << " " << pos.x << ", " << pos.y  << std::endl;
  }
};

class ScrollerGroup : public Scroller 
{
private:
  std::vector<Scroller*> scrollers;

public:
  ScrollerGroup(Control* parent)
    : Scroller(parent)
  {}

  void update(Control* p) {
    Scroller* scroller = dynamic_cast<Scroller*>(p);
    assert(scroller);
    delta = scroller->get_delta();
    notify_parent();
  }

  void add_scroller(Scroller* p) {
    scrollers.push_back(p);
  }
};

class ControllerScroller : public ScrollerGroup
{
private:
  int id;
public:
  ControllerScroller(int id_)
    : ScrollerGroup(0),
      id(id_)
  {}

  virtual void notify_parent() {
    std::cout << "ControlScroller: moved id=" << id << " " << delta.x << ", " << delta.y  << std::endl;
  }
};

} // namespace Input

#endif

/* EOF */