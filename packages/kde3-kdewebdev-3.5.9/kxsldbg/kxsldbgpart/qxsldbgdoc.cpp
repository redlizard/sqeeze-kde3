/**
   File : qxsldbgdoc.cpp
   Author : Keith Isdale
   Date : 19th April 2002
   Description : The document to handle source and breakpoints
 */

#include "qxsldbgdoc.h"
#include <ktexteditor/markinterface.h>
#include <ktexteditor/editorchooser.h>
#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>

QXsldbgDoc::QXsldbgDoc(QWidget *parent, KURL url)
    : QObject(0L, "QXsldbgDoc"), kDoc(0L),kView(0L), locked(false) 
{
    kDoc = KTextEditor::createDocument("libkatepart", 0L,"KTextEditor::Document");
    connect(kDoc, SIGNAL(started(KIO::Job *)), this, SLOT(lockDoc()));
    connect(kDoc, SIGNAL(completed()), this, SLOT(unlockDoc()));
    if (kDoc){
	kView = kDoc->createView(parent, "QXsldbgDocView");
	KURL cleanUrl;
	// convert paths relative to PWD into a absolute path
	QString relUrl = url.prettyURL();
	if (!relUrl.contains(":/")){
	    if (!(relUrl.left(1) == "/"))
		relUrl.prepend(QDir::currentDirPath() + "/");
	    cleanUrl.setFileName(relUrl); 
	}else{
	    cleanUrl = url; 
	}
	kDoc->openURL(cleanUrl);
    }
}


QXsldbgDoc::~QXsldbgDoc()
{
    if (kDoc){
	if (kDoc->views().count() == 1){
	    	kDoc->closeURL(false);	
		delete kDoc;
	}
    }
}

void QXsldbgDoc::slotResult( KIO::Job *job )
{
    if ( job->error() != 0 ){
    }else{
    }
    emit docChanged();
}

KURL QXsldbgDoc::url() const
{
    if (kDoc)
	return kDoc->url();
    else
	return KURL();
}

void QXsldbgDoc::refresh()
{
    if (kDoc){
	KURL lastUrl = kDoc->url();	
	kDoc->closeURL(false);
	kDoc->openURL(lastUrl);
    }
}


void QXsldbgDoc::enableBreakPoint(uint lineNumber, bool state)
{
    if (locked)
	return;

    KTextEditor::MarkInterface *markIf = KTextEditor::markInterface(kDoc);
    if (markIf){
	if (state)
	    markIf->setMark(lineNumber, KTextEditor::MarkInterface::markType02);
	else
	    markIf->setMark(lineNumber, KTextEditor::MarkInterface::markType03);
    }   
}

void QXsldbgDoc::addBreakPoint(uint lineNumber)
{
    enableBreakPoint(lineNumber, true);    
}

void QXsldbgDoc::deleteBreakPoint(uint lineNumber)
{
    if (locked)
	return;

    KTextEditor::MarkInterface *markIf = KTextEditor::markInterface(kDoc);
    if (markIf)
	markIf->clearMark(lineNumber);
}


void QXsldbgDoc::selectBreakPoint(uint lineNumber)
{
    if (locked)
	return;

    KTextEditor::MarkInterface *markIf = KTextEditor::markInterface(kDoc);
    if (markIf)
	markIf->setMark(lineNumber, KTextEditor::MarkInterface::markType05);
}

void QXsldbgDoc::lockDoc()
{
    locked = true;
}

void QXsldbgDoc::unlockDoc()
{
    locked = false;
}
#include "qxsldbgdoc.moc"
