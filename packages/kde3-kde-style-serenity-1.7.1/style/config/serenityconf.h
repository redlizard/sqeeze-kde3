/*
	Serenity Style for KDE 3+
	
		Copyright (C) 2006 Remi Villatel <maxilys@tele2.fr>
	
	Based on Lipstik engine:
	
		Copyright (C) 2004 Patrice Tremblay <tremblaypatrice@yahoo.fr>
	
	Also based on numerous open source styles that helped me to not 
	re-invent the wheel for every line of code. Thanks to everyone.  ;-)
	
	This library is free software; you can redistribute it and/or 
	modify it under the terms of the GNU Library General Public 
	License version 2 as published by the Free Software Foundation.
	
	This library is distributed in the hope that it will be useful, 
	but WITHOUT ANY WARRANTY; without even the implied warranty of 
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
	Library General Public License for more details.
	
	You should have received a copy of the GNU Library General Public 
	License along with this library. (See the file COPYING.) If not, 
	write to:
	The Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
	Boston, MA 02111-1307, USA.
*/

#ifndef SERENITY_CONF_H
#define SERENITY_CONF_H

class QCheckBox;
class QComboBox;
class QSlider;
class ConfigDialog;

class SerenityStyleConfig: public QWidget
{
	Q_OBJECT
	
	public:
		SerenityStyleConfig(QWidget* parent);
		~SerenityStyleConfig();
	
		// This signal and the next two slots are 
		// the plugin page interface
	signals:
		void changed(bool);
	
	public slots:
		void save();
		void defaults();
	
		// Everything below this point is internal.
	protected slots:
		void updateChanged();
	
	protected:
		int limitedTo(int lowest, int highest, int variable);
		//
		// Original settings, for accurate dirtiness tracking
		int	origActiveTabStyle;
		bool	origAlternateSinking;
		bool	origAnimProgressBar;
		bool	origCenterTabs;
		bool	origCustomGlobalColor;
		bool	origCustomOverHighlightColor;
		bool	origDrawFocusRect;
		bool	origDrawToolBarHandle;
		bool	origDrawToolBarSeparator;
		bool	origDrawTriangularExpander;
		bool	origFlatProgressBar;
		QColor	origGlobalColor;
		int	origGlobalStyle;
		bool	origHiliteRubber;
		bool	origMenubarHack;
		int	origMenuGrooveStyle;
		//bool	origMouseOverLabel;
		QColor	origOverHighlightColor;
		int	origPassiveTabStyle;
		int	origProgressBarPattern;
		bool	origPurerHover;
		int	origResizerStyle;
		int	origScrollBarStyle;
		int	origScrollerScheme;
		bool	origSplitTabs;
		bool	origSubArrowFrame;
		int	origSubmenuSpeed;
		int	origTabWarnStyle;
		int	origTextboxHilite;
		int	origTipTint;
		int	origTreeViewStyle;
		bool	origWideSlider;

	private:
		ConfigDialog *dialog_;
};

#endif
