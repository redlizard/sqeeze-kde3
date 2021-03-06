/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOVARIABLEDLGS_H
#define KOVARIABLEDLGS_H

#include "KoVariable.h"

#include <kdialogbase.h>
#include <qptrlist.h>
#include <qstring.h>
#include <klistview.h>
#include <koffice_export.h>
class QComboBox;
class QVBox;
class QResizeEvent;
class KLineEdit;
class QCloseEvent;

/******************************************************************
 *
 * Class: KoVariableNameDia
 *
 ******************************************************************/

class KoVariableNameDia : public KDialogBase
{
    Q_OBJECT

public:
    // For KWMailMergeVariableInsertDia
    KoVariableNameDia( QWidget *parent );
    // For kwview
    KoVariableNameDia( QWidget *parent, const QPtrList<KoVariable> &vars );
    QString getName() const;

protected slots:
    void textChanged ( const QString &_text );
protected:
    void init();

    QComboBox *names;
    QVBox *back;
};

/**
 * Class: KoCustomVariablesListItem
 * Used by KoCustomVariablesDia
 * Represents an item in the listview, holding a lineedit to edit the value of the variable.
 */
class KoCustomVariablesListItem : public QListViewItem
{
public:
    KoCustomVariablesListItem( QListView *parent );

    void setVariable( KoCustomVariable *v );
    KoCustomVariable *getVariable() const;

    virtual void setup();
    virtual int width ( const QFontMetrics & fm, const QListView * lv, int c ) const;
    void update();

    // Gets the value from the lineedit and sets it into the variable
    void applyValue();

protected:
    KoCustomVariable *var;
    KLineEdit *editWidget;
};

/**
 * Class: KoCustomVariablesList
 * The listview.
 * Used by KoCustomVariablesDia
 */
class KoCustomVariablesList : public KListView
{
    Q_OBJECT

public:
    KoCustomVariablesList( QWidget *parent );

    void setValues();
    void updateItems();

protected slots:
    void columnSizeChange( int c, int os, int ns );
    void sectionClicked( int c );

private:
    class Private;
    Private* d; // currently unused
};

/**
 * Class: KoCustomVariablesDia
 * This dialog allows to set the value of the custom variables.
 */
class KOTEXT_EXPORT KoCustomVariablesDia : public KDialogBase
{
    Q_OBJECT

public:
    KoCustomVariablesDia( QWidget *parent, const QPtrList<KoVariable> &variables );
protected slots:
    void slotOk();

protected:
    QVBox *back;
    KoCustomVariablesList *list;

};

/**
 * Class: KoCustomVarDialog
 * This dialog allows to add a new custom variable or
 * to edit an existing one.
 */
class KOTEXT_EXPORT KoCustomVarDialog : public KDialogBase
{
    Q_OBJECT

public:
    /**
     * Add new variable
     */
    KoCustomVarDialog( QWidget *parent );
    /**
     * Edit existing variable @p var
     */
    KoCustomVarDialog( QWidget *parent, KoCustomVariable *var );

    virtual QString name();
    virtual QString value();

protected slots:
    void slotAddOk();
    void slotEditOk();
    void slotTextChanged(const QString&);

protected:
    QVBox *back;
    KLineEdit *m_name;
    KLineEdit *m_value;

private:
    void init();
    KoCustomVariable *m_var;
};

#endif
