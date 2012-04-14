/***************************************************************************
*   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
*   jsgaarde@tdcspace.dk                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef SUBCLASSINGDLG_H
#define SUBCLASSINGDLG_H

#include <qlistview.h>

#include "subclassingdlgbase.h"

class QStringList;
class QDomDocument;
class CppSupportPart;

class SlotItem : public QCheckListItem
{
public:
	SlotItem( QListView *parent, const QString &text,
	          const QString &specifier, const QString &Access,
	          const QString &returnType, bool isFunc,
	          bool callBaseClass = false );
	void setAllreadyInSubclass();
	QString m_access;
	QString m_methodName;
	QString m_returnType;
	QString m_specifier;
	bool m_isFunc;
	bool m_callBaseClass;
	bool m_alreadyInSubclass;
};


class SubclassingDlg : public SubclassingDlgBase
{
public:
	SubclassingDlg( CppSupportPart* cppSupport, const QString &formFile, QStringList &newFileNames,
	                QWidget* parent = 0, const char* name = 0,
	                bool modal = FALSE, WFlags fl = 0 );
	SubclassingDlg( CppSupportPart* cppSupport, const QString &formFile, const QString &filename, QStringList &dummy,
	                QWidget* parent = 0, const char* name = 0,
	                bool modal = FALSE, WFlags fl = 0 );
	~SubclassingDlg();

private:
	void readUiFile();
	QString readBaseClassName();
	void updateDlg();
	bool replaceKeywords( QString &buffer, bool canBeModal = true );
	void replace( QString &string, const QString& search, const QString& replace );
	bool saveBuffer( QString &buffer, const QString& filename );
	bool loadBuffer( QString &buffer, const QString& filename );
	bool alreadyInSubclass( const QString &method );
	bool m_creatingNewSubclass;

public slots:
	virtual void accept();
	virtual void onChangedClassName();

protected:
	QStringList &m_newFileNames;
	QString m_filename;
	QString m_formFile;
	QString m_baseClassName;
	QString m_qtBaseClassName;
	QString m_baseCaption;
	QString m_formName;
	QString m_formPath;
	QStringList m_parsedMethods;
	bool m_canBeModal;
	QValueList<SlotItem*> m_slots;
	QValueList<SlotItem*> m_functions;
	CppSupportPart* m_cppSupport;
};

#endif 
// kate: indent-mode csands; tab-width 4;

