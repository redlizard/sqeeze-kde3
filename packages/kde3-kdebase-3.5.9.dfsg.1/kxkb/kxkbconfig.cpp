//
// C++ Implementation: kxkbconfig
//
// Description: 
//
//
// Author: Andriy Rysin <rysin@kde.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <assert.h>

#include <qregexp.h>
#include <qstringlist.h>
#include <qdict.h>

#include <kconfig.h>
#include <kdebug.h>

#include "kxkbconfig.h"
#include "x11helper.h"



static const char* switchModes[SWITCH_POLICY_COUNT] = {
  "Global", "WinClass", "Window"
};

const LayoutUnit DEFAULT_LAYOUT_UNIT = LayoutUnit("us", "");
const char* DEFAULT_MODEL = "pc104";

LayoutUnit KxkbConfig::getDefaultLayout()
{
	if( m_layouts.size() == 0 )
		return DEFAULT_LAYOUT_UNIT;
	
	return m_layouts[0];
}

bool KxkbConfig::load(int loadMode) 
{
	KConfig *config = new KConfig("kxkbrc", true, false);
	config->setGroup("Layout");

// Even if the layouts have been disabled we still want to set Xkb options
// user can always switch them off now in the "Options" tab
	m_enableXkbOptions = config->readBoolEntry("EnableXkbOptions", false);
	
	if( m_enableXkbOptions == true || loadMode == LOAD_ALL ) {
		m_resetOldOptions = config->readBoolEntry("ResetOldOptions", false);
		m_options = config->readEntry("Options", "");
		kdDebug() << "Xkb options (enabled=" << m_enableXkbOptions << "): " << m_options << endl;
	}
	
	m_useKxkb = config->readBoolEntry("Use", false);
	kdDebug() << "Use kxkb " << m_useKxkb << endl;

	if( (m_useKxkb == false && loadMode == LOAD_ACTIVE_OPTIONS )
	  		|| loadMode == LOAD_INIT_OPTIONS )
		return true;

	m_model = config->readEntry("Model", DEFAULT_MODEL);
	kdDebug() << "Model: " << m_model << endl;
	
	QStringList layoutList;
	if( config->hasKey("LayoutList") ) {
		layoutList = config->readListEntry("LayoutList");
	}
	else { // old config
		QString mainLayout = config->readEntry("Layout", DEFAULT_LAYOUT_UNIT.toPair());
		layoutList = config->readListEntry("Additional");
		layoutList.prepend(mainLayout);
	}
	if( layoutList.count() == 0 )
		layoutList.append("us");
	
	m_layouts.clear();
	for(QStringList::ConstIterator it = layoutList.begin(); it != layoutList.end() ; ++it) {
		m_layouts.append( LayoutUnit(*it) );
		kdDebug() << " layout " << LayoutUnit(*it).toPair() << " in list: " << m_layouts.contains( LayoutUnit(*it) ) << endl;
	}

	kdDebug() << "Found " << m_layouts.count() << " layouts, default is " << getDefaultLayout().toPair() << endl;
	
	QStringList displayNamesList = config->readListEntry("DisplayNames", ',');
	for(QStringList::ConstIterator it = displayNamesList.begin(); it != displayNamesList.end() ; ++it) {
		QStringList displayNamePair = QStringList::split(':', *it );
		if( displayNamePair.count() == 2 ) {
			LayoutUnit layoutUnit( displayNamePair[0] );
			if( m_layouts.contains( layoutUnit ) ) {
				m_layouts[m_layouts.findIndex(layoutUnit)].displayName = displayNamePair[1].left(3);
			}
		}
	}

// 	m_includes.clear();
	if( X11Helper::areSingleGroupsSupported() ) {
		if( config->hasKey("IncludeGroups") ) {
			QStringList includeList = config->readListEntry("IncludeGroups", ',');
			for(QStringList::ConstIterator it = includeList.begin(); it != includeList.end() ; ++it) {
				QStringList includePair = QStringList::split(':', *it );
				if( includePair.count() == 2 ) {
					LayoutUnit layoutUnit( includePair[0] );
					if( m_layouts.contains( layoutUnit ) ) {
						m_layouts[m_layouts.findIndex(layoutUnit)].includeGroup = includePair[1];
						kdDebug() << "Got inc group: " << includePair[0] << ": " << includePair[1] << endl;
					}
				}
			}
		}
		else { //old includes format
			kdDebug() << "Old includes..." << endl;
			QStringList includeList = config->readListEntry("Includes");
			for(QStringList::ConstIterator it = includeList.begin(); it != includeList.end() ; ++it) {
				QString layoutName = LayoutUnit::parseLayout( *it );
				LayoutUnit layoutUnit( layoutName, "" );
				kdDebug() << "old layout for inc: " << layoutUnit.toPair() << " included " << m_layouts.contains( layoutUnit ) << endl;
				if( m_layouts.contains( layoutUnit ) ) {
					QString variantName = LayoutUnit::parseVariant(*it);
					m_layouts[m_layouts.findIndex(layoutUnit)].includeGroup = variantName;
					kdDebug() << "Got inc group: " << layoutUnit.toPair() << ": " <<  variantName << endl;
				}
			}
		}
	}

	m_showSingle = config->readBoolEntry("ShowSingle", false);
	m_showFlag = config->readBoolEntry("ShowFlag", true);
	
	QString layoutOwner = config->readEntry("SwitchMode", "Global");

	if( layoutOwner == "WinClass" ) {
		m_switchingPolicy = SWITCH_POLICY_WIN_CLASS;
	}
	else if( layoutOwner == "Window" ) {
		m_switchingPolicy = SWITCH_POLICY_WINDOW;
	}
	else /*if( layoutOwner == "Global" )*/ {
		m_switchingPolicy = SWITCH_POLICY_GLOBAL;
	}
	
	if( m_layouts.count() < 2 && m_switchingPolicy != SWITCH_POLICY_GLOBAL ) {
		kdWarning() << "Layout count is less than 2, using Global switching policy" << endl;
		m_switchingPolicy = SWITCH_POLICY_GLOBAL;
	}
	
	kdDebug() << "Layout owner mode " << layoutOwner << endl;
	
	m_stickySwitching = config->readBoolEntry("StickySwitching", false);
	m_stickySwitchingDepth = config->readEntry("StickySwitchingDepth", "2").toInt();
	if( m_stickySwitchingDepth < 2 )
		m_stickySwitchingDepth = 2;

	if( m_stickySwitching == true ) {
		if( m_layouts.count() < 3 ) {
			kdWarning() << "Layout count is less than 3, sticky switching will be off" << endl;
			m_stickySwitching = false;
		}
		else	
		if( (int)m_layouts.count() - 1 < m_stickySwitchingDepth ) {
			kdWarning() << "Sticky switching depth is more than layout count -1, adjusting..." << endl;
			m_stickySwitchingDepth = m_layouts.count() - 1;
		}
	}

	delete config;

	return true;
}

void KxkbConfig::save() 
{
	KConfig *config = new KConfig("kxkbrc", false, false);
	config->setGroup("Layout");

	config->writeEntry("Model", m_model);

	config->writeEntry("EnableXkbOptions", m_enableXkbOptions );
	config->writeEntry("ResetOldOptions", m_resetOldOptions);
	config->writeEntry("Options", m_options );

	QStringList layoutList;
	QStringList includeList;
	QStringList displayNamesList;
	
	QValueList<LayoutUnit>::ConstIterator it;
	for(it = m_layouts.begin(); it != m_layouts.end(); ++it) {
		const LayoutUnit& layoutUnit = *it;
		
		layoutList.append( layoutUnit.toPair() );
		
		if( layoutUnit.includeGroup.isEmpty() == false ) {
			QString incGroupUnit = QString("%1:%2").arg(layoutUnit.toPair(), layoutUnit.includeGroup);
			includeList.append( incGroupUnit );
		}
	
		QString displayName( layoutUnit.displayName );
		kdDebug() << " displayName " << layoutUnit.toPair() << " : " << displayName << endl;
		if( displayName.isEmpty() == false && displayName != layoutUnit.layout ) {
			displayName = QString("%1:%2").arg(layoutUnit.toPair(), displayName);
			displayNamesList.append( displayName );
		}
	}
	
	config->writeEntry("LayoutList", layoutList);
	kdDebug() << "Saving Layouts: " << layoutList << endl;
 	
	config->writeEntry("IncludeGroups", includeList);
 	kdDebug() << "Saving includeGroups: " << includeList << endl;
	
//	if( displayNamesList.empty() == false )
		config->writeEntry("DisplayNames", displayNamesList);
// 	else
// 		config->deleteEntry("DisplayNames");

	config->writeEntry("Use", m_useKxkb);
	config->writeEntry("ShowSingle", m_showSingle);
	config->writeEntry("ShowFlag", m_showFlag);

	config->writeEntry("SwitchMode", switchModes[m_switchingPolicy]);
	
	config->writeEntry("StickySwitching", m_stickySwitching);
	config->writeEntry("StickySwitchingDepth", m_stickySwitchingDepth);

	// remove old options 
 	config->deleteEntry("Variants");
	config->deleteEntry("Includes");
	config->deleteEntry("Encoding");
	config->deleteEntry("AdditionalEncodings");
	config->deleteEntry("Additional");
	config->deleteEntry("Layout");
	
	config->sync();

	delete config;
}

void KxkbConfig::setDefaults()
{
	m_model = DEFAULT_MODEL;

	m_enableXkbOptions = false;
	m_resetOldOptions = false;
	m_options = "";

	m_layouts.clear();
	m_layouts.append( DEFAULT_LAYOUT_UNIT );

	m_useKxkb = false;
	m_showSingle = false;
	m_showFlag = true;

	m_switchingPolicy = SWITCH_POLICY_GLOBAL;
	
	m_stickySwitching = false;
	m_stickySwitchingDepth = 2;
}

QStringList KxkbConfig::getLayoutStringList(/*bool compact*/)
{
	QStringList layoutList;
	for(QValueList<LayoutUnit>::ConstIterator it = m_layouts.begin(); it != m_layouts.end(); ++it) {
		const LayoutUnit& layoutUnit = *it;
		layoutList.append( layoutUnit.toPair() );
	}
	return layoutList;
}


QString KxkbConfig::getDefaultDisplayName(const QString& code_)
{
	QString displayName;
	
	if( code_.length() <= 2 ) {
		displayName = code_;
	}
	else {
		int sepPos = code_.find(QRegExp("[-_]"));
		QString leftCode = code_.mid(0, sepPos);
		QString rightCode;
		if( sepPos != -1 )
			rightCode = code_.mid(sepPos+1);
		
		if( rightCode.length() > 0 )
			displayName = leftCode.left(2) + rightCode.left(1).lower();
		else
			displayName = leftCode.left(3);
	}
	
	return displayName;
}

QString KxkbConfig::getDefaultDisplayName(const LayoutUnit& layoutUnit, bool single)
{
	if( layoutUnit.variant == "" )
		return getDefaultDisplayName( layoutUnit.layout );
	
	QString displayName = layoutUnit.layout.left(2);
	if( single == false )
		displayName += layoutUnit.variant.left(1);
	return displayName;
}

/**
 * @brief Gets the single layout part of a layout(variant) string
 * @param[in] layvar String in form layout(variant) to parse
 * @return The layout found in the string
 */
const QString LayoutUnit::parseLayout(const QString &layvar)
{
	static const char* LAYOUT_PATTERN = "[a-zA-Z0-9_/-]*";
	QString varLine = layvar.stripWhiteSpace();
	QRegExp rx(LAYOUT_PATTERN);
	int pos = rx.search(varLine, 0);
	int len = rx.matchedLength();
  // check for errors
	if( pos < 0 || len < 2 )
		return "";
//	kdDebug() << "getLayout: " << varLine.mid(pos, len) << endl;
	return varLine.mid(pos, len);
}

/**
 * @brief Gets the single variant part of a layout(variant) string
 * @param[in] layvar String in form layout(variant) to parse
 * @return The variant found in the string, no check is performed
 */
const QString LayoutUnit::parseVariant(const QString &layvar)
{
	static const char* VARIANT_PATTERN = "\\([a-zA-Z0-9_-]*\\)";
	QString varLine = layvar.stripWhiteSpace();
	QRegExp rx(VARIANT_PATTERN);
	int pos = rx.search(varLine, 0);
	int len = rx.matchedLength();
  // check for errors
	if( pos < 2 || len < 2 )
		return "";
	return varLine.mid(pos+1, len-2);
}
