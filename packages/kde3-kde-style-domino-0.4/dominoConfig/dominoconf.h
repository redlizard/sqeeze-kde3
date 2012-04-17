/*
Copyright (C) 2006 Michael Lentner <michaell@gmx.net>

based on the Plastik configuration dialog:
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

#ifndef DOMINO_CONF_H
#define DOMINO_CONF_H

#include <../domino/domino.h>
#include <qlineedit.h>


class DSpinBox;
class PreviewCheckBox;
class PreviewRadioButton;
class PreviewTabBar;
class IndicatorPrevButton;
class IndicatorPrevWidget;
class DominoKTabWidget;
class TextEffectPrevWidget;
class TextEffectPrevButton;
class DominoKTabPrevWidget;
class TabWidgetIndicator;
class DominoStyleConfig;

DominoStyleConfig* styleConfig;

struct DSurface;

class SchemeEntry {
	public:
		SchemeEntry(const QString &_fileName, const QString &_schemeName, const bool &_deletable)
	: fileName(_fileName), schemeName(_schemeName), deletable(_deletable) { }
		
		QString getSchemeName() const { return schemeName; }
		QString getFileName() const { return fileName; }
		bool isDeletable() const { return deletable; }
		
	private:
		QString fileName;
		QString schemeName;
		bool deletable;
};

class SchemeList : public QPtrList<SchemeEntry> {
	public:
		SchemeList()
			{ setAutoDelete(true); }
};

class DominoStyleConfig: public QWidget
{
	Q_OBJECT
public:
	DominoStyleConfig(QWidget* parent);
	~DominoStyleConfig();
	
	QFrame* frame3;
	QListBox* listBox1;
	QHeader* header1;

	DominoKTabWidget* tabWidget2;
	QWidget* tab1;
	QGroupBox* groupBox22;
	QLabel* textLabel1_2;
	KColorButton* kColorButton27_2;
	QGroupBox* groupBox10_2_2_2;
	QPushButton* pushButton16;
	QGroupBox* groupBox8_2_2_2;
	QLabel* textLabel4_3_2_2_2;
	DSpinBox* spinBox2_2_2_2_2;
	QLabel* textLabel2_2_2_2_2;
	KColorButton* kColorButton1_2_2_2_2;
	QLabel* textLabel3_3_2_2_2;
	DSpinBox* spinBox1_2_2_2_2;
	QLabel* textLabel1_4_2_2_2;
	KColorButton* kColorButton2_4_2_2_2;
	QGroupBox* groupBox9_2_2_2;
	QLabel* textLabel4_4_2_2_2;
	DSpinBox* spinBox2_3_2_2_2;
	QLabel* textLabel3_4_2_2_2;
	DSpinBox* spinBox1_3_2_2_2;
	KColorButton* kColorButton1_3_2_2_2;
	KColorButton* kColorButton2_5_2_2_2;
	QLabel* textLabel1_5_2_2_2;
	QLabel* textLabel2_3_2_2_2;
	QLabel* textLabel7_4;
	QComboBox* comboBox3_4;
	QPushButton* pushButton4_4;
	QWidget* tab2;
	QGroupBox* groupBox22_2;
	QGroupBox* groupBox8_2_2_2_2;
	QLabel* textLabel4_3_2_2_2_2;
	DSpinBox* spinBox2_2_2_2_2_2;
	QLabel* textLabel2_2_2_2_2_2;
	KColorButton* kColorButton1_2_2_2_2_2;
	QLabel* textLabel3_3_2_2_2_2;
	DSpinBox* spinBox1_2_2_2_2_2;
	QLabel* textLabel1_4_2_2_2_2;
	KColorButton* kColorButton2_4_2_2_2_2;
	QGroupBox* groupBox9_2_2_2_2;
	QLabel* textLabel4_4_2_2_2_2;
	DSpinBox* spinBox2_3_2_2_2_2;
	QLabel* textLabel3_4_2_2_2_2;
	DSpinBox* spinBox1_3_2_2_2_2;
	KColorButton* kColorButton1_3_2_2_2_2;
	KColorButton* kColorButton2_5_2_2_2_2;
	QLabel* textLabel1_5_2_2_2_2;
	QLabel* textLabel2_3_2_2_2_2;
	QLabel* textLabel1_2_2;
	KColorButton* kColorButton27_2_2;
	QGroupBox* groupBox10_2_2_2_2;
	QComboBox* comboBox17;
	DSpinBox* spinBox44;
	QLabel* textLabel7_4_2;
	QComboBox* comboBox3_4_2;
	QPushButton* pushButton4_4_2;
	QWidget* tab3;
	QGroupBox* groupBox22_2_2;
	QGroupBox* groupBox8_2_2_2_2_4;
	QLabel* textLabel4_3_2_2_2_2_4;
	DSpinBox* spinBox2_2_2_2_2_2_4;
	QLabel* textLabel2_2_2_2_2_2_4;
	KColorButton* kColorButton1_2_2_2_2_2_4;
	QLabel* textLabel3_3_2_2_2_2_4;
	DSpinBox* spinBox1_2_2_2_2_2_4;
	QLabel* textLabel1_4_2_2_2_2_4;
	KColorButton* kColorButton2_4_2_2_2_2_4;
	QGroupBox* groupBox9_2_2_2_2_4;
	QLabel* textLabel4_4_2_2_2_2_4;
	DSpinBox* spinBox2_3_2_2_2_2_4;
	QLabel* textLabel3_4_2_2_2_2_4;
	DSpinBox* spinBox1_3_2_2_2_2_4;
	KColorButton* kColorButton1_3_2_2_2_2_4;
	KColorButton* kColorButton2_5_2_2_2_2_4;
	QLabel* textLabel1_5_2_2_2_2_4;
	QLabel* textLabel2_3_2_2_2_2_4;
	QGroupBox* groupBox10_2_2_2_2_4;
	QScrollBar* scrollBar5;
	QScrollBar* scrollBar6;
	KColorButton* kColorButton27_2_2_2;
	QLabel* textLabel1_2_2_2;
	QComboBox* comboBox7;
	QLabel* textLabel7_4_2_3;
	QComboBox* comboBox3_4_2_3;
	QPushButton* pushButton4_4_2_3;
	QWidget* tab4;
	QGroupBox* groupBox22_2_2_3;
	KColorButton* kColorButton27_2_2_2_2;
	QLabel* textLabel1_2_2_2_2;
	QGroupBox* groupBox10_2_2_2_2_4_3;
	QGroupBox* groupBox8_2_2_2_2_4_3;
	QLabel* textLabel4_3_2_2_2_2_4_3;
	DSpinBox* spinBox2_2_2_2_2_2_4_3;
	QLabel* textLabel2_2_2_2_2_2_4_3;
	KColorButton* kColorButton1_2_2_2_2_2_4_3;
	QLabel* textLabel3_3_2_2_2_2_4_3;
	DSpinBox* spinBox1_2_2_2_2_2_4_3;
	QLabel* textLabel1_4_2_2_2_2_4_3;
	KColorButton* kColorButton2_4_2_2_2_2_4_3;
	QGroupBox* groupBox9_2_2_2_2_4_3;
	QLabel* textLabel4_4_2_2_2_2_4_3;
	DSpinBox* spinBox2_3_2_2_2_2_4_3;
	QLabel* textLabel3_4_2_2_2_2_4_3;
	DSpinBox* spinBox1_3_2_2_2_2_4_3;
	KColorButton* kColorButton1_3_2_2_2_2_4_3;
	KColorButton* kColorButton2_5_2_2_2_2_4_3;
	QLabel* textLabel1_5_2_2_2_2_4_3;
	QLabel* textLabel2_3_2_2_2_2_4_3;
	QLabel* textLabel7_4_2_3_3;
	QComboBox* comboBox3_4_2_3_3;
	QPushButton* pushButton4_4_2_3_3;
	QWidget* tab5;
	QGroupBox* groupBox22_2_2_3_2;
	QGroupBox* groupBox8_2_2_2_2_4_3_2;
	QLabel* textLabel4_3_2_2_2_2_4_3_2;
	DSpinBox* spinBox2_2_2_2_2_2_4_3_2;
	QLabel* textLabel2_2_2_2_2_2_4_3_2;
	KColorButton* kColorButton1_2_2_2_2_2_4_3_2;
	QLabel* textLabel3_3_2_2_2_2_4_3_2;
	DSpinBox* spinBox1_2_2_2_2_2_4_3_2;
	QLabel* textLabel1_4_2_2_2_2_4_3_2;
	KColorButton* kColorButton2_4_2_2_2_2_4_3_2;
	KColorButton* kColorButton27_2_2_2_3;
	QLabel* textLabel1_2_2_2_3;
	QGroupBox* groupBox10_2_2_2_2_4_3_2;
	PreviewRadioButton* radioButton2;
	PreviewCheckBox* checkBox2;
	QGroupBox* groupBox9_2_2_2_2_4_3_2;
	QLabel* textLabel4_4_2_2_2_2_4_3_2;
	DSpinBox* spinBox2_3_2_2_2_2_4_3_2;
	QLabel* textLabel3_4_2_2_2_2_4_3_2;
	DSpinBox* spinBox1_3_2_2_2_2_4_3_2;
	KColorButton* kColorButton1_3_2_2_2_2_4_3_2;
	KColorButton* kColorButton2_5_2_2_2_2_4_3_2;
	QLabel* textLabel1_5_2_2_2_2_4_3_2;
	QLabel* textLabel2_3_2_2_2_2_4_3_2;
	QLabel* textLabel7_4_2_3_3_2;
	QComboBox* comboBox3_4_2_3_3_2;
	QPushButton* pushButton4_4_2_3_3_2;
// 	KTabWidget *prevTopTabWidget;
// 	KTabWidget *prevBottomTabWidget;
	TabWidgetIndicator* prevBottomPage1;
	TabWidgetIndicator* prevBottomPage2;
	TabWidgetIndicator* prevBottomPage3;
	TabWidgetIndicator* prevTopPage1;
	TabWidgetIndicator* prevTopPage2;
	TabWidgetIndicator* prevTopPage3;
	PreviewTabBar* tabBarBottom;
	PreviewTabBar* tabBarTop;
	QHBoxLayout* tabComboLayout;
	
	
	// tab 3 - load/save
// 	KTabWidget* tabWidget3;
	QWidget* tab;
	QGroupBox* groupBox_ls;
	QListView* listView_ls;
	QPushButton* loadButton;
	QPushButton* saveButton;
	QPushButton* deleteButton;
	QPushButton* importButton;
	QWidget* tab_2;
	
	// focusIndicator
	QGroupBox* indicateFocusFrame;
	QCheckBox* indicateFocus;
	QLabel* labelIndicatorColor;
	QLabel* labelIndicatorBtnColor;
	IndicatorPrevWidget* indicatorPrevWidget;
	IndicatorPrevButton* indicatorPrevButton;
	QLabel* labelIndicatorColorOpacity;
	QLabel* labelIndicatorBtnColorOpacity;
	
	QGroupBox* gb1;
	QGroupBox* gb2;
	QGroupBox* gb3;
	
	// text effect
	QGroupBox* textEffectFrame;
	QLabel* labelTextEffectColor;
	QLabel* labelTextEffectButtonColor;
	TextEffectPrevWidget* textEffectPrevWidget;
	TextEffectPrevButton* textEffectPrevButton;
	QLabel* labelTextEffectColorOpacity;
	QLabel* labelTextEffectButtonColorOpacity;
	QLabel* labelTextEffectPos;
	QLabel* labelTextEffectButtonPos;
	
	QCheckBox* drawButtonSunkenShadow;
	
	QLabel* buttonTypLabel;
	QLabel* labelButtonContourColors;
	QPushButton* buttonContourPrevButton;
	QLabel* labelPopupMenuOptions;
	
signals:
	void changed(bool);

public slots:
	void save();
	void defaults();
	void userLoadTabConfig();
	
protected slots:
	void updateChanged();
	virtual void languageChange();
	void userLoadScrollBarConfig();
	void testCanDecode(const QDragMoveEvent * e, bool & accept);
	
private slots:
	void btnPv1(const QColor&);
	void btnPv2(const QColor&);
	void btnPv3(const QColor&);
	void btnPv4(const QColor&);
	void btnPv5(const QColor&);
	void btnPv6(int);
	void btnPv7(int);
	void btnPv8(int);
	void btnPv9(int);
	
	void hPv1(const QColor&);
	void hPv2(const QColor&);
	void hPv3(const QColor&);
	void hPv4(const QColor&);
	void hPv5(const QColor&);
	void hPv6(int);
	void hPv7(int);
	void hPv8(int);
	void hPv9(int);
	
	void chbPv1(const QColor&);
	void chbPv2(const QColor&);
	void chbPv3(const QColor&);
	void chbPv4(const QColor&);
	void chbPv5(const QColor&);
	void chbPv6(int);
	void chbPv7(int);
	void chbPv8(int);
	void chbPv9(int);
	
	void sbPv1(const QColor&);
	void sbPv2(const QColor&);
	void sbPv3(const QColor&);
	void sbPv4(const QColor&);
	void sbPv5(const QColor&);
	void sbPv6(int);
	void sbPv7(int);
	void sbPv8(int);
	void sbPv9(int);
	
	void tabPv1(const QColor&col);
	void tabPv2(const QColor&col);
	void tabPv3(const QColor&col);
	void tabPv4(const QColor&col);
	void tabPv5(const QColor&col);
	void tabPv6(int);
	void tabPv7(int);
	void tabPv8(int);
	void tabPv9(int);
	
	
	void slotLoad();
	void slotSave();
	void slotDelete();
	void slotImport();
	void slotDelPerm(QListViewItem*);
	void scrollBar1GradientChanged();
	void scrollBar2GradientChanged();
	void scrollBarNumGradientsChanged(int g=0);
	void btn1GradientChanged();
	void btn2GradientChanged();
	void btnNumGradientsChanged(int g=0);
	void header1GradientChanged();
	void header2GradientChanged();
	void headerNumGradientsChanged(int g=0);
	void checkItem1GradientChanged();
	void checkItem2GradientChanged();
	void checkItemNumGradientsChanged(int g=0);
	void tab1GradientChanged();
	void tab2GradientChanged();
	void tabNumGradientsChanged(int g=0);
	
	void copyColorsToButton();
	void copyColorsToTab();
	void copyColorsToScrollBar();
	void copyColorsToHeader();
	void copyColorsToCheckItem();
	
	void resetGroupBoxBackgroundColor();
	
	void textEffectModeChanged();
	void indicatorModeChanged();
	
	void grValueLabelNum(int num);
	void updateButtonContourSettings();
	void updateTextEffectSettings();
	void updateCheckItemIndicators();
	void updateFocusIndicatorSettings();
	void updateGroupBoxSettings();
	
	void rubberSelChanged(int);
	void updateArgbRubberOptions(bool);

protected:
	
	bool hasCompositeExtension;
	
	QVBoxLayout* frame3Layout;
	QVBoxLayout* layout1;
	QGroupBox* grFrame;
	QLabel* grValueLabel;
	
	//We store settings directly in widgets to
	//avoid the hassle of sync'ing things
	QCheckBox* animateProgressBar;
	QCheckBox* drawTriangularExpander;
	QCheckBox* customCheckMarkColor;
	KColorButton* checkMarkColor;
	QCheckBox* customToolTipColor;
	KColorButton* toolTipColor;
	QCheckBox* highlightToolBtnIcons;
	QCheckBox* toolBtnAsBtn;
	
	QLabel* labelRubberOptions;
	QRadioButton* argbRubber;
	QRadioButton* lineRubber;
	QRadioButton* distRubber;
	int rubberBandType;
	
	QLabel* labelArgbRubberColor;
	QLabel* labelArgbRubberOpacity;
	KColorButton* argbRubberColor;
	DSpinBox* argbRubberOpacity;
	
	QCheckBox* smoothScrolling;
	DSpinBox* buttonHeightAdjustment;
	QLabel* buttonHeightAdjustmentLabel;
	QCheckBox* customPopupMenuColor;
	KColorButton* popupMenuColor;
	QCheckBox* centerTabs;
	QCheckBox* customSelMenuItemColor;
	KColorButton* selMenuItemColor;
	QCheckBox* drawPopupMenuGradient;
	QCheckBox* indentPopupMenuItems;
	
	//Original settings, for accurate dirtiness tracking
	bool	   origAnimProgressBar;
	bool origDrawTriangularExpander;
	bool origToolBtnAsBtn;
	bool origHighlightToolBtnIcons;
	
	bool origCustomCheckMarkColor;
	QColor origCheckMarkColor;
	bool origTintGroupBoxBackground;
	int origGroupBoxBrightness;
	bool origCustomGroupBoxBackgroundColor;
	QColor origGroupBoxBackgroundColor;
	
	bool origCustomToolTipColor;
	QColor origToolTipColor;
	
	int origRubberBandType;
	QColor origArgbRubberColor;
	int origArgbRubberOpacity;

	bool origSmoothScrolling;
	int origButtonHeightAdjustment;
	int origButtonContourType;
	bool origCustomPopupMenuColor;
	QColor origPopupMenuColor;
	bool origCustomSelMenuItemColor;
	QColor origSelMenuItemColor;
	bool origDrawPopupMenuGradient;
	bool origIndentPopupMenuItems;
	
	bool origCenterTabs;
	bool origIndicateFocus;
	QColor origIndicatorColor;
	QColor origIndicatorBtnColor;
	int origIndicatorOpacity;
	int origIndicatorBtnOpacity;
	
	bool origDrawUnderline;
	int origTextEffectMode;
	QColor origTextEffectColor;
	QColor origTextEffectButtonColor;
	bool origDrawTextEffect;
	int origTextEffectOpacity;
	int origTextEffectButtonOpacity;
	int origTextEffectPos;
	int origTextEffectButtonPos;
	
	bool origDrawButtonSunkenShadow;
	
	QColor origButtonContourColor;
	QColor origButtonDefaultButtonContourColor;
	QColor origButtonMouseOverContourColor;
	QColor origButtonPressedContourColor;
	
	
	int konqTabBarContrast;
	
	QString currentConfig;
	QLabel* currentConfigLabel;

	SchemeList* schemeList;
	bool configLoaded;
	DSurface* currentScrollBarSurface;
	DSurface* currentTabSurface;
	
	void loadConfig(const char* name);
	void saveConfig(const char* fileName, const char* schemeName);
	void getConfigSchemes();
	

	
	//////////////////////////////////////////////// form:
	//protected:
	QVBoxLayout* tab1Layout;
	QGridLayout* groupBox22Layout;
	QSpacerItem* spacer6;
	QSpacerItem* spacer3;
	QSpacerItem* spacer43;
	QSpacerItem* spacer46;
	QSpacerItem* spacer2;
	QVBoxLayout* groupBox10_2_2_2Layout;
	QGridLayout* groupBox8_2_2_2Layout;
	QGridLayout* groupBox9_2_2_2Layout;
	QHBoxLayout* layout9;
	QSpacerItem* spacer7;
	QVBoxLayout* tab2Layout;
	QGridLayout* groupBox22_2Layout;
	QSpacerItem* spacer46_2;
	QSpacerItem* spacer43_2;
	QSpacerItem* spacer6_2;
	QSpacerItem* spacer2_2;
	QSpacerItem* spacer3_2;
	QGridLayout* groupBox8_2_2_2_2Layout;
	QGridLayout* groupBox9_2_2_2_2Layout;
	QVBoxLayout* groupBox10_2_2_2_2Layout;
	QHBoxLayout* layout9_2;
	QSpacerItem* spacer7_2;
	QVBoxLayout* tab3Layout;
	QGridLayout* groupBox22_2_2Layout;
	QSpacerItem* spacer46_2_2;
	QSpacerItem* spacer43_2_2;
	QSpacerItem* spacer3_2_2;
	QSpacerItem* spacer6_2_2;
	QSpacerItem* spacer2_2_2;
	QGridLayout* groupBox8_2_2_2_2_4Layout;
	QGridLayout* groupBox9_2_2_2_2_4Layout;
	QGridLayout* groupBox10_2_2_2_2_4Layout;
	QSpacerItem* spacer35;
	QHBoxLayout* layout6;
	QSpacerItem* spacer7_2_3;
	QVBoxLayout* tab4Layout;
	QGridLayout* groupBox22_2_2_3Layout;
	QSpacerItem* spacer3_2_2_2;
	QSpacerItem* spacer6_2_2_2;
	QSpacerItem* spacer2_2_2_2;
	QSpacerItem* spacer46_2_2_2;
	QSpacerItem* spacer43_2_2_2;
	QVBoxLayout* groupBox10_2_2_2_2_4_3Layout;
	QGridLayout* groupBox8_2_2_2_2_4_3Layout;
	QGridLayout* groupBox9_2_2_2_2_4_3Layout;
	QHBoxLayout* layout21;
	QSpacerItem* spacer7_2_3_3;
	QVBoxLayout* tab5Layout;
	QGridLayout* groupBox22_2_2_3_2Layout;
	QSpacerItem* spacer3_2_2_3;
	QSpacerItem* spacer6_2_2_3;
	QSpacerItem* spacer2_2_2_3;
	QSpacerItem* spacer46_2_2_3;
	QSpacerItem* spacer43_2_2_2_2;
	QGridLayout* groupBox8_2_2_2_2_4_3_2Layout;
	QVBoxLayout* groupBox10_2_2_2_2_4_3_2Layout;
	QGridLayout* groupBox9_2_2_2_2_4_3_2Layout;
	QHBoxLayout* layout22;
	QSpacerItem* spacer7_2_3_3_2;
	
	// tab 3 - load/save
	QVBoxLayout* tab_lsLayout;
	QHBoxLayout* groupBox_lsLayout;
	QVBoxLayout* layout8;
	QSpacerItem* spacer32;
	
};




class DSpinBox : public QSpinBox
{
public:
	DSpinBox(QWidget *parent, const char* name=0): QSpinBox(parent, name){};
	DSpinBox( int minValue, int maxValue, int step = 1, QWidget* parent=0, const char* name=0 ): QSpinBox(minValue, maxValue, step, parent, name){};
	void setAlignRight() {
		this->editor()->setAlignment(Qt::AlignRight);
	}
	QSpinWidget* spinWidget() {
		return static_cast<QSpinWidget*>(editor()->parentWidget());
	}
};

class PreviewCheckBox : public QCheckBox
{
	public:
		PreviewCheckBox(QWidget *parent, const char * name = 0 ): QCheckBox( parent, name){};
		void paintEvent(QPaintEvent *event);
		void drawButton(QPainter *paint);
};

class PreviewRadioButton : public QRadioButton
{
	public:
		PreviewRadioButton(QWidget *parent, const char * name = 0 ): QRadioButton( parent, name){};
		void paintEvent(QPaintEvent *event);
		void drawButton(QPainter *paint);
};


class PreviewTabBar : public QTabBar
{
	public:
		PreviewTabBar(QWidget *parent, const char * name = 0 ): QTabBar( parent, name){};
		void mousePressEvent(QMouseEvent* event);
		void paintEvent(QPaintEvent *event);
};

class IndicatorPrevButton : public QPushButton
{
	public:
		IndicatorPrevButton(QWidget *parent, const char * name = 0 ): QPushButton( parent, name){};
		void paintEvent(QPaintEvent *event);
};

class IndicatorPrevWidget : public QPushButton /*QWidget*/
{
	public:
		IndicatorPrevWidget(QWidget *parent, const char * name = 0) : QPushButton( parent, name){};
		void paintEvent(QPaintEvent *event);
};

class DominoKTabWidget : public KTabWidget
{
	Q_OBJECT
	friend class DominoStyleConfig;
	public:
		DominoKTabWidget( QWidget* parent=0, const char* name=0 ) : KTabWidget( parent, name ) {}
	protected slots:
		void wheelDelta( int ) {};
};

class DominoKTabPrevWidget : public KTabWidget
{
	friend class DominoStyleConfig;
	friend class TabWidgetIndicator;
	public:
		DominoKTabPrevWidget( QWidget* parent=0, const char* name=0 ) : KTabWidget( parent, name ) {}
};

class TabWidgetIndicator : public QWidget
{
	friend class DominoStyleConfig;
	public:
		TabWidgetIndicator( QWidget* parent=0, const char* name=0 ) : QWidget( parent, name ) {}
		void paintEvent(QPaintEvent *event);
};

class TextEffectPrevButton : public QPushButton
{
	public:
		TextEffectPrevButton(QWidget *parent, const char * name = 0 ): QPushButton( parent, name){};
		void paintEvent(QPaintEvent *event);
};

class TextEffectPrevWidget : public QWidget
{
	public:
		TextEffectPrevWidget(QWidget *parent, const char * name = 0, uint flags = 0 ): QWidget( parent, name, flags){};
		void paintEvent(QPaintEvent *event);
};

class PreviewStyle : public DominoStyle
{
	friend class IndicatorPrevButton;
	friend class DominoStyleConfig;
	public:
};
PreviewStyle* previewStyle;

DSurface origBtnSurface;
DSurface origTabTopSurface;
DSurface origActiveTabTopSurface;
DSurface origTabBottomSurface;
DSurface origActiveTabBottomSurface;
DSurface origScrollBarSurface;
DSurface origScrollBarGrooveSurface;
DSurface origHeaderSurface;
DSurface origCheckItemSurface;

// global
bool initialized;

QComboBox* indicatorModeCombo;
KColorButton* indicatorBtnColor;
KColorButton* indicatorColor;
QString indicatorPreviewText;
QSpinBox* indicatorOpacity;
QSpinBox* indicatorBtnOpacity;

QCheckBox* tintGroupBoxBackground;
QCheckBox* customGroupBoxBackgroundColor;
KColorButton* groupBoxBackgroundColor;
QSlider* grSlider;

// text effect
QComboBox* textEffectModeCombo;
QCheckBox* drawTextEffect;
KColorButton* textEffectColor;
KColorButton* textEffectButtonColor;
QString textEffectPreviewText;
QSpinBox* textEffectOpacity;
QSpinBox* textEffectButtonOpacity;

QComboBox* textEffectPos;
QComboBox* textEffectButtonPos;

// button contour colors
QComboBox* buttonContourType;

KColorButton* buttonContourColor;
KColorButton* buttonDefaultButtonContourColor;
KColorButton* buttonMouseOverContourColor;
KColorButton* buttonPressedContourColor;

QComboBox* tabPosCombo;
QComboBox* tabStateCombo;
DominoKTabPrevWidget *prevTopTabWidget;
DominoKTabPrevWidget *prevBottomTabWidget;

enum DominoStyleFlags {
	Domino_noCache = 0x80000000
};
#endif
