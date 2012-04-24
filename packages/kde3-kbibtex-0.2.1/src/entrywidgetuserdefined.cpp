/***************************************************************************
 *   Copyright (C) 2004-2005 by Thomas Fischer                             *
 *   fischer@unix-ag.uni-kl.de                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <qlayout.h>
#include <qlabel.h>

#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>

#include <settings.h>
#include "entrywidgetuserdefined.h"

namespace KBibTeX
{

    EntryWidgetUserDefined::EntryWidgetUserDefined( BibTeX::File *bibtexfile, bool isReadOnly, QWidget *parent, const char *name ) : EntryWidgetTab( bibtexfile, isReadOnly, parent, name )
    {
        setupGUI();
    }

    EntryWidgetUserDefined::~EntryWidgetUserDefined()
    {
        // nothing
    }

    bool EntryWidgetUserDefined::isModified()
    {
        bool result = FALSE;
        for ( QValueList<FieldLineEdit*>::iterator it = m_widgets.begin(); !FALSE && it != m_widgets.end(); ++it )
            result |= ( *it )->isModified();
        return result;
    }

    void EntryWidgetUserDefined::updateGUI( BibTeX::Entry::EntryType /*entryType*/, bool /*enableAll*/ )
    {
// nothing
    }

    void EntryWidgetUserDefined::apply( BibTeX::Entry *entry )
    {
        Settings * settings = Settings::self();
        for ( unsigned int i = 0; i < settings->userDefinedInputFields.count(); ++i )
        {
            FieldLineEdit *widget = m_widgets[i];
            Settings::UserDefinedInputFields *udif = settings->userDefinedInputFields[i];

            BibTeX::Value * value = widget->value();
            if ( value == NULL )
                entry->deleteField( udif->name );
            else
            {
                if ( value->text().isEmpty() )
                    entry->deleteField( udif->name );
                else
                {
                    BibTeX::EntryField * field = entry->getField( udif->name );
                    if ( field == NULL )
                    {
                        field = new BibTeX::EntryField( udif->name );
                        entry->addField( field );
                    }
                    field->setValue( value );
                }
                delete value;
            }
        }
    }

    void EntryWidgetUserDefined::reset( BibTeX::Entry *entry )
    {
        Settings * settings = Settings::self();
        for ( unsigned int i = 0; i < settings->userDefinedInputFields.count(); ++i )
        {
            FieldLineEdit *widget = m_widgets[i];
            Settings::UserDefinedInputFields *udif = settings->userDefinedInputFields[i];

            BibTeX::EntryField * field = entry->getField( udif->name );
            widget->setValue( field != NULL ? field->value() : NULL );
        }
    }

    void EntryWidgetUserDefined::updateWarnings( BibTeX::Entry::EntryType /*entryType*/, QListView */*listViewWarnings*/ )
    {
// nothing
    }

    void EntryWidgetUserDefined::setupGUI()
    {
        Settings * settings = Settings::self();
        int rows = settings->userDefinedInputFields.count() + 1;
        if ( rows < 2 ) rows = 2;
        QGridLayout *gridLayout = new QGridLayout( this, rows, 2, KDialog::marginHint(), KDialog::spacingHint(), "gridLayout" );
        gridLayout->setRowStretch( rows - 1, 1 );

        if ( settings->userDefinedInputFields.count() > 0 )
            for ( unsigned int i = 0; i < settings->userDefinedInputFields.count(); ++i )
            {
                QLabel *label = new QLabel( settings->userDefinedInputFields[i]->label, this );
                gridLayout->addWidget( label, i, 0 );
                if ( settings->userDefinedInputFields[i]->inputType == FieldLineEdit::itMultiLine )
                    label->setAlignment( Qt::AlignTop );

                FieldLineEdit *widget = new FieldLineEdit( settings->userDefinedInputFields[i]->label, settings->userDefinedInputFields[i]->inputType, m_isReadOnly, this );
                gridLayout->addWidget( widget, i, 1 );

                label->setBuddy( widget );
                m_widgets << widget;
            }
        else
        {
            gridLayout->setColStretch( 0, 0 );
            gridLayout->setColStretch( 1, 1 );

            QLabel *label = new QLabel( this );
            gridLayout->addWidget( label, 0, 0 );
            label->setPixmap( BarIcon( "messagebox_info" ) );
            label->setAlignment( Qt::AlignVCenter | Qt::AlignLeft | Qt::WordBreak );

            label = new QLabel( i18n( "Please use the settings dialog to add user-defined fields here." ), this );
            label->setAlignment( Qt::AlignVCenter | Qt::AlignLeft | Qt::WordBreak );
            gridLayout->addWidget( label, 0, 1 );
        }
    }

}
