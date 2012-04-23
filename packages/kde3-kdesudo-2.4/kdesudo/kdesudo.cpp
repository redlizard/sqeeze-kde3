/***************************************************************************
                          kdesudo.cpp  -  description
                             -------------------
    begin                : Sam Feb 15 15:42:12 CET 2003
    copyright            : (C) 2003 by Robert Gruber <rgruber@users.sourceforge.net>
                           (C) 2007 by Martin Böhm <martin.bohm@kubuntu.org>
                                       Anthony Mercatante <tonio@kubuntu.org>
                                       Canonical Ltd (Jonathan Riddell <jriddell@ubuntu.com>)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdesudo.h"

#include <qfile.h>
#include <qdir.h>
#include <qdatastream.h>
#include <qstring.h>

#include <kcmdlineargs.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kpassdlg.h>
#include <kstandarddirs.h>
#include <kdesu/kcookie.h>
#include <kdebug.h>
#include <ktempfile.h>

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>

KdeSudo::KdeSudo(QWidget *parent, const char *name,const QString& icon, const QString& generic, bool withIgnoreButton)
	: KPasswordDialog(KPasswordDialog::Password, false, (withIgnoreButton ? User1: false), icon, parent, name)
{
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	QString defaultComment = i18n("<b>%1</b> needs administrative privileges. Please enter your password for verification.");
	p=NULL;
	bError=false;

	m_pCookie = new KCookie;

	// Set vars
	bool newDcop = args->isSet("newdcop");
	bool realtime = args->isSet("r");
	bool priority = args->isSet("p");
	bool showCommand = (!args->isSet("d"));
	bool changeUID = true;
	bool noExec = false;
	keepPwd = (!args->isSet("n"));
	emptyPwd = args->isSet("s");
	QString runas = args->getOption("u");
	QString cmd;

	if (!args->isSet("c") && !args->count() && (!args->isSet("s")))
	{
		KMessageBox::information(NULL, i18n("No command arguments supplied!\nUsage: kdesudo [-u <runas>] <command>\nKdeSudo will now exit..."));
		noExec = true;
	}

	p = new KProcess;
	p->clearArguments();

	// Parsins args

	/* Get the comment out of cli args */
	QByteArray commentBytes = args->getOption("comment");
	QTextCodec* tCodecConv = QTextCodec::codecForLocale();
	QString comment = tCodecConv->toUnicode(commentBytes, commentBytes.size());

	if (args->isSet("f"))
	{
		// If file is writeable, do not change uid
		QString filename = QFile::decodeName(args->getOption("f"));
		QString file = filename;
		if (!file.isEmpty())
		{
			if (file.at(0) != '/')
			{
				KStandardDirs dirs;
				dirs.addKDEDefaults();
				file = dirs.findResource("config", file);
				if (file.isEmpty())
				{
					kdError(1206) << "Config file not found: " << file << "\n";
					exit(1);
				}
			}
			QFileInfo fi(file);
			if (!fi.exists())
			{
				kdError(1206) << "File does not exist: " << file << "\n";
				exit(1);
			}
			if (fi.isWritable())
			{
				changeUID = false;
			}
		}
	}

	if (withIgnoreButton)
	{
		setButtonText(User1, i18n("&Ignore"));
	}

	// Apologies for the C code, taken from kdelibs/kdesu/kdesu_stub.c
	// KControl and other places need to use the user's existing DCOP server
	// For that we set DCOPSERVER.  Create a file in /tmp and use iceauth to add magic cookies
	// from the existing server and set ICEAUTHORITY to point to the file
	if (!newDcop) {
		dcopServer = m_pCookie->dcopServer();
		QCString dcopAuth = m_pCookie->dcopAuth();
		QCString iceAuth = m_pCookie->iceAuth();

		FILE *fout;
		char iceauthority[200];
		char *host, *auth;
		host = qstrdup(dcopServer);
		auth = qstrdup(iceAuth);
		int tempfile;
		int oldumask = umask(077);

		strcpy(iceauthority, "/tmp/iceauth.XXXXXXXXXX");
		tempfile = mkstemp(iceauthority);
		umask(oldumask);
		if (tempfile == -1) {
			kdError() << "error in kdesudo mkstemp" << endl;
			exit(1);
		} else {
			// close(tempfile); //FIXME why does this make the connect() call later crash?
		}
		iceauthorityFile = iceauthority;
		//FIXME we should change owner of iceauthority file, but don't have permissions
		setenv("ICEAUTHORITY", iceauthorityFile, 1);
	
		fout = popen("iceauth >/dev/null 2>&1", "w");
		if (!fout) {
			kdError() << "error in kdesudo running iceauth" << endl;
			exit(1);
		}
		fprintf(fout, "add ICE \"\" %s %s\n", host, auth);
		auth = qstrdup(dcopAuth);
		//auth = xstrsep(params[P_DCOP_AUTH].value);
		fprintf(fout, "add DCOP \"\" %s %s\n", host, auth);
		unsetenv("ICEAUTHORITY");
		pclose(fout);
	}

	connect( p, SIGNAL(receivedStdout(KProcess*, char*, int)), this, SLOT(receivedOut(KProcess*, char*, int)) );
	connect( p, SIGNAL(receivedStderr(KProcess*, char*, int)), this, SLOT(receivedOut(KProcess*, char*, int)) );
	connect( p, SIGNAL(processExited (KProcess *)), this, SLOT(procExited(KProcess*)));

	QString xauthenv = QString(getenv("HOME")) + "/.Xauthority";
	p->setEnvironment("XAUTHORITY", xauthenv);

	// Generate the xauth cookie and put it in a tempfile
	// set the environment variables to reflect that.
	// Default cookie-timeout is 60 sec. .
	// 'man xauth' for more info on xauth cookies.
	
	KTempFile temp = KTempFile("/tmp/kdesudo-","-xauth");
	m_tmpname = temp.name();
	
	FILE *f;
	char buf[1024];
	
	QCString disp = m_pCookie->display();
	// command: xauth -q -f m_tmpname generate $DISPLAy . trusted timeout 60
	QString c = "/usr/bin/xauth -q -f " + m_tmpname + " generate " 
		+ QString::fromLocal8Bit(disp) + " . trusted timeout 60";
	blockSigChild(); // pclose uses waitpid()
	
	if (!(f = popen(c, "r"))) {
		kdWarning() << k_lineinfo << "Cannot run: " << c << "\n";
		unblockSigChild();
		return;
	}
	
	// non root users need to be able to read the xauth file.
	// the xauth file is deleted when kdesudo exits. security?
	QFile tf(m_tmpname);
	if (!runas.isEmpty() && runas != "root" && tf.exists())
		chmod(m_tmpname.ascii(),0644);
	
	QCStringList output;
	while (fgets(buf, 1024, f) > 0)
		output += buf; 
	if (pclose(f) < 0) {
		kdError() << k_lineinfo << "Could not run xauth.\n";
		unblockSigChild();
		return;
	}
	unblockSigChild();
	
	p->setEnvironment("DISPLAY", disp); 
	p->setEnvironment("XAUTHORITY", m_tmpname);

	if (emptyPwd)
		*p << "sudo" << "-k";
	else
	{
		if (changeUID)
		{
			*p << "sudo" << "-H" << "-S" << "-p" << "passprompt";

			if (!runas.isEmpty())
				*p << "-u" << runas;
		}

		if (!dcopServer.isEmpty())
			*p << "DCOPSERVER=" + dcopServer;

		if (!iceauthorityFile.isEmpty())
			*p << "ICEAUTHORITY=" + iceauthorityFile;

		if (realtime)
		{
			*p << "nice" << "-n" << "10";
			addLine(i18n("Priority:"), i18n("realtime:") + QChar(' ') + QString("50/100"));
		}
		else if (priority)
		{
			QString n = args->getOption("p");
			int intn = atoi(n);
			intn =  (intn * 40 / 100) - (20 + 0.5);
		
			QString strn;
			strn.sprintf("%d",intn);

			*p << "nice" << "-n" << strn;
			addLine(i18n("Priority:"), n + QString("/100"));
		}

		*p << "--";

		if (args->isSet("c"))
		{
			QString command = args->getOption("c");
			QStringList commandSplit = QStringList::split(" ", command);
			for (int i = 0; i < commandSplit.count(); i++)
			{
				QString toto = validArg(commandSplit[i]);
				*p << toto;
				cmd += validArg(commandSplit[i]) + QChar(' ');
			}
		}
		else if (args->count())
		{
			for (int i = 0; i < args->count(); i++)
			{
				if (i==0)
				{
					QStringList argsSplit = QStringList::split(" ", args->arg(i));
					for (int i = 0; i < argsSplit.count(); i++)
					{
						*p << validArg(argsSplit[i]);
						cmd += validArg(argsSplit[i]) + QChar(' ');
					}
				}
				else
				{
					*p << validArg(args->arg(i));
					cmd += validArg(args->arg(i)) + QChar(' ');
				}
			}
		}
		// strcmd needs to be defined
		if (showCommand && !cmd.isEmpty())
			addLine(i18n("Command:"), cmd);
	}

	if (comment.isEmpty())
	{
		if (!generic.isEmpty())
			setPrompt(defaultComment.arg(generic));
		else
			setPrompt(defaultComment.arg(cmd));
	}
	else
		setPrompt(comment);

	if (noExec)
		exit(0);
	else
		p->start( KProcess::NotifyOnExit, KProcess::All );
}

KdeSudo::~KdeSudo()
{
}

void KdeSudo::receivedOut(KProcess*, char*buffer, int buflen)
{
	char *pcTmp= new char[buflen+1];
	strncpy(pcTmp,buffer,buflen);
	pcTmp[buflen]='\0';
	QString strOut(pcTmp);

	std::cout << strOut << endl;

	static int badpass = 0;

	if (strOut.find("Sorry, try again")!=-1)
	{
		badpass++;
		if (badpass>2)
		{
			bError=true;
			KMessageBox::error(this, i18n("Wrong password! Exiting..."));
			kapp->quit();
		}
	}
	if (strOut.find("command not found")!=-1)
	{
		bError=true;
		KMessageBox::error(this, i18n("Command not found!"));
		kapp->quit();
	}
	if (strOut.find("is not in the sudoers file")!=-1)
	{
		bError=true;
		KMessageBox::error(this, i18n("Your username is unknown to sudo!"));
                kapp->quit();
	}
	if (strOut.find("is not allowed to execute")!=-1)
	{
		bError=true;
		KMessageBox::error(this, i18n("Your user is not allowed to run the specified command!"));
		kapp->quit();
	}
	if (strOut.find("is not allowed to run sudo on")!=-1)
	{
		bError=true;
		KMessageBox::error(this, i18n("Your user is not allowed to run sudo on this host!"));
                kapp->quit();
	}
	if (strOut.find("may not run sudo on")!=-1)
	{
		bError=true;
		KMessageBox::error(this, i18n("Your user is not allowed to run sudo on this host!"));
                kapp->quit();
	}
	if ((strOut.find("passprompt")!=-1) || (strOut.find("PIN (CHV2)")!=-1))
	{
		this->clearPassword();
		this->show();
	}
}

void KdeSudo::procExited(KProcess*)
{
	if (!keepPwd && unCleaned)
	{
		unCleaned = false;
		p->clearArguments();
		*p << "sudo" << "-k";
		p->start( KProcess::NotifyOnExit, KProcess::All );
	}

	if (!newDcop && !iceauthorityFile.isEmpty())
		if (!iceauthorityFile.isEmpty())
		QFile::remove(iceauthorityFile);

	if (!bError) {
		if (!m_tmpname.isEmpty())
			QFile::remove(m_tmpname);
		kapp->quit();
	}
}

void KdeSudo::slotOk()
{
	QString strTmp(password());
	strTmp+="\n";
	p->writeStdin(strTmp.ascii(),(int)strTmp.length());
	this->hide();
}

void KdeSudo::slotUser1()
{
	done(AsUser);
}

void KdeSudo::blockSigChild()
{
	sigset_t sset;
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_BLOCK, &sset, 0L);
}
void KdeSudo::unblockSigChild()
{
	sigset_t sset;
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_UNBLOCK, &sset, 0L);
}

QString KdeSudo::validArg(QString arg)
{
	QChar firstChar = arg.at(0);
	QChar lastChar = arg.at(arg.length() - 1);

	if (arg.find(' ') != -1)
	{
		if ( (firstChar == '"' && lastChar == '"') || (firstChar == '\'' && lastChar == '\'') )
			return arg;
		else
			return KProcess::quote(arg);
	}
	else
	{
		if ( (firstChar == '"' && lastChar == '"') || (firstChar == '\'' && lastChar == '\'') )
		{
			arg = arg.remove(0, 1);
			arg = arg.remove(arg.length() - 1, 1);
		}
		return arg;
	}
}
