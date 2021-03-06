/***************************************************************************
 * This file is part of the KDE project
 * copyright (C) 2006 by Sebastian Sauer (mail@dipe.org)
 * copyright (C) 2006 by Bernd Steindorff (bernd@itii.de)
 * copyright (C) 2006 by Sascha Kupper (kusato@kfnv.de)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KEXIMACRO_KEXIVARIABLE_H
#define KEXIMACRO_KEXIVARIABLE_H

#include "../lib/manager.h"
#include "../lib/exception.h"
#include "../lib/action.h"
#include "../lib/variable.h"

#include <ksharedptr.h>

class KexiMainWindow;

namespace KoMacro {
	class Context;
}

namespace KexiMacro {

	/**
	* Template class to offer common functionality needed by all
	* @a KoMacro::Variable implementations Kexi provides.
	*/
	template<class ACTIONIMPL>
	class KexiVariable : public KoMacro::Variable
	{
		public:

			/**
			* Constructor.
			*/
			KexiVariable(ACTIONIMPL* actionimpl, const QString& name, const QString& caption)
				: KoMacro::Variable()
				, m_actionimpl(actionimpl)
			{
				setName(name);
				setText(caption);
			}

		protected:

			/**
			* @return the @a KexiAction implementation this @a KexiVariable
			* is a child of.
			*/
			ACTIONIMPL* actionImpl() const
			{
				return m_actionimpl;
			}

		private:
			/// The parent @a KexiAction implementation.
			ACTIONIMPL* m_actionimpl;
	};
}

#endif
