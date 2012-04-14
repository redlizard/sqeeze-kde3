/*
    Kopete Yahoo Protocol
    Receive a file

    Copyright (c) 2006 André Duffeck <andre.duffeck@kdemail.net>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef RECEIVEFILETASK_H
#define RECEIVEFILETASK_H

#include "task.h"
#include <qfile.h>
#include <kurl.h>

class QString;
class QFile;
namespace KIO { 
	class Job;
	class TransferJob;
	class MimetypeJob;
}
class YMSGTransfer;

/**
@author André Duffeck
*/
class ReceiveFileTask : public Task
{
	Q_OBJECT
public:
	enum Type { FileTransferAccept, FileTransfer7Accept, FileTransfer7Reject };
	ReceiveFileTask(Task *parent);
	~ReceiveFileTask();
	
	virtual void onGo();
	
	void setRemoteUrl( KURL url );
	void setLocalUrl( KURL url );
	void setFileName( const QString &filename );
	void setTransferId( unsigned int transferId );
	void setType( Type type );
	void setUserId( const QString & userId );
	
	bool take(Transfer *transfer);

protected:
	bool forMe( Transfer *transfer ) const;

signals:
	void bytesProcessed( unsigned int, unsigned int );
	void complete( unsigned int );
	void error( unsigned int, int, const QString & );

private slots:
	void slotData( KIO::Job *job, const QByteArray &data );
	void slotComplete( KIO::Job *job );
	void canceled( unsigned int );

private:
	void parseFileTransfer7Info( YMSGTransfer *transfer );

	KURL m_remoteUrl;
	KURL m_localUrl;
	QString m_fileName;
	QString m_userId;
	QFile *m_file;
	KIO::TransferJob *m_transferJob;
	KIO::MimetypeJob *m_mimetypeJob;
	unsigned int m_transferId;
	unsigned int m_transmitted;
	Type m_type;
};

#endif
