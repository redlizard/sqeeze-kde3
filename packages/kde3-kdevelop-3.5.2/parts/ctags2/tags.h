/***************************************************************************
 *   Copyright (C) 2004 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TAGS_H
#define TAGS_H

#include <qvaluelist.h>
#include <qcstring.h>
#include <qstring.h>
#include <qstringlist.h>

class Tags
{
public:

	struct TagEntry
	{
		TagEntry();
		TagEntry( const QString & tag, const QString & type, const QString & file, const QString & pattern );
		
		QString tag;
		QString type;
		QString file;
		QString pattern;
	};
	
	typedef QValueList<TagEntry> TagList;
	
	/**
	 *    Method to set the tag database filename
	 * @param file the tag database filename
	 */
	static void setTagFiles(const QStringList& tagFiles);
	
	static QStringList getTagFiles();
	
	/**
	 *    Method to check if the tag database contains a specific tag
	 * @param tag Tag to look up
	 * @return returns true if tag database contains 'tag'
	 */
	static bool hasTag( const QString & tag );
	
	static unsigned int numberOfPartialMatches( const QString & tagpart );
	static unsigned int numberOfExactMatches( const QString & tag );
	static unsigned int numberOfMatches( const QString & tagpart, bool partial );
	
	static TagList getPartialMatches( const QString & tagpart );
	static TagList getExactMatches( const QString & tag );
	static TagList getMatches( const QString & tagpart, bool partial, const QStringList & types = QStringList() );	

private:
	static Tags::TagList getMatches(const char* tagFile, 
								   const QString & tagpart, 
								   bool partial,
								   const QStringList & types );
	static unsigned int numberOfMatches(const char* tagFile, 
									   const QString & tagpart, 
									   bool partial );
	static bool hasTag(const char* tagFile, const QString & tag );
	static QStringList _tagFiles;
};

#endif

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
