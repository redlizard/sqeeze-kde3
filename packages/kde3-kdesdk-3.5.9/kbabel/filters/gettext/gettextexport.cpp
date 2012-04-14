/* ****************************************************************************
  This file is part of KBabel

  Copyright (C) 1999-2000 by Matthias Kiefer <matthias.kiefer@gmx.de>
		2001-2002 by Stanislav Visnovsky <visnovsky@kde.org>
  Copyright (C) 2005,2006 by Nicolas GOUTTE <goutte@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt. If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.

**************************************************************************** */

#include "gettextexport.h"

#include <resources.h>
#include "catalog.h"
#include "catalogitem.h"
#include "catalogsettings.h"
#include "kbprojectsettings.h"

#include <qfile.h>
#include <qtextcodec.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kgenericfactory.h>

K_EXPORT_COMPONENT_FACTORY( kbabel_gettextexport, KGenericFactory<GettextExportPlugin> ( "kbabelgettextexportfilter" ) )

using namespace KBabel;

GettextExportPlugin::GettextExportPlugin(QObject* parent, const char* name, const QStringList &) :
    CatalogExportPlugin(parent,name), m_wrapWidth( -1 )
{
}

ConversionStatus GettextExportPlugin::save(const QString& localFile , const QString& mimetype, const Catalog* catalog)
{
   // check, whether we know how to handle the extra data
   if( catalog->importPluginID() != "GNU gettext") 
      return UNSUPPORTED_TYPE;

   // we support on the application/x-gettext MIME type
   if( mimetype != "application/x-gettext") 
      return UNSUPPORTED_TYPE;

   QFile file(localFile);

   if(file.open(IO_WriteOnly))
   {
      int progressRatio = QMAX(100/ QMAX(catalog->numberOfEntries(),1), 1);
      emit signalResetProgressBar(i18n("saving file"),100);

      QTextStream stream(&file);
      
      SaveSettings _saveSettings = catalog->saveSettings();
      
      if(_saveSettings.useOldEncoding && catalog->fileCodec())
      {
	    stream.setCodec(catalog->fileCodec());
      }
      else
      {
         switch(_saveSettings.encoding)
         {
            case ProjectSettingsBase::UTF8:
               stream.setCodec(QTextCodec::codecForName("utf-8"));
               break;
            case ProjectSettingsBase::UTF16:
               stream.setCodec(QTextCodec::codecForName("utf-16"));
               break;
            default:
	       stream.setCodec(QTextCodec::codecForLocale());
               break;
         }
      }

      // only save header if it is not empty
      const QString headerComment( catalog->header().comment() );
      // ### TODO: why is this useful to have a header with an empty msgstr?
      if( !headerComment.isEmpty() || !catalog->header().msgstr().isEmpty() )
      {
            // write header
            writeComment( stream, headerComment );
    
            const QString headerMsgid = catalog->header().msgid().first();
    
            // Gettext PO files should have an empty msgid as header
            if ( !headerMsgid.isEmpty() )
            {
                // ### TODO: perhaps it is grave enough for a user message
                kdWarning() << "Non-empty msgid for the header, assuming empty msgid!" << endl << headerMsgid << "---" << endl;
            }
          
            // ### FIXME: if it is the header, then the msgid should be empty! (Even if KBabel has made something out of a non-header first entry!)
            stream << "msgid \"\"\n";
            
            writeKeyword( stream, "msgstr", catalog->header().msgstr().first() );

            stream << "\n";
      }

      QStringList list;
      for( uint counter = 0; counter < catalog->numberOfEntries() ; counter++ )
      {
          if(counter%10==0) {
             emit signalProgress(counter/progressRatio);
	  }
	  
            // write entry
            writeComment( stream, catalog->comment(counter) );
    
            const QString msgctxt = catalog->msgctxt(counter);
            if (! msgctxt.isEmpty() )
            {
                writeKeyword( stream, "msgctxt", msgctxt );
            }

            writeKeyword( stream, "msgid", catalog->msgid( counter ).first() );
            if( catalog->pluralForm( counter ) == Gettext )
            {
                writeKeyword( stream, "msgid_plural", catalog->msgid( counter ).last() );
            }
	  
            if( catalog->pluralForm(counter) != Gettext)
            {
                writeKeyword( stream, "msgstr", catalog->msgstr( counter ).first() );
            }
            else
            {
                kdDebug(KBABEL) << "Saving gettext plural form" << endl;
                const int forms = catalog->msgstr( counter ).count();
                for ( int i = 0; i < forms; ++i )
                {
                    QString keyword ( "msgstr[" );
                    keyword += QString::number( i );
                    keyword += ']';

                    writeKeyword( stream, keyword, *( catalog->msgstr( counter ).at( i ) ) );
                }
            }
            
            stream << "\n";

	 kapp->processEvents(10);
	 if( isStopped() )
	 {
	    return STOPPED;
	 }
      }

      if( _saveSettings.saveObsolete )
      {
          QValueList<QString>::ConstIterator oit;
	  
	  QStringList _obsolete = catalog->catalogExtraData();

          for( oit = _obsolete.begin(); oit != _obsolete.end(); ++oit )
    	  {
              stream << (*oit) << "\n\n";

	      kapp->processEvents(10);
	      if( isStopped() )
	      {
		  return STOPPED;
	      }
          }
      }
      
      emit signalProgress(100);
      file.close();

      emit signalClearProgressBar();
   }
   else
   {
      //emit signalError(i18n("Wasn't able to open file %1").arg(filename.ascii()));
      return OS_ERROR;
   }

   return OK;
}

void GettextExportPlugin::writeComment( QTextStream& stream, const QString& comment ) const
{
    if( !comment.isEmpty() )
    {
        // We must check that each comment line really starts with a #, to avoid syntax errors
        int pos = 0;
        for(;;)
        {
            const int newpos = comment.find( '\n', pos, false );
            if ( newpos == pos )
            {
                ++pos;
                stream << "\n";
                continue;
            }
            const QString span ( ( newpos == -1 ) ? comment.mid( pos ) : comment.mid( pos, newpos-pos ) );

            const int len = span.length();
            QString spaces; // Stored leading spaces
            for ( int i = 0 ; i < len ; ++i )
            {
                const QChar& ch = span[ i ];
                if ( ch == '#' )
                {
                    stream << spaces << span.mid( i );
                    break;
                }
                else if ( ch == ' ' || ch == '\t' )
                {
                    // We have a leading white space character, so store it temporary
                    spaces += ch;
                }
                else
                {
                    // Not leading white space and not a # character. so consider that the # character was missing at first position.
                    stream << "# " << spaces << span.mid( i );
                    break;
                }
            }
            stream << "\n";

            if ( newpos == -1 )
                break;
            else
                pos = newpos + 1;
        }
    }
}

void GettextExportPlugin::writeKeyword( QTextStream& stream, const QString& keyword, const QString& text ) const
{
    if ( text.isEmpty() )
    {
        // Whatever the wrapping mode, an empty line is an empty line
        stream << keyword << " \"\"\n";
        return; 
    }
    else if ( m_wrapWidth == -1 )
    {
        // Traditional KBabel wrapping
        QStringList list = QStringList::split( '\n', text );

        if ( text.startsWith( "\n" ) )
            list.prepend( QString() );

        if(list.isEmpty())
            list.append( QString() );
    
        if( list.count() > 1 )
            list.prepend( QString() );

        stream << keyword << " ";
    
        QStringList::const_iterator it;
        for( it = list.constBegin(); it != list.constEnd(); ++it )
        {
            stream << "\"" << (*it) << "\"\n";
        }
        return;
    }
    else if ( ( !m_wrapWidth )
        || ( m_wrapWidth < 0 ) // Unknown special wrapping, so assume "no wrap" instead
        )
    {
        // No wrapping (like Gettext's --no.wrap or -w0 )
        
        // we need to remove the \n characters, as they are extra characters
        QString realText( text );
        realText.remove( '\n' );
        stream << keyword << " \"" << realText << "\"\n";
        return;
    }

    // ### TODO: test!
    // Normal wrapping like Gettext's -w parameter with a value bigger than 0
    // From here on, we assume that we have an non-empty text and a positive non-null m_wrapWidth
    
    // we need to remove the \n characters, as they are extra characters
    QString realText( text );
    realText.remove( '\n' );

    bool needFirstEmptyLine = false;
    if ( realText.find( "\\n" ) != -1 )
    {
        // We have more than one (logical) line, so write the extra empty line
        needFirstEmptyLine = true;
    }
    else
    {
        // We must see if the text would fit in one line, including the keyword, a space and two quote characters.
        const int rest = text.length() + keyword.length() + 3 - m_wrapWidth;
        if ( rest > 0 )
        {
            needFirstEmptyLine = true;
        }
    }
    int availableWidth = m_wrapWidth;
    if ( needFirstEmptyLine )
    {
        stream << keyword << " \"\"\n";
    }
    else
    {
        stream << keyword << " ";
        availableWidth -= keyword.length();
        availableWidth--; // The space after the keyword
    }
    
    const int spanLength = realText.length();
    for ( int pos = 0; pos < spanLength; )
    {
        availableWidth -= 2; // Count the quote characters
        if ( availableWidth < 2 )
        {
            // Be sure that at least two useful characters are written, even if the wrap width is too small
            availableWidth = 2;
        }
        const int newlinePos = realText.find ( "\\n", pos );
        if ( ( newlinePos >= 0 ) && ( newlinePos - pos + 2 < availableWidth ) )
        {
            // The newline is near than the maximum available numbers of characters
            availableWidth = newlinePos - pos + 2;
        }
        stream << '\"' << realText.mid( pos, availableWidth ) << "\"\n";
        pos += availableWidth;
    }
}
