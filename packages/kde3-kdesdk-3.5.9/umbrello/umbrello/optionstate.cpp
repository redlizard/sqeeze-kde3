/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "optionstate.h"

namespace Settings{

OptionState pd_optionState;

OptionState& getOptionState() {
    return pd_optionState;
}

void setOptionState(const OptionState& optstate) {
    pd_optionState = optstate;
}

}  // namespace Settings

