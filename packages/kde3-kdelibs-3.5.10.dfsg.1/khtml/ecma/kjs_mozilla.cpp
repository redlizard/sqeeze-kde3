// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2003 George Staikos (staikos@kde.org)
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

#include <klocale.h>
#include <kdebug.h>

#include "kjs_mozilla.h"
#include "kjs/lookup.h"
#include "kjs_binding.h"
#include "khtml_part.h"
#include "kjs_mozilla.lut.h"

using namespace KJS;

namespace KJS {

const ClassInfo MozillaSidebarExtension::info = { "sidebar", 0, &MozillaSidebarExtensionTable, 0 };
/*
@begin MozillaSidebarExtensionTable 1
  addPanel	MozillaSidebarExtension::addPanel	DontDelete|Function 0
@end
*/
}
IMPLEMENT_PROTOFUNC_DOM(MozillaSidebarExtensionFunc)

MozillaSidebarExtension::MozillaSidebarExtension(ExecState *exec, KHTMLPart *p)
  : ObjectImp(exec->interpreter()->builtinObjectPrototype()), m_part(p) { }

Value MozillaSidebarExtension::get(ExecState *exec, const Identifier &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "MozillaSidebarExtension::get " << propertyName.ascii() << endl;
#endif
  return lookupGet<MozillaSidebarExtensionFunc,MozillaSidebarExtension,ObjectImp>(exec,propertyName,&MozillaSidebarExtensionTable,this);
}

Value MozillaSidebarExtension::getValueProperty(ExecState *exec, int token) const
{
  Q_UNUSED(exec);
  switch (token) {
  default:
    kdDebug(6070) << "WARNING: Unhandled token in MozillaSidebarExtension::getValueProperty : " << token << endl;
    return Value();
  }
}

Value MozillaSidebarExtensionFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::MozillaSidebarExtension, thisObj );
  MozillaSidebarExtension *mse = static_cast<MozillaSidebarExtension*>(thisObj.imp());

  KHTMLPart *part = mse->part();
  if (!part)
    return Undefined();

  // addPanel()  id == 0
  KParts::BrowserExtension *ext = part->browserExtension();
  if (ext) {
    QString url, name;
    if (args.size() == 1) {  // I've seen this, don't know if it's legal.
      name = QString::null;
      url = args[0].toString(exec).qstring();
    } else if (args.size() == 2 || args.size() == 3) {
      name = args[0].toString(exec).qstring();
      url = args[1].toString(exec).qstring();
      // 2 is the "CURL" which I don't understand and don't think we need.
    } else {
      return Boolean(false);
    }
    emit ext->addWebSideBar(KURL( url ), name);
    return Boolean(true);
  }

  return Undefined();
}


