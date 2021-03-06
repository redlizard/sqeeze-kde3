/* This file is part of the KDE project
   Copyright (C) 1999 Matthias Kalle Dalheimer <kalle@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "kchartColorConfigPage.h"
#include "kchartColorConfigPage.moc"

#include <kapplication.h>
#include <kdialog.h>
#include <klocale.h>
#include <kcolorbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qwhatsthis.h>
#include <qhbox.h>
#include <klistbox.h>
#include "kchart_params.h"
#include <kdebug.h>

namespace KChart
{

KChartColorConfigPage::KChartColorConfigPage( KChartParams* params,
                                              QWidget* parent, 
					      KDChartTableData *dat ) :
    QWidget( parent ),
    m_params( params ),
    m_data( dat ),
    index( 0 )
{
    QWhatsThis::add( this, i18n( "This page lets you configure the colors "
                                 "in which your chart is displayed. Each "
                                 "part of the chart can be assigned a "
                                 "different color." ) );

    QVBoxLayout* toplevel = new QVBoxLayout( this, 14 );
    QButtonGroup* gb = new QButtonGroup( 0, Qt::Vertical, i18n("Colors"), this );
    gb->layout()->setSpacing(KDialog::spacingHint());
    gb->layout()->setMargin(KDialog::marginHint());
    toplevel->addWidget( gb);
    QString wtstr;
    QGridLayout* grid = new QGridLayout( gb->layout(), 8, 3 );

    const int labelAlign = AlignRight | AlignVCenter;
    int row = -1;
#define ADD_COLOR_BTN(btn,title,whatsthis) \
{                                          \
    ++row;                                 \
    QLabel* l = new QLabel( title, gb );   \
    l->setAlignment( labelAlign );         \
    grid->addWidget( l, row, 0 );          \
    btn = new KColorButton( gb );          \
    l->setBuddy( btn );                    \
    grid->addWidget( btn, row, 1 );        \
    QWhatsThis::add( l, whatsthis );       \
    QWhatsThis::add( btn, whatsthis );     \
}
    ADD_COLOR_BTN(_lineCB, i18n( "&Line color:" ),
        i18n( "This is the color that is used for drawing lines like axes." ))
    ADD_COLOR_BTN(_gridCB, i18n( "&Grid color:" ),
        i18n( "Here you can configure the color that is used for the "
              "chart grid. Of course, this setting will only "
              "take effect if grid drawing is turned on." ))
    ADD_COLOR_BTN(_xtitleCB, i18n( "&X-title color:" ),
        i18n( "This color is used for displaying titles for the "
              "X (horizontal) axis." ))
    ADD_COLOR_BTN(_ytitleCB, i18n( "&Y-title color:" ),
        i18n( "This color is used for displaying titles for the "
              "Y (vertical) axis." ))
#if 0
    ADD_COLOR_BTN(_ytitle2CB, i18n( "Y-title color (2nd axis):" ),
        i18n( "This color is used for displaying titles for the "
              "second Y (vertical) axis. It only takes effect if the "
              "chart is configured to have a second Y axis." ))
#endif
    ADD_COLOR_BTN(_xlabelCB, i18n( "X-label color:" ),
        i18n( "Here you can configure the color that is used for "
              "labeling the X (horizontal) axis" ))
    ADD_COLOR_BTN(_ylabelCB, i18n( "Y-label color:" ),
        i18n( "Here you can configure the color that is used for "
              "labeling the Y (vertical) axis" ))
#if 0
    ADD_COLOR_BTN(_ylabel2CB, i18n( "Y-label color (2nd axis):" ),
        i18n( "Here you can configure the color that is used for "
              "labeling the second Y (vertical) axis. Of course, "
              "this setting only takes effect if the chart is "
              "configured to have two vertical axes." ))
#endif
    ADD_COLOR_BTN(_xlineCB, i18n( "X-line color:" ),
        i18n( "Here you can configure the line color of the X (horizontal) axis" ))
    ADD_COLOR_BTN(_ylineCB, i18n( "Y-line color:" ),
        i18n( "Here you can configure the line color of the Y (vertical) axis" ))
#if 0
    ADD_COLOR_BTN(_yline2CB, i18n( "Y-line color (2nd axis):" ),
        i18n( "Here you can configure the line color of "
              "the second Y (vertical) axis. Of course, "
              "this setting only takes effect if the chart is "
              "configured to have two vertical axes." ))
#endif
    ADD_COLOR_BTN(_xzerolineCB, i18n( "X-Zero-line color:" ),
        i18n( "Here you can configure the zero-line's color of the X "
              "(horizontal) axis. Of course, this setting only takes "
              "effect if the abscissa is displaying a Zero-line." ))
    ADD_COLOR_BTN(_yzerolineCB, i18n( "Y-Zero-line color:" ),
        i18n( "Here you can configure the zero-line's color of the Y (vertical) axis" ))
#if 0
    ADD_COLOR_BTN(_yzeroline2CB, i18n( "Y-Zero-line color (2nd axis):" ),
        i18n( "Here you can configure the color that is used for "
              "the Zero-line of the second Y (vertical) axis. Of course, "
              "this setting only takes effect if the chart is "
              "configured to have two vertical axes." ))
#endif

    QHBox* dataColorHB = new QHBox( gb );
    grid->addMultiCellWidget( dataColorHB,  0, row, 2, 2 );
    _dataColorLB = new KListBox(dataColorHB);
    _dataColorCB = new KColorButton( dataColorHB);
    wtstr = i18n( "Choose a row/column in the list on the left and change its color using this button.");
    QWhatsThis::add( _dataColorCB, wtstr );
    initDataColorList();
    connect( _dataColorLB, SIGNAL(highlighted(int )), this, SLOT(changeIndex(int)));
    connect( _dataColorLB, SIGNAL(doubleClicked ( QListBoxItem * )), this, SLOT(activeColorButton()));



    /*QLabel* edgeLA = new QLabel( i18n( "Edge color (pies only)" ), this );
      edgeLA->setAlignment( AlignRight | AlignVCenter );
      grid->addWidget( edgeLA, 6, 0 );
      _edgeCB = new KColorButton( this );
      grid->addWidget( _edgeCB, 6, 1 );*/

//     for( int i = 0; i < NUMDATACOLORS; i++ ) {
// 	QString labeltext;
// 	labeltext.sprintf( i18n( "Data color #%d:"), i );
// 	QLabel* dataLA = new QLabel( labeltext, this );
// 	dataLA->setAlignment(AlignRight | AlignVCenter);
// 	dataLA->resize( dataLA->sizeHint() );
// 	grid->addWidget( dataLA, i, 2 );
// 	_dataCB[i] = new KColorButton( this );
// 	_dataCB[i]->resize( _dataCB[i]->sizeHint() );
// 	grid->addWidget( _dataCB[i], i, 3 );
// 	grid->addRowSpacing(i,_textCB->height());
// 	grid->setRowStretch(i,0);
// 	grid->addColSpacing(2,dataLA->width() + 20);
//     }
}


void KChartColorConfigPage::changeIndex(int newindex)
{
    if(index > m_params->maxDataColor())
        _dataColorLB->setEnabled(false);
    else
    {
        if(!_dataColorCB->isEnabled())
            _dataColorCB->setEnabled(true);
        extColor[index] = _dataColorCB->color();
        _dataColorCB->setColor(extColor[newindex]);
        index=newindex;
    }
}


void KChartColorConfigPage::activeColorButton()
{
    _dataColorCB->animateClick();
}


void KChartColorConfigPage::initDataColorList()
{
    QStringList lst;
    for(uint i = 0; i < m_data->rows(); i++)
    {
        extColor.resize( m_params->maxDataColor() );
        if(i<m_params->maxDataColor())
        {
            _dataColorLB->insertItem(m_params->legendText( i ).isEmpty() ? i18n("Series %1").arg(i+1) :m_params->legendText( i ) );
            extColor[i] =m_params->dataColor(i);
        }
    }
    _dataColorLB->setCurrentItem(0);

	 //Fix crash when we didn't create data
	if ( m_data->rows() == 0 )
		_dataColorCB->setEnabled(false);
	else
    	_dataColorCB->setColor( extColor[index]);
}


void KChartColorConfigPage::apply()
{
	//Nothing to save
	if ( m_data->rows() == 0 )
		return;

    extColor[index] = _dataColorCB->color();
    for(uint i =0;i<m_data->rows();i++)
        if(i<m_params->maxDataColor())
            m_params->setDataColor(i,extColor[i]);
}

}  //KChart namespace
