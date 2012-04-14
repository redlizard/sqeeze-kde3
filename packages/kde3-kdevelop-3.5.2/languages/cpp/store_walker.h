/***************************************************************************
*   Copyright (C) 2003 by Roberto Raggi                                   *
*   roberto@kdevelop.org                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef __store_walker_h
#define __store_walker_h

#include "tree_parser.h"
#include <codemodel.h>
#include <qstringlist.h>
#include <qvaluestack.h>
#include <hashedstring.h>

class StoreWalker: public TreeParser
{
public:
    StoreWalker( const QString& fileName, CodeModel* store );
	virtual ~StoreWalker();

	FileDom file()
	{
		return m_file;
	}

	// translation-unit
	virtual void parseTranslationUnit( const ParsedFile& );

	// declarations
	virtual void parseDeclaration( DeclarationAST* );
	virtual void parseLinkageSpecification( LinkageSpecificationAST* );
	virtual void parseNamespace( NamespaceAST* );
	virtual void parseNamespaceAlias( NamespaceAliasAST* );
	virtual void parseUsing( UsingAST* );
	virtual void parseUsingDirective( UsingDirectiveAST* );
	virtual void parseTypedef( TypedefAST* );
	virtual void parseTemplateDeclaration( TemplateDeclarationAST* );
	virtual void parseSimpleDeclaration( SimpleDeclarationAST* );
	virtual void parseFunctionDefinition( FunctionDefinitionAST* );
	virtual void parseLinkageBody( LinkageBodyAST* );
	virtual void parseAccessDeclaration( AccessDeclarationAST* );

    void takeTemplateParams( TemplateModelItem& target, TemplateDeclarationAST*);
    
	// type-specifier
	virtual void parseTypeSpecifier( TypeSpecifierAST* );
	virtual void parseClassSpecifier( ClassSpecifierAST* );
	virtual void parseEnumSpecifier( EnumSpecifierAST* );
	virtual void parseElaboratedTypeSpecifier( ElaboratedTypeSpecifierAST* );

	virtual void parseTypeDeclaratation( TypeSpecifierAST* typeSpec );
	virtual void parseDeclaration( GroupAST* funSpec, GroupAST* storageSpec, TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl );
	virtual void parseFunctionDeclaration( GroupAST* funSpec, GroupAST* storageSpec, TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl );
	virtual void parseFunctionArguments( DeclaratorAST* declarator, FunctionDom method );
	virtual void parseBaseClause( BaseClauseAST* baseClause, ClassDom klass );

private:
	NamespaceDom findOrInsertNamespace( NamespaceAST* ast, const QString& name );
	QString typeOfDeclaration( TypeSpecifierAST* typeSpec, DeclaratorAST* declarator );
	QStringList scopeOfName( NameAST* id, const QStringList& scope );
	QStringList scopeOfDeclarator( DeclaratorAST* d, const QStringList& scope );
    ClassDom classFromScope(const QStringList& scope);
    ClassDom findClassFromScope(const QStringList& scope);
    void checkTemplateDeclarator( TemplateModelItem* item );
private:
    
    class CommentPusher {
        StoreWalker& m_ref;
    public:
        CommentPusher( StoreWalker& ref, QString comment ) : m_ref( ref ) {
            m_ref.pushComment( comment );
        }
        ~CommentPusher() {
            m_ref.popComment();
        }
    };
    
    QStringList m_comments;
    
    QString comment() {
        if( m_comments.isEmpty() ) {
            return "";
        } else {
            return m_comments.front();
        }
    }

    //Own implementation that also merges the groups of the overrides
    int mergeGroups( int g1, int g2 );
    
public:
    void pushComment( QString comm ) {
        m_comments.push_front( comm );
    }
    
    void popComment() {
        m_comments.pop_front();
    }

    void setOverrides( const QMap<QString, FileDom>& overrides ) {
      m_overrides = overrides;
    }
private:

    QMap<QString, FileDom> m_overrides;
    
	FileDom m_file;
	QString m_fileName;
    HashedString m_hashedFileName;
	QStringList m_currentScope;
	CodeModel* m_store;
	QValueList<QPair<QMap<QString, ClassDom>, QStringList> > m_imports;
	int m_currentAccess;
	bool m_inSlots;
	bool m_inSignals;
	int m_anon;
	bool m_inStorageSpec;
	bool m_inTypedef;

	DeclaratorAST* m_currentDeclarator;
    QValueStack<TemplateDeclarationAST*> m_currentTemplateDeclarator;
	QValueStack<NamespaceDom> m_currentNamespace;
	QValueStack<ClassDom> m_currentClass;

    QStringList findScope( const QStringList& scope );
    
    
private:
	StoreWalker( const StoreWalker& source );
	void operator = ( const StoreWalker& source );
};

#endif // __store_walker_h 
// kate: indent-mode csands; tab-width 4;

