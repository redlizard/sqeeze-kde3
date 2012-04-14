
#include "rubyconfigwidget.h"
#include "domutil.h"

#include <stdlib.h>

#include <klineedit.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <kurlrequester.h>
#include <kurlcompletion.h>
#include <kdebug.h>

RubyConfigWidget::RubyConfigWidget(QDomDocument &projectDom, QWidget* parent, const char* name)
: RubyConfigWidgetBase(parent,name), dom (projectDom) {
    kdDebug (9019) << "Creating RubyConfigWidget" << endl;
    interpreterEdit->setText(DomUtil::readEntry(dom, "/kdevrubysupport/run/interpreter"));
    if (interpreterEdit->text().isEmpty()) {
        interpreterEdit->setText("ruby");
    }
    shellEdit->setText(DomUtil::readEntry(dom, "/kdevrubysupport/run/shell"));
    if (shellEdit->text().isEmpty()) {
        shellEdit->setText("irb");
    }
    mainProgramEdit->setText(DomUtil::readEntry(dom, "/kdevrubysupport/run/mainprogram"));
    programArgsEdit->setText(DomUtil::readEntry(dom, "/kdevrubysupport/run/programargs"));
    runRadioBox->setButton(DomUtil::readIntEntry(dom, "/kdevrubysupport/run/runmainprogram"));
    terminalCheckbox->setChecked(DomUtil::readBoolEntry(dom, "/kdevrubysupport/run/terminal"));
    characterCodingRadioBox->setButton(DomUtil::readIntEntry(dom, "/kdevrubysupport/run/charactercoding"));
    enableFloatingToolBarBox->setChecked( DomUtil::readBoolEntry(dom, "/kdevrbdebugger/general/floatingtoolbar", false));
    showConstants->setChecked( DomUtil::readBoolEntry(dom, "/kdevrbdebugger/general/showconstants", false));
    traceIntoRuby->setChecked( DomUtil::readBoolEntry(dom, "/kdevrbdebugger/general/traceintoruby", false));
    workingDir->completionObject()->setMode(KURLCompletion::DirCompletion);
    workingDir->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );
    workingDir->setURL(DomUtil::readEntry(dom, "/kdevscriptproject/run/globalcwd", "") );
}

void RubyConfigWidget::accept() {
    DomUtil::writeEntry(dom, "/kdevrubysupport/run/interpreter", interpreterEdit->text());
    DomUtil::writeEntry(dom, "/kdevrubysupport/run/shell", shellEdit->text());
    DomUtil::writeEntry(dom, "/kdevrubysupport/run/mainprogram", mainProgramEdit->text());
    DomUtil::writeEntry(dom, "/kdevrubysupport/run/programargs", programArgsEdit->text());
    DomUtil::writeIntEntry(dom, "/kdevrubysupport/run/runmainprogram", runRadioBox->selectedId());
    DomUtil::writeBoolEntry(dom, "/kdevrubysupport/run/terminal", terminalCheckbox->isChecked());
    DomUtil::writeIntEntry(dom, "/kdevrubysupport/run/charactercoding", characterCodingRadioBox->selectedId());
    DomUtil::writeBoolEntry(dom, "/kdevrbdebugger/general/floatingtoolbar", enableFloatingToolBarBox->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevrbdebugger/general/showconstants", showConstants->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevrbdebugger/general/traceintoruby", traceIntoRuby->isChecked());
    DomUtil::writeEntry(dom, "/kdevscriptproject/run/globalcwd", workingDir->url() );
}

#include "rubyconfigwidget.moc"

