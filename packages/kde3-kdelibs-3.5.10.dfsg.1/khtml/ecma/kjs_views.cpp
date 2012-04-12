// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "ecma/kjs_views.h"
#include "ecma/kjs_css.h"
#include "ecma/kjs_window.h"
#include "kjs_views.lut.h"

using namespace KJS;

// -------------------------------------------------------------------------

const ClassInfo DOMAbstractView::info = { "AbstractView", 0, &DOMAbstractViewTable, 0 };
/*
@begin DOMAbstractViewTable 2
  document		DOMAbstractView::Document		DontDelete|ReadOnly
  getComputedStyle	DOMAbstractView::GetComputedStyle	DontDelete|Function 2
@end
*/
IMPLEMENT_PROTOFUNC_DOM(DOMAbstractViewFunc)

DOMAbstractView::DOMAbstractView(ExecState *exec, DOM::AbstractView av)
  : DOMObject(exec->interpreter()->builtinObjectPrototype()), abstractView(av) {}

DOMAbstractView::~DOMAbstractView()
{
  ScriptInterpreter::forgetDOMObject(abstractView.handle());
}

Value DOMAbstractView::tryGet(ExecState *exec, const Identifier &p) const
{
  if ( p == "document" )
    return getDOMNode(exec,abstractView.document());
  else if ( p == "getComputedStyle" )
    return lookupOrCreateFunction<DOMAbstractViewFunc>(exec,p,this,DOMAbstractView::GetComputedStyle,2,DontDelete|Function);
  else
    return DOMObject::tryGet(exec,p);
}

Value DOMAbstractViewFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMAbstractView, thisObj );
  DOM::AbstractView abstractView = static_cast<DOMAbstractView *>(thisObj.imp())->toAbstractView();
  switch (id) {
    case DOMAbstractView::GetComputedStyle: {
        DOM::Node arg0 = toNode(args[0]);
        if (arg0.nodeType() != DOM::Node::ELEMENT_NODE)
          return Undefined(); // throw exception?
        else
          return getDOMCSSStyleDeclaration(exec,abstractView.getComputedStyle(static_cast<DOM::Element>(arg0),
                                                                              args[1].toString(exec).string()));
      }
  }
  return Undefined();
}

Value KJS::getDOMAbstractView(ExecState *exec, DOM::AbstractView av)
{
  return cacheDOMObject<DOM::AbstractView, DOMAbstractView>(exec, av);
}

DOM::AbstractView KJS::toAbstractView (const Value& val)
{
  Object obj = Object::dynamicCast(val);
  if (!obj.isValid() || !obj.inherits(&DOMAbstractView::info))
    return DOM::AbstractView ();

  // the Window object is considered for all practical purposes as a descendant of AbstractView
  if (obj.inherits(&Window::info))
     return static_cast<const Window *>(obj.imp())->toAbstractView(); 

  const DOMAbstractView  *dobj = static_cast<const DOMAbstractView *>(obj.imp());
  return dobj->toAbstractView ();
}
