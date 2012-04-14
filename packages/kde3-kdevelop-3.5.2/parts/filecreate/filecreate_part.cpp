/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "filecreate_part.h"

#include <qwhatsthis.h>
#include <qdom.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qvbox.h>
#include <qtimer.h>

#include <kdeversion.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <kaction.h>
#include <kapplication.h>
#include <kactionclasses.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"
#include "configwidgetproxy.h"

#include "filetemplate.h"
#include "domutil.h"
#include "urlutil.h"

#include "filecreate_widget2.h"
#include "filecreate_widget3.h"
#include "filecreate_filetype.h"
#include "filecreate_filedialog.h"
#include "filecreate_newfile.h"
#include "fcconfigwidget.h"

#define PROJECTSETTINGSPAGE 1
#define GLOBALSETTINGSPAGE 2

#include "kdevplugininfo.h"

#include "config.h"

static const KDevPluginInfo data("kdevfilecreate");

typedef KDevGenericFactory<FileCreatePart> FileCreateFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevfilecreate, FileCreateFactory( data ) )

using namespace FileCreate;

FileCreatePart::FileCreatePart(QObject *parent, const char *name, const QStringList & )
//    : KDevCreateFile(&data, parent, name ? name : "FileCreatePart"), m_selectedWidget(-1), m_useSideTab(true), m_subPopups(0)
    : KDevCreateFile(&data, parent, name ? name : "FileCreatePart"), m_subPopups(0)
{
  setInstance(FileCreateFactory::instance());
  setXMLFile("kdevpart_filecreate.rc");

  connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
  connect( core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );

	_configProxy = new ConfigWidgetProxy( core() );
	_configProxy->createProjectConfigPage( i18n("File Templates"), PROJECTSETTINGSPAGE, info()->icon() );
	_configProxy->createGlobalConfigPage( i18n("File Templates"), GLOBALSETTINGSPAGE, info()->icon() );
	connect( _configProxy, SIGNAL(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )),
		this, SLOT(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )) );


  KToolBarPopupAction * newAction = new KToolBarPopupAction( i18n("&New"), "filenew", CTRL+Qt::Key_N, this, SLOT(slotNewFile()), actionCollection(), "file_new");
  newAction->setWhatsThis( i18n("<b>New file</b><p>Creates a new file. Also adds it the project if the <b>Add to project</b> checkbox is turned on.") );
  newAction->setToolTip( i18n("Create a new file") );
  m_newPopupMenu = newAction->popupMenu();
  connect(m_newPopupMenu, SIGNAL(aboutToShow()), this, SLOT(slotAboutToShowNewPopupMenu()));

  QTimer::singleShot( 0, this, SLOT(slotGlobalInitialize()) );
}


FileCreatePart::~FileCreatePart()
{
  delete _configProxy;

  m_newPopupMenu->clear();
  delete m_subPopups;
}

void FileCreatePart::insertConfigWidget( const KDialogBase * dlg, QWidget * page, unsigned int pagenumber )
{
	kdDebug() << k_funcinfo << endl;

	switch( pagenumber )
	{
		case PROJECTSETTINGSPAGE:
		{
			FCConfigWidget* w = new FCConfigWidget( this, false, page, "filecreate config widget" );
			connect( dlg, SIGNAL( okClicked( ) ), w, SLOT( accept( ) ) );
		}
		break;

		case GLOBALSETTINGSPAGE:
		{
			FCConfigWidget *w = new FCConfigWidget( this, true, page, "filecreate config widget" );
			connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
		}
		break;
	}
}

void FileCreatePart::slotAboutToShowNewPopupMenu()
{
	KIconLoader * m_iconLoader = KGlobal::iconLoader();
	m_newPopupMenu->clear();
	delete m_subPopups;
	m_subPopups = NULL;
	int id = 0;
	FileType * filetype = m_filetypes.first();
	for(; filetype; filetype=m_filetypes.next())
	{
		if (filetype->enabled())
		{
			if (filetype->subtypes().count()==0)
			{
				QPixmap iconPix = m_iconLoader->loadIcon(
					filetype->icon(), KIcon::Desktop, KIcon::SizeSmall,
					KIcon::DefaultState, NULL, true);
				m_newPopupMenu->insertItem(iconPix, filetype->name(), this,
					SLOT(slotNewFilePopup(int)), 0, ++id );
				m_newPopupMenu->setItemParameter( id, (long)filetype );
			} else
			{
				KPopupMenu* subMenu = NULL;
				QPtrList<FileType> subtypes = filetype->subtypes();
				for(FileType * subtype = subtypes.first(); subtype; subtype=subtypes.next())
				{
					if (subtype->enabled()){
						if( !subMenu )
							subMenu = new KPopupMenu(0,0);
						QPixmap iconPix = m_iconLoader->loadIcon(
							subtype->icon(), KIcon::Desktop, KIcon::SizeSmall,
							KIcon::DefaultState, NULL, true);
						subMenu->insertItem(iconPix, subtype->name(), this,
							SLOT(slotNewFilePopup(int)), 0, ++id );
						subMenu->setItemParameter( id, (long)subtype );
					}
				}
				if( subMenu )
				{
					if( !m_subPopups )
					{
						m_subPopups = new QPtrList<KPopupMenu>;
						m_subPopups->setAutoDelete(true);
					}
					m_subPopups->append( subMenu );
					m_newPopupMenu->insertItem( filetype->name(), subMenu );
				}
			}

		}

	}
}

void FileCreatePart::slotNewFilePopup( int pFileType )
{
	const FileType* filetype = (const FileType*) pFileType;
	slotFiletypeSelected( filetype );
}

void FileCreatePart::slotNewFile() {
  KDevCreateFile::CreatedFile createdFile = createNewFile();
  if (createdFile.status == KDevCreateFile::CreatedFile::STATUS_NOTCREATED)
    KMessageBox::error(0, i18n("Cannot create file. Check whether the directory and filename are valid."));
  else if (createdFile.status != KDevCreateFile::CreatedFile::STATUS_CANCELED)
    openCreatedFile(createdFile);
}

void FileCreatePart::slotProjectOpened() {
    QTimer::singleShot( 0, this, SLOT(slotInitialize()) );
}

void FileCreatePart::addFileType(const QString & filename) {
  FileType * filetype = getType(filename);
  if (!filetype) {
    filetype = new FileType;
    filetype->setName( filename + " files" );
    filetype->setExt( filename );
    filetype->setCreateMethod("template");
    m_filetypes.append(filetype);
  }
  filetype->setEnabled(true);
}

void FileCreatePart::slotProjectClosed() {
  m_filetypes.clear();
  QTimer::singleShot( 0, this, SLOT(slotGlobalInitialize()) );
}

void FileCreatePart::slotFiletypeSelected(const FileType * filetype) {

  KDevCreateFile::CreatedFile createdFile = createNewFile(filetype->ext(),
                                                          QString::null,
                                                          QString::null,
                                                          filetype->subtypeRef());

  openCreatedFile(createdFile);
}

void FileCreatePart::openCreatedFile(const KDevCreateFile::CreatedFile & createdFile)
{
  if ( createdFile.status == KDevCreateFile::CreatedFile::STATUS_OK )
  {
    KURL uu( createdFile.dir + "/" + createdFile.filename );
    partController()->editDocument ( uu );
  }
}

int FileCreatePart::readTypes(const QDomDocument & dom, QPtrList<FileType> &m_filetypes, bool enable) {
  int numRead = 0;
  QDomElement fileTypes = DomUtil::elementByPath(dom,"/kdevfilecreate/filetypes");
  if (!fileTypes.isNull()) {
    for(QDomNode node = fileTypes.firstChild();!node.isNull();node=node.nextSibling()) {

      if (node.isElement() && node.nodeName()=="type") {
        QDomElement element = node.toElement();
        FileType * filetype = new FileType;
        filetype->setName( element.attribute("name") );
        filetype->setExt( element.attribute("ext") );
        filetype->setCreateMethod( element.attribute("create") );

        filetype->setIcon( element.attribute("icon") );
        filetype->setDescr( (DomUtil::namedChildElement(element, "descr")).text() );
        filetype->setEnabled(enable || (filetype->ext()==""));
        m_filetypes.append(filetype);
        numRead++;

        kdDebug(9034) << "node: " << filetype->name().latin1() << endl;

        if (node.hasChildNodes()) {
          for(QDomNode subnode = node.firstChild();!subnode.isNull();subnode=subnode.nextSibling()) {
            kdDebug(9034) << "subnode: " << subnode.nodeName().latin1() << endl;
            if (subnode.isElement() && subnode.nodeName()=="subtype") {
              QDomElement subelement = subnode.toElement();
              FileType * subtype = new FileType;
              subtype->setExt( filetype->ext() );
              subtype->setCreateMethod( filetype->createMethod() );
              subtype->setSubtypeRef( subelement.attribute("ref") );
              subtype->setIcon( subelement.attribute("icon") );
              subtype->setName( subelement.attribute("name") );
              subtype->setDescr( (DomUtil::namedChildElement(subelement, "descr")).text() );
              subtype->setEnabled(enable);
              filetype->addSubtype(subtype);
            }
          }
        }
      }
    }
  }
  return numRead;
}

FileType * FileCreatePart::getType(const QString & ex, const QString subtRef) {

  QString subtypeRef = subtRef;
  QString ext = ex;
  int dashPos = ext.find('-');
  if (dashPos>-1 && subtRef.isNull()) {
    ext = ex.left(dashPos);
    subtypeRef = ex.mid(dashPos+1);
  }

  QPtrList<FileType> filetypes = getFileTypes();
  for(FileType * filetype = filetypes.first();
      filetype;
      filetype=filetypes.next()) {
    if (filetype->ext()==ext) {
      if (subtypeRef.isNull()) return filetype;
      QPtrList<FileType> subtypes = filetype->subtypes();
      for(FileType * subtype = subtypes.first();
          subtype;
          subtype=subtypes.next()) {
        if (subtypeRef==subtype->subtypeRef()) return subtype;
      }
    }
  }
  return NULL;
}

FileType * FileCreatePart::getEnabledType(const QString & ex, const QString subtRef) {

  QString subtypeRef = subtRef;
  QString ext = ex;
  int dashPos = ext.find('-');
  if (dashPos>-1 && subtRef.isNull()) {
    ext = ex.left(dashPos);
    subtypeRef = ex.mid(dashPos+1);
  }

  QPtrList<FileType> filetypes = getFileTypes();
  for(FileType * filetype = filetypes.first();
      filetype;
      filetype=filetypes.next()) {
    if (filetype->ext()==ext) {
      if ( (subtypeRef.isNull()) && (filetype->enabled()) ) return filetype;
      QPtrList<FileType> subtypes = filetype->subtypes();
      for(FileType * subtype = subtypes.first();
          subtype;
          subtype=subtypes.next()) {
        if ( (subtypeRef==subtype->subtypeRef()) && (filetype->enabled()) ) return subtype;
      }
    }
  }
  return NULL;
}

// KDevFileCreate interface

KDevCreateFile::CreatedFile FileCreatePart::createNewFile(QString ext, QString dir, QString name, QString subtype)
{
  KDevCreateFile::CreatedFile result;

  KURL projectURL;
  if ( !project() )
  {
    //result.status = KDevCreateFile::CreatedFile::STATUS_NOTCREATED;
    //return result;
  }
  else
  {
    projectURL = project()->projectDirectory();
  }

  KURL selectedURL;

  NewFileChooser dialog;
  dialog.setFileTypes(m_filetypes);
  const FileType *filetype = getEnabledType(ext,subtype);
  kdDebug(9034) << "Looking for filetype pointer for " << ext << "/" << subtype << endl;
  if (filetype) {
    kdDebug(9034) << "found filetype" << endl;
  } else {
    kdDebug(9034) << "could not find filetype" << endl;
  }
  if (!project())
    dialog.setInProjectMode(false);

  if (!dir.isNull())
    dialog.setDirectory(dir);
  else if (!project())
    dialog.setDirectory(QDir::currentDirPath());
  else
  {
    QString activeDir = project()->activeDirectory();
    dialog.setDirectory( project()->projectDirectory() +
        ( activeDir[0] == '/' ? "" : "/" )
        + activeDir );
  }
  if (!name.isNull()) dialog.setName(name);
  if (filetype) dialog.setCurrent(filetype);

  dialog.setInitialSize(QSize(500, 200));
  int dialogResult = dialog.exec();

  if (dialogResult == KDialogBase::Rejected) {
    result.status = KDevCreateFile::CreatedFile::STATUS_CANCELED;
    return result;
  }

  // OK was pressed

  result.addToProject = dialog.addToProject();
  selectedURL = dialog.url();
  const FileType *selectedFileType = dialog.selectedType();

  if (dialog.addToProject() && !projectURL.isParentOf(selectedURL) && !(project()->options() & KDevProject::UsesQMakeBuildSystem) ) {
    result.status = KDevCreateFile::CreatedFile::STATUS_NOTWITHINPROJECT;
    return result;
  }

  if (selectedFileType) {
    ext = selectedFileType->ext();
    subtype = selectedFileType->subtypeRef();
  }

  QString fullPath = selectedURL.path();
  // add appropriate extension, if not already there
  if ( !ext.isEmpty() && !fullPath.endsWith("." + ext)) fullPath+="." + ext;

  QString filename = URLUtil::filename(fullPath);
  kdDebug(9034) << "full path = " << fullPath << endl;

  // add in subtype, if specified
  if (!subtype.isEmpty())
      ext += "-" + subtype;

  // create file from template
  bool created = false;
  if (FileTemplate::exists(this, ext))
      created = FileTemplate::copy(this, ext, fullPath);
  else {
      // no template, create a blank file instead
      QFile f(fullPath);
      created = f.open( IO_WriteOnly );
      f.close();
  }
  if (!created)
  {
      result.status = KDevCreateFile::CreatedFile::STATUS_NOTCREATED;
      return result;
  }

  if (dialog.addToProject())
  {
    // work out the path relative to the project directory
//    QString relToProj = URLUtil::relativePath(projectURL, selectedURL, URLUtil::SLASH_PREFIX );
	  QString relToProj;
	  if( project()->options() & KDevProject::UsesQMakeBuildSystem )
	  {
		relToProj = URLUtil::relativePathToFile( project()->projectDirectory(), fullPath );
		  project()->addFile(relToProj);
	  }else
	  {
        relToProj = URLUtil::relativePath(projectURL.path(), fullPath, URLUtil::SLASH_PREFIX );
        project()->addFile(relToProj.mid(1));
	  }
  }

  KURL url;
  url.setPath(fullPath);
  partController()->editDocument(url);

  QString fileName = URLUtil::filename(fullPath);
  kdDebug(9034) << "file name = " << filename << endl;

  result.filename = fileName;
  result.dir = URLUtil::directory(fullPath);
  result.status = KDevCreateFile::CreatedFile::STATUS_OK;

  return result;
}

void FileCreatePart::slotNoteFiletype(const FileType * filetype) {
  kdDebug(9034) << "Noting file type: " << (filetype ? filetype->ext() : QString::fromLatin1("Null") ) << endl;
  m_filedialogFiletype = filetype;
}

void FileCreatePart::slotInitialize( )
{
  m_filetypes.clear();

  //read global configuration
  slotGlobalInitialize();

  // read in which global templates are to be used for this project
  QDomElement useGlobalTypes =
    DomUtil::elementByPath(*projectDom(),"/kdevfilecreate/useglobaltypes");
  for(QDomNode node = useGlobalTypes.firstChild();
      !node.isNull();node=node.nextSibling()) {

    if (node.isElement() && node.nodeName()=="type") {
      QDomElement element = node.toElement();
      QString ext = element.attribute("ext");
      QString subtyperef = element.attribute("subtyperef");
      // if an extension has been specified as enabled, ensure it
      // and all its subtypes are enabled
      if (subtyperef.isNull()) {
        FileType * filetype = getType(ext);
        if (filetype) {
          filetype->setEnabled(true);
          if (filetype->subtypes().count())
            filetype->setSubtypesEnabled(true);
        }
      } else {
        // if an extension + subtype have been specified, enable
        // the subtype and the extension (the 'parent')
        FileType * filetype = getType(ext);
        FileType * subtype = getType(ext,subtyperef);
        if (filetype && subtype) {
          filetype->setEnabled(true);
          subtype->setEnabled(true);
        }
      }
    }
  }

  // read in the list of file types for this project
  if ( project() && readTypes( *projectDom(), m_filetypes, true )==0  ) {
    // default by scanning the templates directory if no template info
    // found in project file
    QDir templDir( project()->projectDirectory() + "/templates/" );
    if (templDir.exists()) {
      templDir.setFilter( QDir::Files );
      const QFileInfoList * list = templDir.entryInfoList();
      if( list ){
        QFileInfoListIterator it( *list );
        QFileInfo *fi;
        while ( (fi = it.current()) != 0 ) {
          addFileType(fi->fileName());
          ++it;
        }
      }
    }
/*    else { // it was probably an imported project
      // KLUDGE: we need a better way to determine file types
      // the current method looks a bit too restrictive
      addFileType( "cpp" );
      addFileType( "h" );
    }*/
  }
}

QString FileCreatePart::findGlobalXMLFile() const
{
  int version = 0;
  QString filename;
  QStringList filenames = KGlobal::instance()->dirs()->findAllResources("data", "kdevfilecreate/template-info.xml");
  for( QStringList::const_iterator it = filenames.begin(); it != filenames.end(); ++it )
  {
    QDomDocument globalDom;
    DomUtil::openDOMFile(globalDom,*it);
    QDomElement e = globalDom.documentElement();
    if( !e.hasAttribute( "version" ) && e.attribute( "version" ).toInt() < version )
    {
      continue;
    }else
    {
      version = e.attribute( "version" ).toInt();
      filename = *it;
    }
  }
  return filename;
}

void FileCreatePart::slotGlobalInitialize( )
{
  // read in global template information
  QString globalXMLFile = findGlobalXMLFile();
  kdDebug(9034) << "Found global template info info " << globalXMLFile << endl;
  QDomDocument globalDom;
  if (!globalXMLFile.isNull() && DomUtil::openDOMFile(globalDom,globalXMLFile))
  {
    kdDebug(9034) << "Reading global template info..." << endl;

    readTypes(globalDom, m_filetypes, false);

  }
}

#include "filecreate_part.moc"

// kate: indent-width 2; replace-tabs on; tab-width 4; space-indent on;
