/*************************************************************************** 
                          xsldbglocalvariablesimpl.cpp  -  description 
                             ------------------- 
    begin                : Sat Jan 5 2002 
    copyright            : (C) 2002 by Keith Isdale 
    email                : k_isdale@tpg.com.au 
 ***************************************************************************/ 
 
/*************************************************************************** 
 *                                                                         * 
 *   This program is free software; you can redistribute it and/or modify  * 
 *   it under the terms of the GNU General Public License as published by  * 
 *   the Free Software Foundation; either version 2 of the License, or     * 
 *   (at your option) any later version.                                   * 
 *                                                                         * 
 ***************************************************************************/ 
 
#include <qlistview.h> 
#include <qlineedit.h> 
#include <qlabel.h>
#include <qpushbutton.h>
 
#include "xsldbglocalvariablesimpl.h" 
#include "xsldbglocallistitem.h" 
#include "xsldbgdebugger.h"
#include <klocale.h> 
 
 
XsldbgLocalVariablesImpl::XsldbgLocalVariablesImpl(XsldbgDebugger *debugger, 
	QWidget *parent /*=0*/, const char *name /*=0*/) 
		:  XsldbgLocalVariables(parent, name), XsldbgDialogBase() 
{	 
	this->debugger = debugger; 
	connect(debugger, SIGNAL(variableItem(QString /*name */, QString /* templateContext*/,
					      QString /* fileName */, int /*lineNumber */, 
					      QString /* select XPath */, int /* is it a local variable */)),	
		this, SLOT(slotProcVariableItem(QString /*name */, QString /* templateContext*/,
					        QString /* fileName */, int /*lineNumber */, 
					        QString /* select XPath */, int /* is it a local variable */)));	
	connect(varsListView,  SIGNAL(selectionChanged(QListViewItem *)), 
						this, SLOT(selectionChanged(QListViewItem*))); 
	show(); 
} 
 
 
XsldbgLocalVariablesImpl::~XsldbgLocalVariablesImpl() 
{ 
  debugger = 0L; 
} 
 
void XsldbgLocalVariablesImpl::slotProcVariableItem(QString name , QString templateContext,
  			          QString fileName, int lineNumber, 
			          QString selectXPath, int localVariable)
{ 
 
    if (!name.isNull()){
	varsListView->insertItem(new XsldbgLocalListItem(varsListView, 
		    fileName, lineNumber, name, templateContext, selectXPath, localVariable != 0)); 
    }
 
}	 
 
void XsldbgLocalVariablesImpl::selectionChanged(QListViewItem *item) 
{ 
	XsldbgLocalListItem *localItem = dynamic_cast<XsldbgLocalListItem*>(item); 
	if (localItem){
		variableName->setText(localItem->getVarName());
		xPathEdit->setText(localItem->getXPath());

		if (localItem->isLocalVariable())
		    variableType->setText(i18n("Local"));
		else
		    variableType->setText(i18n("Global"));

		setExpressionButton->setEnabled(!localItem->getXPath().isEmpty());
		xPathEdit->setEnabled(!localItem->getXPath().isEmpty());
		debugger->gotoLine(localItem->getFileName(), localItem->getLineNumber());	 
	}else{
	    // "clear" values in variable editing widgets
	    variableName->setText("");
	    xPathEdit->setText("");
	    variableType->setText("");
	    setExpressionButton->setEnabled(false);
	    xPathEdit->setEnabled(false);
	} 
 
} 

void XsldbgLocalVariablesImpl::refresh() 
{
	if (varsListView){
	    varsListView->clear();
	    debugger->fakeInput("locals -q", true) ; 
	    // "clear" values in variable editing widgets
	    variableName->setText("");
	    xPathEdit->setText("");
	    variableType->setText("");
	    setExpressionButton->setEnabled(false);
	    xPathEdit->setEnabled(false);
	}
} 
 
void XsldbgLocalVariablesImpl::slotEvaluate()
{
  if (debugger != 0L)
    debugger->slotCatCmd( expressionEdit->text() );
}

void XsldbgLocalVariablesImpl::slotSetExpression()
{
    if (debugger != 0L){
	debugger->slotSetVariableCmd( variableName->text(), xPathEdit->text() );
	refresh();
    }
}

 
 
 

#include "xsldbglocalvariablesimpl.moc"
