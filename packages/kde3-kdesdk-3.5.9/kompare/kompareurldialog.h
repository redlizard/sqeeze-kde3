/***************************************************************************
                                kcompareurldialog.h  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2004 Otto Bruggeman
                                  (C) 2001-2003 John Firebaugh
        email                   : otto.bruggeman@home.nl
                                  jfirebaugh@kde.org
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef KOMPAREURLDIALOG_H
#define KOMPAREURLDIALOG_H

#include <kdialogbase.h>
#include <kurl.h>

class QGroupBox;

class KComboBox;
class KConfig;
class KFileDialog;
class KURLComboBox;
class KURLRequester;

class FilesPage;
class FilesSettings;
class DiffPage;
class DiffSettings;
class ViewPage;
class ViewSettings;

class KompareURLDialog : public KDialogBase
{
	Q_OBJECT

public:
	KompareURLDialog( QWidget* parent = 0, const char* name = 0 );
	~KompareURLDialog();

	KURL getFirstURL() const;
	KURL getSecondURL() const;
	QString encoding() const;

	void setFirstGroupBoxTitle ( const QString& title );
	void setSecondGroupBoxTitle( const QString& title );

	void setGroup( const QString& groupName );

	void setFirstURLRequesterMode ( unsigned int mode );
	void setSecondURLRequesterMode( unsigned int mode );

protected slots:
	virtual void slotOk();

private slots:
	void slotEnableOk();

private:
	FilesPage*     m_filesPage;
	FilesSettings* m_filesSettings;
	DiffPage*      m_diffPage;
	DiffSettings*  m_diffSettings;
	ViewPage*      m_viewPage;
	ViewSettings*  m_viewSettings;
};

#endif
