/**
 * kspell_aspelldict.cpp
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "kspell_aspelldict.h"

#include <kdebug.h>

#include <qtextcodec.h>

using namespace KSpell2;

ASpellDict::ASpellDict( const QString& lang )
    : Dictionary( lang )
{
    m_config = new_aspell_config();
    aspell_config_replace( m_config, "lang", lang.latin1() );
    /* All communication with Aspell is done in UTF-8 */
    /* For reference, please look at BR#87250         */
    aspell_config_replace( m_config, "encoding", "utf-8" );

    AspellCanHaveError * possible_err = new_aspell_speller( m_config );

    if ( aspell_error_number( possible_err ) != 0 )
        kdDebug()<< "Error : "<< aspell_error_message( possible_err ) <<endl;
    else
        m_speller = to_aspell_speller( possible_err );

}

ASpellDict::~ASpellDict()
{
    delete_aspell_speller( m_speller );
    delete_aspell_config( m_config );
}

bool ASpellDict::check( const QString& word )
{
    /* ASpell is expecting length of a string in char representation */
    /* word.length() != word.utf8().length() for nonlatin strings    */
    int correct = aspell_speller_check( m_speller, word.utf8(), word.utf8().length() );
    return correct;
}

QStringList ASpellDict::suggest( const QString& word )
{
    /* Needed for Unicode conversion */
    QTextCodec *codec = QTextCodec::codecForName("utf8");

    /* ASpell is expecting length of a string in char representation */
    /* word.length() != word.utf8().length() for nonlatin strings    */
    const AspellWordList * suggestions = aspell_speller_suggest( m_speller,
                                                                 word.utf8(),
                                                                 word.utf8().length() );

    AspellStringEnumeration * elements = aspell_word_list_elements( suggestions );

    QStringList qsug;
    const char * cword;

    while ( (cword = aspell_string_enumeration_next( elements )) ) {
        /* Since while creating the class ASpellDict the encoding is set */
        /* to utf-8, one has to convert output from Aspell to Unicode    */
        qsug.append( codec->toUnicode( cword ) );
    }

    delete_aspell_string_enumeration( elements );
    return qsug;
}

bool ASpellDict::checkAndSuggest( const QString& word,
                                  QStringList& suggestions )
{
    bool c = check( word );
    if ( c )
        suggestions = suggest( word );
    return c;
}

bool ASpellDict::storeReplacement( const QString& bad,
                                   const QString& good )
{
    /* ASpell is expecting length of a string in char representation */
    /* word.length() != word.utf8().length() for nonlatin strings    */
    return aspell_speller_store_replacement( m_speller,
                                             bad.utf8(), bad.utf8().length(),
                                             good.utf8(), good.utf8().length() );
}

bool ASpellDict::addToPersonal( const QString& word )
{
    kdDebug() << "ASpellDict::addToPersonal: word = " << word << endl;
    /* ASpell is expecting length of a string in char representation */
    /* word.length() != word.utf8().length() for nonlatin strings    */
    aspell_speller_add_to_personal( m_speller, word.utf8(),
                                    word.utf8().length() );
    /* Add is not enough, one has to save it. This is not documented */
    /* in ASpell's API manual. I found it in                         */ 
    /* aspell-0.60.2/example/example-c.c                             */
    return aspell_speller_save_all_word_lists( m_speller );
}

bool ASpellDict::addToSession( const QString& word )
{
    /* ASpell is expecting length of a string in char representation */
    /* word.length() != word.utf8().length() for nonlatin strings    */
    return aspell_speller_add_to_session( m_speller, word.utf8(),
                                          word.utf8().length() );
}
