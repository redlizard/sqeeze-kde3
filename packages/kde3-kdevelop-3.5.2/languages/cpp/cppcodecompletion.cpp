/***************************************************************************
                  cppcodecompletion.cpp  -  description
                     -------------------
begin                : Sat Jul 21 2001
copyright            : (C) 2001 by Victor R�er
email                : victor_roeder@gmx.de
copyright            : (C) 2002,2003 by Roberto Raggi
email                : roberto@kdevelop.org
copyright            : (C) 2005 by Adam Treat
email                : manyoso@yahoo.com
copyright            : (C) 2006,2007 by David Nolden
email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "cppcodecompletion.h"


#include "cppcodecompletionconfig.h"
#include "backgroundparser.h"
#include "ast.h"
#include "ast_utils.h"
#include "codeinformationrepository.h"
#include "parser.h"
#include "lexer.h"
#include "tree_parser.h"
#include "cpp_tags.h"
#include "cppsupport_utils.h"
#include "tag_creator.h"

#include <typeinfo>

#include <qpopupmenu.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmessagebox.h>
#include <kparts/part.h>
#include <kstatusbar.h>
#include <ktexteditor/document.h>
#include <kaction.h>

#include <qdatastream.h>
#include <qfile.h>
#include <qmap.h>
#include <qregexp.h>
#include <qstatusbar.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qpair.h>
#include <qvaluestack.h>

#include <kdevpartcontroller.h>
#include <kdevmainwindow.h>
#include <kdevproject.h>
#include <kdevcoderepository.h>
#include <codemodel_utils.h>
#include <codemodel.h>
#include <codebrowserfrontend.h>

#include "codecompletionentry.h"
#include "typedesc.h"
#include "computerecoverypoints.h"
#include "completiondebug.h"
#include "bithelpers.h"
#include "stringhelpers.h"
#include "simpletype.h"
#include "simpletypecachebinder.h"
#include "safetycounter.h"
#include "cppevaluation.h"
#include "simplecontext.h"
#include "simpletypefunction.h"

//#define DISABLE_TRACING

CppCodeCompletion* CppCodeCompletion::m_instance = 0;

const bool disableVerboseForCompletionList = false;
const bool disableVerboseForContextMenu = false;
const bool contextMenuEntriesAtTop = false;
bool showNamespaceAppearances = true;

const char* constructorPrefix = "<constructor>";
const char* destructorPrefix = "<destructor>";
///This enables-disables the automatic processing of the expression under the mouse-cursor
//#define DISABLETOOLTIPS

/**
-- TODO: Does not yet correctly search for overloaded functions and select the right one
-- TODO: The documentation shown in the calltips looks very bad, a better solution must be found(maybe an additional tooltip)
*/

 void statusBarText( const QString& str, int time ) {
   CppCodeCompletion* c = CppCodeCompletion::instance();
   if( c )
     c->addStatusText( str, time );
 }

TypePointer CppCodeCompletion::createGlobalNamespace() {
  KSharedPtr<SimpleTypeCachedNamespace> n = new SimpleTypeCachedNamespace( QStringList(), QStringList() );
  n->addAliases(m_pSupport->codeCompletionConfig()->namespaceAliases() );
  return n.data();
}

template <class Item>
class ItemLocker {
  public:
    ItemLocker( Item& it ) : item( it ) {
      it.lock();
    }
    ~ItemLocker() {
      item.unlock();
    }
  private:
    Item& item;
};

ParsedFilePointer  getParsedFile( CodeModelItem* i ) {
	if( !i || !i->file() || !i->file()->parseResult() ) return 0;
	return dynamic_cast<ParsedFile*>( i->file()->parseResult().data());
}

SafetyCounter safetyCounter;
CppCodeCompletion* cppCompletionInstance = 0;

//file global functions, must be before any "using namespace"
QString cleanForMenu( QString txt ) {
  return txt.replace( "&", "&&" ).replace( "	", "    " );
}

QString buildSignature( TypePointer currType ) {
  SimpleTypeFunctionInterface * f = currType->asFunction();
  if ( !f )
    return "";

  QString ret;
  LocateResult rtt = currType->locateDecType( f->getReturnType() );
  if ( rtt->resolved() || rtt.resolutionCount() > 1 )
    ret = rtt->fullNameChain();
  else
    ret = f->getReturnType().fullNameChain();


  TypeDesc desc = currType->desc();
  desc.decreaseFunctionDepth();

  QString sig = ret + " " + desc.fullNameChain() + f->signature();
  if ( f->isConst() )
    sig += " const";
  return sig;
}

uint PopupTracker::pendingPopups = 0;
PopupTracker* PopupTracker::pt = 0;

/** Multiple empty lines are reduced to one, too long lines wrapped over, and the beginnings of the lines are normalized
*/
QStringList maximumLength( const QStringList& in, int length ) {
  QStringList ret;
  uint firstNonSpace = 50000;
  for ( QStringList::const_iterator it = in.begin(); it != in.end(); ++it )
    for ( uint a = 0; a < ( *it ).length(); a++ )
      if ( !( *it ) [ a ].isSpace() ) {
        if ( firstNonSpace > a )
          firstNonSpace = a;
        break;
      }
  if ( firstNonSpace == 50000 )
    return QStringList();

  bool hadEmptyLine = false;
  for ( QStringList::const_iterator it = in.begin(); it != in.end(); ++it ) {
    if ( ( *it ).length() <= firstNonSpace ) {
      if ( !hadEmptyLine )
        ret << " ";
      hadEmptyLine = true;
    } else {
      hadEmptyLine = false;
      QString str = ( *it ).mid( firstNonSpace );
      while ( !str.isEmpty() ) {
        if ( (int)str.length() < length ) {
          ret << str;
          break;
        } else {
          ret << str.left( length ) + "\\";
          str = str.mid( length );
        }
      }
    }
  }
  return ret;
}

QStringList prepareTextForMenu( const QString& comment, int maxLines, int maxLength ) {
  QStringList in = QStringList::split( "\n", comment );
  QStringList out;
  for ( QStringList::iterator it = in.begin(); it != in.end(); ++it ) {
    out << cleanForMenu( *it );
    if ( (int)out.count() >= maxLines ) {
      out << "[...]";
      break;
    }
  }

  return maximumLength( out, maxLength );
}

QStringList formatComment( const QString& comment, int maxCols = 120 ) {
  QStringList ret;
  SafetyCounter s( 14 );  ///maximum of 14 lines

  QStringList lines = QStringList::split( "\n", comment );
  for ( QStringList::iterator it = lines.begin(); it != lines.end(); ++it ) {
    QStringList words = QStringList::split( " ", *it );
    while ( !words.isEmpty() && s ) {
      QString line = "? ";
      int len = 0;
      while ( !words.isEmpty() && len < maxCols ) {
        len += words.front().length();
        line += words.front() + " ";
        words.pop_front();
      }
      ret << line;
    }
  }
  if ( !s )
    ret << "? comment has too many lines";

  return ret;
}

bool operator < ( const CodeCompletionEntry& e1, const CodeCompletionEntry& e2 ) {
  return e1.text < e2.text;
}

template <class ItemType>
static QValueList<ItemType> unique( const QValueList<ItemType>& entryList ) {

  QValueList< ItemType > l;
  QMap<QString, bool> map;
  typename QValueList< ItemType >::ConstIterator it = entryList.begin();
  while ( it != entryList.end() ) {
    CodeCompletionEntry e = *it++;
    QString key = ( e.type + " " +
                    e.prefix + " " +
                    e.text + " " +
                    e.postfix + " " ).simplifyWhiteSpace().stripWhiteSpace();
    if ( map.find( key ) == map.end() ) {
      map[ key ] = TRUE;
      l << e;
    }
  }
  return l;
}

static QStringList unique( const QStringList& entryList ) {

  QStringList l;
  QMap<QString, bool> map;
  QStringList::ConstIterator it = entryList.begin();
  while ( it != entryList.end() ) {
    QString e = *it++;
    if ( map.find( e ) == map.end() ) {
      map[ e ] = TRUE;
      l << e;
    }
  }
  return l;
}

static QStringList unique( const QValueList<QStringList>& entryList ) {

  QStringList l;
  QMap<QString, bool> map;
  QValueList<QStringList>::ConstIterator it = entryList.begin();
  while ( it != entryList.end() ) {
    QStringList li = ( *it++ );
    QString e = li.join( "\n" );
    if ( map.find( e ) == map.end() ) {
      map[ e ] = TRUE;
      l += li;
    }
  }

  return l;
}


bool tokenAt( const QString& text, const QString& token, int textPos ) {
  if ( text.isEmpty() )
    return false;

  int tokenPos = token.length() - 1;
  if ( tokenPos <= 0 || textPos <= 0 )
    return false;

  while ( text[ textPos ] == token[ tokenPos ] ) {

    --tokenPos;
    --textPos;

    if ( tokenPos == 0 || textPos == 0 ) {
      if ( tokenPos == 0 ) {
        if ( textPos >= 1 && text[ textPos ] == token[ tokenPos ] ) {
          QChar c = text[ textPos - 1 ];
          return c.isSpace() || c == '{' || c == '}' || c == ';';
        } else {
          return false;
        }
      } else {
        return false;
      }
    }
  }
  return false;
}

CppSupportPart* CppCodeCompletion::cppSupport() const {
  return m_pSupport;
}

using namespace CompletionDebug;
using namespace StringHelpers;
using namespace BitHelpers;
using namespace CppEvaluation;

struct PopupFillerHelpStruct {
  CppCodeCompletion* receiver;
	FileList files;
	CppCodeCompletion::PopupActions& m_popupActions;
	PopupFillerHelpStruct( CppCodeCompletion* rec ) : m_popupActions( rec->m_popupActions ) {
    receiver = rec;
	  files = receiver->cppSupport()->codeModel()->fileList();
  }

	bool shouldShowIncludeMenu() const {
		return true;
	}

	QMap<QString, QPopupMenu*> m_namespacePopupCache;

  void insertItem( QPopupMenu* parent, SimpleTypeImpl::MemberInfo d , QString prefix ) {
    Q_UNUSED(prefix);

    QString memType = d.memberTypeToString();

    if ( d.memberType == SimpleTypeImpl::MemberInfo::Typedef && d.type->fullName() == "const int" )
      memType = "enum";

    QString txt = i18n( "Jump to %1 %2" ).arg( memType ).arg( cleanForMenu( d.name ) );
    int id = parent->insertItem( txt, receiver, SLOT( popupAction( int ) ) );

    receiver->m_popupActions.insert( id, d.decl );
  }

  void insertItem ( QPopupMenu* parent, TypeDesc d , QString prefix ) {
    Debug dbg( "#insert# ", 10 );

    QString txt1, txt2;

    if ( d.resolved() && d.resolved() ->isNamespace() ) {
      SimpleTypeCachedNamespace * ns = dynamic_cast<SimpleTypeCachedNamespace*>( d.resolved().data() );
      if ( ns ) {
	      SimpleTypeNamespace::SlaveList slaves = ns->getSlaves( receiver->getIncludeFiles() );
        for ( SimpleTypeNamespace::SlaveList::iterator it = slaves.begin(); it != slaves.end(); ++it ) {
	        SimpleTypeCodeModel* cm = dynamic_cast<SimpleTypeCodeModel*>( ( *it ).first.first.resolved().data() );
	        if ( cm && cm->item() ) {
	          QPopupMenu * m = PopupTracker::createPopup( parent );
	          QString scope = cm->scope().join("::");
	          QMap< QString, QPopupMenu* >::iterator it = m_namespacePopupCache.find( scope );
	          if( it != m_namespacePopupCache.end() ) {
		          parent->insertItem( "Imported Namespace " +  scope, *it );
		          delete m;
	          } else {
		          parent->insertItem( "Imported Namespace " +  scope, m );

							insertItem( m, ( new SimpleTypeCachedCodeModel( cm->item() ) ) ->desc(), prefix );
		          m_namespacePopupCache.insert( scope, m );
	          }
	        } else {
		        SimpleTypeNamespace* cn = dynamic_cast<SimpleTypeNamespace*>( ( *it ).first.first.resolved().data() );
		        if( cn ) {
			        TypePointer t = new SimpleTypeNamespace( cn ); //To avoid endless recursion, this needs to be done(the dynamic-cast above fails)
			        insertItem( parent, t->desc(), prefix );
		        }
	        }
        }
	      return ;
      }
    }

    if ( d.resolved() && receiver->cppSupport() ->codeCompletionConfig() ->showNamespaceAppearances() ) {
	    if ( SimpleTypeCachedCodeModel * item = dynamic_cast<SimpleTypeCachedCodeModel*>( d.resolved().data() ) ) {  ///(1)
        if ( item->item() && item->item() ->isNamespace() ) {
	        NamespaceModel* ns = dynamic_cast<NamespaceModel*>( item->item().data() );
	        QStringList wholeScope = ns->scope();
	        wholeScope << ns->name();
	        for( FileList::iterator it = files.begin(); it != files.end(); ++it ) {
		        //		        if( !safetyCounter ) break;
		        NamespaceModel* ns = (*it).data();

		        for( QStringList::iterator it2 = wholeScope.begin(); it2 != wholeScope.end(); ++it2 ) {
			        if( ns->hasNamespace( (*it2) ) ) {
				        ns =  ns->namespaceByName( *it2 );
				        if( !ns ) break;
			        } else {
				        ns = 0;
				        break;
			        }
		        }

		        if( ns ) {
			        ItemDom i(ns);
			        int sLine, sCol, eLine, eCol;
			        i->getStartPosition( &sLine, &sCol );
			        i->getEndPosition( &eLine, &eCol );
		        	insertItem( parent, (new SimpleTypeCodeModel( i ))->desc(), prefix + " " + (*it)->name() + QString(" (%1 Lines): ").arg( eLine - sLine ) ); ///SimpleTypeCodeModel is used instead of SimpleTypeCachedNodeModel, so the detection at (1) does not trigger, this avoids endless recursion.
		        }

	        }
	        return;
        }
	    }
    }

    if ( d.resolved() ) {
      if ( d.resolved() ->asFunction() ) {
	      txt1 = prefix + i18n( "Jump to declaration of %1(...)" ).arg( d.resolved() ->scope().join( "::" ) );
	      txt2 = prefix + i18n( "Jump to definition of %1(...)" ).arg( d.resolved() ->scope().join( "::" ) );
      } else {
        txt1 = prefix + i18n( "Jump to %1" ).arg( cleanForMenu( d.resolved() ->scope().join( "::" ) ) );
      }
    } else {
			if( !BuiltinTypes::isBuiltin( d ) ) {
	      txt1 = prefix + d.name() + i18n( " is unresolved" );
			} else {
				txt1 = prefix + d.name() + i18n( "  (builtin " ) + BuiltinTypes::comment( d ) + ")";
			}
    }

    int id = parent->insertItem( txt1, receiver, SLOT( popupAction( int ) ) );
    if ( d.resolved() )
      receiver->m_popupActions.insert( id, d.resolved() ->getDeclarationInfo() );

    if ( !txt2.isEmpty() ) {
      int id2 = parent->insertItem( txt2, receiver, SLOT( popupDefinitionAction( int ) ) );
      if ( d.resolved() )
        receiver->m_popupDefinitionActions.insert( id2, d.resolved() ->getDeclarationInfo() );
    }
  }
};

ItemDom itemFromScope( const QStringList& scope, NamespaceDom startNamespace ) {
  if ( scope.isEmpty() )
    return ItemDom();

  NamespaceDom glob = startNamespace;
  if ( !glob )
    return ItemDom();

  ClassModel* curr = glob ;

  QStringList::const_iterator mit = scope.begin();

  while ( curr->isNamespace() && mit != scope.end() && ( ( NamespaceModel* ) curr ) ->hasNamespace( *mit ) ) {
    curr = &( *( ( ( NamespaceModel* ) curr ) ->namespaceByName( *mit ) ) );
    ++mit;
  }

  while ( ( curr->isNamespace() || curr->isClass() ) && mit != scope.end() && curr->hasClass( *mit ) ) {
    ClassList cl = curr->classByName( *mit );
    curr = &( **cl.begin() );
    ++mit;
  }

  if ( mit != --scope.end() )
    return ItemDom();

  TypeAliasList l = curr->typeAliasByName( *mit );
  if ( !l.isEmpty() )
    return model_cast<ItemDom>( l.front() );

  VariableDom v = curr->variableByName( *mit );
  if ( v )
    return model_cast<ItemDom>( v );

  ClassList c = curr->classByName( *mit );
  if ( !c.isEmpty() )
    return model_cast<ItemDom>( c.front() );

  EnumDom en = curr->enumByName( *mit );
  if ( en )
    return model_cast<ItemDom>( en );

  FunctionList f = curr->functionByName( *mit );
  if ( !f.isEmpty() )
    return model_cast<ItemDom>( f.front() );

  FunctionDefinitionList fd = curr->functionDefinitionByName( *mit );
  if ( !fd.isEmpty() )
    return model_cast<ItemDom>( fd.front() );

  return ItemDom();
}

struct PopupClassViewFillerHelpStruct {
  CppCodeCompletion* receiver;
	CppCodeCompletion::PopupActions& m_popupActions;
	PopupClassViewFillerHelpStruct( CppCodeCompletion* rec ) : m_popupActions( rec->m_popupActions ) {
    receiver = rec;
  }

	bool shouldShowIncludeMenu() const {
		return false;
	}

  void insertItem( QPopupMenu* parent, SimpleTypeImpl::MemberInfo d , QString prefix ) {
    Q_UNUSED(prefix);
    FileDom f = receiver->m_pSupport->codeModel() ->fileByName( d.decl.file );
    if ( !f )
      return ;

    ItemDom dom = itemFromScope( QStringList::split( "::", d.name ), model_cast<NamespaceDom>( f ) );

    QString memType = d.memberTypeToString();

    if ( d.memberType == SimpleTypeImpl::MemberInfo::Typedef && d.type->fullName() == "const int" )
      memType = "enum";

    QString txt = i18n( "Show %1 %2" ).arg( memType ).arg( cleanForMenu( d.name ) );
    int id = parent->insertItem( txt, receiver, SLOT( popupClassViewAction( int ) ) );

    receiver->m_popupClassViewActions.insert( id, dom );
  }

  void insertItem ( QPopupMenu* parent, TypeDesc d , QString prefix ) {
    Debug dbg( "#insert# ", 10 );

    QString txt;
    if ( !d.resolved() )
      return ;

    ItemDom dom;

    if ( d.resolved() ) {
      SimpleTypeCodeModel * cm = dynamic_cast<SimpleTypeCodeModel*>( d.resolved().data() );
      if ( cm )
        dom = cm->item();
    }

    if ( d.resolved() ) {
      if ( !dom && d.resolved() ->isNamespace() ) {
        SimpleTypeCachedNamespace * ns = dynamic_cast<SimpleTypeCachedNamespace*>( d.resolved().data() );
        if ( ns ) {
	        SimpleTypeNamespace::SlaveList slaves = ns->getSlaves( receiver->getIncludeFiles() );
          for ( SimpleTypeNamespace::SlaveList::iterator it = slaves.begin(); it != slaves.end(); ++it ) {
	          SimpleTypeCodeModel* cm = dynamic_cast<SimpleTypeCodeModel*>( ( *it ).first.first.resolved().data() );
	          if ( cm && cm->item() ) {
              insertItem( parent, ( new SimpleTypeCachedCodeModel( cm->item() ) ) ->desc(), prefix );
	          }  else {
		          SimpleTypeNamespace* cn = dynamic_cast<SimpleTypeNamespace*>( ( *it ).first.first.resolved().data() );
		          if( cn ) {
			          TypePointer t = new SimpleTypeNamespace( cn ); //to avoid endless recursion (caching would be better)
			          insertItem( parent, t->desc(), prefix );
		          }
	          }
          }
	        return ;
        }
      } else {
        if ( dom ) {
          QString n = d.resolved() ->scope().join( "::" );
          //QString n = d.fullNameChain();
          if ( d.resolved() ->asFunction() ) {
            n = buildSignature( d.resolved() );
          }
          txt = prefix + i18n( "Show %1" ).arg( cleanForMenu( n ) );
        } else {
          txt = prefix + d.name() + " not in code-model";
        }
      }
    } else {
			if( !BuiltinTypes::isBuiltin( d ) ) {
	      txt = prefix + d.name() + i18n( " is unresolved" );
			} else {
				txt = prefix + d.name() + i18n( "  (builtin " ) + BuiltinTypes::comment( d ) + ")";
			}
    }

    int id = parent->insertItem( txt, receiver, SLOT( popupClassViewAction( int ) ) );

    if ( dom )
      receiver->m_popupClassViewActions.insert( id, dom );
  }
};

template <class HelpStruct = PopupFillerHelpStruct>
class PopupFiller {
    HelpStruct struk;
    QString depthAdd;
    SafetyCounter s;
  public:
    PopupFiller( HelpStruct str , QString dAdd, int maxCount = 100 ) : struk( str ), depthAdd( dAdd ), s( maxCount ) {}

	    
		void  fillIncludes( const DeclarationInfo& decl, QPopupMenu* parent, bool& needSeparator ) {
			if( !struk.receiver->getIncludeFiles()[ HashedString( decl.file ) ] ) {
				QString file = decl.file;
				//The include-file seems to be missing
				if( needSeparator ) {
					needSeparator = false;
					parent->insertSeparator();
				}

				QString includeFile = file;
				QFileInfo info( file );

				Driver* driver = struk.receiver->cppSupport()->driver();
				if( driver ) {
					QStringList elements = QStringList::split( "/", file );
					includeFile = elements.back();
					elements.pop_back();

					Dependence d;
					d.first = includeFile;
					d.second = Dep_Local;
					while( driver->findIncludeFile( d, struk.receiver->activeFileName() ) != file && !elements.empty() ) {
						//kdDebug( 9007 ) << "could not find include-file \"" << d.first << "\"" << endl;
						includeFile = elements.back() + "/" + includeFile;
						d.first = includeFile;
						elements.pop_back();
					}
					if( elements.empty() )
						includeFile = "/" + includeFile;

					//kdDebug( 9007 ) << "found include-file \"" << includeFile << "\"" << endl;
				}
				int id = parent->insertItem( i18n( "#include \"%1\" ( defines %2 )" ).arg ( includeFile ).arg( decl.name ), struk.receiver, SLOT( popupAction( int ) ) );
				DeclarationInfo fakeDec;
				fakeDec.name = decl.name;
				fakeDec.file = includeFile;
				fakeDec.startLine = -1; //Use startline -1 to indicate that instead of jumping to the file, the file should be included.
				struk.m_popupActions.insert( id, fakeDec );
			}
		}
	
    void fill( QPopupMenu * parent, LocateResult d, QString prefix = "", const DeclarationInfo & sourceVariable = DeclarationInfo() ) {
      Debug dbg( "#fl# ", 10 )
      ;

	    
      if ( !s || !dbg ) {
        //dbgMajor() << "safety-counter triggered while filling \"" << d.fullNameChain() << "\"" << endl;
        return ;
      }

      if ( !sourceVariable.name.isEmpty() && sourceVariable.name != "this" ) {
        SimpleTypeImpl::MemberInfo f;
        f.decl = sourceVariable;
        f.name = sourceVariable.name;
        f.type = d.desc();
        f.memberType = SimpleTypeImpl::MemberInfo::Variable;

        /*int id = m->insertItem( i18n("jump to variable-declaration \"%1\"").arg( type.sourceVariable.name ) , this, SLOT( popupAction( int ) ) );

        m_popupActions.insert( id, type.sourceVariable );*/
        struk.insertItem( parent, f, prefix );

        parent->insertSeparator();

        if ( !sourceVariable.comment.isEmpty() ) {
          QPopupMenu * m = PopupTracker::createPopup( parent );
          parent->insertItem( i18n( "Comment on %1" ).arg( sourceVariable.name ), m );
          QStringList ls = prepareTextForMenu( sourceVariable.comment, 15, 100 );
          for ( QStringList::iterator it = ls.begin(); it != ls.end(); ++it ) {
            m->insertItem( *it, 0, SLOT( popupClassViewAction( int ) ) );
          }
          parent->insertSeparator();
        }
      }

      struk.insertItem( parent, d, prefix );

	    if( d->resolved() && !d->resolved()->specialization().isEmpty() ) {
		    SimpleType p = d->resolved()->parent();
		    LocateResult r = p->locateDecType( d->name() );
		    if( r ) {
			    QPopupMenu * m = PopupTracker::createPopup( parent );
			    parent->insertItem( i18n( "Specialized from \"%1\"" ).arg( cleanForMenu( r->fullNameChain() ) ), m );
					fill( m, r );
		    }
	    }

      TypeDesc::TemplateParams p = d->templateParams();
      for ( TypeDesc::TemplateParams::iterator it = p.begin(); it != p.end(); ++it ) {
        //if( (*it)->resolved() ) {
        QPopupMenu * m = PopupTracker::createPopup( parent );
        parent->insertItem( i18n( "Template-param \"%1\"" ).arg( cleanForMenu( ( *it ) ->fullNameChain() ) ), m );
        fill( m, **it );
        /*} else {
           fill( parent, **it, prefix + depthAdd );
         }*/
      }

      if ( d->resolved() ) {
        if ( d->resolved() ->asFunction() ) {
          LocateResult rt = d->resolved() ->locateDecType( d->resolved() ->asFunction() ->getReturnType() );
          if ( rt ) {
            QPopupMenu * m = PopupTracker::createPopup( parent );
            parent->insertItem( i18n( "Return-type \"%1\"" ).arg( cleanForMenu( rt->fullNameChain() ) ), m );
            fill( m, rt );
          }

          QValueList<TypeDesc> args = d->resolved() ->asFunction() ->getArgumentTypes();
          QStringList argNames = d->resolved() ->asFunction() ->getArgumentNames();
          if ( !args.isEmpty() ) {
            QPopupMenu * m = PopupTracker::createPopup( parent );
            parent->insertItem( i18n( "Argument-types" ), m );
            QStringList::iterator it2 = argNames.begin();
            for ( QValueList<TypeDesc>::iterator it = args.begin(); it != args.end(); ++it ) {
              LocateResult at = d->resolved() ->locateDecType( *it );
              QString name = "";
              if ( it2 != argNames.end() ) {
                name = *it2;
                ++it2;
              }
              QPopupMenu * mo = PopupTracker::createPopup( m );
              m->insertItem( i18n( "Argument \"%1\"" ).arg( cleanForMenu( at->fullNameChain() + " " + name ) ), mo );
              fill( mo, at );

            }
          }
        }
      }
#ifndef DISABLE_TRACING
      if ( d.trace() ) {
        QValueList<QPair<SimpleTypeImpl::MemberInfo, TypeDesc> > trace = d.trace() ->trace();
        if ( !trace.isEmpty() ) {
          QPopupMenu * m = PopupTracker::createPopup( parent );
          parent->insertItem( i18n( "Trace" ), m );

          for ( QValueList<QPair<SimpleTypeImpl::MemberInfo, TypeDesc> >::iterator it = trace.begin(); it != trace.end(); ++it ) {
            QPopupMenu * mo = PopupTracker::createPopup( m );
            QString tail = ( *it ).second.fullNameChain();
            if ( !tail.isEmpty() )
              tail = "::" + tail;
            m->insertItem( i18n( "%1 -> %2" ).arg( cleanForMenu( ( *it ).first.name + tail ) ).arg( cleanForMenu( ( *it ).first.type->fullNameChain() + tail ) ), mo );

            struk.insertItem( mo, ( *it ).first, prefix );

            if ( !( *it ).first.decl.comment.isEmpty() ) {
              mo->insertSeparator();
              QPopupMenu * m = PopupTracker::createPopup( mo );
              mo->insertItem( i18n( "Comment" ), m );
              QStringList ls = prepareTextForMenu( ( *it ).first.decl.comment, 15, 100 );
              for ( QStringList::iterator it = ls.begin(); it != ls.end(); ++it ) {
                m->insertItem( *it, 0, SLOT( popupClassViewAction( int ) ) );
              }
            }

	          /*bool needSeparator = true;
	          if( struk.shouldShowIncludeMenu() && struk.receiver->cppSupport()->codeCompletionConfig()->preProcessAllHeaders() &&  !(*it).first.decl.file.operator QString().isEmpty() )
	          	fillIncludes( (*it).first.decl, mo, needSeparator );*/
          }
        }
      }
#endif

      if ( d->resolved() ) {
        QValueList<LocateResult> bases = d->resolved() ->getBases();
        for ( QValueList<LocateResult>::iterator it = bases.begin(); it != bases.end(); ++it ) {
          QPopupMenu * m = PopupTracker::createPopup( parent );
          parent->insertItem( i18n( "Base-class \"%1\"" ).arg( cleanForMenu( ( *it ) ->fullNameChain() ) ), m );
          fill( m, *it );
        }

        if ( d->resolved() ->parent() && d->resolved() ->parent() ->desc() ) {
          QPopupMenu * m = PopupTracker::createPopup( parent );
          parent->insertItem( i18n( "Nested in \"%1\"" ).arg( cleanForMenu( d->resolved() ->parent() ->fullTypeResolved() ) ), m );
          fill( m, d->resolved() ->parent() ->desc() );
        }

        if ( !d->resolved() ->comment().isEmpty() ) {
          parent->insertSeparator();
          QPopupMenu * m = PopupTracker::createPopup( parent );
          parent->insertItem( i18n( "Comment on %1" ).arg( cleanForMenu( d->name() ) ), m );
          QStringList ls = prepareTextForMenu( d->resolved() ->comment(), 15, 100 );
          for ( QStringList::iterator it = ls.begin(); it != ls.end(); ++it ) {
            m->insertItem( *it, 0, SLOT( popupClassViewAction( int ) ) );
          }
        }
      }

	    //Add entries for including missing include-files
	    if( struk.shouldShowIncludeMenu() && struk.receiver->cppSupport()->codeCompletionConfig()->preProcessAllHeaders() ) {
					bool needSeparator = true;
					//Show the include-files for the whole trace, because usually the first in the trace should be the one to include
					if ( d.trace() ) {
						QValueList<QPair<SimpleTypeImpl::MemberInfo, TypeDesc> > trace = d.trace() ->trace();
						if ( !trace.isEmpty() ) {
							for ( QValueList<QPair<SimpleTypeImpl::MemberInfo, TypeDesc> >::iterator it = trace.begin(); it != trace.end(); ++it ) {
								if( struk.shouldShowIncludeMenu() && struk.receiver->cppSupport()->codeCompletionConfig()->preProcessAllHeaders() &&  !(*it).first.decl.file.operator QString().isEmpty() )
									fillIncludes( (*it).first.decl, parent, needSeparator );
							}
						}
					}

		    	//Show the include-file for the item itself
				 	if( d->resolved() && !d->resolved()->isNamespace() && struk.receiver->cppSupport() ) {
						fillIncludes( d->resolved()->getDeclarationInfo(), parent, needSeparator );
					}
    	}
    }
};

struct CompTypeProcessor : public TypeProcessor {
  SimpleType m_scope;
  bool m_processArguments;

  CompTypeProcessor( SimpleType scope, bool processArguments ) : m_scope( scope ), m_processArguments( processArguments ) {}

  virtual QString parentType() {
    return m_scope->fullType();
  }

  virtual QString processType( const QString& type ) {
    if ( !m_processArguments )
      return type;
    LocateResult t = m_scope->locateDecType( type );
    if ( t )
      return t->fullNameChain();
    else
      return type;
  }
};

struct CppCodeCompletionData {
  QPtrList<RecoveryPoint> recoveryPoints;
	//QStringList classNameList;

  CppCodeCompletionData() {
    recoveryPoints.setAutoDelete( true );
  }

  RecoveryPoint* findRecoveryPoint( int line, int column ) {
    if ( recoveryPoints.count() == 0 )
      return 0;

    QPair<int, int> pt = qMakePair( line, column );

    QPtrListIterator<RecoveryPoint> it( recoveryPoints );
    RecoveryPoint* recPt = 0;

    while ( it.current() ) {
      QPair<int, int> startPt = qMakePair( it.current() ->startLine, it.current() ->startColumn );
      QPair<int, int> endPt = qMakePair( it.current() ->endLine, it.current() ->endColumn );

      if ( pt < startPt ) {
        break;
      }

      if ( startPt < pt && pt < endPt )
        recPt = it.current();

      ++it;
    }

    return recPt;
  }

};

CppCodeCompletion::CppCodeCompletion( CppSupportPart* part )
    : d( new CppCodeCompletionData ),
    //Matches on includes
    m_includeRx( "^\\s*#\\s*include\\s+[\"<]" ),
    //Matches on C++ and C style comments as well as literal strings
    m_cppCodeCommentsRx( "(//([^\n]*)(\n|$)|/\\*.*\\*/|\"([^\\\\]|\\\\.)*\")" ),
    //Matches on alpha chars and '.'
    m_codeCompleteChRx( "([A-Z])|([a-z])|(\\.)" ),
    //Matches on "->" and "::"
m_codeCompleteCh2Rx( "(->)|(\\:\\:)" ) {
  m_instance = this;
  cppCompletionInstance = this;
  m_cppCodeCommentsRx.setMinimal( true );

  m_pSupport = part;

  connect( m_pSupport->codeCompletionConfig(), SIGNAL( stored() ), this, SLOT( emptyCache() ) );

  m_activeCursor = 0;
  m_activeEditor = 0;
  m_activeCompletion = 0;
  m_activeHintInterface = 0;
  m_activeView = 0;
  m_ccTimer = new QTimer( this );
  m_showStatusTextTimer = new QTimer( this );

  m_ccLine = 0;
  m_ccColumn = 0;
  connect( m_ccTimer, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
  connect( m_showStatusTextTimer, SIGNAL( timeout() ), this, SLOT( slotStatusTextTimeout() ) );

  computeFileEntryList();

  CppSupportPart* cppSupport = m_pSupport;
  connect( cppSupport->project(), SIGNAL( addedFilesToProject( const QStringList& ) ),
           this, SLOT( computeFileEntryList() ) );
  connect( cppSupport->project(), SIGNAL( removedFilesFromProject( const QStringList& ) ),
           this, SLOT( computeFileEntryList() ) );
  connect( cppSupport, SIGNAL( synchronousParseReady( const QString&, ParsedFilePointer ) ), this, SLOT( synchronousParseReady( const QString&, ParsedFilePointer ) ) );

  m_bArgHintShow = false;
  m_bCompletionBoxShow = false;
  m_blockForKeyword = false;
  m_demandCompletion = false;
  m_completionMode = NormalCompletion;

  m_repository = new CodeInformationRepository( cppSupport->codeRepository() );

	connect( cppSupport->codeRepository(), SIGNAL(catalogRegistered( Catalog* )), this, SLOT( emptyCache() ) );
	connect( cppSupport->codeRepository(), SIGNAL(catalogUnregistered( Catalog* )), this, SLOT( emptyCache() ) );
	connect( cppSupport->codeRepository(), SIGNAL(catalogChanged( Catalog* )), this, SLOT( emptyCache() ) );
  
	setupCodeInformationRepository();

  if ( part->partController() ->parts() ) {
    QPtrListIterator<KParts::Part> it( *part->partController() ->parts() );
    while ( KParts::Part * part = it.current() ) {
      integratePart( part );
      ++it;
    }
  }

  if ( part->partController() ->activePart() )
    slotActivePartChanged( part->partController() ->activePart() );

  connect( part->partController( ), SIGNAL( partAdded( KParts::Part* ) ),
           this, SLOT( slotPartAdded( KParts::Part* ) ) );
  connect( part->partController( ), SIGNAL( activePartChanged( KParts::Part* ) ),
           this, SLOT( slotActivePartChanged( KParts::Part* ) ) );

  connect( part, SIGNAL( fileParsed( const QString& ) ),
           this, SLOT( slotFileParsed( const QString& ) ) );
  connect( part, SIGNAL( codeModelUpdated( const QString& ) ),
           this, SLOT( slotCodeModelUpdated( const QString& ) ) );

  KAction * action = new KAction( i18n("Jump to declaration under cursor"), 0, CTRL + Key_Comma,
    this, SLOT(slotJumpToDeclCursorContext()), part->actionCollection(), "jump_to_declaration_cursor_context" );
  action->plug( &m_DummyActionWidget );

  action = new KAction( i18n("Jump to definition under cursor"), 0, CTRL + Key_Period, 
    this, SLOT(slotJumpToDefCursorContext()), part->actionCollection(), "jump_to_defintion_cursor_context" );
  action->plug( &m_DummyActionWidget );
}

CppCodeCompletion::~CppCodeCompletion( ) {
  delete m_repository;
  delete d;
}

void CppCodeCompletion::addStatusText( QString text, int timeout ) {
  m_statusTextList.append( QPair<int, QString>( timeout, text ) );
  if ( !m_showStatusTextTimer->isActive() ) {
    slotStatusTextTimeout();
  }
}

void CppCodeCompletion::clearStatusText() {
  m_statusTextList.clear();
  m_showStatusTextTimer->stop();
}

void CppCodeCompletion::slotStatusTextTimeout() {
  if ( m_statusTextList.isEmpty() || !m_pSupport )
    return ;
//   m_pSupport->mainWindow() ->statusBar() ->message( m_statusTextList.front().second, m_statusTextList.front().first );
  m_showStatusTextTimer->start( m_statusTextList.front().first , true );
  m_statusTextList.pop_front();
}

void CppCodeCompletion::slotTimeout() {
  if ( !m_activeCursor || !m_activeEditor || !m_activeCompletion )
    return ;

  uint nLine, nCol;
  m_activeCursor->cursorPositionReal( &nLine, &nCol );

  if ( nLine != m_ccLine || nCol != m_ccColumn )
    return ;

  QString textLine = m_activeEditor->textLine( nLine );
  QChar ch = textLine[ nCol ];
  if ( ch.isLetterOrNumber() || ch == '_' )
    return ;

  completeText();
}

void CppCodeCompletion::slotArgHintHidden() {
  //kdDebug(9007) << "CppCodeCompletion::slotArgHintHidden()" << endl;
  m_bArgHintShow = false;
}

void CppCodeCompletion::slotCompletionBoxHidden() {
  //kdDebug( 9007 ) << "CppCodeCompletion::slotCompletionBoxHidden()" << endl;
  m_bCompletionBoxShow = false;
}


void CppCodeCompletion::integratePart( KParts::Part * part ) {
  if ( !part || !part->widget() )
    return ;

  KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( part );
  if ( doc ) {
    kdDebug( 9007 ) << k_funcinfo << "integrate document: " << doc << endl;

	  if ( m_pSupport ) { //The slot should connected even when automatic completion is disabled, so it can be enabled any time
      kdDebug( 9007 ) << k_funcinfo << "enabling code completion" << endl;
      connect( part, SIGNAL( textChanged() ), this, SLOT( slotTextChanged() ) );
      connect( part->widget(), SIGNAL( completionDone() ), this,
               SLOT( slotCompletionBoxHidden() ) );
      connect( part->widget(), SIGNAL( completionAborted() ), this,
               SLOT( slotCompletionBoxHidden() ) );
      connect( part->widget(), SIGNAL( argHintHidden() ), this,
               SLOT( slotArgHintHidden() ) );
    }
  }
}

void CppCodeCompletion::slotPartAdded( KParts::Part * part ) {
  integratePart( part );
}

void CppCodeCompletion::slotActivePartChanged( KParts::Part * part ) {
  emptyCache();
	this->d->recoveryPoints.clear();
  if ( m_activeHintInterface && m_activeView ) {
    disconnect( m_activeView , SIGNAL( needTextHint( int, int, QString & ) ), this, SLOT( slotTextHint( int, int, QString& ) ) );

    m_activeHintInterface = 0;
  }
  if ( !part )
    return ;

  kdDebug( 9007 ) << k_funcinfo << endl;

  m_activeFileName = QString::null;

  KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( part );
  if ( !doc )
    return ;

  m_activeFileName = doc->url().path();

  // if the interface stuff fails we should disable codecompletion automatically
  m_activeEditor = dynamic_cast<KTextEditor::EditInterface*>( part );
  if ( !m_activeEditor ) {
    kdDebug( 9007 ) << "Editor doesn't support the EditDocumentIface" << endl;
    return ;
  }

  m_activeCursor = dynamic_cast<KTextEditor::ViewCursorInterface*>( part->widget() );
  if ( !m_activeCursor ) {
    kdDebug( 9007 ) << "The editor doesn't support the CursorDocumentIface!" << endl;
    return ;
  }

  m_activeCompletion = dynamic_cast<KTextEditor::CodeCompletionInterface*>( part->widget() );
  if ( !m_activeCompletion ) {
    kdDebug( 9007 ) << "Editor doesn't support the CompletionIface" << endl;
    return ;
  }

  m_activeView = part ? dynamic_cast<KTextEditor::View*>( part->widget() ) : 0;

  if ( m_activeView )
    m_activeHintInterface = dynamic_cast<KTextEditor::TextHintInterface*>( m_activeView );

  char* q = 0;
  kdDebug() << q << endl;

  if ( m_activeHintInterface ) {
#ifndef DISABLETOOLTIPS
    m_activeHintInterface->enableTextHints( 500 );
    connect( m_activeView, SIGNAL( needTextHint( int, int, QString & ) ), this, SLOT( slotTextHint( int, int, QString& ) ) );
#endif

  } else {
    kdDebug( 9007 ) << "editor has no text-hint-interface" << endl;
  }

  kdDebug( 9007 ) << k_funcinfo << "-- end" << endl;
}

void CppCodeCompletion::slotTextChanged() {
  m_ccTimer->stop();

  if ( !m_activeCursor )
    return ;

	unsigned int nLine, nCol;
  m_activeCursor->cursorPositionReal( &nLine, &nCol );

  QString strCurLine = m_activeEditor->textLine( nLine );
  QString ch = strCurLine.mid( nCol - 1, 1 );
  QString ch2 = strCurLine.mid( nCol - 2, 2 );

  // Tell the completion box to _go_away_ when the completion char
  // becomes empty or whitespace and the box is already showing.
  // !!WARNING!! This is very hackish, but KTE doesn't offer a way
  // to tell the completion box to _go_away_
  if ( ch.simplifyWhiteSpace().isEmpty() &&
       !strCurLine.simplifyWhiteSpace().contains( "virtual" ) &&
       m_bCompletionBoxShow ) {
    QValueList<KTextEditor::CompletionEntry> entryList;
    m_bCompletionBoxShow = true;
    m_activeCompletion->showCompletionBox( entryList, 0 );
  }

  m_ccLine = 0;
  m_ccColumn = 0;

  bool argsHint = m_pSupport->codeCompletionConfig() ->automaticArgumentsHint();
  bool codeComplete = m_pSupport->codeCompletionConfig() ->automaticCodeCompletion();
	bool headComplete = codeComplete; //m_pSupport->codeCompletionConfig() ->automaticHeaderCompletion();

  // m_codeCompleteChRx completes on alpha chars and '.'
  // m_codeCompleteCh2Rx completes on "->" and "::"

  if ( ( argsHint && ch == "(" ) ||
       ( codeComplete && strCurLine.simplifyWhiteSpace().contains( "virtual" ) ) ||
       ( codeComplete && ( m_codeCompleteChRx.search( ch ) != -1 ||
                           m_codeCompleteCh2Rx.search( ch2 ) != -1 ) ) ||
       ( headComplete && ( ch == "\"" || ch == "<" ) && m_includeRx.search( strCurLine ) != -1 ) ) {
    int time;
    m_ccLine = nLine;
    m_ccColumn = nCol;
    if ( ch == "(" )
      time = m_pSupport->codeCompletionConfig() ->argumentsHintDelay();
    else
      time = m_pSupport->codeCompletionConfig() ->codeCompletionDelay();
    m_ccTimer->start( time, true );
  }
	
	fitContextItem( nLine, nCol );
}

void CppCodeCompletion::fitContextItem( int nLine, int nCol ) {
	if( !SimpleType::globalNamespace() ) {
		kdDebug( 9007 ) << "no global namespace was set, clearing cache" << endl;
		emptyCache();
	}
  ///Find out whether the cache may be used on, or has to be cleared.
  if ( m_cachedFromContext ) {
    int sLine, sCol, eLine, eCol;
    m_cachedFromContext->getStartPosition( &sLine, &sCol );
    m_cachedFromContext->getEndPosition( &eLine, &eCol );

    if ( ( nLine < sLine || ( nLine == sLine && nCol < sCol ) ) || ( nLine > eLine || ( nLine == eLine && nCol >= eCol ) ) ) {
      ///The stored item was left. First check whether the item was expanded.
      FileDom file = m_pSupport->codeModel() ->fileByName( m_activeFileName );

      if ( file ) {
        CodeModelUtils::CodeModelHelper fileModel( m_pSupport->codeModel(), file );
        if ( m_cachedFromContext->isClass() ) {
          ClassDom klass = fileModel.classAt( nLine, nCol );
          if ( klass ) {
            ClassDom oldClass = dynamic_cast<ClassModel*>( m_cachedFromContext.data() );
            if ( oldClass && oldClass->name() == klass->name() && oldClass->scope() == klass->scope() ) {
              m_cachedFromContext = klass.data();
            } else {
              emptyCache();
            }
          } else {
            emptyCache();
          }
        } else if ( m_cachedFromContext->isFunction() ) {
          FunctionDom function = fileModel.functionAt( nLine, nCol );
          if ( function ) {
            FunctionDom oldFunction = dynamic_cast<FunctionModel*>( m_cachedFromContext.data() );
            if ( oldFunction && oldFunction->name() == function->name() && function->scope() == oldFunction->scope() && oldFunction->argumentList().count() == function->argumentList().count() ) {
              ArgumentList l1 = oldFunction->argumentList();
              ArgumentList l2 = function->argumentList();
              ArgumentList::iterator it = l1.begin();
              ArgumentList::iterator it2 = l2.begin();
              bool match = true;
              while ( it != l1.end() ) {
                if ( ( *it ) ->type() != ( *it2 ) ->type() ) {
                  match = false;
                  break;
                }
                ++it;
                ++it2;
              }
              if ( match ) {
                m_cachedFromContext = function.data();
              } else {
                emptyCache();
              }

            } else {
              emptyCache();
            }
          } else {
            emptyCache();
          }
        } else {
          emptyCache();
        }
      } else {
        emptyCache();
      }
    }
  }
}

enum { T_ACCESS, T_PAREN, T_BRACKET, T_IDE, T_UNKNOWN, T_TEMP };

QString CppCodeCompletion::replaceCppComments( const QString& contents ) {
  QString text = contents;

  int pos = 0;
  while ( ( pos = m_cppCodeCommentsRx.search( text, pos ) ) != -1 ) {
    if ( m_cppCodeCommentsRx.cap( 1 ).startsWith( "//" ) ) {
      QString before = m_cppCodeCommentsRx.cap( 1 );
      QString after;
      after.fill( ' ', before.length() - 5 );
      after.prepend( "/*" );
      after.append( "*/" );
      text.replace( pos, before.length() - 1, after );
      pos += after.length();
    } else {
      pos += m_cppCodeCommentsRx.matchedLength();
    }
  }
  return text;
}

int CppCodeCompletion::expressionAt( const QString& contents, int index ) {
  kdDebug( 9007 ) << k_funcinfo << endl;

  /* C++ style comments present issues with finding the expr so I'm
  	matching for them and replacing them with empty C style comments
  	of the same length for purposes of finding the expr. */

  QString text = clearComments( contents );

  int last = T_UNKNOWN;
  int start = index;
  --index;

  while ( index > 0 ) {
    while ( index > 0 && text[ index ].isSpace() ) {
      --index;
    }

    QChar ch = text[ index ];
    QString ch2 = text.mid( index - 1, 2 );
    if ( ( last != T_IDE ) && ( ch.isLetterOrNumber() || ch == '_' ) ) {
      while ( index > 0 && ( text[ index ].isLetterOrNumber() || text[ index ] == '_' ) ) {
        --index;
      }
      last = T_IDE;
    } else if ( last != T_IDE && ch == ')' ) {
      int count = 0;
      while ( index > 0 ) {
        QChar ch = text[ index ];
        if ( ch == '(' ) {
          ++count;
        } else if ( ch == ')' ) {
          --count;
        } else if ( count == 0 ) {
          //index;
          last = T_PAREN;
          break;
        }
        --index;
      }
    } else if ( last != T_IDE && ch == '>' && ch2 != "->" ) {
      int count = 0;
      while ( index > 0 ) {
        QChar ch = text[ index ];
        if ( ch == '<' ) {
          ++count;
        } else if ( ch == '>' ) {
          --count;
        } else if ( count == 0 ) {
          //--index;
          last = T_TEMP;
          break;
        }
        --index;
      }
    } else if ( ch == ']' ) {
      int count = 0;
      while ( index > 0 ) {
        QChar ch = text[ index ];
        if ( ch == '[' ) {
          ++count;
        } else if ( ch == ']' ) {
          --count;
        } else if ( count == 0 ) {
          //--index;
          last = T_BRACKET;
          break;
        }
        --index;
      }
    } else if ( ch == '.' ) {
      --index;
      last = T_ACCESS;
    } else if ( ch2 == "::" ) {
      index -= 2;
      last = T_ACCESS;
    } else if ( ch2 == "->" ) {
      index -= 2;
      last = T_ACCESS;
    } else {
      if ( start > index ) {
        ++index;
      }
      last = T_UNKNOWN;
      break;
    }
  }

  ///If we're at the first item, the above algorithm cannot be used safely,
  ///so just determine whether the sign is valid for the beginning of an expression, if it isn't reject it.
  if ( index == 0 && start > index && !( text[ index ].isLetterOrNumber() || text[ index ] == '_' || text[ index ] == ':' ) ) {
    ++index;
  }

  return index;
}

QStringList CppCodeCompletion::splitExpression( const QString& text ) {
#define ADD_CURRENT()\
 if( current.length() ) { l << current; /*kdDebug(9007) << "add word " << current << endl;*/ current = ""; }

  QStringList l;
  uint index = 0;
  QString current;
  while ( index < text.length() ) {
    QChar ch = text[ index ];
    QString ch2 = text.mid( index, 2 );

    if ( ch == '.' ) {
      current += ch;
      ADD_CURRENT();
      ++index;
    } else if ( ch == '(' ) {
      int count = 0;
      while ( index < text.length() ) {
        QChar ch = text[ index ];
        if ( ch == '(' ) {
          ++count;
        } else if ( ch == ')' ) {
          --count;
        } else if ( count == 0 ) {
          break;
        }
        current += ch;
        ++index;
      }
    } else if ( ch == '[' ) {
      int count = 0;
      while ( index < text.length() ) {
        QChar ch = text[ index ];
        if ( ch == '[' ) {
          ++count;
        } else if ( ch == ']' ) {
          --count;
        } else if ( count == 0 ) {
          break;
        }
        current += ch;
        ++index;
      }
    } else if ( ch2 == "->" ) {
      current += ch2;
      ADD_CURRENT();
      index += 2;
    } /*else if ( ch2 == "::" )
                            		{
                            			current += ch2;
                            			ADD_CURRENT();
                            			index += 2;
                            		}*/
    else {
      current += text[ index ];
      ++index;
    }
  }
  ADD_CURRENT();
  return l;
}

///Before calling this, a SimpleTypeConfiguration-object should be created, so that the ressources will be freed when that object is destroyed
EvaluationResult CppCodeCompletion::evaluateExpressionAt( int line, int column , SimpleTypeConfiguration& conf, bool ifUnknownSetType ) {
  kdDebug( 9007 ) << "CppCodeCompletion::evaluateExpressionAt( " << line << ", " << column << " )" << endl;

  if ( !m_pSupport || !m_activeEditor )
    return EvaluationResult();
  if ( line < 0 || line >= ( int ) m_activeEditor->numLines() )
    return EvaluationResult();
	if ( column < 0 || column >= m_activeEditor->lineLength( line ) )
    return EvaluationResult();

  {
    QString curLine = m_activeEditor->textLine( line );

    ///move column to the last letter of the pointed word
    while ( column + 1 < ( int ) curLine.length() && isValidIdentifierSign( curLine[ column ] ) && isValidIdentifierSign( curLine[ column + 1 ] ) )
      column++;

    //if( column > 0 ) column--;

    if ( column >= ( int ) curLine.length() || curLine[ column ].isSpace() )
      return EvaluationResult();

    QString expr = curLine.left( column + 1 );
    kdDebug( 9007 ) << "evaluating line \"" << expr.stripWhiteSpace() << "\"" << endl;

    if ( curLine[ column ] == '-' || curLine[ column ] == ';' )
      --column;

    EvaluationResult type = evaluateExpressionType( line, column + 1, conf, ifUnknownSetType ? addFlag( DefaultEvaluationOptions, DefaultAsTypeExpression ) : DefaultEvaluationOptions );

    kdDebug( 9007 ) << "type: " << type->fullNameChain() << endl;

    return type;
  }
}

void CppCodeCompletion::popupAction( int number ) {
  PopupActions::iterator it = m_popupActions.find( number );
  if ( it != m_popupActions.end() ) {
    QString fileName = ( *it ).file == "current_file" ? m_activeFileName : ( *it ).file.operator QString();
	  if( (*it).startLine == -1 ) {
		  //startLine -1 indicates that the file should be added to the include-files
		  m_activeEditor->insertLine( 0, QString("#include \"%1\" /* defines %2 */").arg( fileName ).arg( (*it).name ) );
	  } else {
    	m_pSupport->partController() ->editDocument( fileName, ( *it ).startLine );
	  }
  } else {
    kdDebug( 9007 ) << "error" << endl;
  }
}

void CppCodeCompletion::popupDefinitionAction( int number ) {
  PopupActions::iterator it = m_popupDefinitionActions.find( number );
  if ( it != m_popupDefinitionActions.end() ) {
    QString fileName = ( *it ).file == "current_file" ? m_activeFileName : ( *it ).file.operator QString();
    if ( !m_pSupport->switchHeaderImpl( fileName, ( *it ).startLine, ( *it ).startCol ) )
      m_pSupport->partController() ->editDocument( fileName, ( *it ).startLine );
  } else {
    kdDebug( 9007 ) << "error" << endl;
  }
}

void CppCodeCompletion::selectItem( ItemDom item ) {
  Extensions::KDevCodeBrowserFrontend * f = m_pSupport->extension< Extensions::KDevCodeBrowserFrontend > ( "KDevelop/CodeBrowserFrontend" );

  if ( f != 0 ) {
    ItemDom itemDom( &( *item ) );
    f->jumpedToItem( itemDom );
  } else {
    kdDebug() << "could not find the proper extension" << endl;
  }
}

void CppCodeCompletion::popupClassViewAction( int number ) {
  PopupClassViewActions::iterator it = m_popupClassViewActions.find( number );
  if ( it != m_popupClassViewActions.end() ) {
    if ( ( *it ) )
      selectItem( *it );
  } else {
    kdDebug( 9007 ) << "error" << endl;
  }
}

void CppCodeCompletion::contextEvaluationMenus ( QPopupMenu *popup, const Context *context, int line, int column ) {
  clearStatusText();
  Q_UNUSED(context);
  if ( !m_pSupport->codeCompletionConfig() ->showEvaluationContextMenu() )
    return ;

  kdDebug( 9007 ) << "CppCodeCompletion::contextEvaluationMenu()" << endl;

	PopupTracker::print();

  m_popupActions.clear();
  m_popupDefinitionActions.clear();
  m_popupClassViewActions.clear();

  if ( !m_pSupport || !m_activeEditor )
    return ;

  struct SetDbgState {
    DBGStreamType& st;
    bool oldState;
    SetDbgState( DBGStreamType& targ, bool state ) : st( targ ) {
      oldState = targ.state();
      targ.setState( state );
    }
    ~SetDbgState() {
      st.setState( oldState );
    }
  };

  int cpos = 0;

  SetDbgState stt( dbgState, disableVerboseForContextMenu );

  SimpleTypeConfiguration conf( m_activeFileName );

  EvaluationResult type = evaluateExpressionAt( line, column, conf );

	///Test if it is a macro
	if( type.isMacro ) {
		QPopupMenu * m = PopupTracker::createPopup( popup );
		int gid;
		if ( contextMenuEntriesAtTop )
			gid = popup->insertItem( i18n( "Navigate by Macro \"%1\"" ).arg( cleanForMenu( type.macro.name() ) ), m, 5, cpos++ );
		else
			gid = popup->insertItem( i18n( "Navigate by Macro \"%1\"" ).arg( cleanForMenu( type.macro.name() ) ), m );

		int id = m->insertItem( i18n( "Jump to %1" ).arg( cleanForMenu( type.macro.name() ) ), this, SLOT( popupAction( int ) ) );
		QPopupMenu * b = PopupTracker::createPopup( m );
		m->insertItem( i18n( "Body" ), b );

		DeclarationInfo i;
		i.file = type.macro.fileName();
		i.startCol = type.macro.column();
		i.startLine = type.macro.line();
		i.endCol = type.macro.column();
		i.endLine = type.macro.line();
		m_popupActions.insert( id, i );

		QStringList ls = prepareTextForMenu( type.macro.body(), 20, 100 );
		for ( QStringList::iterator it = ls.begin(); it != ls.end(); ++it ) {
			b->insertItem( *it, 0, SLOT( popupClassViewAction( int ) ) );
		}
	}

	///Test if it is an include-directive
	QString includeFileName, includeFilePath;
	bool simpleAlgorithm = false;
	bool isIncludeDirective = getIncludeInfo( line, includeFileName, includeFilePath, simpleAlgorithm );
	if( isIncludeDirective ) {
		///Add menu entry
		if( !includeFilePath.isEmpty() ) {
			int gid;
			QPopupMenu * m = PopupTracker::createPopup( popup );
			if ( contextMenuEntriesAtTop )
				gid = popup->insertItem( i18n( "Goto Include File: %1" ).arg( cleanForMenu( includeFileName ) ), m, 5, cpos++ );
			else
				gid = popup->insertItem( i18n( "Goto Include File: %1" ).arg( cleanForMenu( includeFileName ) ), m );

			int id = m->insertItem( i18n( "Jump to %1" ).arg( cleanForMenu( includeFilePath ) ), this, SLOT( popupAction( int ) ) );

			DeclarationInfo i;
			i.file = includeFilePath;
			i.startCol = 0;
			i.startLine = 0;
			i.endCol = 0;
			i.endLine = 0;
			m_popupActions.insert( id, i );

			if( simpleAlgorithm && cppSupport()->codeCompletionConfig()->resolveIncludePaths() ) {
				//Add a notification that the correct algorithm failed in finding the include-file correctly
				m->insertItem( i18n( "This include-file could not be located regularly, and was selected from the project file list." ) );
			}
		} else {
			///Could not find include-file
			if ( contextMenuEntriesAtTop )
				popup->insertItem( i18n( "Not Found: \"%1\"" ).arg( includeFileName ), 5, cpos++ );
			else
				popup->insertItem( i18n( "Not Found: \"%1\"" ).arg( includeFileName ) );
		}
	}

	///Break if we cannot show additional information
	if ( isIncludeDirective || (!type->resolved() && !type.sourceVariable && ( !type.resultType.trace() || type.resultType.trace() ->trace().isEmpty() ) && !BuiltinTypes::isBuiltin( type.resultType ) ) )
		return ;

	QString name = type->fullNameChain();
	if ( type.sourceVariable )
		name += " " + type.sourceVariable.name;
	if ( type.resultType->resolved() && type.resultType->resolved() ->asFunction() )
		name = buildSignature( type.resultType->resolved() );

  ///Fill the jump-menu
  {
    PopupFillerHelpStruct h( this );
    PopupFiller<PopupFillerHelpStruct> filler( h, "" );

    QPopupMenu * m = PopupTracker::createPopup( popup );
    int gid;
    if ( contextMenuEntriesAtTop )
      gid = popup->insertItem( i18n( "Navigate by \"%1\"" ).arg( cleanForMenu( name ) ), m, 5, cpos++ );
    else
      gid = popup->insertItem( i18n( "Navigate by \"%1\"" ).arg( cleanForMenu( name ) ), m );

    popup->setWhatsThis( gid, i18n( "<b>Navigation</b><p>Provides a menu to navigate to positions of items that are involved in this expression" ) );

    /*if( type.sourceVariable && type.sourceVariable.name != "this" ) {
    	int id = m->insertItem( i18n("jump to variable-declaration \"%1\"").arg( type.sourceVariable.name ) , this, SLOT( popupAction( int ) ) );

    	m_popupActions.insert( id, type.sourceVariable );
    }*/

    filler.fill( m, type, "", type.sourceVariable );
  }
  if ( type->resolved() ) {
    ///Now fill the class-view-browsing-stuff
    {
      QPopupMenu * m = PopupTracker::createPopup( popup );
      int gid;
      if ( contextMenuEntriesAtTop )
        gid = popup->insertItem( i18n( "Navigate Class-View by \"%1\"" ).arg( cleanForMenu( name ) ), m, 6, cpos++ );
      else
        gid = popup->insertItem( i18n( "Navigate Class-View by \"%1\"" ).arg( cleanForMenu( name ) ), m );

      popup->setWhatsThis( gid, i18n( "<b>Navigation</b><p>Provides a menu to show involved items in the class-view " ) );

      PopupClassViewFillerHelpStruct h( this );
      PopupFiller<PopupClassViewFillerHelpStruct> filler( h, "" );

      filler.fill( m, type );
    }
  }

  if ( contextMenuEntriesAtTop )
    popup->insertSeparator( cpos );
}

void CppCodeCompletion::slotTextHint( int line, int column, QString &text ) {
  if ( ! m_pSupport->codeCompletionConfig() ->statusBarTypeEvaluation() )
    return ;

  kdDebug( 9007 ) << "CppCodeCompletion::slotTextHint()" << endl;

  clearStatusText();

  if ( m_lastHintTime.msecsTo( QTime::currentTime() ) < 300 ) {
    kdDebug( 9007 ) << "slotNeedTextHint called too often" << endl;
    return ;
  }

  m_lastHintTime = QTime::currentTime();

  clearStatusText();
  text = "";
  if ( !m_pSupport || !m_activeEditor )
    return ;

  SimpleTypeConfiguration conf( m_activeFileName );

  EvaluationResult type = evaluateExpressionAt( line, column, conf );

  if ( type.expr.expr().stripWhiteSpace().isEmpty() )
    return ; ///Expression could not be found

  if ( type.sourceVariable ) {
    text += type.sourceVariable.toText() + "\n";
  }

  if ( type->resolved() ) {
    /*SimpleTypeFunctionInterface* f = type->resolved()->asFunction();
    if( f ) {
    	text += "function: \"" + buildSignature( type->resolved() ) + "\"";
    } else {
    	QValueList<TypeDesc> trace = type.resultType->trace();
    	if( !trace.isEmpty() ) {
    		for( QValueList<TypeDesc>::iterator it = trace.begin(); it != trace.end(); ++it ) {
    			text += (*it).fullNameChain() + " --> ";
    		}
    		text += "\n";
    	}
    	text += "type: \"" + type.resultType->fullTypeResolved() + "\"";
    }
    if( type.resultType->parent()) text += "\nnested in: \"" + type.resultType->parent()->fullTypeResolvedWithScope() + "\"";
    DeclarationInfo i = type.resultType->getDeclarationInfo();
    if( i ) text += "\n" + i.locationToText();

    if( !type.resultType->comment().isEmpty() ) text +=  "\n\n" + type.resultType->comment() + "";*/

  } else {}

  kdDebug( 9007 ) << "showing: \n" << text << endl;
  const int timeout = 2000;

  if ( type->resolved() ) {
    addStatusText( i18n( "Type of \"%1\" is \"%2\"" ).arg( type.expr.expr() ).arg( type->fullNameChain() ), timeout );
    if ( type.sourceVariable && !type.sourceVariable.comment.isEmpty() ) {
      addStatusText( i18n( "Comment on variable \"%1\": \"%2\"" ).arg( type.sourceVariable.name ).arg( type.sourceVariable.comment ) , 10000 );
    }
    if ( !type->resolved() ->comment().isEmpty() ) {
      addStatusText( i18n( "Comment on \"%1\": \"%2\"" ).arg( type->name() ).arg( type->resolved() ->comment() ) , 10000 );
    }
    if ( type->resolved() ->comment().isEmpty() ) {
      addStatusText( i18n( "\"%1\" has no comment" ).arg( type->name() ) , timeout );
    }
  } else {
    if ( type ) {
			if( !BuiltinTypes::isBuiltin( type.resultType ) ) {
				addStatusText( i18n( "Type of \"%1\" is unresolved, name: \"%2\"" ).arg( type.expr.expr() ).arg( type->fullNameChain() ), 2 * timeout );
			} else {
				addStatusText( i18n( "\"%1\" is of builtin type \"%2\", a %3" ).arg( type.expr.expr() ).arg( type->fullNameChain() ).arg(BuiltinTypes::comment( type.resultType )), 2 * timeout );
			}


    } else {
      addStatusText( i18n( "Type of \"%1\" could not be evaluated! Tried to evaluate expression as \"%2\"" ).arg( type.expr.expr() ).arg( type.expr.typeAsString() ), 2 * timeout );
    }
  }

  text = ""; ///Don't really use tooltips since those are not implemented in katepart, and don't work right in the qt-designer based part
}

///not good..
bool CppCodeCompletion::isTypeExpression( const QString& expr ) {
  TypeDesc d( expr );
  if ( !d.isValidType() )
    return false;

  QString ex = d.fullNameChain();
  QStringList lex = QStringList::split( " ", ex );
  QStringList lexpr = QStringList::split( " ", expr );
  return lex.join( " " ) == lexpr.join( " " );
}

bool CppCodeCompletion::mayBeTypeTail( int line, int column, QString& append, bool inFunction ) {
  QString tail = clearComments( m_activeEditor->text( line, column + 1, line + 10 > ( int ) m_activeEditor->numLines() ? ( int ) m_activeEditor->numLines() : line + 10, 0 ) );
  tail.replace( "\n", " " );
  SafetyCounter s ( 100 );
  bool hadSpace = false;
  while ( !tail.isEmpty() && s ) {
    if ( tail[ 0 ] == ';' ) {
      return false;
    } else if ( ( !inFunction && tail[ 0 ] == ',' ) || tail[ 0 ] == '&' || tail[ 0 ] == '*' || tail[ 0 ] == '{' || tail[ 0 ] == ':' ) {
      return true;
    } else if ( isTypeOpenParen( tail[ 0 ] ) ) {
      ///TODO: use findClose to make the whole expression include template-params
      int to = findClose( tail, 0 );
      if ( to != -1 ) {
        append = tail.left( to + 1 );
        tail = tail.mid( to + 1 );
      } else {
        return false;
      }
    } else if ( isTypeCloseParen( tail[ 0 ] ) ) {
      return true;
    } else if ( tail[ 0 ].isSpace() ) {
      tail = tail.mid( 1 );
      hadSpace = true;
    } else if ( tail[ 0 ].isLetter() ) {
      return hadSpace;
    } else {
      break;
    }
  }

  return false;
}

bool CppCodeCompletion::canBeTypePrefix( const QString& prefix, bool inFunction ) {

  for ( int p = prefix.length() - 1 ; p >= 0; --p ) {
    if ( prefix[ p ].isSpace() ) {
      continue;
    }

    if ( prefix[ p ] == ';' || prefix[ p ] == '<' || prefix[ p ] == ':' || ( !inFunction && ( prefix[ p ] == '(' || prefix[ p ] == ',' ) ) || prefix[ p ] == '}' || prefix[ p ] == '{' ) {
      return true;
    }

    ///@todo: make this a simple regex
    if ( prefix[ p ].isLetterOrNumber() && ( tokenAt( prefix, "class", p ) || tokenAt( prefix, "struct", p ) || tokenAt( prefix, "const", p ) || tokenAt( prefix, "typedef", p ) || tokenAt( prefix, "public", p ) || tokenAt( prefix, "protected", p ) || tokenAt( prefix, "private", p ) || tokenAt( prefix, "virtual", p ) || tokenAt( prefix, "static", p ) || tokenAt( prefix, "virtual", p ) ) )
      return true;
    else {
      return false;
    }
  }

  return true;
}

///This function is just a litte hack und should be remade, it doesn't work for all cases
ExpressionInfo CppCodeCompletion::findExpressionAt( int line, int column, int startLine, int startCol, bool inFunction ) {
  ExpressionInfo ret;

  QString contents = clearComments( getText( startLine, startCol, line, column ) );


  int start_expr = expressionAt( contents, contents.length() );

  if ( start_expr != int( contents.length() ) ) {
    QString str = contents.mid( start_expr, contents.length() - start_expr ).stripWhiteSpace();
    if ( str.startsWith( "new " ) ) {
      str = str.mid( 4 ).stripWhiteSpace();
    }
    ret.setExpr( str );
    if ( !ret.expr().isEmpty() )
      ret.t = ExpressionInfo::NormalExpression;
  }

  if ( ret ) {
    ///Check whether it may be a type-expression
    bool mayBeType = true;
    QString append;
    if ( !mayBeTypeTail( line, column - 1, append, inFunction ) )
      mayBeType = false;
    if ( mayBeType ) {
      if ( !canBeTypePrefix( contents.left( start_expr ), inFunction ) )
        mayBeType = false;
    }

    //make this a regexp
    QString e = ret.expr();
    if ( e.contains( "." ) || e.contains( "->" ) || e.contains( "(" ) || e.contains( ")" ) || e.contains( "=" ) || e.contains( "-" ) )
      mayBeType = false;

    if ( mayBeType ) {
      ret.setExpr( ret.expr() + append );
      ret.t = ExpressionInfo::TypeExpression;
    }
  }

  return ret;
}

void macrosToDriver( Driver& d, FileDom file ) {
  return;  //Deactivate this for now, because macros can cause inconsistency of line/column-numbers between processed text and the not-processed text of the buffer
	ParseResultPointer p;
	if( file )
		p = file->parseResult();
	ParsedFile* pf = dynamic_cast<ParsedFile*>( p.data() );
	if( pf ) {
		d.insertMacros( pf->usedMacros() ); ///Add macros
	}
}

SimpleContext* CppCodeCompletion::computeFunctionContext( FunctionDom f, int line, int col, SimpleTypeConfiguration& conf ) {
  Q_UNUSED(conf);
  if ( !f )
    return 0;
  int modelStartLine, modelStartColumn;
  int modelEndLine, modelEndColumn;

  f->getStartPosition( &modelStartLine, &modelStartColumn );
  f->getEndPosition( &modelEndLine, &modelEndColumn );

  QString textLine = m_activeEditor->textLine( modelStartLine );
  kdDebug( 9007 ) << "startLine = " << textLine << endl;

  QString contents = getText( modelStartLine, modelStartColumn, line, col );

  Driver d;
  Lexer lexer( &d );
	macrosToDriver( d, f->file() );

  lexer.setSource( contents );
  Parser parser( &d, &lexer );

  DeclarationAST::Node recoveredDecl;
	RecoveryPoint* recoveryPoint = this->d->findRecoveryPoint( line, col );///@todo recovery-points are not needed anymore

  parser.parseDeclaration( recoveredDecl );
  if ( recoveredDecl.get() ) {
    bool isFunDef = recoveredDecl->nodeType() == NodeType_FunctionDefinition;
    kdDebug( 9007 ) << "is function definition= " << isFunDef << endl;

    int startLine, startColumn;
    int endLine, endColumn;
    recoveredDecl->getStartPosition( &startLine, &startColumn );
    recoveredDecl->getEndPosition( &endLine, &endColumn );
    /*if( startLine != modelStartLine || endLine != modelEndLine || startColumn != modelStartColumn || endColumn != modelEndColumn ) {
    kdDebug( 9007 ) << "code-model and real file are out of sync \nfunction-bounds in code-model: " << endl;
    	kdDebug( 9007 ) << "(l " << modelStartLine << ", c " << modelStartColumn << ") - (l " << modelEndLine  << ", c " << modelEndColumn << ") " << "parsed function-bounds: " << endl;
    	kdDebug( 9007 ) << "(l " << startLine << ", c " << startColumn << ") - (l " << endLine << ", c " << endColumn << ") " << endl;
    }*/

    if ( isFunDef ) {
      FunctionDefinitionAST * def = static_cast<FunctionDefinitionAST*>( recoveredDecl.get() );

	    SimpleContext* ctx = computeContext( def, endLine, endColumn, modelStartLine, modelStartColumn );
      if ( !ctx )
        return 0;

      QStringList scope = f->scope();


      if ( !scope.isEmpty() ) {
        SimpleType parentType;
        /* if( !m_cachedFromContext ) {
          TypePointer t = SimpleType(QStringList())->locateDecType( scope.join("") ).desc().resolved();;
          if( t )
            parentType = SimpleType( t.data() );
          else
            parentType = SimpleType( scope );
        } else {*/
	      parentType = SimpleType( scope, getIncludeFiles() );
        //}
        parentType->descForEdit().setTotalPointerDepth( 1 );
        ctx->setContainer( parentType );
      }

      SimpleType global = ctx->global();

	    if( dynamic_cast<SimpleTypeNamespace*>( &(*global) ) ) {
        SimpleTypeNamespace* globalNs = static_cast <SimpleTypeNamespace*>( &(*global) );
				QValueList<QPair<QString, QString> > localImports = ctx->imports();
				for( QValueList<QPair<QString, QString> >::const_iterator it = localImports.begin(); it != localImports.end(); ++it )
					globalNs->addAliasMap( (*it).first, (*it).second );
	    }

	    /* //Should not be necessary any more
				if( !getParsedFile( f->file().data() ) || getParsedFile( f->file().data() )->includeFiles().size() <= 1 ) {
				if ( !m_cachedFromContext ) {
					conf.setGlobalNamespace( &( *global ) );
					if ( recoveryPoint ) {
						recoveryPoint->registerImports( global, m_pSupport->codeCompletionConfig() ->namespaceAliases() );
					} else {
						kdDebug( 9007 ) << "no recovery-point, cannot use imports" << endl;
					}
				}
	    }*/

      ///Insert the "this"-type(container) and correctly resolve it using imported namespaces
      if ( ctx->container() ) {
        if ( !m_cachedFromContext ) {
          TypeDesc td = ctx->container() ->desc();
	        td.setIncludeFiles( getIncludeFiles() );
          td.makePrivate();

          td.resetResolved( );
          TypePointer tt = ctx->container() ->locateDecType( td, SimpleTypeImpl::LocateBase ) ->resolved();
          if ( tt ) {
            ctx->setContainer( SimpleType( tt ) );
          } else {
            kdDebug( 9007 ) << "could not resolve local this-type \"" << td.fullNameChain() << "\"" << endl;
          }
        }

        SimpleType this_type = ctx->container();

        this_type->descForEdit().setTotalPointerDepth( 1 );

        SimpleVariable var;
        var.type = this_type->desc();
        var.name = "this";
        var.comment = this_type->comment();
        ctx->add
        ( var );
        ctx->setContainer( this_type );
      }

      return ctx;
    } else {
      kdDebug( 9007 ) << "computeFunctionContext: context is no function-definition" << endl;
    }
  } else {
    kdDebug( 9007 ) << "computeFunctionContext: could not find a valid declaration to recover" << endl;
  }
  return 0;
}

bool CppCodeCompletion::functionContains( FunctionDom f , int line, int col ) {
  if ( !f )
    return false;
  int sl, sc, el, ec;
  f->getStartPosition( &sl, &sc );
  f->getEndPosition( &el, &ec );
  QString t = clearComments( getText( sl, sc, el, ec ) );
  if ( t.isEmpty() )
    return false;

	//int i = t.find( '{' );
	int i = t.find( '(' ); //This now includes the argument-list
	if ( i == -1 )
    return false;
  int lineCols = 0;
  for ( int a = 0; a < i; a++ ) {
    if ( t[ a ] == '\n' ) {
      sl++;
      lineCols = 0;
    } else {
      lineCols++;
    }
  }

  sc += lineCols;

  return ( line > sl || ( line == sl && col >= sc ) ) && ( line < el || ( line == el && col < ec ) );
}

void CppCodeCompletion::getFunctionBody( FunctionDom f , int& line, int& col ) {
	if ( !f )
		return;
	int sl, sc, el, ec;
	f->getStartPosition( &sl, &sc );
	f->getEndPosition( &el, &ec );
	QString t = clearComments( getText( sl, sc, el, ec ) );
	if ( t.isEmpty() )
		return;

	int i = t.find( '{' );
	if ( i == -1 )
		return;
	i++;
  if( (uint)i >= t.length() )
		return;
	int lineCols = 0;
	for ( int a = 0; a < i; a++ ) {
		if ( t[ a ] == '\n' ) {
			sl++;
			lineCols = 0;
		} else {
			lineCols++;
		}
	}

	sc += lineCols;

	line = sl;
	col = sc;
}

void CppCodeCompletion::emptyCache() {
  m_cachedFromContext = 0;
  SimpleTypeConfiguration c; ///Will automatically destroy the type-store when the function is closed
  kdDebug( 9007 ) << "completion-cache emptied" << endl;
}

void CppCodeCompletion::needRecoveryPoints() {

  if ( this->d->recoveryPoints.isEmpty() ) {
    kdDebug( 9007 ) << "missing recovery-points for file " << m_activeFileName << " they have to be computed now" << endl;
    m_pSupport->backgroundParser() ->lock ()
    ;

    std::vector<CppCodeCompletion> vec;

	  TranslationUnitAST * ast = *m_pSupport->backgroundParser() ->translationUnit( m_activeFileName );
    m_pSupport->backgroundParser() ->unlock();
    if ( !ast ) {
      kdDebug( 9007 ) << "background-parser is missing the translation-unit. The file needs to be reparsed." << endl;
	    m_pSupport->parseFileAndDependencies( m_activeFileName, true );
// 	    m_pSupport->mainWindow() ->statusBar() ->message( i18n( "Background-parser is missing the necessary translation-unit. It will be computed, but this completion will fail." ).arg( m_activeFileName ), 2000 );
	    return;
    } else {
      computeRecoveryPointsLocked();
    }
	  if ( this->d->recoveryPoints.isEmpty() ) {
		  kdDebug( 9007 ) << "Failed to compute recovery-points for " << m_activeFileName << endl;
// 		  m_pSupport->mainWindow() ->statusBar() ->message( i18n( "Failed to compute recovery-points for %1" ).arg( m_activeFileName ), 1000 );
	  } else {
		  kdDebug( 9007 ) << "successfully computed recovery-points for " << m_activeFileName << endl;
	  }
  }
}

EvaluationResult CppCodeCompletion::evaluateExpressionType( int line, int column, SimpleTypeConfiguration& conf, EvaluateExpressionOptions opt ) {
  EvaluationResult ret;
	safetyCounter.init();

  FileDom file = m_pSupport->codeModel() ->fileByName( m_activeFileName );

  if ( !file ) {
//     m_pSupport->mainWindow() ->statusBar() ->message( i18n( "File %1 does not exist in the code-model" ).arg( m_activeFileName ), 1000 );
    kdDebug( 9007 ) << "Error: file " << m_activeFileName << " could not be located in the code-model, code-completion stopped\n";
    return SimpleType();
  }

  needRecoveryPoints();

  CodeModelUtils::CodeModelHelper fileModel( m_pSupport->codeModel(), file );
  ItemDom contextItem;

  int nLine = line, nCol = column;

	//  emptyCache();
  fitContextItem( line, column );

  QString strCurLine = m_activeEditor->textLine( nLine );

  QString ch = strCurLine.mid( nCol - 1, 1 );
  QString ch2 = strCurLine.mid( nCol - 2, 2 );

  while ( ch[ 0 ].isSpace() && nCol >= 3 ) {
    nCol -= 1;
    ch = strCurLine.mid( nCol - 1, 1 );
    ch2 = strCurLine.mid( nCol - 2, 2 );
  }

  if ( ch2 == "->" || ch == "." || ch == "(" ) {
    int pos = ch2 == "->" ? nCol - 3 : nCol - 2;
    QChar c = strCurLine[ pos ];
    while ( pos > 0 && c.isSpace() )
      c = strCurLine[ --pos ];

    if ( !( c.isLetterOrNumber() || c == '_' || c == ')' || c == ']' || c == '>' ) ) {
      conf.invalidate();
      return SimpleType();
    }
  }
  bool showArguments = false;

  if ( ch == "(" ) {
    --nCol;
    while ( nCol > 0 && strCurLine[ nCol ].isSpace() )
      --nCol;
    showArguments = true;
  }

  QString word;

  {
    ExpressionInfo exp_ = findExpressionAt( line, column , line, 0 );

    if( file->parseResult() ) {
      ParsedFilePointer p = dynamic_cast<ParsedFile*>( file->parseResult().data());
      if( p ) {
        if( p->usedMacros().hasMacro( exp_.expr() ) ) {
          //It is a macro, return it
          ret.expr = exp_.expr();
          ret.isMacro = true;
          ret.macro = p->usedMacros().macro( exp_.expr() );
          return ret;
        }
      }
    }
  }

	if ( !m_cachedFromContext )
		conf.setGlobalNamespace( createGlobalNamespace() );

  ItemLocker<BackgroundParser> block( *m_pSupport->backgroundParser() );

  FunctionDom currentFunction = fileModel.functionAt( line, column );

  bool functionFailed = true;

  if ( opt & SearchInFunctions ) {
    //currentFunction = fileModel.functionAt( line, column );

	  if ( currentFunction && functionContains( currentFunction, line, column ) ) {
		  ///Evaluate the context of the function-body if we're in the argument-list
		  int realLine = line, realColumn = column;
		  getFunctionBody( currentFunction, realLine, realColumn );
		  if( realLine < line || ( realLine == line && realColumn < column ) ) {
			  realLine = line;
			  realColumn = column;
		  }

      SimpleContext * ctx = computeFunctionContext( currentFunction, realLine, realColumn, conf );
      contextItem = currentFunction.data();

      if ( ctx ) {
        opt = remFlag( opt, SearchInClasses );
        int startLine, endLine;
        currentFunction->getStartPosition( &startLine, &endLine );
        ExpressionInfo exp = findExpressionAt( line, column , startLine, endLine, true );
        if ( ( opt & DefaultAsTypeExpression ) && ( !exp.canBeNormalExpression() && !exp.canBeTypeExpression() ) && !exp.expr().isEmpty() )
          exp.t = ExpressionInfo::TypeExpression;

        if ( exp.canBeTypeExpression() ) {
          {
            if ( ! ( opt & IncludeTypeExpression ) ) {
              kdDebug( 9007 ) << "recognized a type-expression, but another expression-type is desired" << endl;
            } else {
	            TypeDesc d( exp.expr() );
	            d.setIncludeFiles( getIncludeFiles() );
              ret.resultType = ctx->container() ->locateDecType( d );
              ret.expr = exp;
            }
          }
        }
        if (        /*exp.canBeNormalExpression() &&*/ !ret.resultType->resolved() ) { ///It is not cleary possible to recognize the kind of an expression from the syntax as long as it's not written completely
          {
            if ( ! ( opt & IncludeStandardExpressions ) ) {
              kdDebug( 9007 ) << "recognized a standard-expression, but another expression-type is desired" << endl;
            } else {
              ///Remove the not completely typed last word while normal completion
              if ( !showArguments && ( opt & CompletionOption ) ) {
                QString e = exp.expr();
                int idx = e.length() - 1;
                while ( e[ idx ].isLetterOrNumber() || e[ idx ] == '_' )
                  --idx;

                if ( idx != int( e.length() ) - 1 ) {
                  ++idx;
                  word = e.mid( idx ).stripWhiteSpace();
                  exp.setExpr( e.left( idx ).stripWhiteSpace() );
                }
              }

              functionFailed = false;
              ret = evaluateExpression( exp, ctx );
            }
          }
        }
      } else {
        kdDebug( 9007 ) << "could not compute context" << endl;
      }
      if ( ctx )
        delete ctx;
    } else {
      kdDebug( 9007 ) << "could not find context-function in code-model" << endl;
    }
  }

  if ( ( opt & SearchInClasses ) && !ret->resolved() && functionFailed ) {
    ClassDom currentClass = fileModel.classAt( line, column );
    int startLine = 0, startCol = 0;

	  RecoveryPoint* recoveryPoint = this->d->findRecoveryPoint( line, column );

    QStringList scope;

    if ( !currentClass ) {
      kdDebug( 9007 ) << "no container-class found" << endl;
      if ( !recoveryPoint ) {
        kdDebug( 9007 ) << "no recovery-point found" << endl;
      } else {
        startLine = recoveryPoint->startLine;
        startCol = recoveryPoint->startColumn;
        scope = recoveryPoint->scope;
      }
    } else {
      contextItem = currentClass.data();
      scope = currentClass->scope();
      scope << currentClass->name();
      currentClass->getStartPosition( &startLine, &startCol );
    }

	  SimpleType container;
    if ( m_cachedFromContext ) {
	    TypeDesc d( scope.join( "::" ) );
	    d.setIncludeFiles( getIncludeFiles() );
	    SimpleTypeImpl * i = SimpleType( QStringList(), getIncludeFiles() ) ->locateDecType( d ).desc().resolved().data();
      if ( i )
        container = i;
      else
	      container = SimpleType( scope, getIncludeFiles() );
    } else {
	    container = SimpleType( scope, getIncludeFiles() );
    }

    ExpressionInfo exp = findExpressionAt( line, column , startLine, startCol );
    exp.t = ExpressionInfo::TypeExpression;	///Outside of functions, we can only handle type-expressions
    ret.expr = exp;

    if ( exp && ( exp.t & ExpressionInfo::TypeExpression ) ) {
      kdDebug( 9007 ) << "locating \"" << exp.expr() << "\" in " << container->fullTypeResolvedWithScope() << endl;
	    TypeDesc d( exp.expr() );
	    d.setIncludeFiles( getIncludeFiles() );
      ret.resultType = container->locateDecType( d );
    } else {
      if ( exp ) {
        kdDebug( 9007 ) << "wrong expression-type recognized" << endl;
      } else {
        kdDebug( 9007 ) << "expression could not be recognized" << endl;
      }
    }
  }

  CppCodeCompletionConfig * cfg = m_pSupport->codeCompletionConfig();
  if( cfg->usePermanentCaching() && contextItem ) {
    conf.invalidate();
    m_cachedFromContext = contextItem;
  }

  return ret;
}

bool isAfterKeyword( const QString& str, int column ) {
  QStringList keywords;
  keywords << "new";
  keywords << "throw";
  keywords << "return";
  keywords << "emit"; ///This could be done even better by only showing signals for completion..
  for ( QStringList::iterator it = keywords.begin(); it != keywords.end(); ++it ) {
    int len = ( *it ).length();
    if ( column >= len && str.mid( column - len, len ) == *it )
      return true;
  }
  return false;
}

void CppCodeCompletion::setMaxComments( int count ) {
	m_maxComments = count;
}

///TODO: make this use findExpressionAt etc. (like the other expression-evaluation-stuff)
void CppCodeCompletion::completeText( bool invokedOnDemand /*= false*/ ) {
  kdDebug( 9007 ) << "CppCodeCompletion::completeText()" << endl;
  clearStatusText();

  if ( !m_pSupport || !m_activeCursor || !m_activeEditor || !m_activeCompletion )
    return ;

	setMaxComments( 1000 );

  needRecoveryPoints();

  CppCodeCompletionConfig * cfg = m_pSupport->codeCompletionConfig();
  m_demandCompletion = invokedOnDemand;

  FileDom file = m_pSupport->codeModel() ->fileByName( m_activeFileName );

  if ( !file ) {
//     m_pSupport->mainWindow() ->statusBar() ->message( i18n( "File %1 does not exist in the code-model" ).arg( m_activeFileName ), 1000 );
    kdDebug( 9007 ) << "Error: file " << m_activeFileName << " could not be located in the code-model, code-completion stopped\n";
    return ;
  }

  CodeModelUtils::CodeModelHelper fileModel( m_pSupport->codeModel(), file );

  ItemDom contextItem;

  unsigned int line, column;
  m_activeCursor->cursorPositionReal( &line, &column );

  fitContextItem( line, column );

  ///Check whether the cursor is within a comment
  int surroundingStartLine = line - 10, surroundingEndLine = line + 10;
  if ( surroundingStartLine < 0 )
    surroundingStartLine = 0;
  if ( surroundingEndLine > m_activeEditor->numLines() - 1 )
    surroundingEndLine = m_activeEditor->numLines() - 1;
  int surroundingEndCol = m_activeEditor->lineLength( surroundingEndLine );

  QString pre = getText( surroundingStartLine, 0, line, column );
  int pos = pre.length();
  pre += getText( line, column, surroundingEndLine, surroundingEndCol );
  QString cleared = clearComments( pre );
  if ( cleared[ pos ] != pre[ pos ] ) {
    kdDebug( 9007 ) << "stopping completion because we're in a coment" << endl;
    return ;
  }

  int nLine = line, nCol = column;

  QString strCurLine = clearComments( m_activeEditor->textLine( nLine ) );

  QString ch = strCurLine.mid( nCol - 1, 1 );
  QString ch2 = strCurLine.mid( nCol - 2, 2 );

  while ( ch[ 0 ].isSpace() && nCol >= 3 ) {
    nCol -= 1;
    ch = strCurLine.mid( nCol - 1, 1 );
    ch2 = strCurLine.mid( nCol - 2, 2 );
  }

  if ( m_includeRx.search( strCurLine ) != -1 ) {
    if ( !m_fileEntryList.isEmpty() ) {
      m_bCompletionBoxShow = true;
      m_activeCompletion->showCompletionBox( m_fileEntryList, column - m_includeRx.matchedLength() );
    }
    return ;
  }

  bool showArguments = false;
  bool isInstance = true;
  m_completionMode = NormalCompletion;

  if ( ch2 == "->" || ch == "." || ch == "(" ) {
    int pos = ch2 == "->" ? nCol - 3 : nCol - 2;
    QChar c = strCurLine[ pos ];
    while ( pos > 0 && c.isSpace() )
      c = strCurLine[ --pos ];

    if ( !( c.isLetterOrNumber() || c == '_' || c == ')' || c == ']' || c == '>' ) )
      return ;
  }

  if ( ch == "(" ) {
    --nCol;
    while ( nCol > 0 && strCurLine[ nCol - 1 ].isSpace() )
      --nCol;

    ///check whether it is a value-definition using constructor
    int column = nCol;
    bool s1 = false, s2 = false;
    while ( column > 0 && isValidIdentifierSign( strCurLine[ column - 1 ] ) ) {
      column--;
      s1 = true;
    }

    ///skip white space
    while ( column > 0 && strCurLine[ column - 1 ].isSpace() ) {
      --column;
      s2 = true;
    }

    if ( s1 && s2 && isValidIdentifierSign( strCurLine[ column - 1 ] ) ) {
      if ( isAfterKeyword( strCurLine, column ) ) {
        ///Maybe a constructor using "new", or "throw", "return", ...
      } else {
        ///it is a local constructor like "QString name("David");"
        nCol = column;
      }
    }



    showArguments = TRUE;
  }

  EvaluationResult type;
  SimpleType this_type;
  QString expr, word;

  DeclarationAST::Node recoveredDecl;
  TypeSpecifierAST::Node recoveredTypeSpec;

  SimpleContext* ctx = 0;
  SimpleTypeConfiguration conf( m_activeFileName );

	if ( !m_cachedFromContext )
			conf.setGlobalNamespace( createGlobalNamespace() );

  ItemLocker<BackgroundParser> block( *m_pSupport->backgroundParser() );

  FunctionDom currentFunction = fileModel.functionAt( line, column );

  RecoveryPoint * recoveryPoint = d->findRecoveryPoint( line, column );
  if ( recoveryPoint || currentFunction ) {
    contextItem = currentFunction.data();
    QStringList scope;

    int startLine, startColumn;
    if ( currentFunction ) { ///maybe change the priority of these
      kdDebug( 9007 ) << "using code-model for completion" << endl;
      currentFunction->getStartPosition( &startLine, &startColumn );
      scope = currentFunction->scope();
    } else {
      kdDebug( 9007 ) << "recovery-point, node-kind = " << nodeTypeToString( recoveryPoint->kind ) << endl;
      startLine = recoveryPoint->startLine;
      startColumn = recoveryPoint->startColumn;
      scope = recoveryPoint->scope;
    }

    QString textLine = m_activeEditor->textLine( startLine );
    kdDebug( 9007 ) << "startLine = " << textLine << endl;

    if ( currentFunction || recoveryPoint->kind == NodeType_FunctionDefinition ) {

      QString textToReparse = clearComments( getText( startLine, startColumn, line, showArguments ? nCol : column ) );

      kdDebug( 9007 ) << "-------------> reparse text" << endl << textToReparse << endl
      << "--------------------------------------------" << endl;

      Driver d;
      Lexer lexer( &d );

      macrosToDriver( d, file );

      lexer.setSource( textToReparse );
      Parser parser( &d, &lexer );

      parser.parseDeclaration( recoveredDecl );
      /*			kdDebug(9007) << "recoveredDecl = " << recoveredDecl.get() << endl;*/
      if ( recoveredDecl.get() ) {

        bool isFunDef = recoveredDecl->nodeType() == NodeType_FunctionDefinition;
        kdDebug( 9007 ) << "is function definition= " << isFunDef << endl;

        int endLine, endColumn;
        recoveredDecl->getEndPosition( &endLine, &endColumn );
        kdDebug( 9007 ) << "endLine = " << endLine << ", endColumn " << endColumn << endl;

        /// @todo check end position

        if ( isFunDef ) {
          FunctionDefinitionAST * def = static_cast<FunctionDefinitionAST*>( recoveredDecl.get() );

          /// @todo remove code duplication

          QString contents = textToReparse;
          int start_expr = expressionAt( contents, contents.length() );

          // kdDebug(9007) << "start_expr = " << start_expr << endl;
          if ( start_expr != int( contents.length() ) )
            expr = contents.mid( start_expr, contents.length() - start_expr ).stripWhiteSpace();

          if ( expr.startsWith( "SIGNAL" ) || expr.startsWith( "SLOT" ) ) {
            m_completionMode = expr.startsWith( "SIGNAL" ) ? SignalCompletion : SlotCompletion;

            showArguments = false;
            int end_expr = start_expr - 1;
            while ( end_expr > 0 && contents[ end_expr ].isSpace() )
              --end_expr;

            if ( contents[ end_expr ] != ',' ) {
              expr = QString::null;
            } else {
              start_expr = expressionAt( contents, end_expr );
              expr = contents.mid( start_expr, end_expr - start_expr ).stripWhiteSpace();
            }
          } else {
            if ( !showArguments ) {
              int idx = expr.length() - 1;
              while ( expr[ idx ].isLetterOrNumber() || expr[ idx ] == '_' )
                --idx;

              if ( idx != int( expr.length() ) - 1 ) {
                ++idx;
                word = expr.mid( idx ).stripWhiteSpace();
                expr = expr.left( idx ).stripWhiteSpace();
              }
            }
          }

          ctx = computeContext( def, endLine, endColumn, startLine, startColumn );
          DeclaratorAST* d = def->initDeclarator() ->declarator();
          NameAST* name = d->declaratorId();

          QStringList nested;

          QPtrList<ClassOrNamespaceNameAST> l;
          if ( name ) {
            l = name->classOrNamespaceNameList();
          }
          //		    QPtrList<ClassOrNamespaceNameAST> l = name->classOrNamespaceNameList();
          QPtrListIterator<ClassOrNamespaceNameAST> nameIt( l );
          while ( nameIt.current() ) {
            if ( nameIt.current() ->name() ) {
              nested << nameIt.current() ->name() ->text();
            }
            ++nameIt;
          }

          if ( currentFunction ) {
            scope = currentFunction->scope();
            if ( !scope.isEmpty() ) {
              //scope.pop_back();
            } else {
              kdDebug( 9007 ) << "scope is empty" << endl;
            }
						if( dynamic_cast<SimpleTypeNamespace*>( SimpleType::globalNamespace().data() ) ) {
							SimpleTypeNamespace* globalNs = static_cast <SimpleTypeNamespace*>( SimpleType::globalNamespace().data() );
							QValueList<QPair<QString, QString> > localImports = ctx->imports();
							for( QValueList<QPair<QString, QString> >::const_iterator it = localImports.begin(); it != localImports.end(); ++it )
								globalNs->addAliasMap( (*it).first, (*it).second );
						}
          } else {
            scope += nested;
          }

          if ( !scope.isEmpty() ) {
            SimpleType parentType;

	          /*if( !m_cachedFromContext ) {
		          TypePointer t = createGlobalNamespace();
		          conf.setGlobalNamespace( t );
			          SimpleTypeNamespace * n = dynamic_cast<SimpleTypeNamespace*>( t.data() );
			          if ( !n ) {
			          QString str = QString( "the global namespace was not resolved correctly , real type: " ) + typeid( n ).name() + QString( " name: " ) + n->scope().join( "::" ) + " scope-size: " + n->scope().count();
				          kdDebug( 9007 ) << str << endl;
				          m_pSupport->mainWindow() ->statusBar() ->message( str , 1000 );
			          } else {
			          }
		          	this_type = SimpleType(t);
	          }*/

            if ( m_cachedFromContext ) {
	            TypeDesc d( scope.join( "::" ) );
	            d.setIncludeFiles( getIncludeFiles() );
	            SimpleTypeImpl * i = SimpleType( QStringList(), getIncludeFiles() ) ->locateDecType( d ).desc().resolved().data();
              if ( i ) {
                parentType = i;
              } else {
	              parentType = SimpleType( scope, getIncludeFiles() );
              }
            } else {
	            parentType = SimpleType( scope, getIncludeFiles() );
            }
            this_type = parentType;
            this_type->descForEdit().setTotalPointerDepth( 1 );
            ctx->setContainer( this_type );
          }

          ///Now locate the local type using the imported namespaces
          if ( !scope.isEmpty() ) {
            if ( !m_cachedFromContext ) {
              TypeDesc td = ctx->container() ->desc();
              td.makePrivate();
              td.resetResolved( );
	            td.setIncludeFiles( getIncludeFiles() );
              TypePointer tt = ctx->container() ->locateDecType( td, SimpleTypeImpl::LocateBase ) ->resolved();
              if ( tt ) {
                ctx->setContainer( SimpleType( tt ) );
              } else {
                kdDebug( 9007 ) << "could not resolve local this-type \"" << td.fullNameChain() << "\"" << endl;
              }
            }

            SimpleType this_type = ctx->container();

            this_type->descForEdit().setTotalPointerDepth( 1 );

            SimpleVariable var;
            var.type = this_type->desc();
            var.name = "this";
            var.comment = this_type->comment();
            ctx->add
            ( var );
            ctx->setContainer( this_type );
          }

          ExpressionInfo exp( expr );
          exp.t = ( ExpressionInfo::Type ) ( ExpressionInfo::NormalExpression | ExpressionInfo::TypeExpression );
          type = evaluateExpression( exp, ctx );
        }

      } else {
        kdDebug( 9007 ) << "no valid declaration to recover!!!" << endl;
      }
    } else if ( recoveryPoint->kind == NodeType_ClassSpecifier ) {
      QString textToReparse = getText( recoveryPoint->startLine, recoveryPoint->startColumn,
                                       recoveryPoint->endLine, recoveryPoint->endColumn, line );
      // 			kdDebug(9007) << "-------------> please reparse only text" << endl << textToReparse << endl
      // 			             << "--------------------------------------------" << endl;

      Driver d;
      Lexer lexer( &d );
    	macrosToDriver( d, file );

      lexer.setSource( textToReparse );
      Parser parser( &d, &lexer );

      parser.parseClassSpecifier( recoveredTypeSpec );
      /*			kdDebug(9007) << "recoveredDecl = " << recoveredTypeSpec.get() << endl;*/
      if ( recoveredTypeSpec.get() ) {

        //ClassSpecifierAST * clazz = static_cast<ClassSpecifierAST*>( recoveredTypeSpec.get() );

        QString keyword = getText( line, 0, line, column ).simplifyWhiteSpace();

        kdDebug( 9007 ) << "===========================> keyword is: " << keyword << endl;

        if ( keyword == "virtual" ) { /*
                                                                      					BaseClauseAST *baseClause = clazz->baseClause();
                                                                      					if ( baseClause )
                                                                      					{
                                                                      						QPtrList<BaseSpecifierAST> baseList = baseClause->baseSpecifierList();
                                                                      						QPtrList<BaseSpecifierAST>::iterator it = baseList.begin();

                                                                      						for ( ; it != baseList.end(); ++it )
                                                                      							type.append( ( *it )->name()->text() );

                                                                      						ctx = new SimpleContext();

                                                                      						showArguments = false;
                                                                      						m_completionMode = VirtualDeclCompletion;

                                                                      						kdDebug(9007) << "------> found virtual keyword for class specifier '"
                                                                      									<< clazz->text() << "'" << endl;
                                                                      					}*/
        } else if ( QString( "virtual" ).find( keyword ) != -1 )
          m_blockForKeyword = true;
        else
          m_blockForKeyword = false;
      }
    }
  }

	///@todo is all this necessary?
  if ( !recoveredDecl.get() && !recoveredTypeSpec.get() ) {
    TranslationUnitAST * ast = *m_pSupport->backgroundParser() ->translationUnit( m_activeFileName );
    if ( AST * node = findNodeAt( ast, line, column ) ) {
      kdDebug( 9007 ) << "------------------- AST FOUND --------------------" << endl;
      kdDebug( 9007 ) << "node-kind = " << nodeTypeToString( node->nodeType() ) << endl;

      if ( FunctionDefinitionAST * def = functionDefinition( node ) ) {
        kdDebug( 9007 ) << "------> found a function definition" << endl;

        int startLine, startColumn;
        def->getStartPosition( &startLine, &startColumn );

        QString contents = getText( startLine, startColumn, line, showArguments ? nCol : column );


        /// @todo remove code duplication
        int start_expr = expressionAt( contents, contents.length() );

        // kdDebug(9007) << "start_expr = " << start_expr << endl;
        if ( start_expr != int( contents.length() ) )
          expr = contents.mid( start_expr, contents.length() - start_expr ).stripWhiteSpace();

        if ( expr.startsWith( "SIGNAL" ) || expr.startsWith( "SLOT" ) ) {
          m_completionMode = expr.startsWith( "SIGNAL" ) ? SignalCompletion : SlotCompletion;

          showArguments = false;
          int end_expr = start_expr - 1;
          while ( end_expr > 0 && contents[ end_expr ].isSpace() )
            --end_expr;

          if ( contents[ end_expr ] != ',' ) {
            expr = QString::null;
          } else {
            start_expr = expressionAt( contents, end_expr );
            expr = contents.mid( start_expr, end_expr - start_expr ).stripWhiteSpace();
          }
        } else {
          int idx = expr.length() - 1;
          while ( expr[ idx ].isLetterOrNumber() || expr[ idx ] == '_' )
            --idx;

          if ( idx != int( expr.length() ) - 1 ) {
            ++idx;
            word = expr.mid( idx ).stripWhiteSpace();
            expr = expr.left( idx ).stripWhiteSpace();
          }
        }

        ctx = computeContext( def, line, column, startLine, startColumn );

        QStringList scope;
        scopeOfNode( def, scope );
	      this_type = SimpleType( scope, getIncludeFiles() );

        if ( scope.size() ) { /*
                                                                      					SimpleVariable var;
                                                                      					var.type = scope;
                                                                      					var.name = "this";
                                                                      					ctx->add( var );*/
          //kdDebug(9007) << "add variable " << var.name << " with type " << var.type << endl;
        }

        ExpressionInfo exp( expr );
        exp.t = ( ExpressionInfo::Type ) ( ExpressionInfo::NormalExpression | ExpressionInfo::TypeExpression );
        type = evaluateExpression( exp, ctx );
      }
    }
  }

  if ( !ctx )
    return ;

  if ( ch2 == "::" ) {
    QString str = clearComments( expr );
    if ( !str.contains( '.' ) && !str.contains( "->" ) )        ///Necessary, because the expression may also be like user->BaseUser::
      isInstance = false;
  }

	QString resolutionType = "(resolved)";
	if( !type->resolved() ) {
		if( BuiltinTypes::isBuiltin( type.resultType ) ) {
			resolutionType = "(builtin " + BuiltinTypes::comment( type.resultType ) +  ")";
		} else {
			resolutionType = "(unresolved)";
		}
	}

  kdDebug( 9007 ) << "===========================> type is: " << type->fullNameChain() << resolutionType << endl;
  kdDebug( 9007 ) << "===========================> word is: " << word << endl;

  if ( !showArguments ) {
    QValueList<CodeCompletionEntry> entryList;

    if ( !type && this_type && ( expr.isEmpty() || expr.endsWith( ";" ) ) ) {

	    bool alwaysIncludeNamespaces = cfg->alwaysIncludeNamespaces();
      {
        SimpleType t = this_type;
        ///First, all static data.
        bool ready = false;
        SafetyCounter cnt( 20 );
        int depth = 0;
        while ( !ready & cnt ) {
          if ( t->scope().isEmpty() ) {
            ready = true;
          }
	       if( !t->isNamespace() || invokedOnDemand || alwaysIncludeNamespaces )
            computeCompletionEntryList( t, entryList, t->scope(), false, depth );
          t = t->parent();
          depth++;
        }
      }
      {
        SimpleType t = this_type;
        ///Now find non-static(if we have an instance) and global data
        bool ready = false;
        SafetyCounter cnt( 20 );
        int depth = 0;
        bool first = true;
        while ( !ready & cnt ) {
          if ( t->scope().isEmpty() ) {
            ready = true;
          }
	       if ( ( (t->isNamespace() && invokedOnDemand) || alwaysIncludeNamespaces ) || ( first && isInstance ) )
            computeCompletionEntryList( t, entryList, t->scope(), t->isNamespace() ? true : isInstance, depth );
          t = t->parent();
          depth++;
          first = false;
        }
      }
	    if( ctx ) computeCompletionEntryList( entryList, ctx, isInstance );
    } else if ( type->resolved() && expr.isEmpty() ) {
	    if( ctx ) computeCompletionEntryList( entryList, ctx, isInstance );

      // 			if ( m_pSupport->codeCompletionConfig() ->includeGlobalFunctions() )
      // 				computeCompletionEntryList( type, entryList, QStringList(), false );

      computeCompletionEntryList( type, entryList, QStringList(), false );

      if ( this_type.scope().size() )
        computeCompletionEntryList( this_type, entryList, this_type.scope(), isInstance );
      computeCompletionEntryList( type, entryList, type->resolved() ->scope() , isInstance );
    } else if ( type->resolved() ) {
      if ( type->resolved() )
        computeCompletionEntryList( type, entryList, type->resolved() ->scope() , isInstance );
    }

    QStringList trueMatches;

    if ( invokedOnDemand ) {
      // find matching words
      QValueList<CodeCompletionEntry>::Iterator it;
      for ( it = entryList.begin(); it != entryList.end(); ++it ) {
        if ( ( *it ).text.startsWith( word ) ) {
          trueMatches << ( *it ).text;

          // if more than one entry matches, abort immediately
          if ( trueMatches.size() > 1 )
            break;
        }
      }
    }

    if ( invokedOnDemand && trueMatches.size() == 1 ) {
      // erbsland: get the cursor position now, because m_ccLine and m_ccColumn
      //           are not set until the first typed char.
      unsigned int nLine, nCol;
      m_activeCursor->cursorPositionReal( &nLine, &nCol );
      // there is only one entry -> complete immediately
      m_activeEditor->insertText( nLine, nCol,
                                  trueMatches[ 0 ].right( trueMatches[ 0 ].length() - word.length() ) );
    } else if ( entryList.size() ) {
      entryList = unique( entryList );
      qHeapSort( entryList );

      m_bCompletionBoxShow = true;
      ///Warning: the conversion is only possible because CodeCompletionEntry is binary compatible with KTextEditor::CompletionEntry,
      ///never change that!
      m_activeCompletion->showCompletionBox( *( ( QValueList<KTextEditor::CompletionEntry>* ) ( &entryList ) ), word.length() );
    }
  } else {
    QValueList<QStringList> signatureList;

    signatureList = computeSignatureList( type );

    QString methodName = type->name();

    ///Search for variables with ()-operator in the context
    if ( ctx ) {
      SimpleVariable var = ctx->findVariable( methodName );
      if ( !var.name.isEmpty() ) {
        signatureList += computeSignatureList( ctx->container() ->locateDecType( var.type ) );
      }
    }

    ///search for fitting methods/classes in the current context
    SimpleType t = this_type;
    bool ready = false;
    SafetyCounter s( 20 );
    do {
      if ( !t )
        ready = true;
	    TypeDesc d( methodName );
	    d.setIncludeFiles( getIncludeFiles() );
      SimpleType method = t->typeOf( d );
      if ( method )
        signatureList += computeSignatureList( method );
      if ( t )
        t = t->parent();
    } while ( !ready && s );

    if ( !signatureList.isEmpty() ) {
      //signatureList = unique( signatureList );
      //qHeapSort( signatureList );
      m_bArgHintShow = true;
      m_activeCompletion->showArgHint( unique( signatureList ), "()", "," );
    }
  }

  delete( ctx );
  ctx = 0;

  if ( cfg->usePermanentCaching() ) {
    conf.invalidate();
    m_cachedFromContext = contextItem;
  }
}


QValueList<QStringList> CppCodeCompletion::computeSignatureList( EvaluationResult result ) {
  SimpleType type = result;

	if ( result.expr.t == ExpressionInfo::TypeExpression ) {
		TypeDesc d( result->name() );
		d.setIncludeFiles( getIncludeFiles() );
    type = type->typeOf( d, SimpleTypeImpl::MemberInfo::Function ); ///Compute the signature of the constructor
	}

  QValueList<QStringList> retList;
  SimpleTypeFunctionInterface* f = type->asFunction();
  SimpleType currType = type;

  if ( !f && !type->isNamespace() ) {
	  SimpleType t = type->typeOf( TypeDesc("operator ( )"), SimpleTypeImpl::MemberInfo::Function );

    if ( t ) {
      f = t->asFunction();
      currType = t;
    }
  }

  while ( f ) {
    QStringList lst;
    QString sig = buildSignature( currType.get() );
    QString comment = currType->comment();
    QStringList commentList;
    if ( m_pSupport->codeCompletionConfig() ->showCommentWithArgumentHint() ) {

      if ( !comment.isEmpty() ) {
        if ( sig.length() + comment.length() < 130 ) {
          sig += ":  \"" + currType->comment() + "\"";
        } else {
          commentList = formatComment( comment );
        }
      }
    }

    lst << sig;
    lst += commentList;

    currType = f->nextFunction();

    ///Maybe try to apply implicit template-params in this place

    retList << lst;
    f = currType->asFunction();
  }
  return retList;
}

void CppCodeCompletion::synchronousParseReady( const QString& file, ParsedFilePointer unit ) {
  if ( file == m_activeFileName ) {
    computeRecoveryPoints( unit );
  }
}

void CppCodeCompletion::slotCodeModelUpdated( const QString& fileName ) {
  if ( fileName != m_activeFileName || !m_pSupport || !m_activeEditor )
    return ;

// 	m_pSupport->mainWindow() ->statusBar() ->message( i18n( "Current file updated %1" ).arg( m_activeFileName ), 1000 );

  computeRecoveryPointsLocked();
}

void CppCodeCompletion::slotFileParsed( const QString& fileName ) {
  if ( fileName != m_activeFileName || !m_pSupport || !m_activeEditor )
    return ;

// 	m_pSupport->mainWindow() ->statusBar() ->message( i18n( "Current file parsed %1 (cache emptied)" ).arg( m_activeFileName ), 1000 );

  emptyCache(); ///The cache has to be emptied, because the code-model changed. @todo Better: Only refresh the code-model(tell all code-model-types to refresh themselves on demand)

  computeRecoveryPointsLocked();
}

void CppCodeCompletion::setupCodeInformationRepository( ) {}

SimpleContext* CppCodeCompletion::computeContext( FunctionDefinitionAST * ast, int line, int col, int lineOffset, int colOffset ) {
  kdDebug( 9007 ) << "CppCodeCompletion::computeContext() -- main" << endl;

  SimpleContext* ctx = new SimpleContext();

  if ( ast && ast->initDeclarator() && ast->initDeclarator() ->declarator() ) {
    DeclaratorAST * d = ast->initDeclarator() ->declarator();
    if ( ParameterDeclarationClauseAST * clause = d->parameterDeclarationClause() ) {
      if ( ParameterDeclarationListAST * params = clause->parameterDeclarationList() ) {
        QPtrList<ParameterDeclarationAST> l( params->parameterList() );
        QPtrListIterator<ParameterDeclarationAST> it( l );
        while ( it.current() ) {
          ParameterDeclarationAST * param = it.current();
          ++it;

          SimpleVariable var;

          QStringList ptrList;
          QPtrList<AST> ptrOpList = param->declarator() ->ptrOpList();
          QPtrList<AST>::iterator it = ptrOpList.begin();
          for ( ; it != ptrOpList.end(); ++it ) {
            ptrList.append( ( *it ) ->text() );
          }

          var.ptrList = ptrList;
          var.type = param->typeSpec() ->text() + ptrList.join( "" );
          var.name = declaratorToString( param->declarator(), QString::null, true );
          var.comment = param->comment();
          param->getStartPosition( &var.startLine, &var.startCol );
          param->getEndPosition( &var.endLine, &var.endCol );

          if ( var.type ) {
            ctx->add
            ( var );
            //kdDebug(9007) << "add argument " << var.name << " with type " << var.type << endl;
          }
        }
      }
    }
  }


  if ( ast )
    computeContext( ctx, ast->functionBody(), line, col );

  if ( ctx ) {
    ctx->offset( lineOffset, colOffset );
  }

  return ctx;
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, StatementAST* stmt, int line, int col ) {
  if ( !stmt )
    return ;

  switch ( stmt->nodeType() ) {
    case NodeType_IfStatement:
    computeContext( ctx, static_cast<IfStatementAST*>( stmt ), line, col );
    break;
    case NodeType_WhileStatement:
    computeContext( ctx, static_cast<WhileStatementAST*>( stmt ), line, col );
    break;
    case NodeType_DoStatement:
    computeContext( ctx, static_cast<DoStatementAST*>( stmt ), line, col );
    break;
    case NodeType_ForStatement:
    computeContext( ctx, static_cast<ForStatementAST*>( stmt ), line, col );
    break;
    case NodeType_SwitchStatement:
    computeContext( ctx, static_cast<SwitchStatementAST*>( stmt ), line, col );
    break;
    case NodeType_TryBlockStatement:
    computeContext( ctx, static_cast<TryBlockStatementAST*>( stmt ), line, col );
    break;
    case NodeType_DeclarationStatement:
    computeContext( ctx, static_cast<DeclarationStatementAST*>( stmt ), line, col );
    break;
    case NodeType_StatementList:
    computeContext( ctx, static_cast<StatementListAST*>( stmt ), line, col );
    break;
    case NodeType_ExpressionStatement:
    break;
  }
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, StatementListAST* ast, int line, int col ) {
  if ( !inContextScope( ast, line, col, false, true ) )
    return ;

  QPtrList<StatementAST> l( ast->statementList() );
  QPtrListIterator<StatementAST> it( l );
  while ( it.current() ) {
    StatementAST * stmt = it.current();
    ++it;

    computeContext( ctx, stmt, line, col );
  }
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, IfStatementAST* ast, int line, int col ) {
  if ( !inContextScope( ast, line, col ) )
    return ;

  computeContext( ctx, ast->condition(), line, col );
  computeContext( ctx, ast->statement(), line, col );
  computeContext( ctx, ast->elseStatement(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, ForStatementAST* ast, int line, int col ) {
  if ( !inContextScope( ast, line, col ) )
    return ;

  computeContext( ctx, ast->initStatement(), line, col );
  computeContext( ctx, ast->condition(), line, col );
  computeContext( ctx, ast->statement(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, DoStatementAST* ast, int line, int col ) {
  if ( !inContextScope( ast, line, col ) )
    return ;

  //computeContext( ctx, ast->condition(), line, col );
  computeContext( ctx, ast->statement(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, WhileStatementAST* ast, int line, int col ) {
  if ( !inContextScope( ast, line, col ) )
    return ;

  computeContext( ctx, ast->condition(), line, col );
  computeContext( ctx, ast->statement(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, SwitchStatementAST* ast, int line, int col ) {
  if ( !inContextScope( ast, line, col ) )
    return ;

  computeContext( ctx, ast->condition(), line, col );
  computeContext( ctx, ast->statement(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, TryBlockStatementAST* ast, int line, int col ) {
  if ( !inContextScope( ast, line, col ) )
    return ;

  computeContext( ctx, ast->statement(), line, col );
  computeContext( ctx, ast->catchStatementList(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, CatchStatementListAST* ast, int line, int col ) {
  /*if ( !inContextScope( ast, line, col, false, true ) )
  	return;*/

  QPtrList<CatchStatementAST> l( ast->statementList() );
  QPtrListIterator<CatchStatementAST> it( l );
  while ( it.current() ) {
    CatchStatementAST * stmt = it.current();
    ++it;

    computeContext( ctx, stmt, line, col );
  }
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, CatchStatementAST* ast, int line, int col ) {
  if ( !ast->statement() )
    return ;
  if ( !inContextScope( ast->statement(), line, col ) )
    return ;

  computeContext( ctx, ast->condition(), line, col );
  computeContext( ctx, ast->statement(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, DeclarationStatementAST* ast, int line, int col ) {
	///@todo respect NodeType_Typedef
	if( ast->declaration() && ast->declaration() ->nodeType() == NodeType_UsingDirective ) {
	  UsingDirectiveAST* usingDecl = static_cast<UsingDirectiveAST*>( ast->declaration() );
		  QString name;
		if( usingDecl->name() ) {
			name = usingDecl->name()->text();

  		if( !name.isNull() )
	  		ctx->addImport( QPair<QString, QString>( "", name ) );
		}
	}

	if( ast->declaration() && ast->declaration() ->nodeType() == NodeType_NamespaceAlias ) {
	  NamespaceAliasAST* namespaceAlias = static_cast<NamespaceAliasAST*>( ast->declaration() );
		QString name;

		if( namespaceAlias ->namespaceName() && namespaceAlias->aliasName() ) {
			ctx->addImport( QPair<QString, QString>( namespaceAlias->namespaceName()->text(), namespaceAlias->aliasName()->text() ) );
		}
	}

  if ( !ast->declaration() || ast->declaration() ->nodeType() != NodeType_SimpleDeclaration )
    return ;

  if ( !inContextScope( ast, line, col, true, false ) )
    return ;

  SimpleDeclarationAST* simpleDecl = static_cast<SimpleDeclarationAST*>( ast->declaration() );
  TypeSpecifierAST* typeSpec = simpleDecl->typeSpec();

  InitDeclaratorListAST* initDeclListAST = simpleDecl->initDeclaratorList();
  if ( !initDeclListAST )
    return ;

  QPtrList<InitDeclaratorAST> l = initDeclListAST->initDeclaratorList();
  QPtrListIterator<InitDeclaratorAST> it( l );
  while ( it.current() ) {
    DeclaratorAST * d = it.current() ->declarator();
    ++it;

    if ( d->declaratorId() ) {
      SimpleVariable var;

      QStringList ptrList;
      QPtrList<AST> ptrOpList = d->ptrOpList();
      QPtrList<AST>::iterator it = ptrOpList.begin();
      for ( ; it != ptrOpList.end(); ++it ) {
        ptrList.append( ( *it ) ->text() );
      }

      for( int a = 0; a < d->arrayDimensionList().count(); a++ )
        ptrList.append("*");

      var.ptrList = ptrList;
      var.type = typeSpec->text() + ptrList.join( "" );
      var.name = toSimpleName( d->declaratorId() );
      var.comment = d->comment();
      d->getStartPosition( &var.startLine, &var.startCol );
      d->getEndPosition( &var.endLine, &var.endCol );

      ctx->add
      ( var );
      //kdDebug(9007) << "add variable " << var.name << " with type " << var.type << endl;
    }
  }
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, ConditionAST* ast, int line, int col ) {
  if ( !ast->typeSpec() || !ast->declarator() || !ast->declarator() ->declaratorId() )
    return ;

  if ( !inContextScope( ast, line, col, true, false ) )
    return ;

  SimpleVariable var;

  QStringList ptrList;
  QPtrList<AST> ptrOpList = ast->declarator() ->ptrOpList();
  QPtrList<AST>::iterator it = ptrOpList.begin();
  for ( ; it != ptrOpList.end(); ++it ) {
    ptrList.append( ( *it ) ->text() );
  }

  var.ptrList = ptrList;
  var.type = ast->typeSpec() ->text() + ptrList.join( "" );
  var.name = toSimpleName( ast->declarator() ->declaratorId() );
  var.comment = ast->comment();
  ast->getStartPosition( &var.startLine, &var.startCol );
  ast->getEndPosition( &var.endLine, &var.endCol );
  ctx->add
  ( var );
  //kdDebug(9007) << "add variable " << var.name << " with type " << var.type << endl;
}

bool CppCodeCompletion::inContextScope( AST* ast, int line, int col, bool checkStart, bool checkEnd ) {
  int startLine, startColumn;
  int endLine, endColumn;
  ast->getStartPosition( &startLine, &startColumn );
  ast->getEndPosition( &endLine, &endColumn );

  // 	kdDebug(9007) << k_funcinfo << endl;
  // 	kdDebug(9007) << "current char line: " << line << " col: " << col << endl;
  //
  // 	kdDebug(9007) << nodeTypeToString( ast->nodeType() )
  // 		<< " start line: " << startLine
  // 		<< " col: " << startColumn << endl;
  // 	kdDebug(9007) << nodeTypeToString( ast->nodeType() )
  // 		<< " end line: " << endLine
  // 		<< " col: " << endColumn << endl;

  bool start = line > startLine || ( line == startLine && col >= startColumn );
  bool end = line < endLine || ( line == endLine && col <= endColumn );

  if ( checkStart && checkEnd )
    return start && end;
  else if ( checkStart )
    return start;
  else if ( checkEnd )
    return end;

  return false;
}

FunctionDefinitionAST * CppCodeCompletion::functionDefinition( AST* node ) {

  while ( node ) {
    if ( node->nodeType() == NodeType_FunctionDefinition )
      return static_cast<FunctionDefinitionAST*>( node );
    node = node->parent();
  }
  return 0;
}

QString CppCodeCompletion::getText( int startLine, int startColumn, int endLine, int endColumn, int omitLine ) {
  if ( startLine == endLine ) {
    QString textLine = m_activeEditor->textLine( startLine );
    return textLine.mid( startColumn, endColumn - startColumn );
  }

  QStringList contents;

  for ( int line = startLine; line <= endLine; ++line ) {
    if ( line == omitLine )
      continue;

    QString textLine = m_activeEditor->textLine( line );

    if ( line == startLine )
      textLine = textLine.mid( startColumn );
    if ( line == endLine )
      textLine = textLine.left( endColumn );

    contents << textLine;
  }
  return contents.join( "\n" );
}

void CppCodeCompletion::computeRecoveryPointsLocked() {
  m_pSupport->backgroundParser() ->lock ()
  ;
  ParsedFilePointer unit = m_pSupport->backgroundParser() ->translationUnit( m_activeFileName );
  computeRecoveryPoints( unit );
  m_pSupport->backgroundParser() ->unlock();
}

void CppCodeCompletion::computeRecoveryPoints( ParsedFilePointer unit ) {
  if ( m_blockForKeyword )
    return ;

  kdDebug( 9007 ) << "CppCodeCompletion::computeRecoveryPoints" << endl;

  d->recoveryPoints.clear();
  if ( !unit )
    return ;

  ComputeRecoveryPoints walker( d->recoveryPoints );
  walker.parseTranslationUnit( *unit );
}

QString codeModelAccessToString( CodeModelItem::Access access ) {
  switch ( access ) {
    case CodeModelItem::Public:
    return "public";
    case CodeModelItem::Protected:
    return "protected";
    case CodeModelItem::Private:
    return "private";
    default:
    return "unknown";
  }
}

#define MAXCOMMENTCOLUMNS 45


QString CppCodeCompletion::commentFromItem( const SimpleType& parent, const ItemDom& item ) {
	--m_maxComments;
	static QString maxReached = " ";
	if( m_maxComments < 0 ) {
		return maxReached;
	}
  QString ret;
  int line, col;
  item->getStartPosition( &line, &col );


  if ( !parent->scope().isEmpty() ) {
    ret += "Container: " + parent->fullTypeResolvedWithScope();
  }

  if ( item->isEnum() ) {
    ret += "\nKind: Enum";
    ret += "\nValues:";
    const EnumModel* en = dynamic_cast<const EnumModel*>( item.data() );
    if ( en ) {
      EnumeratorList values = en->enumeratorList();
      for ( EnumeratorList::iterator it = values.begin(); it != values.end(); ++it ) {
        ret += "\n  " + ( *it ) ->name();
        if ( !( *it ) ->value().isEmpty() ) {
          ret + " = " + ( *it ) ->value();
        }
      }

      ret += "\n\nAccess: " + codeModelAccessToString( ( CodeModelItem::Access ) en->access() );
    } else {}

  }

  if ( item->isFunction() || item->isFunctionDefinition() ) {
    const FunctionModel * f = dynamic_cast<const FunctionModel*>( item.data() );
    ret += "\nKind: Function";
    if ( f ) {
      QString state;
      if ( f->isStatic() )
        state += "static ";
      if ( f->isVirtual() )
        state += "virtual ";
      if ( f->isAbstract() )
        state += "abstract ";
      //if( f->isTemplateable() ) state += "template ";
      if ( f->isConstant() )
        state += "const ";
      if ( f->isSlot() )
        state += "slot ";
      if ( f->isSignal() )
        state += "signal ";

      if ( !state.isEmpty() )
        ret += "\nModifiers: " + state;

      ret += "\nAccess: " + codeModelAccessToString( ( CodeModelItem::Access ) f->access() );
    }
  }

  if ( item->isEnumerator() ) {
    const EnumeratorModel * f = dynamic_cast<const EnumeratorModel*>( item.data() );
    ret += "\nKind: Enumerator";
    if ( f ) {
      if ( !f->value().isEmpty() )
        ret += "\nValue: " + f->value();

      //ret += "\nAccess: " + codeModelAccessToString( f->() );
    }
  } else {
    if ( item->isVariable() ) {
      const VariableModel * f = dynamic_cast<const VariableModel*>( item.data() );
      if ( f ) {
        if ( !f->isEnumeratorVariable() ) {
          ret += "\nKind: Variable";
          if ( f->isStatic() )
            ret += "\nModifiers: static";
        } else {
          ret += "\nKind: Enumerator";
          ret += "\nEnum: " + f->type();
        }

        ret += "\nAccess: " + codeModelAccessToString( ( CodeModelItem::Access ) f->access() );
      }
    }
  }

  if ( item->isTypeAlias() ) {
    const TypeAliasModel * t = dynamic_cast<const TypeAliasModel*>( item.data() );
    ret += "\nKind: Typedef";
    if ( t ) {
      ret += "\nType: " + t->type();
      LocateResult r = parent->locateDecType( t->type() );
      if ( r.desc().resolved() )
        ret += "\nResolved type: " + r.desc().resolved() ->fullTypeResolvedWithScope();
      else
        ret += "\nPartially resolved type: " + r.desc().fullNameChain();
    }
  }

  if ( item->isClass() ) {
    ret += "\nKind: Class";
  }

  ret += QString( "\nFile: %1\nLine: %2 Column: %3" ).arg( prepareTextForMenu( item->fileName(), 3, MAXCOMMENTCOLUMNS ).join( "\n" ) ).arg( line ).arg( col );
  if ( !item->comment().isEmpty() )
    ret += "\n\n" + prepareTextForMenu( item->comment(), 3, MAXCOMMENTCOLUMNS ).join( "\n" );
  return ret;
}

QString CppCodeCompletion::commentFromTag( const SimpleType& parent, Tag& tag ) {
	--m_maxComments;
	static QString maxReached = " ";
	if( m_maxComments < 0 ) {
		return maxReached;
	}

  int line, col;
  tag.getStartPosition( &line, &col );
  QString ret; // = tag.comment();

  if ( !parent->scope().isEmpty() ) {
    ret += "Container: " + parent->fullTypeResolvedWithScope();
  }
  /*
  if( tag.kind() == Tag::Kind_Enum ) {
  ret += "\nKind: Enum";
  ret += "\nValues:";
  	EnumModel* en = dynamic_cast<EnumModel*>( item.data() );
  	if( en ) {
  		EnumeratorList values =en->enumeratorList();
  		for( EnumeratorList::iterator it = values.begin(); it != values.end(); ++it )
  		{
  			ret += "\n  " + (*it)->name();
  			if( !(*it)->value().isEmpty() ) {
  				ret + " = " + (*it)->value();
  			}
  	}

  	ret += "\n\nAccess: " + codeModelAccessToString( (CodeModelItem::Access)en->access() );
  	} else {
  	}
  }*/

  if ( tag.kind() == Tag::Kind_Function || tag.kind() == Tag::Kind_FunctionDeclaration ) {
    CppFunction<Tag> function( tag );

    ret += "\nKind: Function";

    QString state;
    if ( function.isStatic() )
      state += "static ";
    if ( function.isVirtual() )
      state += "virtual ";
    //if( function.isVolatile() ) state += "volatile ";
    if ( function.isConst() )
      state += "const ";
    if ( function.isSlot() )
      state += "slot ";
    if ( function.isSignal() )
      state += "signal ";
    if ( !state.isEmpty() )
      ret += "\nModifiers: " + state;

    ret += "\nAccess: " + TagUtils::accessToString( function.access() );
  }

  /*if( item->isEnumerator() ) {
  	EnumeratorModel* f = dynamic_cast<EnumeratorModel*>( item.data() );
  ret += "\nKind: Enumerator";
  	if( f ) {
  		if( !f->value().isEmpty() )
  			ret += "\nValue: " + f->value();

  		//ret += "\nAccess: " + codeModelAccessToString( f->() );
  	}
  } else {
  	if( item->isVariable() ) {
  		VariableModel* f = dynamic_cast<VariableModel*>( item.data() );
  	ret += "\nKind: Variable";
  		if( f ) {
  		ret += "\nAccess: " + codeModelAccessToString( (CodeModelItem::Access)f->access() );
  		}
  	}
  }*/

  if ( tag.kind() == Tag::Kind_Enum ) {
    CppVariable<Tag> var( tag );

    ret += "\nKind: Enum";
  }

  if ( tag.kind() == Tag::Kind_Enumerator ) {
    CppVariable<Tag> var( tag );

    ret += "\nKind: Enumerator";
    if ( tag.hasAttribute( "enum" ) && tag.attribute( "enum" ).asString() != "int" )
      ret += "\nEnum: " + tag.attribute( "enum" ).asString();
  }

  if ( tag.kind() == Tag::Kind_Variable ) {
    CppVariable<Tag> var( tag );

    ret += "\nKind: Variable";
    if ( var.isStatic() )
      ret += "\nModifiers: static";
    ret += "\nAccess: " + TagUtils::accessToString( var.access() );
  }

  if ( tag.kind() == Tag::Kind_Typedef ) {
    ret += "\nKind: Typedef";
    ret += "\nType: " + tagType( tag );
    LocateResult r = parent->locateDecType( tagType( tag ) );
    if ( r.desc().resolved() )
      ret += "\nResolved type: " + r.desc().resolved() ->fullTypeResolvedWithScope();
    else
      ret += "\nPartially resolved type: " + r.desc().fullNameChain();
  }

  if ( tag.kind() == Tag::Kind_Class ) {
    ret += "\nKind: Class";
  }
  if ( tag.kind() == Tag::Kind_Struct ) {
    ret += "\nKind: Struct";
  }

  ret += QString( "\nFile: %1\nLine: %2 Column: %3" ).arg( prepareTextForMenu( tag.fileName(), 3, MAXCOMMENTCOLUMNS ).join( "\n" ) ).arg( line ).arg( col );
  if ( !tag.comment().isEmpty() ) {
    ret += "\n\n" + prepareTextForMenu( tag.comment(), 20, MAXCOMMENTCOLUMNS ).join( "\n" );
  }
  return ret;
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType typeR, QValueList<CodeCompletionEntry>& entryList, const QStringList& type, SimpleTypeNamespace* ns, std::set<HashedString>& ignore, bool isInstance, int depth ) {
	 HashedString myName = HashedString( ns->scope().join("::") +"%"+typeid(*ns).name() );
  if ( ignore.find( myName ) != ignore.end() )
    return ;
  ignore.insert( myName );
	SimpleTypeNamespace::SlaveList slaves = ns->getSlaves( getIncludeFiles() );
  for ( SimpleTypeNamespace::SlaveList::iterator it = slaves.begin(); it != slaves.end(); ++it ) {
    SimpleTypeNamespace* nns = dynamic_cast<SimpleTypeNamespace*>( (*it).first.first.resolved().data() );
	  if ( !nns ) {
	    if( ( *it ).first.first.resolved() ) computeCompletionEntryList( SimpleType((*it).first.first.resolved()), entryList, ( *it ).first.first.resolved()->scope(), isInstance, depth );
	  } else {
	    if( ( *it ).first.first.resolved() ) computeCompletionEntryList( SimpleType(( *it ).first.first.resolved()), entryList, ( *it ).first.first.resolved()->scope(), nns, ignore, isInstance, depth );
	  }
  }
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType typeR, QValueList< CodeCompletionEntry > & entryList, const QStringList & type, bool isInstance, int depth ) {
  dbgState.setState( disableVerboseForCompletionList );

  Debug d( "#cel#" );
  if ( !safetyCounter || !d )
    return ;
  SimpleTypeImpl* m = &( *typeR ) ;

  if ( SimpleTypeNamespace * ns = dynamic_cast<SimpleTypeNamespace*>( m ) ) {
    std::set<HashedString> ignore;
    computeCompletionEntryList( typeR, entryList, type, ns, ignore, isInstance, depth );
  } else if ( dynamic_cast<SimpleTypeCodeModel*>( m ) ) {
    ItemDom item = ( dynamic_cast<SimpleTypeCodeModel*>( m ) ) ->item();
    if ( item )
      if ( ClassModel * mod = dynamic_cast<ClassModel*> ( &( *item ) ) )
        computeCompletionEntryList( typeR, entryList, ClassDom( mod ) , isInstance, depth );
  } else {
    QValueList<Catalog::QueryArgument> args;
    QValueList<Tag> tags;

	  QStringList ts = type;
	  if( !ts.isEmpty() ) {
		  QString s = ts.back() + typeR->specialization();
		  ts.pop_back();
		  ts.push_back( s );
	  }

    args.clear();
    args << Catalog::QueryArgument( "kind", Tag::Kind_FunctionDeclaration )
    << Catalog::QueryArgument( "scope", ts );
    tags = m_repository->query( args );
    computeCompletionEntryList( typeR, entryList, tags, isInstance, depth );

    args.clear();
    args << Catalog::QueryArgument( "kind", Tag::Kind_Variable )
		  << Catalog::QueryArgument( "scope", ts );
    tags = m_repository->query( args );
    computeCompletionEntryList( typeR, entryList, tags, isInstance, depth );

    if ( !isInstance ) {
      args.clear();
      args << Catalog::QueryArgument( "kind", Tag::Kind_Enumerator )
		    << Catalog::QueryArgument( "scope", ts );
      tags = m_repository->query( args );
      computeCompletionEntryList( typeR, entryList, tags, isInstance, depth );

      args.clear();
      args << Catalog::QueryArgument( "kind", Tag::Kind_Enum )
		    << Catalog::QueryArgument( "scope", ts );
      tags = m_repository->query( args );
      computeCompletionEntryList( typeR, entryList, tags, isInstance, depth );

      args.clear();
      args << Catalog::QueryArgument( "kind", Tag::Kind_Typedef )
		    << Catalog::QueryArgument( "scope", ts );
      tags = m_repository->query( args );
      computeCompletionEntryList( typeR, entryList, tags, isInstance, depth );

      args.clear();
      args << Catalog::QueryArgument( "kind", Tag::Kind_Class )
		    << Catalog::QueryArgument( "scope", ts );
      tags = m_repository->query( args );
      computeCompletionEntryList( typeR, entryList, tags, isInstance, depth );

      args.clear();
      args << Catalog::QueryArgument( "kind", Tag::Kind_Struct )
		    << Catalog::QueryArgument( "scope", ts );
      tags = m_repository->query( args );
      computeCompletionEntryList( typeR, entryList, tags, isInstance, depth );
    }

    args.clear();
    args << Catalog::QueryArgument( "kind", Tag::Kind_Base_class );
    QString fullname = type.join( "::" )+typeR->specialization();
    /*    	if( fullname.length() >=2 )
                args << Catalog::QueryArgument( "prefix", fullname.left(2) );*/
    args << Catalog::QueryArgument( "name", fullname );


	  QValueList<LocateResult> parents = typeR->getBases( );
    for ( QValueList<LocateResult>::Iterator it = parents.begin(); it != parents.end(); ++it ) {
      if ( !( *it ) ->resolved() )
        continue;
      SimpleType tp = SimpleType( ( *it ) ->resolved() );
      if ( tp )
        computeCompletionEntryList( tp, entryList, tp.scope(), isInstance, depth + 1 );
    }
  }
  dbgState.setState( true );
}


void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, QValueList< Tag > & tags, bool isInstance, int depth ) {
  Debug d( "#cel#" );
  if ( !safetyCounter || !d )
    return ;
  QString className = type->desc().name();

  bool isNs = type->isNamespace();

  CompTypeProcessor proc( type, m_pSupport->codeCompletionConfig() ->processFunctionArguments() && type->usingTemplates() );
  bool resolve = m_pSupport->codeCompletionConfig() ->processPrimaryTypes() && type->usingTemplates();

  QValueList<Tag>::Iterator it = tags.begin();
  while ( it != tags.end() ) {
    Tag & tag = *it;
    ++it;

    int subSorting = 0;

    if ( tag.name().isEmpty() ) {
      continue;
    } else if ( m_completionMode != NormalCompletion ) {
      if ( tag.kind() != Tag::Kind_FunctionDeclaration )
        continue;
    }

    if ( tag.kind() == Tag::Kind_Function || tag.kind() == Tag::Kind_FunctionDeclaration ) {
      CppFunction<Tag> info( tag );

      if ( m_completionMode == SlotCompletion && !info.isSlot() )
        continue;
      else if ( m_completionMode == SignalCompletion && !info.isSignal() )
        continue;
      else if ( m_completionMode == VirtualDeclCompletion && !info.isVirtual() )
        continue;

      if ( info.isConst() )
        subSorting = 1;
      if ( info.isSlot() )
        subSorting = 2;
      if ( info.isSignal() )
        subSorting = 3;
      if ( info.isVirtual() )
        subSorting = 4;
      if ( info.isStatic() )
        subSorting = 5;
    }

    CodeCompletionEntry e = CodeInformationRepository::toEntry( tag, m_completionMode, &proc );

    TagFlags fl;
    fl.flags = tag.flags();
    int num = fl.data.access;

    QString str = "public";
    if ( num != 0 ) {
      str = TagUtils::accessToString( num );
    } else {
      num = 0;
    }
    // 0 = protected, 1 = public, 2 = private

    if ( str == "public" )
      num = 0;
    else if ( str == "protected" )
      num = 1;
    else if ( str == "private" )
      num = 2;

    int sortPosition = 0;

    switch ( tag.kind() ) {
      case Tag::Kind_Enum:
      sortPosition = 3;
      if ( isInstance )
        continue;
      break;
      case Tag::Kind_Enumerator:
      sortPosition = 4;
      if ( isInstance )
        continue;
      break;
      case Tag::Kind_Struct:
      case Tag::Kind_Union:
      case Tag::Kind_Class:
      sortPosition = 5;
      if ( isInstance )
        continue;
      break;
      case Tag::Kind_VariableDeclaration:
      case Tag::Kind_Variable:
      sortPosition = 2;
      if ( !isInstance && !CppVariable<Tag>( tag ).isStatic() && !isNs )
        continue;
      break;
      case Tag::Kind_FunctionDeclaration:
      case Tag::Kind_Function:
      sortPosition = 1;
      if ( !isInstance && !CppFunction<Tag>( tag ).isStatic() && !isNs )
        continue;
      break;
      case Tag::Kind_Typedef:
      sortPosition = 6;
      if ( isInstance )
        continue;
      break;
    }

    e.userdata = QString( "%1%2%3%4%5" ).arg( num ).arg( depth ).arg( className ).arg( sortPosition ).arg( subSorting );

    if ( m_completionMode != SignalCompletion ) {
      if ( !type->isNamespace() ) {
        if ( num == 1 )
          e.postfix += ";   (protected)"; // in " + proc.parentType() + ")";
        if ( num == 2 )
          e.postfix += ";   (private)"; // in " + proc.parentType() + ")";
      }
    }


    QString prefix = tagType( tag ).stripWhiteSpace();

    if ( tag.kind() == Tag::Kind_Enumerator && tag.hasAttribute( "enum" ) ) {
      prefix = tag.attribute( "enum" ).asString();
      e.userdata += prefix; ///Sort enumerators together
    } else if ( tag.kind() == Tag::Kind_Enum ) {
      prefix = "enum";
    } else {

      if ( tag.kind() == Tag::Kind_FunctionDeclaration || tag.kind() == Tag::Kind_Function || tag.kind() == Tag::Kind_Variable || tag.kind() == Tag::Kind_Typedef ) {
        if ( !prefix.isEmpty() && resolve ) {
          LocateResult et = type->locateDecType( prefix );

          if ( et )
            prefix = et->fullNameChain();
        }
      }

      if ( tag.kind() == Tag::Kind_FunctionDeclaration || tag.kind() == Tag::Kind_Function ) {
        if ( prefix.isEmpty() ) {
          if ( tag.name() == className )
            prefix = constructorPrefix;
          else if ( tag.name().startsWith( "~" ) )
            prefix = destructorPrefix;
        }
      }

      if ( tag.kind() == Tag::Kind_Class || tag.kind() == Tag::Kind_Function )
        prefix = "";
    }

    e.comment = commentFromTag( type, tag );

    if ( e.prefix.isEmpty() )
      e.prefix = prefix;
    else
      e.prefix += " " + prefix;

    e.prefix = e.prefix.stripWhiteSpace();
    e.prefix = stringMult( depth, "  " ) + e.prefix.stripWhiteSpace();

    e.text = e.text.stripWhiteSpace();

    if ( str != "private" )
      entryList << e;
  }
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, ClassDom klass, bool isInstance, int depth ) {
  Debug d( "#cel#" );
  if ( !safetyCounter || !d )
    return ;

  computeCompletionEntryList( type, entryList, klass->functionList(), isInstance, depth );

  ///Find all function-definitions that have no functions. Those may be inlined functions and need to be treated too.
  FunctionDefinitionList definitions = klass->functionDefinitionList();
  FunctionList l;

	QStringList classScope = klass->scope();
	classScope << klass->name();

  for ( FunctionDefinitionList::iterator it = definitions.begin(); it != definitions.end(); ++it ) {
    FunctionList fl = klass->functionByName( ( *it ) ->name() );

    ArgumentList args = ( *it ) ->argumentList();

    if ( !l.isEmpty() ) {
      bool matched = false;
      for ( FunctionList::iterator it = fl.begin(); it != fl.end(); ++it ) {
        ArgumentList fArgs = ( *it ) ->argumentList();
        if ( fArgs.count() != args.count() )
          continue;
        ArgumentList::iterator it = args.begin();
        ArgumentList::iterator it2 = fArgs.begin();
        bool hit = true;
        while ( it != args.end() ) {
          if ( ( *it ) ->type() != ( *it2 ) ->type() ) {
            hit = false;
            break;
          }
          ++it;
          ++it2;
        }
        if ( hit ) {
          matched = true;
          break;
        }

      }

      if ( matched )
        continue;
    }

	  ///The function-definition belongs to some sub-class
	  if( (*it)->scope() != classScope && !(*it)->scope().isEmpty() ) continue;
    l << ( FunctionModel* ) ( *it ).data();
  }

  if ( !l.isEmpty() )
    computeCompletionEntryList( type, entryList, l, isInstance, depth );

  if ( m_completionMode == NormalCompletion )
    computeCompletionEntryList( type, entryList, klass->variableList(), isInstance, depth );

  if ( !isInstance ) {
    computeCompletionEntryList( klass->name(), type, entryList, klass->classList(), isInstance, depth );
    computeCompletionEntryList( klass->name(), type, entryList, klass->typeAliasList(), isInstance, depth );
  }

	QValueList<LocateResult> parents = type->getBases( );

	for ( QValueList<LocateResult>::Iterator it = parents.begin(); it != parents.end(); ++it ) {
    if ( !( *it ) ->resolved() )
      continue;

    SimpleTypeImpl* i = ( *it ) ->resolved();
		computeCompletionEntryList( i, entryList, i->scope(), isInstance, depth + 1 );
		/*
    SimpleTypeCodeModel* m = dynamic_cast<SimpleTypeCodeModel*> ( i );
    if ( m ) {
      ItemDom item = m->item();
      ClassModel* kl = dynamic_cast<ClassModel*> ( &( *item ) );
      if ( kl ) {
        computeCompletionEntryList( SimpleType( ( *it ) ->resolved() ), entryList, ClassDom ( kl ), isInstance, depth + 1 );
      }
    }*/
  }
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, NamespaceDom scope, bool isInstance, int depth ) {
  Debug d( "#cel#" );
  if ( !safetyCounter || !d )
    return ;

  CppCodeCompletionConfig * cfg = m_pSupport->codeCompletionConfig();

  computeCompletionEntryList( type, entryList, ClassDom( scope.data() ), isInstance, depth );
  if ( !isInstance )
    computeCompletionEntryList( type, entryList, scope->namespaceList(), isInstance, depth );
}

void CppCodeCompletion::computeCompletionEntryList( QString parent, SimpleType type, QValueList< CodeCompletionEntry > & entryList, const ClassList & lst, bool isInstance, int depth ) {
  Debug d( "#cel#" );
  if ( !safetyCounter || !d )
    return ;


  ClassList::ConstIterator it = lst.begin();
  while ( it != lst.end() ) {
    ClassDom klass = *it;
    ++it;

    CodeCompletionEntry entry;
    entry.prefix = "class";
    entry.prefix = stringMult( depth, "  " ) + entry.prefix.stripWhiteSpace();
    entry.text = klass->name();
    entry.comment = commentFromItem( type, klass.data() );
    if ( isInstance )
      continue;

    entry.userdata = QString( "%1%2%3%4%5" ).arg( CodeModelItem::Public ).arg( depth ).arg( parent ).arg( 6 );

    entryList << entry;


    // 		if ( cfg->includeTypes() )
    /*{
    	computeCompletionEntryList( type, entryList, klass->classList(), isInstance, depth );
    }*/
  }
}

void CppCodeCompletion::computeCompletionEntryList( QString parent, SimpleType type, QValueList< CodeCompletionEntry > & entryList, const TypeAliasList & lst, bool isInstance, int depth ) {
  Debug d( "#cel#" );
  if ( !safetyCounter || !d )
    return ;


  TypeAliasList::ConstIterator it = lst.begin();
  while ( it != lst.end() ) {
    TypeAliasDom klass = *it;
    ++it;

    CodeCompletionEntry entry;

    LocateResult et = type->locateDecType( klass->type() );
    if ( et )
      entry.prefix = "typedef " + et->fullNameChain();
    else
      entry.prefix = "typedef " + klass->type();

    entry.prefix = stringMult( depth, "  " ) + entry.prefix.stripWhiteSpace();
    entry.text = klass->name();
    entry.comment = commentFromItem( type, klass.data() );
    entry.userdata = QString( "%1%2%3%4%5" ).arg( CodeModelItem::Public ).arg( depth ).arg( parent ).arg( 5 );
    entryList << entry;
  }
}
void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, const NamespaceList & lst, bool /*isInstance*/, int depth ) {
  Debug d( "#cel#" );
  if ( !safetyCounter || !d )
    return ;

  NamespaceList::ConstIterator it = lst.begin();
  while ( it != lst.end() ) {
    NamespaceDom scope = *it;
    ++it;

    CodeCompletionEntry entry;
    entry.prefix = "namespace";
    entry.prefix = stringMult( depth, "  " ) + entry.prefix.stripWhiteSpace();
    entry.text = scope->name();
    entry.comment = commentFromItem( type, scope.data() );
    entryList << entry;
  }
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, const FunctionList & methods, bool isInstance, int depth ) {
  Debug d( "#cel#" );
  if ( !safetyCounter || !d )
    return ;
  QString className = type->desc().name();
  bool isNs = type->isNamespace();

  bool resolve = type->usingTemplates() && m_pSupport->codeCompletionConfig() ->processPrimaryTypes();

  CompTypeProcessor proc( type, m_pSupport->codeCompletionConfig() ->processFunctionArguments() && type->usingTemplates() );

  FunctionList::ConstIterator it = methods.begin();
  while ( it != methods.end() ) {
    FunctionDom meth = *it;
    ++it;

    if ( isInstance && meth->isStatic() )
      continue;
    else if ( m_completionMode == SignalCompletion && !meth->isSignal() )
      continue;
    else if ( m_completionMode == SlotCompletion && !meth->isSlot() )
      continue;
    else if ( m_completionMode == VirtualDeclCompletion && !meth->isVirtual() )
      continue;

    if ( !isInstance && !meth->isStatic() && !isNs )
      continue;

    CodeCompletionEntry entry;

    entry.comment = commentFromItem( type, model_cast<ItemDom>( meth ) );

    if ( ! resolve ) {
      entry.prefix = meth->resultType();
    } else {
      QString tt = meth->resultType();
      LocateResult t = type->locateDecType( tt );
      if ( t ) {
        entry.prefix = t->fullNameChain();
      } else
        entry.prefix = meth->resultType();
    }

    if ( entry.prefix.isEmpty() && meth->name() == className )
      entry.prefix = constructorPrefix;
    if ( entry.prefix.isEmpty() && meth->name().startsWith( "~" ) )
      entry.prefix = destructorPrefix;

    entry.prefix = stringMult( depth, "  " ) + entry.prefix.stripWhiteSpace();
    QString text;

    ArgumentList args = meth->argumentList();
    ArgumentList::Iterator argIt = args.begin();
    /*
    if ( m_completionMode == VirtualDeclCompletion )
    {
    	//Ideally the type info would be a entry.prefix, but we need them to be
    	//inserted upon completion so they have to be part of entry.text
    	entry.text = meth->resultType();
    	entry.text += " ";
    	entry.text += meth->name();
    }
    else*/
    entry.text = meth->name();

    entry.text += formattedOpeningParenthesis( args.size() == 0 );

    while ( argIt != args.end() ) {
      ArgumentDom arg = *argIt;
      ++argIt;

      text += proc.processType( arg->type() );
      if ( m_completionMode == NormalCompletion ||
           m_completionMode == VirtualDeclCompletion )
        text += QString( " " ) + arg->name();

      if ( argIt != args.end() )
        text += ", ";
    }

    if ( args.size() == 0 ) {
      entry.text += formattedClosingParenthesis( true );
    } else {
      text += formattedClosingParenthesis( false );
    }

    int subSorting = 0;
    if ( meth->isConstant() )
      subSorting = 1;
    if ( meth->isSlot() )
      subSorting = 2;
    if ( meth->isSignal() )
      subSorting = 3;
    if ( meth->isVirtual() )
      subSorting = 4;
    if ( meth->isStatic() )
      subSorting = 5;

    entry.userdata += QString( "%1%2%3%4%5" ).arg( meth->access() ).arg( depth ).arg( className ).arg( 1 ).arg( subSorting );

    if ( m_completionMode == VirtualDeclCompletion )
      entry.text += text + ";";
    if ( m_completionMode != NormalCompletion )
      entry.text += text;
    else
      entry.postfix = text;

    if ( meth->isConstant() )
      entry.postfix += " const";
    if ( m_completionMode != SignalCompletion ) {
      if ( !type->isNamespace() ) {
        if ( meth->access() == CodeModelItem::Protected )
          entry.postfix += "; (protected)"; // in " + type->fullType() + ")";
        if ( meth->access() == CodeModelItem::Private )
          entry.postfix += "; (private)"; // in " + type->fullType() + ")";
      }
    }

    entry.text = entry.text.stripWhiteSpace();

    entryList << entry;
  }
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, const VariableList & attributes, bool isInstance, int depth ) {
  Debug d( "#cel#" );
  QString className = type->desc().name();
  bool isNs = type->isNamespace();

  if ( !safetyCounter || !d )
    return ;

  if ( m_completionMode != NormalCompletion )
    return ;
  bool resolve = type->usingTemplates() && m_pSupport->codeCompletionConfig() ->processPrimaryTypes();

  VariableList::ConstIterator it = attributes.begin();
  while ( it != attributes.end() ) {
    VariableDom attr = *it;
    ++it;

    if ( isInstance && attr->isStatic() )
      continue;
    if ( !isInstance && !attr->isStatic() && !isNs )
      continue;

    CodeCompletionEntry entry;
    entry.text = attr->name();
    entry.comment = commentFromItem( type, model_cast<ItemDom>( attr ) );
    entry.userdata += QString( "%1%2%3%4" ).arg( attr->access() ).arg( depth ).arg( className ).arg( 2 );


    if ( !attr->isEnumeratorVariable() ) {
      if ( ! resolve ) {
        entry.prefix = attr->type();
      } else {
        QString tt = attr->type();
        LocateResult t = type->locateDecType( tt );
        //SimpleType t = type->typeOf( attr->name() );
        if ( t )
          entry.prefix = t->fullNameChain();
        else
          entry.prefix = attr->type();
      }
    } else {
      entry.prefix = attr->type();
      entry.userdata += attr->type(); ///Sort enumerators by their enum
    }
    if ( attr->access() == CodeModelItem::Protected )
      entry.postfix += "; (protected)"; // in " + type->fullType() + ")";
    if ( attr->access() == CodeModelItem::Private )
      entry.postfix += "; (private)"; // in " + type->fullType() + ")";

    entry.prefix = stringMult( depth, "  " ) + entry.prefix.stripWhiteSpace();

    entryList << entry;
  }
}

void CppCodeCompletion::computeCompletionEntryList( QValueList< CodeCompletionEntry > & entryList, SimpleContext * ctx, bool /*isInstance*/, int depth ) {
  Debug d( "#cel#" );
  if ( !safetyCounter || !d )
    return ;

  while ( ctx ) {
    QValueList<SimpleVariable> vars = ctx->vars();
    QValueList<SimpleVariable>::ConstIterator it = vars.begin();
    while ( it != vars.end() ) {
      const SimpleVariable & var = *it;
      ++it;

      CodeCompletionEntry entry;
	    entry.prefix = var.type.fullNameChain();
      entry.text = var.name;
      entry.userdata = "000";
	    entry.comment = "Local variable";
      entryList << entry;

    }
    ctx = ctx->prev();
  }

}

EvaluationResult CppCodeCompletion::evaluateExpression( ExpressionInfo expr, SimpleContext* ctx ) {
  safetyCounter.init();

	//d->classNameList = typeNameList( m_pSupport->codeModel() );

	CppEvaluation::ExpressionEvaluation obj( this, expr, AllOperators, getIncludeFiles(), ctx );

  EvaluationResult res;
  res = obj.evaluate();

	QString resolutionType = "(resolved)";
	if( !res->resolved() ) {
		if( BuiltinTypes::isBuiltin( res.resultType ) ) {
			resolutionType = "(builtin " + BuiltinTypes::comment( res.resultType ) +  ")";
		} else {
			resolutionType = "(unresolved)";
		}
	}

  addStatusText( i18n( "Type of \"%1\" is \"%2\", %3" ).arg( expr.expr() ).arg( res->fullNameChain() ).arg( resolutionType ), 5000 );

  return res;
}

void CppCodeCompletion::computeFileEntryList( ) {
  m_fileEntryList.clear();

  QStringList fileList = m_pSupport->project() ->allFiles();
  for ( QStringList::Iterator it = fileList.begin(); it != fileList.end(); ++it ) {
    if ( !m_pSupport->isHeader( *it ) )
      continue;

    CodeCompletionEntry entry;
    entry.text = QFileInfo( *it ).fileName();
	  
    m_fileEntryList.push_back( entry );
  }

  m_fileEntryList = unique( m_fileEntryList );
}

HashedStringSet CppCodeCompletion::getIncludeFiles( const QString& fi ) {
	QString file = fi;
	if( file.isEmpty() )
		file = m_activeFileName;

	FileDom f = m_pSupport->codeModel() ->fileByName( file );
	if( f ) {
		ParseResultPointer p = f->parseResult();
		if( p ) {
			ParsedFilePointer pp = dynamic_cast<ParsedFile*>( p.data() );
			if( pp ) {
				return pp->includeFiles();
			}
		}
	}
	return HashedStringSet();
}

void CppCodeCompletion::slotJumpToDeclCursorContext()
{
	kdDebug(9007) << k_funcinfo << endl;
	jumpCursorContext( Declaration );
}

void CppCodeCompletion::slotJumpToDefCursorContext()
{
	kdDebug(9007) << k_funcinfo << endl;
	jumpCursorContext( Definition );
}

void CppCodeCompletion::jumpCursorContext( FunctionType f )
{
	if ( !m_activeCursor ) return;
	
	SimpleTypeConfiguration conf( m_activeFileName );
	
	unsigned int line;
	unsigned int column;
	m_activeCursor->cursorPositionReal( &line, &column );
	
	EvaluationResult result = evaluateExpressionAt( line, column, conf );
	
	// Determine the declaration info based on the type of item we are dealing with.
	DeclarationInfo d;
	
	QString includeFileName, includeFilePath;
	bool unused;
	
	if ( result.isMacro ) {
		d.name = result.macro.name();
		d.file = result.macro.fileName();
		d.startLine = d.endLine = result.macro.line();
		d.startCol = d.endCol = result.macro.column();
	} else if ( getIncludeInfo( line, includeFileName, includeFilePath, unused ) ) {
		d.name = includeFileName;
		d.file = includeFilePath;
	} else {
		d = result.sourceVariable;
	}
	if ( !d ) {
		LocateResult type = result.resultType;
		if ( type && type->resolved() ) {
			// Is it a namespace?
			if ( type->resolved()->isNamespace() ) {
				SimpleTypeCachedNamespace * ns = dynamic_cast<SimpleTypeCachedNamespace*>( type->resolved().data() );
				if ( ns ) {
					SimpleTypeNamespace::SlaveList slaves = ns->getSlaves( getIncludeFiles() );
					if ( slaves.begin() != slaves.end() ) {
						SimpleTypeCachedCodeModel * item = dynamic_cast<SimpleTypeCachedCodeModel*>( ( *slaves.begin() ).first.first.resolved().data() );
						if ( item && item->item() && item->item()->isNamespace() ) {
							NamespaceModel* ns = dynamic_cast<NamespaceModel*>( item->item().data() );
							QStringList wholeScope = ns->scope();
							wholeScope << ns->name();
							FileList files = cppSupport()->codeModel()->fileList();
							for ( FileList::iterator it = files.begin(); it != files.end(); ++it ) {
								NamespaceModel* ns = (*it).data();
								for ( QStringList::iterator it2 = wholeScope.begin(); it2 != wholeScope.end(); ++it2 ) {
									if ( ns->hasNamespace( (*it2) ) ) {
										ns = ns->namespaceByName( *it2 );
										if ( !ns ) break;
									} else {
										ns = 0;
										break;
									}
								}
								if ( ns ) {
									d.name = ns->name();
									ns->getStartPosition( &d.startLine, &d.startCol );
									ns->getEndPosition( &d.endLine, &d.endCol );
									d.file = ns->fileName();
									break;
								}
							}
						}
					}
				}
			} else {
				// Not a namespace, we can get the declaration info straight from the type description.
				d = type->resolved()->getDeclarationInfo();
			}
		}
		// Unresolved, maybe its a named enumeration?
		else if ( type && type.trace() ) {
			QValueList< QPair<SimpleTypeImpl::MemberInfo, TypeDesc> > trace = type.trace()->trace();
			if ( !trace.isEmpty() ) {
				if ( trace.begin() != trace.end() ) {
					d = ( *trace.begin() ).first.decl;
				}
			}
		}
	}
	if ( d ) {
		QString fileName = d.file == "current_file" ? m_activeFileName : d.file.operator QString();
		if ( f == Definition && cppSupport()->switchHeaderImpl( fileName, d.startLine, d.startCol ) )
			return;
		cppSupport()->partController()->editDocument( fileName, d.startLine );
	}
}

QString CppCodeCompletion::createTypeInfoString( int line, int column ) 
{
	QString typeInfoString;
	
	SimpleTypeConfiguration conf( m_activeFileName );
	EvaluationResult type = evaluateExpressionAt( line, column, conf );
	
	if ( type.expr.expr().stripWhiteSpace().isEmpty() )
		return typeInfoString;
	
	typeInfoString += type.expr.expr() + QString(" : " );
	
	if ( type->resolved() ) 
	{ 
		QString scope = type->resolved()->scope().join("::");
		int pos = scope.findRev("::");
		if ( scope.isEmpty() || pos == -1 )
		{
			scope = "::";
		}
		else
		{
			scope.truncate( pos + 2 );
		}
		
		typeInfoString += scope + type->fullNameChain()  + QString( i18n(" (resolved) ") );
	}
	else
	{
		if ( type ) 
		{
			if( !BuiltinTypes::isBuiltin( type.resultType ) ) 
			{
				typeInfoString += type->fullNameChain() + QString( i18n(" (unresolved) ") );
			} 
			else 
			{
				typeInfoString += type->fullNameChain() + ", " + BuiltinTypes::comment( type.resultType ) + QString( i18n(" (builtin type) ") );
			}
		}
		else
		{
			typeInfoString += QString( i18n(" (unresolved) ") );
		}
	}

	if( cppSupport() && type->resolved() && cppSupport()->codeCompletionConfig()->preProcessAllHeaders() ) {
		    DeclarationInfo decl = type->resolved()->getDeclarationInfo();
		    if( !getIncludeFiles()[ HashedString( decl.file ) ] ) {
			    typeInfoString += " [header not included] ";
		    }
	}
	
	return typeInfoString;
}

bool CppCodeCompletion::getIncludeInfo( int line, QString& includeFileName, QString& includeFilePath, bool& usedProjectFiles )
{
	bool isIncludeDirective = false;
	QString lineText = getText( line, 0, line+1, 0 );
	QRegExp includeRx( "(?:#include[\\s]*(?:\\\"|\\<))([^\\n]*)(\\\"|\\>)" );
	if( includeRx.search( lineText ) != -1 ) {
		//It is an include-directive. The regular expression captures the string, and the closing sign('"' or '>').
		isIncludeDirective = true;
		usedProjectFiles = false;
		QStringList captured = includeRx.capturedTexts();
		if( captured.size() == 3 ) {
			Dependence d;
			d.first = captured[1];
			d.second = captured[2] == "\"" ? Dep_Local : Dep_Global;
			includeFilePath = cppSupport()->driver()->findIncludeFile( d, activeFileName() );
			if( includeFilePath.isEmpty() ) {
				//A simple backup-algorithm that can only find files within the same project
				includeFilePath = cppSupport()->findHeaderSimple( d.first );
				usedProjectFiles = true;
			}
			includeFileName = d.first;
		} else {
			kdDebug( 9007 ) << "wrong count of captured items" << endl;
		}
	}
	return isIncludeDirective;
}


#include "cppcodecompletion.moc"
//kate: indent-mode csands; tab-width 2; space-indent off;

