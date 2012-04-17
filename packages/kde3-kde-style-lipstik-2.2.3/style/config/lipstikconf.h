/*
Copyright (C) 2004-2007 Patrice Tremblay <tremblaypatrice@yahoo.fr>
			http://www.poplix.info/lipstik

based on plastik:
Copyright (C) 2003 Sandro Giessl <ceebx@users.sourceforge.net>

based on the Keramik configuration dialog:
Copyright (c) 2003 Maksim Orlovich <maksim.orlovich@kdemail.net>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#ifndef LIPSTIK_CONF_H
#define LIPSTIK_CONF_H

class QCheckBox;

class LipstikStyleConfig: public QWidget
{
	Q_OBJECT
public:
	LipstikStyleConfig(QWidget* parent);
	~LipstikStyleConfig();

	//This signal and the next two slots are the plugin
	//page interface
signals:
	void changed(bool);

public slots:
	void save();
	void defaults();

	//Everything below this is internal.
protected slots:
	void updateChanged();

protected:
	//We store settings directly in widgets to
	//avoid the hassle of sync'ing things
	QCheckBox* 	 animateProgressBar;
	QCheckBox* 	 reverseGradients;

	QString 	 scrollBarType( int listnr );
	int	   	 scrollBarItem( QString kSBType );
	QComboBox* 	 scrollBarStyle;

	QString 	 scrollBarWidthType( int widthListnr );
	int	   	 scrollBarWidthItem( QString kSBWidth );
	QComboBox* 	 scrollBarWidth;

	QString 	 sliderType( int sliderlistnr );
	int	   	 sliderItem( QString kSliderType );
	QComboBox* 	 sliderStyle;

	QSpinBox*	 menuSpacing;
	QSpinBox*	 toolBarSpacing;

	QCheckBox* 	 paintGroove;
	QCheckBox* 	 sharperMenuGradient;
	QCheckBox* 	 alterMenuGradients;
	QCheckBox* 	 menuLeftSideBar;
	QCheckBox* 	 drawStatusBarFrame;
	QCheckBox* 	 drawToolBarHandle;
	QCheckBox* 	 comboboxColored;
	QCheckBox* 	 coloredMenuBar;
	QCheckBox* 	 kickerHighlight;
	QCheckBox* 	 drawTabHighlight;
	QCheckBox* 	 flatStripe;
	QCheckBox* 	 compressedMenus;
	QCheckBox* 	 drawToolBarSeparator;
	QCheckBox* 	 drawToolBarGradient;
	QCheckBox* 	 invertBarGrad;
	QCheckBox*       drawToolBarItemSeparator;
 	QCheckBox*       drawFocusRect;
 	QCheckBox*       newKickerButtons;
 	QCheckBox*       flatHeaders;
 	QCheckBox*       tickCheckMark;
	QCheckBox*       drawTriangularExpander;
	QCheckBox*       inputFocusHighlight;
	QCheckBox*       customOverHighlightColor;
	KColorButton*    overHighlightColor;

	QCheckBox*       customMenuStripeColor;
	KColorButton*    menuStripeColor;

	QCheckBox*       customFocusHighlightColor;
	KColorButton*    focusHighlightColor;
	QCheckBox*       customCheckMarkColor;
	KColorButton*    checkMarkColor;
	QCheckBox*       customTabHighlightColor;
	KColorButton*    tabHighlightColor;

	//Original settings, for accurate dirtiness tracking
	bool       origAnimProgressBar;
    	bool       origReverseGradients;
    	bool       origPaintGroove;
    	bool       origSharperMenuGradient;
    	bool       origAlterMenuGradients;
    	bool       origMenuLeftSideBar;
    	bool       origDrawStatusBarFrame;
    	bool       origDrawToolBarHandle;
	bool       origComboboxColored;
	bool       origColoredMenuBar;
	bool       origKickerHighlight;
	bool       origDrawTabHighlight;
    	bool       origFlatStripe;
    	bool       origCompressedMenus;
	bool       origDrawToolBarSeparator;
	bool       origDrawToolBarGradient;
	bool       origInvertBarGrad;
	bool       origDrawToolBarItemSeparator;
 	bool       origDrawFocusRect;
 	bool       origNewKickerButtons;
 	bool       origFlatHeaders;
 	bool       origTickCheckMark;
	bool       origDrawTriangularExpander;
	bool       origInputFocusHighlight;
	bool       origCustomOverHighlightColor;
	QColor     origOverHighlightColor;

	bool       origCustomMenuStripeColor;
	QColor     origMenuStripeColor;

	bool       origCustomFocusHighlightColor;
	QColor     origFocusHighlightColor;
	bool       origCustomCheckMarkColor;
	QColor     origCheckMarkColor;
	bool       origCustomTabHighlightColor;
	QColor     origTabHighlightColor;
	QString    origScrollBarStyle;
	QString    origScrollBarWidth;
	QString    origSliderStyle;

	int        origMenuSpacing;
	int        origToolBarSpacing;
};

#endif
