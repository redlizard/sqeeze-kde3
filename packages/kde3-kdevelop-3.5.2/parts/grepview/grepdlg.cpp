/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann and the KDevelop Team       *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "grepdlg.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qhbox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qstringlist.h>
#include <kfiledialog.h>
#include <kbuttonbox.h>
#include <kpushbutton.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <qlabel.h>
#include <kcombobox.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>
#include <kstdguiitem.h>
#include <kparts/part.h>
#include <kdevpartcontroller.h>
#include <klineedit.h>

#include "grepviewpart.h"


const char *template_desc[] = {
    "verbatim",
    "assignment",
    "->MEMBER(",
    "class::MEMBER(",
    "OBJECT->member(",
    0
};

const char *template_str[] = {
    "%s",
    "\\<%s\\>[\\t ]*=[^=]",
    "\\->[\\t ]*\\<%s\\>[\\t ]*\\(",
    "[a-z0-9_$]+[\\t ]*::[\\t ]*\\<%s\\>[\\t ]*\\(",
    "\\<%s\\>[\\t ]*\\->[\\t ]*[a-z0-9_$]+[\\t ]*\\(",
    0
};

const char *filepatterns[] = {
    "*.h,*.hxx,*.hpp,*.hh,*.h++,*.H,*.tlh,*.cpp,*.cc,*.C,*.c++,*.cxx,*.ocl,*.inl,*.idl,*.c,*.m,*.mm,*.M",
    "*.cpp,*.cc,*.C,*.c++,*.cxx,*.ocl,*.inl,*.c,*.m,*.mm,*.M",
    "*.h,*.hxx,*.hpp,*.hh,*.h++,*.H,*.tlh,*.idl",
    "*.adb",
    "*.cs",
    "*.f",
    "*.html,*.htm",
    "*.hs",
    "*.java",
    "*.js",
    "*.php,*.php3,*.php4",
    "*.pl",
    "*.pp,*.pas",
    "*.py",
    "*.js,*.css,*.yml,*.rb,*.rhtml,*.html.erb,*.rjs,*.js.rjs,*.rxml,*.xml.builder",
    "CMakeLists.txt,*.cmake",
    "*",
    0
};


GrepDialog::GrepDialog( GrepViewPart * part, QWidget *parent, const char *name )
    : QDialog(parent, name, false), m_part( part )
{
    setCaption(i18n("Find in Files"));

    config = GrepViewFactory::instance()->config();
    config->setGroup("GrepDialog");

    QGridLayout *layout = new QGridLayout(this, 9, 2, 10, 4);
    layout->setColStretch(0, 0);
    layout->setColStretch(1, 20);

    QLabel *pattern_label = new QLabel(i18n("&Pattern:"), this);
    layout->addWidget(pattern_label, 0, 0, AlignRight | AlignVCenter);

    pattern_combo = new KHistoryCombo(true, this);
    pattern_label->setBuddy(pattern_combo);
    pattern_combo->setFocus();
    pattern_combo->setHistoryItems(config->readListEntry("LastSearchItems"), true);
    pattern_combo->setInsertionPolicy(QComboBox::NoInsertion);
    layout->addWidget(pattern_combo, 0, 1);

    QLabel *template_label = new QLabel(i18n("&Template:"), this);
    layout->addWidget(template_label, 1, 0, AlignRight | AlignVCenter);

    QBoxLayout *template_layout = new QHBoxLayout(4);
    layout->addLayout(template_layout, 1, 1);

    template_edit = new KLineEdit(this);
    template_label->setBuddy(template_edit);
    template_edit->setText(template_str[0]);
    template_layout->addWidget(template_edit, 1);

    KComboBox *template_combo = new KComboBox(false, this);
    template_combo->insertStrList(template_desc);
    template_layout->addWidget(template_combo, 0);

    QBoxLayout *search_opts_layout = new QHBoxLayout(15);
    layout->addLayout(search_opts_layout, 2, 1);

    regexp_box = new QCheckBox(i18n("&Regular Expression"), this);
    regexp_box->setChecked(config->readBoolEntry("regexp", false ));
    search_opts_layout->addWidget(regexp_box);

    case_sens_box = new QCheckBox(i18n("C&ase sensitive"), this);
    case_sens_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    case_sens_box->setChecked(config->readBoolEntry("case_sens", true));
    search_opts_layout->addWidget(case_sens_box);

    QLabel *dir_label = new QLabel(i18n("&Directory:"), this);
    layout->addWidget(dir_label, 3, 0, AlignRight | AlignVCenter);

    QBoxLayout *dir_layout = new QHBoxLayout(4);
    layout->addLayout(dir_layout, 3, 1);

    dir_combo = new KComboBox( true, this );
    dir_combo->insertStringList(config->readPathListEntry("LastSearchPaths"));
    dir_combo->setInsertionPolicy(QComboBox::NoInsertion);
    dir_combo->setEditText(QDir::homeDirPath());

    url_requester = new KURLRequester( dir_combo, this );
    url_requester->completionObject()->setMode(KURLCompletion::DirCompletion);
    url_requester->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );

    dir_label->setBuddy( url_requester );
    dir_combo->setMinimumWidth(dir_combo->fontMetrics().maxWidth()*25);
    dir_layout->addWidget( url_requester, 10 );

    synch_button = new KPushButton( this );
    QIconSet set = SmallIconSet( "dirsynch" );
    QPixmap pix = set.pixmap( QIconSet::Small, QIconSet::Normal );
    synch_button->setFixedSize( pix.width()+8, pix.height()+8 );
    synch_button->setIconSet( set );
    synch_button->setAccel( QKeySequence( "Alt+y") );
    QToolTip::add( synch_button, i18n("Set directory to that of the current file (Alt+Y)") );
    dir_layout->addWidget( synch_button );

    QBoxLayout *dir_opts_layout = new QHBoxLayout(15);
    layout->addLayout(dir_opts_layout, 4, 1);

    recursive_box = new QCheckBox(i18n("Rec&ursive"), this);
    recursive_box->setChecked(config->readBoolEntry("recursive", true));
    dir_opts_layout->addWidget(recursive_box);

    use_project_box = new QCheckBox(i18n("Limit search to &project files"), this);
    use_project_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    use_project_box->setChecked(config->readBoolEntry("search_project_files", true));
    dir_opts_layout->addWidget(use_project_box);

    QLabel *files_label = new QLabel(i18n("&Files:"), this);
    layout->addWidget(files_label, 5, 0, AlignRight | AlignVCenter);

    files_combo = new KComboBox(true, this);
    files_label->setBuddy(files_combo->focusProxy());
    files_combo->insertStrList(filepatterns);
    layout->addWidget(files_combo, 5, 1);

    QLabel *exclude_label = new QLabel(i18n("&Exclude:"), this);
    layout->addWidget(exclude_label, 6, 0, AlignRight | AlignVCenter);

    QStringList exclude_list = config->readListEntry("exclude_patterns");
    exclude_combo = new KComboBox(true, this);
    exclude_label->setBuddy(files_combo->focusProxy());
    if (exclude_list.count()) {
        exclude_combo->insertStringList(exclude_list);
    }
    else
    {
        exclude_combo->insertItem("/CVS/,/SCCS/,/\\.svn/,/_darcs/");
        exclude_combo->insertItem("");
    }
    layout->addWidget(exclude_combo, 6, 1);

    QBoxLayout *other_opts_layout = new QHBoxLayout(15);
    layout->addLayout(other_opts_layout, 7, 1);

    keep_output_box = new QCheckBox(i18n("New view"), this);
    keep_output_box->setChecked(config->readBoolEntry("new_view", true));
    other_opts_layout->addWidget(keep_output_box);

    no_find_err_box = new QCheckBox(i18n("&Suppress find errors"), this);
    no_find_err_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    no_find_err_box->setChecked(config->readBoolEntry("no_find_errs", true));
    other_opts_layout->addWidget(no_find_err_box);

    QBoxLayout *button_layout = new QHBoxLayout(4);
    layout->addLayout(button_layout, 8, 1);
    search_button = new KPushButton(KGuiItem(i18n("Sea&rch"),"grep"), this);
    search_button->setDefault(true);
    KPushButton *done_button = new KPushButton(KStdGuiItem::cancel(), this);
    button_layout->addStretch();
    button_layout->addWidget(search_button);
    button_layout->addWidget(done_button);

    resize(sizeHint());

    QWhatsThis::add(pattern_combo,
		    i18n("<qt>Enter the regular expression you want to search for here.<p>"
			 "Possible meta characters are:"
                         "<ul>"
			 "<li><b>.</b> - Matches any character"
			 "<li><b>^</b> - Matches the beginning of a line"
			 "<li><b>$</b> - Matches the end of a line"
	                 "<li><b>\\&lt;</b> - Matches the beginning of a word"
                         "<li><b>\\&gt;</b> - Matches the end of a word"
			 "</ul>"
			 "The following repetition operators exist:"
                         "<ul>"
			 "<li><b>?</b> - The preceding item is matched at most once"
			 "<li><b>*</b> - The preceding item is matched zero or more times"
			 "<li><b>+</b> - The preceding item is matched one or more times"
			 "<li><b>{<i>n</i>}</b> - The preceding item is matched exactly <i>n</i> times"
			 "<li><b>{<i>n</i>,}</b> - The preceding item is matched <i>n</i> or more times"
			 "<li><b>{,<i>n</i>}</b> - The preceding item is matched at most <i>n</i> times"
			 "<li><b>{<i>n</i>,<i>m</i>}</b> - The preceding item is matched at least <i>n</i>, "
			 "but at most <i>m</i> times."
			 "</ul>"
			 "Furthermore, backreferences to bracketed subexpressions are "
			 "available via the notation \\<i>n</i>.</qt>"
			 ));
    QWhatsThis::add(files_combo,
		    i18n("Enter the file name pattern of the files to search here. "
			 "You may give several patterns separated by commas"));
    QWhatsThis::add(template_edit,
		    i18n("You can choose a template for the pattern from the combo box "
			 "and edit it here. The string %s in the template is replaced "
			 "by the pattern input field, resulting in the regular expression "
			 "to search for."));

    connect( template_combo, SIGNAL(activated(int)),
	     SLOT(templateActivated(int)) );
    connect( search_button, SIGNAL(clicked()),
	     SLOT(slotSearchClicked()) );
    connect( done_button, SIGNAL(clicked()),
	     SLOT(hide()) );
    connect( pattern_combo->lineEdit(), SIGNAL( textChanged ( const QString & ) ),
             SLOT( slotPatternChanged( const QString & ) ) );
	connect( synch_button, SIGNAL(clicked()), this, SLOT(slotSynchDirectory()) );
    slotPatternChanged( pattern_combo->currentText() );
}

// Returns the contents of a QComboBox as a QStringList
static QStringList qCombo2StringList( QComboBox* combo )
{
    QStringList list;
    if (!combo)
	return list;
    for (int i = 0; i < combo->count(); ++i ) {
	list << combo->text(i);
    }
    return list;
}

GrepDialog::~GrepDialog()
{
    config->setGroup("GrepDialog");
    // memorize the last patterns and paths
    config->writeEntry("LastSearchItems", qCombo2StringList(pattern_combo));
    config->writePathEntry("LastSearchPaths", qCombo2StringList(dir_combo));
	config->writeEntry("regexp", regexp_box->isChecked());
	config->writeEntry("recursive", recursive_box->isChecked());
    config->writeEntry("search_project_files", use_project_box->isChecked());
	config->writeEntry("case_sens", case_sens_box->isChecked());
	config->writeEntry("new_view", keep_output_box->isChecked());
	config->writeEntry("no_find_errs", no_find_err_box->isChecked());
	config->writeEntry("exclude_patterns", qCombo2StringList(exclude_combo));
}

void GrepDialog::slotPatternChanged( const QString & _text )
{
    search_button->setEnabled( !_text.isEmpty() );
}

void GrepDialog::templateActivated(int index)
{
    template_edit->setText(template_str[index]);
}

// Find out whether the string s is already contained in combo
static bool qComboContains( const QString& s, QComboBox* combo )
{
    if (!combo)
	return false;
    for (int i = 0; i < combo->count(); ++i ) {
	if (combo->text(i) == s) {
	    return true;
	}
    }
    return false;
}

void GrepDialog::slotSearchClicked()
{
    if (pattern_combo->currentText().isEmpty()) {
	KMessageBox::sorry(this, i18n("Please enter a search pattern"));
	pattern_combo->setFocus();
	return;
    }
    // add the last patterns and paths to the combo
    if (!qComboContains(pattern_combo->currentText(), pattern_combo)) {
	pattern_combo->addToHistory(pattern_combo->currentText());
    }
    if (pattern_combo->count() > 15) {
	pattern_combo->removeFromHistory(pattern_combo->text(15));
    }
    if (!qComboContains(exclude_combo->currentText(), exclude_combo)) {
	exclude_combo->insertItem(exclude_combo->currentText(), 0);
    }
    if (exclude_combo->count() > 15) {
	exclude_combo->removeItem(15);
    }
    if (!qComboContains(dir_combo->currentText(), dir_combo)) {
	dir_combo->insertItem(dir_combo->currentText(), 0);
    }
    if (dir_combo->count() > 15) {
	dir_combo->removeItem(15);
    }

    emit searchClicked();
    hide();
}

void GrepDialog::show()
{
    // not beautiful, but works with all window
    // managers and Qt versions
    if (isVisible())
        hide();
//         QDialog::hide();
    QDialog::show();
    pattern_combo->setFocus();
}

void GrepDialog::hide()
{
	pattern_combo->setFocus();
	QDialog::hide();
}

void GrepDialog::slotSynchDirectory( )
{
	KParts::ReadOnlyPart * part = dynamic_cast<KParts::ReadOnlyPart*>( m_part->partController()->activePart() );
	if ( part )
	{
		KURL url = part->url();
		if ( url.isLocalFile() )
		{
			dir_combo->setEditText( url.upURL().path( +1 ) );
		}
	}
}

void GrepDialog::setEnableProjectBox(bool enable)
{
	use_project_box->setEnabled(enable);
	if (!enable) use_project_box->setChecked(false);
}

#include "grepdlg.moc"
