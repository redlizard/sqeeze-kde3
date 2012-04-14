#ifndef __PARTCONTROLLER_H__
#define __PARTCONTROLLER_H__

#include "kdevpartcontroller.h"

#include <qwidget.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <kurl.h>
#include <qmap.h>
#include <qguardedptr.h>
#include <qpair.h>

namespace KParts
{
  class Part;
  class Factory;
  class PartManager;
  class ReadOnlyPart;
  class ReadWritePart;
}

namespace KTextEditor
{
  class Document;
  class Editor;
}

namespace Kate { class Document; }

class QTabWidget;
class QPopupMenu;
class KAction;
class KToolBarPopupAction;
class KRecentFilesAction;
class HTMLDocumentationPart;
class HistoryEntry;
class KDirWatch;

/**
Part controler implementation.
*/
class PartController : public KDevPartController
{
  Q_OBJECT

public:

  PartController(QWidget *toplevel);
  static void createInstance(QWidget *parent);
  static PartController *getInstance();

  ///// KDevPartController interface

  void setEncoding(const QString &encoding);
  void editDocument(const KURL &inputUrl, int lineNum=-1, int col=-1);
  void splitCurrentDocument(const KURL &inputUrl, int lineNum=-1, int col=-1);
  void scrollToLineColumn(const KURL &url, int lineNum=-1, int col=-1, bool storeHistory = false );
  void editDocumentInternal(const KURL &inputUrl, int lineNum=-1, int col=-1,
                            bool activate = true, bool addToCurrentBuffer = false );
  void integrateTextEditorPart(KTextEditor::Document* doc);

  void showDocument(const KURL &url, bool newWin = false);
  void showPart( KParts::Part* part, const QString& name, const QString& shortDescription );

  KParts::ReadOnlyPart *partForURL(const KURL &url);
  KParts::ReadOnlyPart *qtDesignerPart();
  KParts::Part * partForWidget( const QWidget * widget );

  void activatePart( KParts::Part * part );
  bool closePart( KParts::Part * part );

  KURL::List openURLs();

  bool querySaveFiles();

  bool saveAllFiles();
  bool saveFiles( const KURL::List & list);
  bool saveFile( const KURL & url, bool force = false );

  void revertAllFiles();
  void revertFiles( const KURL::List & list );

  bool closeAllFiles();
  bool closeFiles( const KURL::List & list );

  DocumentState documentState( KURL const & );

  ////////////////////////////////////////

  bool readyToClose();

  bool closeFile( const KURL & );
  bool closeAllOthers( const KURL & );
  void reloadFile( const KURL & url );

  KTextEditor::Editor *openTextDocument( bool activate = true );
  KParts::Factory *findPartFactory(const QString &mimeType,
                                   const QString &partType,
                                   const QString &preferredName = QString::null );

public slots:

  void slotActivePartChanged( KParts::Part* part );
  void slotCloseWindow();
  void slotCloseOtherWindows();
  void slotCloseAllWindows();

  void slotSave();
  void slotReload();

protected:

  ~PartController();

private slots:

  void slotWaitForFactoryHack();

  void slotDocumentUrlChanged();
  void slotSaveAllFiles();
  void slotRevertAllFiles();

  void slotOpenFile();
  void slotOpenRecent(const KURL&);

  void slotBack();
  void slotForward();
  void slotBackAboutToShow();
  void slotForwardAboutToShow();
  void slotBackPopupActivated( int id );
  void slotForwardPopupActivated( int id );

  void slotPartAdded( KParts::Part* );
  void slotPartRemoved( KParts::Part* );

  void slotUploadFinished();

  void updateMenuItems();

  void slotDocumentDirty( Kate::Document * doc, bool isModified, unsigned char reason );
  void slotDocumentDirtyStepTwo( void * );
  void slotNewStatus();
  void slotNewDesignerStatus(const QString &formName, int status);
  void textChanged();
  void gotoLastEditPos();

private:
  KURL findURLInProject(const KURL& url);
  KParts::Part* findOpenDocument(const KURL& url);

  void setupActions();

  bool closeFilesDialog( KURL::List const & ignoreList );
  bool saveFilesDialog( KURL::List const & ignoreList );

  void doEmitState( KURL const & );

  KTextEditor::Editor * createEditorPart( bool activate,
                                          bool addToCurrentBuffer = false,
                                          const KURL &url = KURL() );

  void integratePart(KParts::Part *part, const KURL &url, QWidget* widget = 0,
                       bool isTextEditor=false, bool activate=true,
                       bool addToCurrentBuffer = false );

  // returns a list of modified documents
  KURL::List modifiedDocuments();
  void clearModified( KURL::List const & filelist );

  bool isDirty( KURL const & url );
  bool reactToDirty( KURL const & url, unsigned char reason );

  KURL storedURLForPart( KParts::ReadOnlyPart * );
  void updatePartURL( KParts::ReadOnlyPart * );
  bool partURLHasChanged( KParts::ReadOnlyPart * );

  static PartController *s_instance;

  KAction *m_closeWindowAction, *m_saveAllFilesAction, *m_revertAllFilesAction;
  KAction *m_closeAllWindowsAction, *m_closeOtherWindowsAction;
  KRecentFilesAction *m_openRecentAction;
  QString m_presetEncoding;

  KToolBarPopupAction* m_backAction;
  KToolBarPopupAction* m_forwardAction;
  KAction * m_gotoLastEditPosAction;

  bool m_openNextAsText;

  QValueList<KParts::ReadWritePart*> _dirtyDocuments;

  QMap< KParts::ReadOnlyPart*, KURL > _partURLMap;	// used to note when a URL changes (a file changes name)

  QGuardedPtr<KParts::Factory> _editorFactory;

	struct HistoryEntry
	{
		HistoryEntry() {}
		HistoryEntry( const KURL & url, int line, int col );

		KURL url;
		int line;
		int col;
		int id;
	};

        void addHistoryEntry( KParts::ReadOnlyPart * part = 0 );
        HistoryEntry createHistoryEntry( KParts::ReadOnlyPart * part = 0 );
	void jumpTo( const HistoryEntry & );

	QValueList<HistoryEntry> m_backHistory;
	QValueList<HistoryEntry> m_forwardHistory;
	bool m_isJumping;

	struct LastEditPos
	{
		KURL url;
		QPair<unsigned int,unsigned int> pos;

		LastEditPos() : pos( -1, -1) {}
	};

	LastEditPos m_lastEditPos;

	KParts::Part * m_currentActivePart;
	bool m_removingActivePart;
};



#endif
