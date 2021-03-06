
#include "kchartWizardSelectDataFormatPage.h"
#include "kchart_view.h"
#include "kchart_part.h"

#include <qhbox.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qbuttongroup.h>
#include <qvbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qpainter.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdialog.h>

#include "kchart_params.h"

namespace KChart
{

KChartWizardSelectDataFormatPage::KChartWizardSelectDataFormatPage( QWidget* parent,
                                                                        KChartPart* chart ) :
  QWidget( parent ),
  m_chart( chart )
{
    QGridLayout *grid1 = new QGridLayout(this, 6, 1, KDialog::marginHint(),
					 KDialog::spacingHint());

    // The Data Area
    QButtonGroup *gb1 = new QVButtonGroup( i18n( "Data Area" ), this );

    QHBox   *hbox = new QHBox( gb1 );
    (void) new QLabel( i18n("Area: "), hbox);
    m_dataArea = new QLineEdit( hbox );
    grid1->addWidget(gb1, 0, 0);

    // The row/column as label checkboxes. 
    m_firstRowAsLabel = new QCheckBox( i18n( "First row as label" ), gb1);
    m_firstColAsLabel = new QCheckBox( i18n( "First column as label" ), gb1);

    // The Data Format button group
    QButtonGroup *gb = new QVButtonGroup( i18n( "Data Format" ), this );

    m_rowMajor = new QRadioButton( i18n( "Data in rows" ), gb );
    m_rowMajor->resize( m_rowMajor->sizeHint() );

    m_colMajor = new QRadioButton( i18n( "Data in columns" ), gb );
    m_colMajor->resize( m_colMajor->sizeHint() );

    grid1->addWidget(gb, 3, 0);

    QLabel *lbl = new QLabel( i18n( 
        "\n"
	"If the selected data area does not match the data you want,\n"
	"select the data now.\n"
	"\n"
	"Include cells that you want to use as row and column labels,\n"
	"if you want them in the chart.\n"
	), this);
    grid1->addWidget(lbl, 4, 0);

    grid1->setColStretch(5, 0);

    grid1->activate();

    // Enter the data into the widgets.
    if ( m_chart->params()->dataDirection() == KChartParams::DataColumns)
      m_colMajor->setChecked(true);
    else
      m_rowMajor->setChecked(true);

    m_dataArea->setText( m_chart->params()->dataArea() );
}


QString KChartWizardSelectDataFormatPage::dataArea() const
{
    return m_dataArea->text();
}

void KChartWizardSelectDataFormatPage::setDataArea( const QString &area )
{
    m_dataArea->setText( area );
}


void KChartWizardSelectDataFormatPage::apply()
{
    if (m_rowMajor->isChecked())
      m_chart->params()->setDataDirection( KChartParams::DataRows );
    else
      m_chart->params()->setDataDirection( KChartParams::DataColumns );

    m_chart->params()->setFirstRowAsLabel( m_firstRowAsLabel->isChecked() );
    m_chart->params()->setFirstColAsLabel( m_firstColAsLabel->isChecked() );

    m_chart->params()->setDataArea( m_dataArea->text() );
    // FIXME: Actually take use the new data area.
}


}  //KChart namespace

#include "kchartWizardSelectDataFormatPage.moc"
