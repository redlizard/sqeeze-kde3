/***************************************************************************
                          xsldbgconfigimpl.cpp  -  description
                             -------------------
    begin                : Fri Jan 4 2002
    copyright            : (C) 2002 by Keith Isdale
    email                :  k_isdale@tpg.com.au
 ***************************************************************************/

/***********************************************************************************
 *                                                                         										*
 *   This program is free software; you can redistribute it and/or modify       *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or         *
 *   (at your option) any later version.                                   							*
 *                                                                         										*
 ************************************************************************************/

#include <klocale.h>
#include <kfiledialog.h>

#include <qdialog.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qmessagebox.h>

#include "xsldbgconfigimpl.h"
#include "xsldbgdebugger.h"
#include <kdebug.h> 

LibxsltParam::LibxsltParam(const QString &name, const QString &value)
	: QObject(0L, 0L)
{
	_name = name;
 _value = value;
}


LibxsltParam::~LibxsltParam()
{
}


QString LibxsltParam::getName() const 
{
 return _name;
}

void LibxsltParam::setName(const QString &name)
{
	_name = name;
}


QString LibxsltParam::getValue() const
{
	return _value;
}


void LibxsltParam::setValue(const QString & value)
{
	_value = value;
}


bool LibxsltParam::isValid() const 
{
	bool result = true;
	if ((_name.length() > 0) && ( _value.length() == 0))
		result = false;

	return result;
}


XsldbgConfigImpl::XsldbgConfigImpl(XsldbgDebugger *debugger,
	QWidget *parent /*=0*/, const char *name /*=0*/)
		:  XsldbgConfig(parent, name)
{
	this->debugger = debugger;
	connect(debugger, SIGNAL(parameterItem(QString /* name*/, QString /* value */)),
		this, SLOT(slotProcParameterItem(QString /* name*/, QString /* value */)));
	connect(debugger, SIGNAL(fileDetailsChanged()),
		this, SLOT(slotReloadFileNames()));
	paramIndex= 0;
	catalogs = false;
	debug = false;
	html = false;
	nonet = false;
	docbook = false;
}

XsldbgConfigImpl::~XsldbgConfigImpl()
{
  debugger = 0L;
}


QString XsldbgConfigImpl::getSourceFile()
{
  if (xslSourceEdit != 0L)
    return xslSourceEdit->text();
  else
    return QString();
}


QString XsldbgConfigImpl::getDataFile()
{
  if (xmlDataEdit != 0L)
    return xmlDataEdit->text();
  else
    return QString();
}


QString XsldbgConfigImpl::getOutputFile()
{
  if (outputFileEdit != 0L)
    return outputFileEdit->text();
  else
    return QString();
}


LibxsltParam *XsldbgConfigImpl::getParam(int paramNumber)
{
	return paramList.at(paramNumber);
}

LibxsltParam *XsldbgConfigImpl::getParam(QString name)
{
	LibxsltParam *param = 0L;
	for (param = paramList.first(); param != 0L; param = paramList.next())
	{
		if (param->getName() == name)
			break;
  }

	return param;
}


int XsldbgConfigImpl::getParamCount()
{
	return paramList.count();
}


void XsldbgConfigImpl::addParam(QString name, QString value)
{
	LibxsltParam *param;
	if ((name.length() == 0) || (value.length() == 0))
		return;

	param = getParam(name);
	if (param == 0L)
	{
		param = new LibxsltParam(name, value);
		if (param != 0L)
		  paramList.append(param);

  }else
		param->setValue(value);
}


void XsldbgConfigImpl::deleteParam(QString name)
{
	bool isOk = false;
	LibxsltParam *param;
	if (name.length() == 0)
		return;

	param = getParam(name);
	if (param != 0L)
		isOk = paramList.remove(param);

	if (isOk == false)
	  kdDebug() << QString(" Param %1 dosn't exist").arg(name) << endl;
	else
	  kdDebug() << "Deleted param " << name << endl;
}


bool XsldbgConfigImpl::isValid(QString &errorMsg)
{
  bool isOK = true;
  errorMsg = "";
  if (xslSourceEdit->text().isEmpty())
    errorMsg.append( i18n("\t\"XSL source\" \n"));
  if (xmlDataEdit->text().isEmpty())
    errorMsg.append(i18n("\t\"XML data\" \n"));
  if (outputFileEdit->text().isEmpty())
    errorMsg.append(i18n("\t\"Output file\" \n"));
  if (!errorMsg.isEmpty()){
    errorMsg.prepend(i18n("Missing values for \n"));
    isOK = false;
  }else if (( xslSourceEdit->text() == outputFileEdit->text())  ||
	    (xmlDataEdit->text() == outputFileEdit->text())){
    errorMsg.append(i18n("Output file is the same as either XSL Source or "
		    "XML Data file\n"));
    isOK = false;
  }

  // make it a warning when parameters are invalid
  LibxsltParam *param;
  QString emptyParams = "";
  for (param = paramList.first(); param != 0L; param = paramList.next())
    {
      if (!param->isValid()){
	if (emptyParams.isEmpty())
	  emptyParams = param->getName();
	else
	  emptyParams.append(", "). append(param->getName());
      }
    }

  if (!emptyParams.isEmpty()){
    errorMsg.append(i18n("The following libxslt parameters are empty\n\t"));
    errorMsg.append(emptyParams);
  }

  return isOK;
}


/*we previously said that isValid() == true so we must commit our changes */
void XsldbgConfigImpl::update()
{
	QString msg;
	if (debugger == 0L)
	  return;

	/* update source, data, output file name if needed */
	slotSourceFile(xslSourceEdit->text());
	slotDataFile(xmlDataEdit->text());
	slotOutputFile(outputFileEdit->text());

	/* ensure entered param are updated */
	slotAddParam();

	if (debugger->start() == false)
		return ; /* User has killed xsldbg and we can't restart it */

	/* always update the libxslt parameters */
	debugger->fakeInput("delparam", true);



 	LibxsltParam *param;
 	for (param = paramList.first(); param != 0L; param = paramList.next())
 	{
 		if (debugger->start() == false)
 			return ; /* User has killed xsldbg and we can't restart it */
 		if (param->isValid()){
 			msg = "addparam ";
 			msg.append(param->getName()).append(" ").append(param->getValue());
 			debugger->fakeInput(msg, true);
 		}
 	}

 	/* now set the xsldbg options*/
 	if (catalogsChkBox->isChecked() != catalogs){
 		catalogs =  catalogsChkBox->isChecked();
		debugger->setOption("catalogs", catalogs);
	 }
	 if (debugChkBox->isChecked() != debug){
	 	debug=  debugChkBox->isChecked();
 		debugger->setOption("debug", debug);
 	 }
 	 if (htmlChkBox->isChecked() != html){
  	 	html =  htmlChkBox->isChecked();
   	debugger->setOption("html", html);
 	 }
 	 if (docbookChkBox->isChecked() != docbook){
 	 	docbook	 =  docbookChkBox->isChecked();
 		debugger->setOption("docbook", docbook);
 	 }
 	 if (nonetChkBox->isChecked() != nonet){
 	 	nonet	 =  nonetChkBox->isChecked();
 	 	debugger->setOption("nonet", nonet);
 	 }
 	 if (novalidChkBox->isChecked() != novalid){
 	 	novalid	 =  novalidChkBox->isChecked();
 	 	debugger->setOption("novalid", novalid);
 	 }
 	 if (nooutChkBox->isChecked() != noout){
 	 	noout	 =  nooutChkBox->isChecked();
 	 	debugger->setOption("noout", noout);
 	 }
 	 if (timingChkBox->isChecked() != timing){
 	 	timing	 =  timingChkBox->isChecked();
 	  debugger->setOption("timing", timing);
 	 }
 	 if (profileChkBox->isChecked() != profile){
 	 	profile	 =  profileChkBox->isChecked();
 	 	debugger->setOption("profile", profile);
 	 }

	debugger->setOption("preferhtml", true);
	debugger->setOption("utf8input", true);
        debugger->slotRunCmd();
        hide();
}


void XsldbgConfigImpl::refresh()
{
  paramIndex = 0;
  repaintParam();
  xslSourceEdit->setText(debugger->sourceFileName());
  xmlDataEdit->setText(debugger->dataFileName());
  outputFileEdit->setText(debugger->outputFileName());
  /*
	if (debugger->start() == false)
		return ;

	qDebug("XsldbgConfigImpl::refresh");
  */
	/* we'll get the list of parameters via paramItem(..) in this class */
  /*	debugger->fakeInput("showparam", true);
   */

}


void XsldbgConfigImpl::slotSourceFile(QString xslFile)
{
	if (debugger->start() == false)
		return ; /* User has killed xsldbg and we can't restart it */
	
	if (debugger->sourceFileName() == xslFile)
		return;

	QString msg("source ");
	msg.append(XsldbgDebugger::fixLocalPaths(xslFile));
	debugger->fakeInput(msg, true);
}

void XsldbgConfigImpl::slotDataFile(QString xmlFile)
{
	if (debugger->start() == false)
		return ; /* User has killed xsldbg and we can't restart it */
	
	if (debugger->dataFileName() == xmlFile)
		return;

	QString msg("data ");
	msg.append(XsldbgDebugger::fixLocalPaths(xmlFile));
	debugger->fakeInput(msg, true);
}

void XsldbgConfigImpl::slotOutputFile(QString outputFile)
{
	if (debugger->start() == false)
 			return ; /* User has killed xsldbg and we can't restart it */

	if (debugger->outputFileName() == outputFile)
		    return;

	QString msg("output ");
	msg.append(XsldbgDebugger::fixLocalPaths(outputFile));
	debugger->fakeInput(msg, true);
}

void XsldbgConfigImpl::slotChooseSourceFile()
{
	KURL url = KFileDialog::getOpenURL(QString::null, "*.xsl; *.XSL; *.Xsl \n *.*", this,
		i18n("Choose XSL Source to Debug"));
	QString fileName = url.prettyURL();

	if ((!fileName.isNull()) && (fileName.length() > 0)){
	    xslSourceEdit->setText(XsldbgDebugger::fixLocalPaths(fileName));
	}
}


void XsldbgConfigImpl::slotChooseDataFile()
{
	KURL url = KFileDialog::getOpenURL(QString::null, "*.xml; *.XML; *.Xml \n*.docbook \n *.html;*.HTML; *.htm ; *HTM \n *.*", this,
									i18n("Choose XML Data to Debug"));
	QString fileName = url.prettyURL();

	if ((!fileName.isNull()) && (fileName.length() > 0))
		xmlDataEdit->setText(XsldbgDebugger::fixLocalPaths(fileName));
}


void XsldbgConfigImpl::slotChooseOutputFile()
{
	KURL url = KFileDialog::getSaveURL(QString::null, "*.xml; *.XML; *.Xml \n*.docbook \n *.txt; *.TXT \n *.htm;*.HTM;*.htm;*.HTML \n*.*", this,
									i18n("Choose Output File for XSL Transformation"));
	QString fileName;

	if (url.isLocalFile()){
	    fileName = url.prettyURL();
	    if ((!fileName.isNull()) && (fileName.length() > 0))
		outputFileEdit->setText(XsldbgDebugger::fixLocalPaths(fileName));
	}
}

void XsldbgConfigImpl::slotReloadFileNames()
{
    if (debugger != 0){
	xslSourceEdit->setText(debugger->sourceFileName());		
	xmlDataEdit->setText(debugger->dataFileName());		
	outputFileEdit->setText(debugger->outputFileName());		
    }
}


void XsldbgConfigImpl::repaintParam()
{
	if (paramIndex < getParamCount()){
		LibxsltParam *param = getParam(paramIndex);
		parameterNameEdit->setText(param->getName());
		parameterValueEdit->setText(param->getValue());
	}else{
		parameterNameEdit->setText("");
		parameterValueEdit->setText("");
	}
}

void XsldbgConfigImpl::slotAddParam()
{
	addParam(parameterNameEdit->text(), parameterValueEdit->text());
	if (paramIndex < getParamCount())
		paramIndex++;

  	repaintParam();
}

void XsldbgConfigImpl::slotDeleteParam()
{
   deleteParam(parameterNameEdit->text());
   repaintParam();
}


void XsldbgConfigImpl::slotNextParam()
{
	addParam(parameterNameEdit->text(), parameterValueEdit->text());
	if (paramIndex < getParamCount())
		paramIndex++;

   repaintParam();
}

void XsldbgConfigImpl::slotPrevParam()
{
	addParam(parameterNameEdit->text(), parameterValueEdit->text());
	if (paramIndex > 0)
		paramIndex--;

   repaintParam();
}

void XsldbgConfigImpl::slotProcParameterItem(QString name, QString value)
{
	if (name.isNull()){
		paramList.clear();
		paramIndex = 0;
		parameterNameEdit->setText("");
		parameterValueEdit->setText("");
	}else{
		addParam(name, value);
		if(paramList.count() == 1){
			parameterNameEdit->setText(name);
			parameterValueEdit->setText(value);
		}
	}
}



void XsldbgConfigImpl::slotApply()
{

    // Validate the users choices before applying it 
    QString msg;
    if (isValid(msg)){
	if (!msg.isEmpty())
	    QMessageBox::information(this, i18n("Suspect Configuration"),
		    msg, QMessageBox::Ok);
	update();
    }else{
	QMessageBox::information(this, i18n("Incomplete or Invalid Configuration"),
		msg, QMessageBox::Ok);
    }
}


void XsldbgConfigImpl::slotCancel()
{
  hide();
}

#include "xsldbgconfigimpl.moc"
