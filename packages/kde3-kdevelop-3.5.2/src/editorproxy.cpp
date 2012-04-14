#include <qwidget.h>
#include <qpopupmenu.h>
#include <qtimer.h>

#include <kdeversion.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kapplication.h>

#include <kparts/part.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/popupmenuinterface.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/selectioninterface.h>
#include <ktexteditor/view.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <kmainwindow.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kstdaccel.h>

#include "toplevel.h"
#include "partcontroller.h"
#include "core.h"
#include "multibuffer.h"
#include "kdeveditorutil.h"

#include "editorproxy.h"

using namespace KTextEditor;

EditorProxy *EditorProxy::s_instance = 0;


EditorProxy::EditorProxy()
  : QObject()
{
	m_delayedLineTimer = new QTimer( this );
	connect( m_delayedLineTimer,  SIGNAL( timeout() ), this, SLOT(setLineNumberDelayed()) );
	KConfig *config = kapp->config();

	m_delayedViewCreationCompatibleUI = true;

	KAction *ac = new KAction( i18n("Show Context Menu"), 0, this,
		SLOT(showPopup()), TopLevel::getInstance()->main()->actionCollection(), "show_popup" );
    KShortcut cut ;/*= KStdAccel::shortcut(KStdAccel::PopupMenuContext);*/
    cut.append(KKey(CTRL+Key_Return));
    ac->setShortcut(cut);
}


EditorProxy *EditorProxy::getInstance()
{
  if (!s_instance)
    s_instance = new EditorProxy;

  return s_instance;
}

void EditorProxy::setLineNumberDelayed()
{
	if( !this ) return; ///This should fix a strange crash I use to encounter, where setLineNumberDelayed is called with this == NULL
  setLineNumber(m_delayedPart, m_delayedLine, m_delayedCol);
}


void EditorProxy::setLineNumber(KParts::Part *part, int lineNum, int col)
{
  if (!part || !part->inherits("KTextEditor::Document"))
    return;

  if ( lineNum < 0 )
    return;

  KURL url = dynamic_cast<KParts::ReadOnlyPart*>( part )->url();
  ViewCursorInterface *iface = dynamic_cast<ViewCursorInterface*>(part->widget());
  if (iface)
  {
#if KDE_IS_VERSION(3,5,5)
#else
    if (!part->widget()->hasFocus()) //workaround for QXIMInputContext crashes. Keep for KDE <=3.5.4!
    {
      m_delayedPart = part;
      m_delayedLine = lineNum;
      m_delayedCol = col;
			m_delayedLineTimer->start( 1, true );
		} else
#endif
     iface->setCursorPositionReal(lineNum, col == -1 ? 0 : col);
  }
  else {
    // Save the position for a rainy day (or when the view gets activated and wants its position)
    for (QValueList<MultiBuffer*>::ConstIterator it = m_editorParts.begin(); it != m_editorParts.end(); ++it)
        if ((*it)->hasURL( url )) {
            (*it)->registerDelayedActivation( part, lineNum, col );
        return;
      }

    // Shouldn't hit this?
    Q_ASSERT(false);
  }
}

void EditorProxy::installPopup( KParts::Part * part )
{

	if ( part->inherits("KTextEditor::Document") && part->widget())
	{
		PopupMenuInterface *iface = dynamic_cast<PopupMenuInterface*>(part->widget());
		if (iface)
		{
			KTextEditor::View * view = static_cast<KTextEditor::View*>( part->widget() );

			QPopupMenu * popup = static_cast<QPopupMenu*>( part->factory()->container("ktexteditor_popup", view ) );

			if (!popup)
			{
				kdWarning() << k_funcinfo << "Popup not found!" << endl;
				return;
			}

			KAction * action = NULL;
			//If there is a tab for this file, we don't need to plug the closing menu entries here
			KConfig *config = KGlobal::config();
			config->setGroup("UI");
			bool m_tabBarShown = ! config->readNumEntry("TabWidgetVisibility", 0);
			if (!m_tabBarShown)
			{
				action = TopLevel::getInstance()->main()->actionCollection()->action( "file_close" );
				if ( action && !action->isPlugged( popup ) )
				{
					popup->insertSeparator( 0 );
					action->plug( popup, 0 );
				}
				action = TopLevel::getInstance()->main()->actionCollection()->action( "file_closeother" );
				if ( action && !action->isPlugged( popup ) )
					action->plug( popup, 1 );
			}

			iface->installPopup( popup );

			connect(popup, SIGNAL(aboutToShow()), this, SLOT(popupAboutToShow()));

			// ugly hack: mark the "original" items
			m_popupIds.resize(popup->count());
			for (uint index=0; index < popup->count(); ++index)
				m_popupIds[index] = popup->idAt(index);
		}
	}
}

void EditorProxy::popupAboutToShow()
{
	QPopupMenu *popup = (QPopupMenu*)sender();
	if (!popup)
	return;
	
	// ugly hack: remove all but the "original" items
	for (int index=popup->count()-1; index >= 0; --index)
	{
		int id = popup->idAt(index);
		if (m_popupIds.contains(id) == 0)
		{
			QMenuItem *item = popup->findItem(id);
			if ( item && item->popup() )
				delete item->popup();
			else
				popup->removeItemAt(index);
		} 
	}

	KTextEditor::Document * doc = dynamic_cast<KTextEditor::Document*>( PartController::getInstance()->activePart() );
	if (!doc ) return;

	unsigned int line;
	unsigned int col;
	if ( !KDevEditorUtil::currentPositionReal( &line, &col, doc ) ) return;

	QString wordstr;
	QString selection = KDevEditorUtil::currentSelection( doc );
	if ( !selection.isEmpty() && selection.contains('\n') != 0 )
	{
		wordstr = selection;
	}
	else
	{
		wordstr = KDevEditorUtil::currentWord( doc );
	}

	QString linestr = KDevEditorUtil::currentLine( doc );
	
	EditorContext context( doc->url(), line, col, linestr, wordstr );
	Core::getInstance()->fillContextMenu( popup, &context );

	// Remove redundant separators (any that are first, last, or doubled)
	bool lastWasSeparator = true;
	for( uint i = 0; i < popup->count(); ) 
	{
		int id = popup->idAt( i );
		if( lastWasSeparator && popup->findItem( id )->isSeparator() ) 
		{
			popup->removeItem( id );
			// Since we removed an item, don't increment i
		} else 
		{
			lastWasSeparator = false;
			i++;
		}
	}
	if( lastWasSeparator && popup->count() > 0 )
		popup->removeItem( popup->idAt( popup->count() - 1 ) );
}

void EditorProxy::showPopup( )
{
	kdDebug(9000) << k_funcinfo << endl;

	if ( KParts::Part * part = PartController::getInstance()->activePart() )
	{
		ViewCursorInterface *iface = dynamic_cast<ViewCursorInterface*>( part->widget() );
		if ( iface )
		{
			KTextEditor::View * view = static_cast<KTextEditor::View*>( part->widget() );
			QPopupMenu * popup = static_cast<QPopupMenu*>( view->factory()->container("ktexteditor_popup", view ) );

			if ( popup )
			{
				popup->exec( view->mapToGlobal( iface->cursorCoordinates() ) );
			}
		}
	}
}

void EditorProxy::registerEditor(MultiBuffer* wrapper)
{
  m_editorParts.append(wrapper);
}

void EditorProxy::deregisterEditor(MultiBuffer* wrapper)
{
  m_editorParts.remove(wrapper);
}

QWidget * EditorProxy::widgetForPart( KParts::Part * part )
{
	if ( !part ) return 0;

	if (part->widget())
		return part->widget();

    KURL url = dynamic_cast<KParts::ReadOnlyPart*>( part )->url();

    for (QValueList<MultiBuffer*>::ConstIterator it = m_editorParts.begin(); it != m_editorParts.end(); ++it)
        if ((*it)->hasURL( url ))
			return *it;

	return 0L;
}

QWidget * EditorProxy::topWidgetForPart( KParts::Part * part )
{
	if ( !part ) return 0;

    KURL url = dynamic_cast<KParts::ReadOnlyPart*>( part )->url();

    for (QValueList<MultiBuffer*>::ConstIterator it = m_editorParts.begin(); it != m_editorParts.end(); ++it)
        if ((*it)->hasURL( url ))
			return *it;

	if (part->widget())
		return part->widget();

	return 0L;
}

bool EditorProxy::isDelayedViewCapable( )
{
	return m_delayedViewCreationCompatibleUI;
}

QWidget *EditorProxy::findPartWidget(KParts::Part *part)
{
    for (QValueList<MultiBuffer*>::ConstIterator it = m_editorParts.begin(); it != m_editorParts.end(); ++it)
        if ((*it)->hasPart(part))
            return *it;

    if (part->widget())
        return part->widget();

    return 0L;
}

#include "editorproxy.moc"
