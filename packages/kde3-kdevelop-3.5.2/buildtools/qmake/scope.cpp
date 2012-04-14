/***************************************************************************
*   Copyright (C) 2006 by Andreas Pakulat                                 *
*   apaku@gmx.de                                                          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "scope.h"

#include <kdebug.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qpair.h>
#include <qmakedriver.h>
#include <qregexp.h>
#include <qtimer.h>

#include <kdirwatch.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <stdlib.h>

#include "urlutil.h"
#include "trollprojectpart.h"
#include "qmakedefaultopts.h"

const QStringList Scope::KnownVariables = QStringList() << "QT" << "CONFIG" << "TEMPLATE" << "SUBDIRS" << "VERSION" << "LIBS" << "target.path" << "INSTALLS" << "MAKEFILE" << "TARGETDEPS" << "INCLUDEPATH" << "TARGET" << "DESTDIR" << "DEFINES" << "QMAKE_CXXFLAGS_DEBUG" << "QMAKE_CXXFLAGS_RELEASE" << "OBJECTS_DIR" << "UI_DIR" << "MOC_DIR" << "IDL_COMPILER" << "IDL_OPTIONS" << "RCC_DIR" << "IDLS" << "RESOURCES" << "IMAGES" << "LEXSOURCES" << "DISTFILES" << "YACCSOURCES" << "TRANSLATIONS" << "HEADERS" << "SOURCES" << "INTERFACES" << "FORMS" ;

const QStringList Scope::KnownConfigValues = QStringList() << "debug" << "release" << "debug_and_release" << "warn_on" << "warn_off" << "staticlib" << "dll" << "plugin" << "designer" << "create_pkgconf" << "create_libtool" << "qt" << "console" << "windows" << "x11" << "thread" << "exceptions" << "stl" << "rtti" << "opengl" << "thread" << "ordered" << "precompile_header" << "qtestlib" << "uitools" << "dbus" << "assistant" << "build_all";

Scope::Scope( const QMap<QString, QString>& env, const QString &filename, TrollProjectPart* part )
    : m_root( 0 ), m_incast( 0 ), m_parent( 0 ), m_num(0), m_isEnabled( true ), m_part(part), m_defaultopts(0), m_environment( env )
{
    if ( !loadFromFile( filename ) )
    {
        if( !QFileInfo( filename ).exists() )
        {
            m_root = new QMake::ProjectAST();
            m_root->setFileName( filename );
        }else
        {
            delete m_root;
            m_root = 0;
        }
    }
    loadDefaultOpts();
    if( m_root )
    {
        m_part->dirWatch()->addFile(filename);
    }
    init();
}

Scope::~Scope()
{
    QMap<unsigned int, Scope*>::iterator it;
    for ( it = m_scopes.begin() ; it != m_scopes.end() ; ++it )
    {
        Scope* s = it.data();
        delete s;
    }
    m_scopes.clear();

    m_customVariables.clear();
    if ( m_root && m_root->isProject() && !m_incast )
    {
        delete m_root;
        m_root = 0;
        delete m_defaultopts;
        m_defaultopts = 0;
    }

}

// Simple/Function Scopes
Scope::Scope( const QMap<QString, QString>& env, unsigned int num, Scope* parent, QMake::ProjectAST* scope,
              QMakeDefaultOpts* defaultopts, TrollProjectPart* part )
    : m_root( scope ), m_incast( 0 ), m_parent( parent ), m_num(num), m_isEnabled( true ),
        m_part(part), m_defaultopts(defaultopts), m_environment( env )
{
    init();
}

//Subdirs
Scope::Scope( const QMap<QString, QString>& env, unsigned int num, Scope* parent, const QString& filename,
              TrollProjectPart* part, bool isEnabled )
    : m_root( 0 ), m_incast( 0 ), m_parent( parent ), m_num(num), m_isEnabled( isEnabled ),
    m_part(part), m_defaultopts(0), m_environment( env )
{
    if ( !loadFromFile( filename ) )
    {
        if( !QFileInfo( filename ).exists() && QFileInfo( QFileInfo( filename ).dirPath( true ) ).exists() )
        {
            m_root = new QMake::ProjectAST();
            m_root->setFileName( filename );
        }else
        {
            delete m_root;
            m_root = 0;
            m_isEnabled = false;
        }
    }
    loadDefaultOpts();
    if( m_root )
        m_part->dirWatch()->addFile(filename);
    init();
}

//Include Scope
Scope::Scope( const QMap<QString, QString>& env, unsigned int num, Scope* parent, QMake::IncludeAST* incast, const QString& path,
              const QString& incfile, QMakeDefaultOpts* defaultopts, TrollProjectPart* part )
    : m_root( 0 ), m_incast( incast ), m_parent( parent ), m_num(num), m_isEnabled( true ),
    m_part(part), m_defaultopts(defaultopts), m_environment( env )
{
    QString absfilename;
    QString tmp = incfile.stripWhiteSpace();
    if( tmp.contains(")" ) )
        tmp = tmp.mid(0, tmp.find(")") );

    if( tmp.startsWith( "\"" ) )
        tmp = tmp.mid( 1, tmp.length()-2 );

    if( QFileInfo(tmp).isRelative() )
    {
        absfilename = QDir::cleanDirPath( path + QString( QChar( QDir::separator() ) ) + tmp );
    }else
        absfilename = QDir::cleanDirPath( tmp );
    if ( !loadFromFile( absfilename ) )
    {
        if( !QFileInfo( absfilename ).exists() && QFileInfo( QFileInfo( absfilename ).dirPath( true ) ).exists() )
        {
            m_root = new QMake::ProjectAST();
            m_root->setFileName( absfilename );
        }else
        {
            delete m_root;
            m_root = 0;
            m_isEnabled = false;
        }
    }
    if( m_root )
        m_part->dirWatch()->addFile( m_root->fileName() );
    init();
}

bool Scope::loadFromFile( const QString& filename )
{
    if ( !QFileInfo(filename).exists() || QMake::Driver::parseFile( filename, &m_root, 0 ) != 0 )
    {
        kdDebug( 9024 ) << "Couldn't parse project: " << filename << endl;
        if( DomUtil::readBoolEntry( *m_part->projectDom(),
            "/kdevtrollproject/qmake/showParseErrors", true ) )
        {
            KMessageBox::error( 0, i18n( "Couldn't parse project file: %1" ).arg( filename ),
                    i18n( "Couldn't parse project file" ) );
        }
        m_root = 0;
        return false;
    }
//     init();
    return true;
}

void Scope::saveToFile() const
{
    if ( !m_root )
        return ;

    if ( scopeType() != ProjectScope && scopeType() != IncludeScope )
    {
        m_parent->saveToFile();
        return;
    }

    QString filename;
    if ( scopeType() == ProjectScope )
        filename = m_root->fileName() ;
    else if ( scopeType() == IncludeScope )
        filename = m_parent->projectDir() + QString( QChar( QDir::separator() ) ) + m_incast->projectName;
    if ( filename.isEmpty() )
        return ;
    m_part->dirWatch()->stopScan();
    QFile file( filename );
    if ( file.open( IO_WriteOnly ) )
    {

        QTextStream out( &file );
        QString astbuffer;
        m_root->writeBack( astbuffer );
        out << astbuffer;
        file.close();
    }else
    {
        KMessageBox::error( 0, i18n( "Couldn't write project file: %1" ).arg( filename ),
                i18n( "Couldn't write project file" ) );
    }
#ifdef DEBUG
    Scope::PrintAST pa;
    pa.processProject(m_root);
#endif
    m_part->dirWatch()->startScan();
}

void Scope::addToPlusOp( const QString& variable, const QStringList& values )
{
    if ( !m_root )
        return ;

    updateVariable( variable, "+=", values, false );
}

void Scope::removeFromPlusOp( const QString& variable, const QStringList& values )
{
    if ( !m_root )
        return ;

    updateVariable( variable, "+=", values, true );
}


void Scope::addToMinusOp( const QString& variable, const QStringList& values )
{
    if ( !m_root )
        return ;

    updateVariable( variable, "-=", values, false );
}

void Scope::removeFromMinusOp( const QString& variable, const QStringList& values )
{
    if ( !m_root )
        return ;

    updateVariable( variable, "-=", values, true );
}

void Scope::addToEqualOp( const QString& variable, const QStringList& values )
{
    if ( !m_root )
        return ;

    updateVariable( variable, "=", values, false );
}

void Scope::removeFromEqualOp( const QString& variable, const QStringList& values )
{
    if ( !m_root )
        return ;

    updateVariable( variable, "=", values, true );
}

void Scope::setPlusOp( const QString& variable, const QStringList& values )
{
    if( !m_root || Scope::listsEqual(values, variableValuesForOp(variable, "+=") ) )
        return;

    updateVariable( variable, "+=", variableValuesForOp( variable, "+=" ), true );
    updateVariable( variable, "+=", values, false );
}

void Scope::setEqualOp( const QString& variable, const QStringList& values )
{
    if( !m_root || Scope::listsEqual(values, variableValuesForOp(variable, "=") ) )
        return;

    updateVariable( variable, "=", variableValuesForOp( variable, "=" ), true );
    updateVariable( variable, "=", values, false );
}

void Scope::setMinusOp( const QString& variable, const QStringList& values )
{
    if( !m_root || Scope::listsEqual(values, variableValuesForOp(variable, "-=") ) )
        return;

    updateVariable( variable, "-=", variableValuesForOp( variable, "-=" ), true );
    updateVariable( variable, "-=", values, false );
}

QStringList Scope::variableValuesForOp( const QString& variable , const QString& op ) const
{
    QStringList result;

    if( !m_root )
        return result;

    QValueList<QMake::AST*>::const_iterator it;
    for ( it = m_root->m_children.begin(); it != m_root->m_children.end(); ++it )
    {
        QMake::AST* ast = *it;
        if ( ast->nodeType() == QMake::AST::AssignmentAST )
        {
            QMake::AssignmentAST * assign = static_cast<QMake::AssignmentAST*>( ast );
            if ( assign->scopedID == variable && assign->op == op )
            {
                result += assign->values;
            }
        }
    }
    result = cleanStringList(result);
    return result;
}

QStringList Scope::variableValues( const QString& variable, bool checkIncParent, bool fetchFromParent, bool evaluateSubScopes )
{
    QStringList result;

    if ( !m_root )
        return result;

    if( m_varCache.contains( variable ) && fetchFromParent && ( checkIncParent || scopeType() != Scope::IncludeScope ) )
    {
        return m_varCache[variable];
    }

    calcValuesFromStatements( variable, result, checkIncParent, 0, fetchFromParent, true, evaluateSubScopes );
    result = cleanStringList(result);
    if( ( scopeType() != Scope::IncludeScope || checkIncParent  ) && fetchFromParent )
    {
        m_varCache[ variable ] = result;
    }
    return result;
}

void Scope::calcValuesFromStatements( const QString& variable, QStringList& result, bool checkIncParent, QMake::AST* stopHere, bool fetchFromParent, bool setDefault, bool evaluateSubScopes ) const
{
    if( !m_root )
        return;

    /* For variables that we don't know and which are not QT/CONFIG find the default value */
    if( setDefault && m_defaultopts
        && m_defaultopts->variables().findIndex(variable) != -1
        && ( variable == "TEMPLATE" || variable == "QT" || KnownVariables.findIndex(variable) == -1 || variable == "CONFIG" ) )
    {
        result = m_defaultopts->variableValues(variable);
    }

    if ( ( scopeType() == FunctionScope || scopeType() == SimpleScope ) && fetchFromParent )
    {
        m_parent->calcValuesFromStatements( variable, result, checkIncParent, this->m_root, fetchFromParent, setDefault, evaluateSubScopes );
    }
    else if ( scopeType() == IncludeScope && checkIncParent && fetchFromParent )
    {
        m_parent->calcValuesFromStatements( variable, result, true, this->m_incast, fetchFromParent, setDefault, evaluateSubScopes );
    }

    QValueList<QMake::AST*>::const_iterator it;
    for ( it = m_root->m_children.begin(); it != m_root->m_children.end(); ++it )
    {
        if ( stopHere && *it == stopHere )
            return ;
        QMake::AST* ast = *it;
        if ( ast->nodeType() == QMake::AST::AssignmentAST )
        {
            QMake::AssignmentAST * assign = static_cast<QMake::AssignmentAST*>( ast );
            if ( assign->scopedID == variable )
            {
                if ( assign->op == "=" )
                {
                    result = assign->values;
                }
                else if ( assign->op == "+=" )
                {
                    for ( QStringList::const_iterator sit = assign->values.begin(); sit != assign->values.end() ; ++sit )
                    {
                        if ( result.findIndex( *sit ) == -1 )
                            result.append( *sit );
                    }
                }
                else if ( assign->op == "-=" )
                {
                    for ( QStringList::const_iterator sit = assign->values.begin(); sit != assign->values.end() ; ++sit )
                    {
                        if ( result.findIndex( *sit ) != -1 )
                            result.remove( *sit );
                    }
                }
            }
        }else if( evaluateSubScopes )
        {
            if( ast->nodeType() == QMake::AST::IncludeAST )
            {
                QMake::IncludeAST* iast = static_cast<QMake::IncludeAST*>(ast);
                QValueList<unsigned int> l = m_scopes.keys();
                for( unsigned int i = 0; i < l.count(); ++i )
                {
                    int num = l[ i ];
                    if( m_scopes.contains( num ) )
                    {
                        Scope* s = m_scopes[num];
                        if( s && s->scopeType() == IncludeScope && s->m_incast == iast )
                        {
                            s->calcValuesFromStatements( variable, result, false, 0, false, false,  evaluateSubScopes );
                        }
                    }
                }

            }
            else if( ast->nodeType() == QMake::AST::ProjectAST )
            {
                QMake::ProjectAST* past = static_cast<QMake::ProjectAST*>(ast);
                if( past->isFunctionScope() || past->isScope() )
                {
                    QValueList<unsigned int> l = m_scopes.keys();
                    for( unsigned int i = 0; i < l.count(); ++i )
                    {
                        int num = l[ i ];
                        if( m_scopes.contains( num ) )
                        {
                            Scope* s = m_scopes[num];
                            if( s && s->m_root == past && s->m_root->scopedID == past->scopedID  )
                            {
                                s->calcValuesFromStatements( variable, result, false, 0, false, false, evaluateSubScopes );
                            }
                        }
                    }
                }
            }
        }
    }

    result = cleanStringList( result );
    return ;
}

Scope::ScopeType Scope::scopeType() const
{
    if ( !m_root )
        return InvalidScope;
    else if ( m_incast )
        return IncludeScope;
    else if ( m_root->isProject() )
        return ProjectScope;
    else if ( m_root->isScope() )
        return SimpleScope;
    else if ( m_root->isFunctionScope() )
        return FunctionScope;
    return InvalidScope;
}

QString Scope::scopeName() const
{
    if ( !m_root )
        return "";
    if ( m_incast )
        return "include<" + m_incast->projectName + ">";
    else if ( m_root->isFunctionScope() )
        return funcScopeKey( m_root );
    else if ( m_root->isScope() )
        return m_root->scopedID;
    else if ( m_root->isProject() )
    {
        if( m_parent && QDir::cleanDirPath( m_parent->projectDir() ) != QDir::cleanDirPath( projectDir() ) )
        {
            return URLUtil::getRelativePath( m_parent->projectDir(), projectDir() );
        }else if ( m_parent && QDir::cleanDirPath( m_parent->projectDir() ) == QDir::cleanDirPath( projectDir() ) )
        {
            return fileName();
        }else
            return QFileInfo( projectDir() ).fileName() ;
    }
    return QString();
}

QString Scope::fileName() const
{
    if( !m_root )
        return "";
    if ( m_incast )
        return m_incast->projectName;
    else if ( m_root->isProject() )
        return QFileInfo( m_root->fileName() ).fileName();
    else
        return m_parent->fileName();
}

Scope* Scope::createFunctionScope( const QString& funcName, const QString& args )
{
    if ( !m_root )
        return 0;

    QMake::ProjectAST* ast = new QMake::ProjectAST( QMake::ProjectAST::FunctionScope );
    ast->scopedID = funcName;
    ast->args = args;
    ast->setDepth( m_root->depth() );
    ast->addChildAST( new QMake::NewLineAST() );
    m_root->addChildAST( ast );
    m_root->addChildAST( new QMake::NewLineAST() );
    Scope* funcScope = new Scope( m_environment, getNextScopeNum(), this, ast, m_defaultopts, m_part );
    if( funcScope->scopeType() != Scope::InvalidScope )
    {
        m_scopes.insert( getNextScopeNum(), funcScope );
        return funcScope;
    }else
        delete funcScope;
    return 0;
}

Scope* Scope::createSimpleScope( const QString& scopename )
{
    if ( !m_root )
        return 0;

    QMake::ProjectAST* ast = new QMake::ProjectAST( QMake::ProjectAST::Scope );
    ast->scopedID = scopename;
    ast->addChildAST( new QMake::NewLineAST() );
    ast->setDepth( m_root->depth() );
    m_root->addChildAST( ast );
    m_root->addChildAST( new QMake::NewLineAST() );
    /* We can't unconditionally add the scope name to CONFIG, scope might be win32 which may only be in CONFIG under windows.
    if ( m_part->isQt4Project() )
        addToPlusOp( "CONFIG", QStringList( scopename ) );
    */
    Scope* simpleScope = new Scope( m_environment, getNextScopeNum(), this, ast, m_defaultopts, m_part );

    if( simpleScope->scopeType() != Scope::InvalidScope )
    {
        m_scopes.insert( getNextScopeNum(), simpleScope );
        return simpleScope;
    }else
        delete simpleScope;
    return 0;

}

Scope* Scope::createIncludeScope( const QString& includeFile, bool negate )
{
    if ( !m_root )
        return 0;

    Scope* funcScope;
    if ( negate )
    {
        funcScope = createFunctionScope( "!include", includeFile );
    }
    else
    {
        funcScope = createFunctionScope( "include", includeFile );
    }
    if( funcScope == 0 )
        return 0;

    QMake::IncludeAST* ast = new QMake::IncludeAST();
    ast->setDepth( m_root->depth() );
    ast->projectName = includeFile;
    Scope* incScope = new Scope( m_environment, funcScope->getNextScopeNum(), funcScope, ast, projectDir(), resolveVariables( ast->projectName ), m_defaultopts, m_part );
    if ( incScope->scopeType() != InvalidScope )
    {
        funcScope->m_root->addChildAST( ast );
        funcScope->m_scopes.insert( funcScope->getNextScopeNum(), incScope );
        return funcScope;
    }
    else
    {
        deleteFunctionScope( m_scopes.keys().last() );
        delete incScope;
    }
    return 0;

}

Scope* Scope::createSubProject( const QString& projname )
{
    if( !m_root )
        return 0;

    if( variableValuesForOp( "SUBDIRS", "-=").findIndex( projname ) != -1 )
        removeFromMinusOp( "SUBDIRS", projname );

    QString realprojname = resolveVariables(projname);

    if( variableValuesForOp( "SUBDIRS", "-=").findIndex( realprojname ) != -1 )
        removeFromMinusOp( "SUBDIRS", realprojname );

    QDir curdir( projectDir() );

    if ( variableValues("TEMPLATE").findIndex( "subdirs" ) != -1 )
    {
        QString filename;
        if( !realprojname.endsWith(".pro") )
        {
            if ( !curdir.exists( realprojname ) )
                if ( !curdir.mkdir( realprojname ) )
                    return 0;
            curdir.cd( realprojname );
            QStringList entries = curdir.entryList("*.pro", QDir::Files);

            if ( !entries.isEmpty() && entries.findIndex( curdir.dirName()+".pro" ) == -1 )
                filename = curdir.absPath() + QString(QChar(QDir::separator()))+entries.first();
            else
                filename = curdir.absPath() + QString(QChar(QDir::separator()))+curdir.dirName()+".pro";
        }else
            filename = curdir.absPath() + QString(QChar(QDir::separator())) + realprojname;

        kdDebug( 9024 ) << "Creating subproject with filename:" << filename << endl;

        Scope* s = new Scope( m_environment, getNextScopeNum(), this, filename, m_part );
        s->loadDefaultOpts();
        if ( s->scopeType() != InvalidScope )
        {
            if( s->variableValues("TEMPLATE").isEmpty() )
                s->setEqualOp("TEMPLATE", QStringList("app"));
            s->saveToFile();
            addToPlusOp( "SUBDIRS", QStringList( realprojname ) );
            m_scopes.insert( getNextScopeNum(), s );
            return s;
        } else
        {
            delete s;
        }
    }

    return 0;
}

bool Scope::deleteFunctionScope( unsigned int num )
{
    if ( !m_root || !m_scopes.contains( num ) )
        return false;

    Scope* funcScope = m_scopes[ num ];
    if ( funcScope )
    {
        QMake::AST* ast = m_root->m_children[ m_root->m_children.findIndex( funcScope->m_root ) ];
        if( !ast )
            return false;
        m_scopes.remove( num );
        m_root->removeChildAST( funcScope->m_root );
        delete funcScope;
        delete ast;
        return true;
    }
    return false;
}

bool Scope::deleteSimpleScope( unsigned int num )
{
    if ( !m_root || !m_scopes.contains( num ) )
        return false;

    Scope* simpleScope = m_scopes[ num ];
    if ( simpleScope )
    {
        QMake::AST* ast = m_root->m_children[ m_root->m_children.findIndex( simpleScope->m_root ) ];
        if( !ast )
            return false;
        m_scopes.remove( num );
        removeFromPlusOp( "CONFIG", simpleScope->m_root->scopedID );
        m_root->removeChildAST( simpleScope->m_root );
        delete simpleScope;
        delete ast;
        return true;
    }
    return false;
}

bool Scope::deleteIncludeScope( unsigned int num )
{
    if ( !m_root || !m_scopes.contains( num ) )
        return false;

    Scope * incScope = m_scopes[ num ];
    if( !incScope )
        return false;
    QMake::AST* ast = incScope->m_incast;
    if( !ast )
        return false;
    m_scopes.remove( num );
    m_root->removeChildAST( incScope->m_incast);
    delete incScope;
    delete ast;

    return m_parent->deleteFunctionScope( getNum() );
}

bool Scope::deleteSubProject( unsigned int num, bool deleteSubdir )
{
    if ( !m_root || !m_scopes.contains( num ) )
        return false;

    QValueList<QMake::AST*>::iterator it = findExistingVariable( "TEMPLATE" );
    if ( it != m_root->m_children.end() )
    {
        QMake::AssignmentAST * tempast = static_cast<QMake::AssignmentAST*>( *it );
        if ( tempast->values.findIndex( "subdirs" ) != -1 || findExistingVariable( "TEMPLATE" ) != m_root->m_children.end() )
        {
            Scope* project = m_scopes[ num ];
            if( !project )
                return false;

            QString projdir = project->scopeName();
            if ( deleteSubdir )
            {
                QDir projdir = QDir( projectDir() );
                QString dir = project->scopeName();
                if( !dir.endsWith(".pro") )
                {
                    QDir subdir = QDir( projectDir() + QString( QChar( QDir::separator() ) ) + dir );
                    if ( subdir.exists() )
                    {
                        QStringList entries = subdir.entryList();
                        for ( QStringList::iterator eit = entries.begin() ; eit != entries.end() ; ++eit )
                        {
                            if( *eit == "." || *eit == ".." )
                                continue;
                            if( !subdir.remove( *eit ) )
                                kdDebug( 9024 ) << "Couldn't delete " << *eit << " from " << subdir.absPath() << endl;
                        }
                        if( !projdir.rmdir( dir ) )
                            kdDebug( 9024 ) << "Couldn't delete " << dir << " from " << projdir.absPath() << endl;
                    }
                }else
                {
                    QDir d( project->projectDir() );
                    kdDebug(9024) << "removed subproject?:" << d.remove( dir ) << endl;
                }
            }
            QValueList<QMake::AST*>::iterator foundit = findExistingVariable( "SUBDIRS" );
            if ( foundit != m_root->m_children.end() )
            {
                QMake::AssignmentAST * ast = static_cast<QMake::AssignmentAST*>( *foundit );
                updateValues( ast->values, QStringList( projdir ), true, ast->indent );
                if( m_varCache.contains( "SUBDIRS" ) )
                    m_varCache.erase( "SUBDIRS" );
            }else
                return false;
            m_scopes.remove( num );
            delete project;
            return true;
        }
    }
    return false;
}

void Scope::updateValues( QStringList& origValues, const QStringList& newValues, bool remove, QString indent )
{
    if( !m_root )
        return;

    for ( QStringList::const_iterator it = newValues.begin(); it != newValues.end() ; ++it )
    {
        if ( origValues.findIndex( *it ) == -1 && !remove )
        {
            while ( !origValues.isEmpty() && origValues.last() == getLineEndingString() )
                origValues.pop_back();
            if ( origValues.count() > 0 && !containsContinue( origValues.last() ) && !isComment( origValues.last() ) )
            {
                origValues.append( " " );
                origValues.append( "\\"+getLineEndingString() );
                if( indent != "" )
                    origValues.append( indent );
            }else if ( !origValues.isEmpty() && containsContinue( origValues.last() ) && !isComment( origValues.last() ) )
            {
                if( indent != "" )
                    origValues.append( indent );
            }else if ( !origValues.isEmpty() && isComment( origValues.last() ) )
            {
                origValues[origValues.count()-1] = "\\ "+origValues[origValues.count()-1];
                if( indent != "" )
                    origValues.append( indent );
            }else if ( origValues.isEmpty() )
                origValues.append(" ");
            QString newval = *it;
            QRegExp re("([^$])\\$([^$\\(\\)\\{\\} /]*)( |\\)|/)");
            newval.replace(re, "\\1$(\\2)\\3");
            if( (newval).contains(" ") || (newval).contains("\t") || (newval).contains( getLineEndingString() ) || (newval).contains("#") )
                origValues.append( "\""+newval+"\"" );
            else
                origValues.append( newval );
            origValues.append( getLineEndingString() );
        } else if ( origValues.findIndex( *it ) != -1 && remove )
        {
            QStringList::iterator posit = origValues.find( *it );
            posit = origValues.remove( posit );
            while( posit != origValues.end() && ( (*posit).find( QRegExp("\\\\[\\s]*"+getLineEndingString() ) ) != -1
                    || (*posit).stripWhiteSpace() == "" ) )
            {
                posit = origValues.remove( posit );
            }
        }
    }
    while( !origValues.isEmpty() && (origValues.last() == "\\"+getLineEndingString()
            || origValues.last() == getLineEndingString()
            || origValues.last().stripWhiteSpace() == "" ) && !origValues.isEmpty() )
        origValues.pop_back();
    if( !origValues.isEmpty() && origValues.last().find( QRegExp("\\\\[ \t]*#") ) != -1 )
        origValues[origValues.count()-1] = origValues[origValues.count()-1].mid(origValues[origValues.count()-1].find( "#") );
    if( !origValues.isEmpty() && origValues.last().find( getLineEndingString() ) == -1 )
        origValues.append(getLineEndingString());
}

void Scope::updateVariable( const QString& variable, const QString& op, const QStringList& values, bool removeFromOp )
{
    if ( !m_root || listIsEmpty( values ) )
        return ;

    if( m_varCache.contains( variable ) )
        m_varCache.erase( variable );

    for ( int i = m_root->m_children.count() - 1; i >= 0; --i )
    {
        if ( m_root->m_children[ i ] ->nodeType() == QMake::AST::AssignmentAST )
        {
            QMake::AssignmentAST * assignment = static_cast<QMake::AssignmentAST*>( m_root->m_children[ i ] );
            if ( assignment->scopedID == variable && Scope::isCompatible( assignment->op, op ) )
            {
                updateValues( assignment->values, values, removeFromOp, assignment->indent );
                if ( removeFromOp && listIsEmpty( assignment->values ) )
                {
                    m_root->removeChildAST( assignment );
                    delete assignment;
                }
                return ;
            }
            else if ( assignment->scopedID == variable && !Scope::isCompatible( assignment->op, op ) )
            {
                for ( QStringList::const_iterator it = values.begin() ; it != values.end() ; ++it )
                {
                    if ( op == "+=" && !removeFromOp && assignment->values.findIndex( *it ) != -1 )
                    {
                        if ( assignment->op == "=" )
                        {
                            updateValues( assignment->values, values, false, assignment->indent );
                            return ;
                        }
                        else if ( assignment->op == "-=" )
                        {
                            updateValues( assignment->values, QStringList( *it ), true, assignment->indent );
                            if ( listIsEmpty( assignment->values ) )
                            {
                                m_root->removeChildAST( assignment );
                                delete assignment;
                                break;
                            }
                        }
                    }
                    else if ( op == "-=" && !removeFromOp && assignment->values.findIndex( *it ) != -1 )
                    {
                        updateValues( assignment->values, QStringList( *it ), true, assignment->indent );
                        if ( listIsEmpty( assignment->values ) )
                        {
                            m_root->removeChildAST( assignment );
                            delete assignment;
                            break;
                        }
                    }
                    else if ( op == "=" )
                    {
                        if ( !removeFromOp )
                        {
                            m_root->removeChildAST( assignment );
                            delete assignment;
                        }
                        else if ( assignment->op == "+=" && assignment->values.findIndex( *it ) != -1 )
                        {
                            updateValues( assignment->values, QStringList( *it ), true, assignment->indent );
                            if ( listIsEmpty( assignment->values ) )
                            {
                                m_root->removeChildAST( assignment );
                                delete assignment;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    if ( !removeFromOp )
    {
        QMake::AssignmentAST * ast = new QMake::AssignmentAST();
        ast->scopedID = variable;
        ast->op = op;
        updateValues( ast->values, values );
        if( scopeType() == ProjectScope )
            ast->setDepth( m_root->depth() );
        else
            ast->setDepth( m_root->depth()+1 );
        m_root->addChildAST( ast );
        if ( values.findIndex( getLineEndingString() ) == -1 )
        {
            ast->values.append( getLineEndingString() );
        }
    }
}

QValueList<QMake::AST*>::iterator Scope::findExistingVariable( const QString& variable )
{
    QValueList<QMake::AST*>::iterator it;
    QStringList ops;
    ops << "=" << "+=";

    for ( it = m_root->m_children.begin(); it != m_root->m_children.end() ; ++it )
    {
        if ( ( *it ) ->nodeType() == QMake::AST::AssignmentAST )
        {
            QMake::AssignmentAST * assignment = static_cast<QMake::AssignmentAST*>( *it );
            if ( assignment->scopedID == variable && ops.findIndex( assignment->op ) != -1 )
            {
                return it;
            }
        }
    }
    return m_root->m_children.end();
}

void Scope::init()
{
    if( !m_root )
        return;

    kdDebug(9024) << "Initializing Scope: " << scopeName() << this << endl;
    m_maxCustomVarNum = 1;

    QValueList<QMake::AST*>::const_iterator it;
    for ( it = m_root->m_children.begin(); it != m_root->m_children.end(); ++it )
    {
        if ( ( *it ) ->nodeType() == QMake::AST::ProjectAST )
        {
            QMake::ProjectAST * p = static_cast<QMake::ProjectAST*>( *it );
            m_scopes.insert( getNextScopeNum(), new Scope( m_environment, getNextScopeNum(), this, p, m_defaultopts, m_part ) );
        }
        else if ( ( *it ) ->nodeType() == QMake::AST::IncludeAST )
        {
            QMake::IncludeAST * i = static_cast<QMake::IncludeAST*>( *it );
            QString filename = i->projectName;
            if( i->projectName.stripWhiteSpace().startsWith("$") )
            {
                filename = resolveVariables(i->projectName, *it);
            }
            m_scopes.insert( getNextScopeNum(), new Scope( m_environment, getNextScopeNum(), this, i, projectDir(), filename, m_defaultopts, m_part ) );
        }
        else if ( ( *it ) ->nodeType() == QMake::AST::AssignmentAST )
        {
            QMake::AssignmentAST * m = static_cast<QMake::AssignmentAST*>( *it );
            // Check wether TEMPLATE==subdirs here too!
            if ( m->scopedID == "SUBDIRS" && variableValues("TEMPLATE").findIndex("subdirs") != -1 )
            {
                for ( QStringList::const_iterator sit = m->values.begin() ; sit != m->values.end(); ++sit )
                {
                    QString str = *sit;
                    if ( containsContinue( str ) || isComment( str ) || str == getLineEndingString() || str == "." || str == "./" || (str).stripWhiteSpace() == "" )
                        continue;
                    QDir subproject;
                    QString projectfile;
                    kdDebug(9024) << "reading subproject: " << str << endl;
                    if( str.startsWith("$") )
                        str = resolveVariables(str, *it);
                    if( str.endsWith(".pro") )
                    {
                        subproject = QDir( projectDir(), "*.pro", QDir::Name | QDir::IgnoreCase, QDir::Files );
                        projectfile = str;
                    }else
                    {
                        QString dir = str;
                        if( QFileInfo( dir ).isRelative() )
                            dir = projectDir() + QString( QChar( QDir::separator() ) ) + dir;
                        subproject = QDir(  dir,
                                           "*.pro", QDir::Name | QDir::IgnoreCase, QDir::Files );
                        if( !subproject.exists() )
                        {
                            kdDebug(9024) << "Project Dir doesn't exist, trying to find name.subdir variable:" << str <<  endl;
                            if( !variableValues(str+".subdir").isEmpty() )
                            {
                                kdDebug(9024) << "Found name.subdir variable for " << str << endl;
                                subproject = QDir( projectDir() + QString( QChar( QDir::separator() ) )
                                    + variableValues(str+".subdir").first(),
                                    "*.pro", QDir::Name | QDir::IgnoreCase, QDir::Files );
                            }else
                                continue;
                        }
                        if ( subproject.entryList().isEmpty() || subproject.entryList().findIndex( str + ".pro" ) != -1 )
                            projectfile = (str) + ".pro";
                        else
                            projectfile = subproject.entryList().first();

                    }
                    kdDebug( 9024 ) << "Parsing subproject: " << projectfile << endl;
                    m_scopes.insert( getNextScopeNum(), new Scope( m_environment, getNextScopeNum(), this,
                                                subproject.absFilePath( projectfile ),
                                         m_part, ( m->op != "-=" )) );
                }
            }
            else
            {
                if ( !(
                         KnownVariables.findIndex( m->scopedID ) != -1
                         && ( m->op == "=" || m->op == "+=" || m->op == "-=")
                       )
                      && !(
                            ( m->scopedID.contains( ".files" ) || m->scopedID.contains( ".path" ) )
                            && variableValues("INSTALLS").findIndex(m->scopedID.left( m->scopedID.findRev(".") != -1 ) )
                          )
                      && !(
                            ( m->scopedID.contains( ".subdir" ) )
                            && variableValues("SUBDIRS").findIndex(m->scopedID.left( m->scopedID.findRev(".") != -1 ) )
                          )
                    )
                {
                    m_customVariables[ m_maxCustomVarNum++ ] = m;
                }
            }
        }
    }
}

QString Scope::projectName() const
{
    if( !m_root )
        return "";

    return QFileInfo( projectDir() ).fileName();
}

QString Scope::projectDir() const
{
    if( !m_root )
        return "";
    if ( m_root->isProject() )
    {
        return QFileInfo( m_root->fileName() ).dirPath( true );
    }
    else
    {
        return m_parent->projectDir();
    }
}

const QMap<unsigned int, QMap<QString, QString> > Scope::customVariables() const
{
    QMap<unsigned int, QMap<QString, QString> > result;
    if( !m_root )
        return result;

    QMap<unsigned int, QMake::AssignmentAST*>::const_iterator it = m_customVariables.begin();
    for ( ; it != m_customVariables.end(); ++it )
    {
        QMap<QString,QString> temp;
        temp[ "var" ] = it.data()->scopedID;
        temp[ "op" ] = it.data()->op;
        temp[ "values" ] = it.data()->values.join("").stripWhiteSpace();
        result[ it.key() ] = temp;
    }
    return result;
}

void Scope::updateCustomVariable( unsigned int id, const QString& name, const QString& newop, const QString& newvalues )
{
    if( !m_root )
        return;
    if ( id > 0 && m_customVariables.contains( id ) )
    {
        m_customVariables[ id ] ->values.clear();
        updateValues( m_customVariables[ id ] ->values, newvalues.stripWhiteSpace() );
        if( m_varCache.contains( m_customVariables[ id ]->scopedID ) )
            m_varCache.erase( m_customVariables[ id ]->scopedID );
        m_customVariables[ id ] ->op = newop;
        m_customVariables[ id ] ->scopedID = name;
    }
}

unsigned int Scope::addCustomVariable( const QString& var, const QString& op, const QString& values )
{
    QMake::AssignmentAST* newast = new QMake::AssignmentAST();
    newast->scopedID = var;
    newast->op = op;
    newast->values.append(values.stripWhiteSpace());
    if( scopeType() == ProjectScope )
        newast->setDepth( m_root->depth() );
    else
        newast->setDepth( m_root->depth()+1 );
    m_root->addChildAST( newast );
    m_customVariables[ m_maxCustomVarNum++ ] = newast;
    return (m_maxCustomVarNum-1);
}

void Scope::removeCustomVariable( unsigned int id )
{
    if( m_customVariables.contains(id) )
    {
        QMake::AssignmentAST* m = m_customVariables[id];
        m_customVariables.remove(id);
        m_root->m_children.remove( m );
    }
}

bool Scope::isVariableReset( const QString& var )
{
    bool result = false;
    if( !m_root )
        return result;
    QValueList<QMake::AST*>::const_iterator it = m_root->m_children.begin();
    for ( ; it != m_root->m_children.end(); ++it )
    {
        if ( ( *it ) ->nodeType() == QMake::AST::AssignmentAST )
        {
            QMake::AssignmentAST * ast = static_cast<QMake::AssignmentAST*>( *it );
            if ( ast->scopedID == var && ast->op == "=" )
            {
                result = true;
                break;
            }
        }
    }
    return result;
}

void Scope::removeVariable( const QString& var, const QString& op )
{
    if ( !m_root )
        return ;

    QMake::AssignmentAST* ast = 0;

    QValueList<QMake::AST*>::iterator it = m_root->m_children.begin();
    for ( ; it != m_root->m_children.end(); ++it )
    {
        if ( ( *it ) ->nodeType() == QMake::AST::AssignmentAST )
        {
            ast = static_cast<QMake::AssignmentAST*>( *it );
            if ( ast->scopedID == var && ast->op == op )
            {
                m_root->m_children.remove( ast );
                it = m_root->m_children.begin();
            }
        }
    }
}

bool Scope::listIsEmpty( const QStringList& values )
{
    if ( values.size() < 1 )
        return true;
    for ( QStringList::const_iterator it = values.begin(); it != values.end(); ++it )
    {
        if ( ( *it ).stripWhiteSpace() != "" && ( *it ).stripWhiteSpace() != "\\" )
            return false;
    }
    return true;
}

bool Scope::isCompatible( const QString& op1, const QString& op2)
{
    if( op1 == "+=" )
        return ( op2 == "+=" || op2 == "=" );
    else if ( op1 == "-=" )
        return ( op2 == "-=" );
    else if ( op1 == "=" )
        return ( op2 == "=" || op2 == "+=" );
    return false;
}

bool Scope::listsEqual(const QStringList& l1, const QStringList& l2)
{
    QStringList left = l1;
    QStringList right = l2;
//     left.sort();
//     right.sort();
    return (left == right);
}

QStringList Scope::cleanStringList(const QStringList& list) const
{
    QStringList result;
    for( QStringList::const_iterator it = list.begin(); it != list.end(); ++it )
    {
        QString s = *it;
        if( s.stripWhiteSpace() != ""
                && !containsContinue(s)
                && s.stripWhiteSpace() != getLineEndingString()
                && !isComment(s) )
            result.append(s);
    }
    return result;
}

bool Scope::isQt4Project() const
{
    return m_part->isQt4Project();
}

void Scope::reloadProject()
{
    if ( !m_root || !m_root->isProject() )
        return;

    QString filename = m_root->fileName();
    QMap<unsigned int, Scope*>::iterator it;
    for ( it = m_scopes.begin() ; it != m_scopes.end() ; ++it )
    {
        Scope* s = it.data();
        delete s;
    }
    m_scopes.clear();

    m_customVariables.clear();

    m_varCache.clear();

    if ( m_root->isProject() )
        delete m_root;
    if ( !loadFromFile( filename ) && !QFileInfo( filename ).exists() )
    {
        m_root = new QMake::ProjectAST();
        m_root->setFileName( filename );
    }
    init();
}

Scope* Scope::disableSubproject( const QString& dir)
{
    if( !m_root || ( m_root->isProject() && !m_incast ) )
        return 0;

    if( scopeType() != Scope::IncludeScope && variableValuesForOp( "SUBDIRS", "+=").findIndex( dir ) != -1 )
        removeFromPlusOp( "SUBDIRS", dir );
    else if( scopeType() != Scope::IncludeScope )
        removeFromPlusOp( "SUBDIRS", dir );

    QDir curdir( projectDir() );

    if ( variableValues("TEMPLATE").findIndex( "subdirs" ) != -1 )
    {
        curdir.cd(dir);
        QString filename;
        QStringList entries = curdir.entryList("*.pro", QDir::Files);

        if ( !entries.isEmpty() && entries.findIndex( curdir.dirName()+".pro" )  != -1 )
            filename = curdir.absPath() + QString(QChar(QDir::separator()))+entries.first();
        else
            filename = curdir.absPath() + QString(QChar(QDir::separator()))+curdir.dirName()+".pro";

        kdDebug( 9024 ) << "Disabling subproject with filename:" << filename << endl;

        Scope* s = new Scope( m_environment, getNextScopeNum(), this, filename, m_part, false );
        addToMinusOp( "SUBDIRS", QStringList( dir ) );
        m_scopes.insert( getNextScopeNum(), s );
        return s;
    }

    return 0;
}

QString Scope::resolveVariables( const QString& value ) const
{
    return resolveVariables(QStringList(value),  0).front();
}


QString Scope::resolveVariables( const QString& value, QMake::AST* stopHere ) const
{
    return resolveVariables(QStringList(value),  stopHere).front();
}

QStringList Scope::variableValues( const QString& variable, QMake::AST* stopHere, bool fetchFromParent ) const
{
    QStringList result;

    if ( !m_root )
        return result;

    calcValuesFromStatements( variable, result, true, stopHere, fetchFromParent );
    result = cleanStringList(result);
    return result;
}

QStringList Scope::resolveVariables( const QStringList& values, QMake::AST* stopHere ) const
{
    QStringList result = values;
    QMap<QString, QStringList> variables;
    for( QStringList::iterator it = result.begin(); it != result.end(); ++it )
    {
        QRegExp re("\\$\\$([^{}\\) /]*)( |\\)|/|$)");
        int pos = 0;
        while( pos >= 0 )
        {
            pos = re.search( (*it), pos );
            if( pos > -1 )
            {
                if( !variables.contains( re.cap(1) ) )
                {
                    variables[re.cap(1)] = resolveVariables( variableValues( re.cap(1), stopHere ) );
                    if( variables[re.cap(1)].isEmpty() && re.cap(1) == "TARGET" )
                    {
                        variables[re.cap(1)] = QFileInfo( fileName() ).baseName();
                    }
                }
                pos += re.matchedLength();
            }
        }
        re = QRegExp("\\$\\$\\{([^\\)\\}]*)\\}");
        pos = 0;
        while( pos >= 0 )
        {
            pos = re.search( (*it), pos );
            if( pos > -1 )
            {
                if( !variables.contains( re.cap(1) ) )
                {
                    variables[re.cap(1)] = resolveVariables( variableValues( re.cap(1), stopHere ) );
                    if( variables[re.cap(1)].isEmpty() && re.cap(1) == "TARGET" )
                    {
                        variables[re.cap(1)] = QFileInfo( fileName() ).baseName();
                    }
                }
                pos += re.matchedLength();
            }
        }
        re = QRegExp("\\$\\$\\(([^\\)\\}]*)\\)");
        pos = 0;
        QMap<QString, QString> envvars;
        while( pos >= 0 )
        {
            pos = re.search( (*it), pos );
            if( pos > -1 )
            {
                if( !envvars.contains( re.cap(1) ) )
                    if( m_environment.contains( re.cap(1) ) != -1 )
                        envvars[re.cap(1)] = m_environment[ re.cap(1) ];
                    else if ( ::getenv( re.cap(1).local8Bit() ) != 0 )
                        envvars[re.cap(1)] = QString::fromLocal8Bit( ::getenv( re.cap(1).local8Bit() ) );
                pos += re.matchedLength();
            }
        }
        for( QMap<QString, QString>::const_iterator it2 = envvars.begin(); it2 != envvars.end(); ++it2 )
        {
            (*it).replace("$$("+it2.key()+")", it2.data() );
        }
        for( QMap<QString, QStringList>::const_iterator it2 = variables.begin(); it2 != variables.end(); ++it2 )
        {
            for( QStringList::const_iterator it3 = it2.data().begin(); it3 != it2.data().end(); ++it3 )
            {
                (*it).replace("$$"+it2.key(), *it3 );
                (*it).replace("$${"+it2.key()+"}", *it3 );
            }
        }
    }
    return result;
}

void Scope::allFiles( const QString& projectDirectory, std::set<QString>& res )
{

    QString myRelPath = URLUtil::getRelativePath( projectDirectory, projectDir() );
    QString file;
    QStringList values;
    QString header = "";
    if( variableValues("TEMPLATE",false ).findIndex("subdirs") == -1 )
    {
        values = variableValues( "INSTALLS" ,false, false );
        QStringList::const_iterator it;
        for ( it = values.begin(); it != values.end(); ++it )
        {
            if ( ( *it ) == "target" )
                continue;

            QStringList files = variableValues( *it + ".files" ,false, false );
            QStringList::iterator filesit = files.begin();
            for ( ;filesit != files.end(); ++filesit )
            {
                file = myRelPath + QString(QChar(QDir::separator())) + *filesit;
                file = resolveVariables( file );
                if( file.contains("*") )
                {
                    QFileInfo fi( projectDirectory + QString( QChar( QDir::separator() ) ) + file );
                    QDir absDir = fi.dir( true );
                    absDir.setNameFilter( fi.fileName() );
                    absDir.setFilter( QDir::Files | QDir::Readable | QDir::NoSymLinks );
                    QStringList list = absDir.entryList();
                    for( QStringList::const_iterator it = list.begin(); it != list.end(); ++it )
                    {
                        res.insert( QDir::cleanDirPath( URLUtil::getRelativePath( projectDirectory, absDir.path()+QString( QChar( QDir::separator() ) )+*it ) ) );
                    }
                }
                else
                {
                    res.insert( QDir::cleanDirPath( file ) );
                }
            }
        }

        values = variableValues( "LEXSOURCES" ,false, false );
        for ( it = values.begin(); it != values.end(); ++it )
        {
            file = myRelPath + QString(QChar(QDir::separator())) + *it;
            file = resolveVariables( file );
            res.insert( QDir::cleanDirPath( file ) );
        }

        values = variableValues( "YACCSOURCES" ,false, false );
        for ( it = values.begin(); it != values.end(); ++it )
        {
            file = myRelPath + QString(QChar(QDir::separator())) + *it;
            file = resolveVariables( file );
            res.insert( QDir::cleanDirPath( file ) );
        }

        values = variableValues( "DISTFILES" ,false, false );
        for ( it = values.begin(); it != values.end(); ++it )
        {
            file = myRelPath + QString(QChar(QDir::separator())) + *it;
            file = resolveVariables( file );
            if( file.contains("*") )
            {
                QFileInfo fi( projectDirectory + QString( QChar( QDir::separator() ) ) + file );
                QDir absDir = fi.dir( true );
                absDir.setNameFilter( fi.fileName() );
                absDir.setFilter( QDir::Files | QDir::Readable | QDir::NoSymLinks );
                QStringList list = absDir.entryList();
                for( QStringList::const_iterator it = list.begin(); it != list.end(); ++it )
                {
                    res.insert( QDir::cleanDirPath( URLUtil::getRelativePath( projectDirectory, absDir.path()+QString( QChar( QDir::separator() ) )+*it ) ) );
                }
            }
            else
            {
                res.insert( QDir::cleanDirPath( file ) );
            }
        }

        if ( isQt4Project() )
        {
            values = variableValues( "RESOURCES" ,false, false );
            for ( it = values.begin(); it != values.end(); ++it )
            {
                file = myRelPath + QString(QChar(QDir::separator())) + *it;
                file = resolveVariables( file );
                res.insert( QDir::cleanDirPath( file ) );
            }
        }
        values = variableValues( "IMAGES" ,false, false );
        for ( it = values.begin(); it != values.end(); ++it )
        {
            file = myRelPath + QString(QChar(QDir::separator())) + *it;
            file = resolveVariables( file );
            res.insert( QDir::cleanDirPath( file ) );
        }

        values = variableValues( "TRANSLATIONS" ,false, false );
        for ( it = values.begin(); it != values.end(); ++it )
        {
            file = myRelPath + QString(QChar(QDir::separator())) + *it;
            file = resolveVariables( file );
            res.insert( QDir::cleanDirPath( file ) );
        }

        values = variableValues( "IDLS" ,false, false );
        for ( it = values.begin(); it != values.end(); ++it )
        {
            file = myRelPath + QString(QChar(QDir::separator())) + *it;
            file = resolveVariables( file );
            res.insert( QDir::cleanDirPath( file ) );
        }

        if ( m_part->isTMakeProject() )
        {
            values = variableValues( "INTERFACES" ,false, false );
            for ( it = values.begin(); it != values.end(); ++it )
            {
                file = myRelPath + QString(QChar(QDir::separator())) + *it;
                file = resolveVariables( file );
                res.insert( QDir::cleanDirPath( file ) );
                if( QFileInfo(projectDir()+QString(QChar(QDir::separator())) + *it+".h").exists() )
                    res.insert( QDir::cleanDirPath( file+".h" ) );
            }
        }
        else
        {
            values = variableValues( "FORMS" ,false, false );
            for ( it = values.begin(); it != values.end(); ++it )
            {
                file = myRelPath + QString(QChar(QDir::separator())) + *it;
                file = resolveVariables( file );
                res.insert( QDir::cleanDirPath( file ) );

                if( !m_part->isQt4Project())
                {
                    header = projectDir()+QString(QChar(QDir::separator())) + *it+".h";
                    if( QFileInfo(header).exists() )
                        res.insert( QDir::cleanDirPath( header ) );
                    header = projectDir()+QString(QChar(QDir::separator())) + *it+".cpp";
                    if( QFileInfo(header).exists() )
                        res.insert( QDir::cleanDirPath( header ) );
                }
                else
                {
                    header = projectDir()+QString(QChar(QDir::separator())) + "ui_" +*it;
                    header.replace(QRegExp("\\.ui$"),".h");
                    if( QFileInfo(header).exists() )
                        res.insert( QDir::cleanDirPath( header ) );
                }
            }
        }

        values = variableValues( "SOURCES" ,false, false );
        kdDebug(9024)  << "scope:" << scopeType() << " found values: " << values << endl;
        for ( it = values.begin(); it != values.end(); ++it )
        {
            file = myRelPath + QString(QChar(QDir::separator())) + *it;
            file = resolveVariables( file );
            res.insert( QDir::cleanDirPath( file ) );
        }

        values = variableValues( "HEADERS" ,false, false );
        for ( it = values.begin(); it != values.end(); ++it )
        {
            file = myRelPath + QString(QChar(QDir::separator())) + *it;
            file = resolveVariables( file );
            res.insert( QDir::cleanDirPath( file ) );
        }
    }
    QMap<unsigned int, Scope*>::const_iterator it = m_scopes.begin();
    for( ; it != m_scopes.end(); ++it )
    {
        it.data()->allFiles( projectDirectory, res );
    }
}

QStringList Scope::allFiles( const QString& projectDir )
{
    QStringList result;
    std::set<QString> files;
    allFiles( projectDir, files );
    for( std::set<QString>::const_iterator it = files.begin(); it != files.end() ; ++it )
        result.append( *it );
    kdDebug(9024) << "all files: " << result << endl;
    return result;
}

QString Scope::findCustomVarForPath( const QString& path )
{
    QString result;
    if( !m_root )
        return result;

    QMap<unsigned int, QMake::AssignmentAST*>::const_iterator it = m_customVariables.begin();
    for( ; it != m_customVariables.end(); ++it )
    {
        kdDebug(9024) << "Checking " << path << " against " << cleanStringList(it.data()->values) << endl;
        if( !it.data()->values.isEmpty() && cleanStringList(it.data()->values).front() == path )
        {
            return it.data()->scopedID;
        }
    }
    if( scopeType() != ProjectScope )
    {
        return parent()->findCustomVarForPath( path );
    }
    return result;
}

void Scope::loadDefaultOpts()
{
    if( !m_defaultopts && m_root )
    {
        m_defaultopts = new QMakeDefaultOpts();
        if( DomUtil::readBoolEntry( *m_part->projectDom(), "/kdevtrollproject/qmake/disableDefaultOpts", true ) )
        {
            m_defaultopts->readVariables( m_part->qmakePath(), QFileInfo( m_root->fileName() ).dirPath( true ) );
        }
    }
}

QString Scope::getLineEndingString() const
{

    if( scopeType() == ProjectScope )
    {
        switch( m_root->lineEnding() )
        {
            case QMake::ProjectAST::Windows:
                return QString("\r\n");
                break;
            case QMake::ProjectAST::MacOS:
                return QString("\r");
                break;
            case QMake::ProjectAST::Unix:
                return QString("\n");
                break;
        }
    }else if( m_parent )
    {
        return m_parent->getLineEndingString();
    }
    return "\n";
}

QString Scope::replaceWs(QString s)
{
    return s.replace( getLineEndingString(), "%nl").replace("\t", "%tab").replace(" ", "%spc");
}

bool Scope::containsContinue(const QString& s ) const
{
    return( s.find( QRegExp( "\\\\\\s*"+getLineEndingString() ) ) != -1
            || s.find( QRegExp( "\\\\\\s*#" ) ) != -1 );
}

bool Scope::isComment( const QString& s) const
{
    return s.startsWith("#");
}

#ifdef DEBUG
void Scope::printTree()
{
    PrintAST p;
    p.processProject(m_root);
}

Scope::PrintAST::PrintAST() : QMake::ASTVisitor()
{
    indent = 0;
}

void Scope::PrintAST::processProject( QMake::ProjectAST* p )
{
    QMake::ASTVisitor::processProject(p);
}

void Scope::PrintAST::enterRealProject( QMake::ProjectAST* p )
{
    kdDebug(9024) << getIndent() << "--------- Entering Project: " << replaceWs(p->fileName()) << " --------------" << endl;
    indent += 4;
    QMake::ASTVisitor::enterRealProject(p);
}

void Scope::PrintAST::leaveRealProject( QMake::ProjectAST* p )
{
    indent -= 4;
    kdDebug(9024) << getIndent() << "--------- Leaving Project: " << replaceWs(p->fileName()) << " --------------" << endl;
    QMake::ASTVisitor::leaveRealProject(p);
}

void Scope::PrintAST::enterScope( QMake::ProjectAST* p )
{
    kdDebug(9024) << getIndent() << "--------- Entering Scope: " << replaceWs(p->scopedID) << " --------------" << endl;
    indent += 4;
    QMake::ASTVisitor::enterScope(p);
}

void Scope::PrintAST::leaveScope( QMake::ProjectAST* p )
{
    indent -= 4;
    kdDebug(9024) << getIndent() << "--------- Leaving Scope: " << replaceWs(p->scopedID) << " --------------" << endl;
    QMake::ASTVisitor::leaveScope(p);
}

void Scope::PrintAST::enterFunctionScope( QMake::ProjectAST* p )
{
    kdDebug(9024) << getIndent() << "--------- Entering FunctionScope: " << replaceWs(p->scopedID) << "(" << replaceWs(p->args) << ")"<< " --------------" << endl;
    indent += 4;
    QMake::ASTVisitor::enterFunctionScope(p);
}

void Scope::PrintAST::leaveFunctionScope( QMake::ProjectAST* p )
{
    indent -= 4;
    kdDebug(9024) << getIndent() << "--------- Leaving FunctionScope: " << replaceWs(p->scopedID) << "(" << replaceWs(p->args) << ")"<< " --------------" << endl;
    QMake::ASTVisitor::leaveFunctionScope(p);
}

QString Scope::PrintAST::replaceWs(QString s)
{
    return s.replace("\n", "%nl").replace("\t", "%tab").replace(" ", "%spc");
}

void Scope::PrintAST::processAssignment( QMake::AssignmentAST* a)
{
    kdDebug(9024) << getIndent() << "Assignment: " << replaceWs(a->scopedID) << " " << replaceWs(a->op) << " "
        << replaceWs(a->values.join("|"))<< endl;
    QMake::ASTVisitor::processAssignment(a);
}

void Scope::PrintAST::processNewLine( QMake::NewLineAST* n)
{
    kdDebug(9024) << getIndent() << "Newline " << endl;
    QMake::ASTVisitor::processNewLine(n);
}

void Scope::PrintAST::processComment( QMake::CommentAST* a)
{
    kdDebug(9024) << getIndent() << "Comment: " << replaceWs(a->comment) << endl;
    QMake::ASTVisitor::processComment(a);
}

void Scope::PrintAST::processInclude( QMake::IncludeAST* a)
{
    kdDebug(9024) << getIndent() << "Include: " << replaceWs(a->projectName) << endl;
    QMake::ASTVisitor::processInclude(a);
}

QString Scope::PrintAST::getIndent()
{
    QString ind;
    for( int i = 0 ; i < indent ; i++)
        ind += " ";
    return ind;
}
#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
