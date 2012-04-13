/**
 *  kcmaccess.cpp
 *
 *  Copyright (c) 2000 Matthias H�zer-Klpfel
 *
 */


#include <stdlib.h>
#include <math.h>

#include <dcopref.h>

#include <qtabwidget.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>
#include <qslider.h>
#include <qspinbox.h>


#include <kcombobox.h>
#include <kstandarddirs.h>
#include <kcolorbutton.h>
#include <kfiledialog.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kshortcut.h>
#include <kkeynative.h>
#include <knotifydialog.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/keysymdef.h>

#include "kcmaccess.moc"


ExtendedIntNumInput::ExtendedIntNumInput
				(QWidget* parent, const char* name)
	: KIntNumInput(parent, name)
{
}

ExtendedIntNumInput::~ExtendedIntNumInput () {
}

void ExtendedIntNumInput::setRange(int min, int max, int step, bool slider) {
	KIntNumInput::setRange (min,max,step, slider);

	if (slider) {
		disconnect(m_slider, SIGNAL(valueChanged(int)),
					  m_spin, SLOT(setValue(int)));
		disconnect(m_spin, SIGNAL(valueChanged(int)),
					  this, SLOT(spinValueChanged(int)));

		this->min = min;
		this->max = max;
		sliderMax = (int)floor (0.5
				+ 2*(log(max)-log(min)) / (log(max)-log(max-1)));
		m_slider->setRange(0, sliderMax);
		m_slider->setSteps(step, sliderMax/10);
		m_slider->setTickInterval(sliderMax/10);

		double alpha  = sliderMax / (log(max) - log(min));
		double logVal = alpha * (log(value())-log(min));
		m_slider->setValue ((int)floor (0.5 + logVal));

		connect(m_slider, SIGNAL(valueChanged(int)),
				  this, SLOT(slotSliderValueChanged(int)));
		connect(m_spin, SIGNAL(valueChanged(int)),
				  this, SLOT(slotSpinValueChanged(int)));
	}
}

// Basically the slider values are logarithmic whereas
// spinbox values are linear.

void ExtendedIntNumInput::slotSpinValueChanged(int val)
{

	if(m_slider) {
		double alpha  = sliderMax / (log(max) - log(min));
		double logVal = alpha * (log(val)-log(min));
		m_slider->setValue ((int)floor (0.5 + logVal));
	}

	emit valueChanged(val);
}

void ExtendedIntNumInput::slotSliderValueChanged(int val)
{
	double alpha  = sliderMax / (log(max) - log(min));
	double linearVal = exp (val/alpha + log(min));
	m_spin->setValue ((int)floor(0.5 + linearVal));
}

static bool needToRunKAccessDaemon( KConfig *config )
{
	// We always start the KAccess Daemon, if it is not needed,
	// it will terminate itself after configuring the AccessX
	// features.
	return true;
}

QString mouseKeysShortcut (Display *display) {
  // Calculate the keycode
  KeySym sym = XK_MouseKeys_Enable;
  KeyCode code = XKeysymToKeycode(display, sym);
  if (code == 0) {
     sym = XK_Pointer_EnableKeys;
     code = XKeysymToKeycode(display, sym);
     if (code == 0)
        return ""; // No shortcut available?
  }

  // Calculate the modifiers by searching the keysym in the X keyboard mapping
  XkbDescPtr xkbdesc = XkbGetMap(display, XkbKeyTypesMask | XkbKeySymsMask, XkbUseCoreKbd);
  if (!xkbdesc)
     return ""; // Failed to obtain the mapping from server

  bool found = false;
  unsigned char modifiers = 0;
  int groups = XkbKeyNumGroups(xkbdesc, code);
  for (int grp = 0; grp < groups && !found; grp++)
  {
     int levels = XkbKeyGroupWidth(xkbdesc, code, grp);
     for (int level = 0; level < levels && !found; level++)
     {
        if (sym == XkbKeySymEntry(xkbdesc, code, level, grp))
        {
           // keysym found => determine modifiers
           int typeIdx = xkbdesc->map->key_sym_map[code].kt_index[grp];
           XkbKeyTypePtr type = &(xkbdesc->map->types[typeIdx]);
           for (int i = 0; i < type->map_count && !found; i++)
           {
              if (type->map[i].active && (type->map[i].level == level))
              {
                 modifiers = type->map[i].mods.mask;
                 found = true;
              }
           }
        }
     }
  }
  XkbFreeClientMap (xkbdesc, 0, true);

  if (!found)
     return ""; // Somehow the keycode -> keysym mapping is flawed

  XEvent ev;
  ev.xkey.display = display;
  ev.xkey.keycode = code;
  ev.xkey.state = 0;
  KKey key = KKey(KKeyNative(&ev));
  QString keyname = key.toString();

  unsigned int AltMask   = KKeyNative::modX(KKey::ALT);
  unsigned int WinMask   = KKeyNative::modX(KKey::WIN);
  unsigned int NumMask   = KKeyNative::modXNumLock();
  unsigned int ScrollMask= KKeyNative::modXScrollLock();

  unsigned int MetaMask  = XkbKeysymToModifiers (display, XK_Meta_L);
  unsigned int SuperMask = XkbKeysymToModifiers (display, XK_Super_L);
  unsigned int HyperMask = XkbKeysymToModifiers (display, XK_Hyper_L);
  unsigned int AltGrMask = XkbKeysymToModifiers (display, XK_Mode_switch)
                         | XkbKeysymToModifiers (display, XK_ISO_Level3_Shift)
                         | XkbKeysymToModifiers (display, XK_ISO_Level3_Latch)
                         | XkbKeysymToModifiers (display, XK_ISO_Level3_Lock);

  unsigned int mods = ShiftMask | ControlMask | AltMask | WinMask
                    | LockMask | NumMask | ScrollMask;

  AltGrMask &= ~mods;
  MetaMask  &= ~(mods | AltGrMask);
  SuperMask &= ~(mods | AltGrMask | MetaMask);
  HyperMask &= ~(mods | AltGrMask | MetaMask | SuperMask);

  if ((modifiers & AltGrMask) != 0)
    keyname = i18n("AltGraph") + "+" + keyname;
  if ((modifiers & HyperMask) != 0)
    keyname = i18n("Hyper") + "+" + keyname;
  if ((modifiers & SuperMask) != 0)
    keyname = i18n("Super") + "+" + keyname;
  if ((modifiers & WinMask) != 0)
    keyname = KKey::modFlagLabel(KKey::WIN) + "+" + keyname;
  if ((modifiers & AltMask) != 0)
    keyname = KKey::modFlagLabel(KKey::ALT) + "+" + keyname;
  if ((modifiers & ControlMask) != 0)
    keyname = KKey::modFlagLabel(KKey::CTRL) + "+" + keyname;
  if ((modifiers & ShiftMask) != 0)
    keyname = KKey::modFlagLabel(KKey::SHIFT) + "+" + keyname;

  QString result;
  if ((modifiers & ScrollMask) != 0)
    if ((modifiers & LockMask) != 0)
      if ((modifiers & NumMask) != 0)
        result = i18n("Press %1 while NumLock, CapsLock and ScrollLock are active");
      else
        result = i18n("Press %1 while CapsLock and ScrollLock are active");
    else if ((modifiers & NumMask) != 0)
        result = i18n("Press %1 while NumLock and ScrollLock are active");
      else
        result = i18n("Press %1 while ScrollLock is active");
  else if ((modifiers & LockMask) != 0)
      if ((modifiers & NumMask) != 0)
        result = i18n("Press %1 while NumLock and CapsLock are active");
      else
        result = i18n("Press %1 while CapsLock is active");
    else if ((modifiers & NumMask) != 0)
        result = i18n("Press %1 while NumLock is active");
      else
        result = i18n("Press %1");

  return result.arg(keyname);
}

KAccessConfig::KAccessConfig(QWidget *parent, const char *)
  : KCModule(parent, "kcmaccess")
{

  KAboutData *about =
  new KAboutData(I18N_NOOP("kaccess"), I18N_NOOP("KDE Accessibility Tool"),
                 0, 0, KAboutData::License_GPL,
                 I18N_NOOP("(c) 2000, Matthias Hoelzer-Kluepfel"));

  about->addAuthor("Matthias Hoelzer-Kluepfel", I18N_NOOP("Author") , "hoelzer@kde.org");

  setAboutData( about );

  QVBoxLayout *main = new QVBoxLayout(this, 0, KDialogBase::spacingHint());
  QTabWidget *tab = new QTabWidget(this);
  main->addWidget(tab);

  // bell settings ---------------------------------------
  QWidget *bell = new QWidget(this);

  QVBoxLayout *vbox = new QVBoxLayout(bell, KDialogBase::marginHint(),
      KDialogBase::spacingHint());

  QGroupBox *grp = new QGroupBox(i18n("Audible Bell"), bell);
  grp->setColumnLayout( 0, Qt::Horizontal );
  vbox->addWidget(grp);

  QVBoxLayout *vvbox = new QVBoxLayout(grp->layout(),
      KDialogBase::spacingHint());

  systemBell = new QCheckBox(i18n("Use &system bell"), grp);
  vvbox->addWidget(systemBell);
  customBell = new QCheckBox(i18n("Us&e customized bell"), grp);
  vvbox->addWidget(customBell);
  QWhatsThis::add( systemBell, i18n("If this option is checked, the default system bell will be used. See the"
    " \"System Bell\" control module for how to customize the system bell."
    " Normally, this is just a \"beep\".") );
  QWhatsThis::add( customBell, i18n("Check this option if you want to use a customized bell, playing"
    " a sound file. If you do this, you will probably want to turn off the system bell.<p> Please note"
    " that on slow machines this may cause a \"lag\" between the event causing the bell and the sound being played.") );

  QHBoxLayout *hbox = new QHBoxLayout(vvbox, KDialogBase::spacingHint());
  hbox->addSpacing(24);
  soundEdit = new QLineEdit(grp);
  soundLabel = new QLabel(soundEdit, i18n("Sound &to play:"), grp);
  hbox->addWidget(soundLabel);
  hbox->addWidget(soundEdit);
  soundButton = new QPushButton(i18n("Browse..."), grp);
  hbox->addWidget(soundButton);
  QString wtstr = i18n("If the option \"Use customized bell\" is enabled, you can choose a sound file here."
    " Click \"Browse...\" to choose a sound file using the file dialog.");
  QWhatsThis::add( soundEdit, wtstr );
  QWhatsThis::add( soundLabel, wtstr );
  QWhatsThis::add( soundButton, wtstr );

  connect(soundButton, SIGNAL(clicked()), this, SLOT(selectSound()));

  connect(customBell, SIGNAL(clicked()), this, SLOT(checkAccess()));

  connect(systemBell, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(customBell, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(soundEdit, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));

  // -----------------------------------------------------

  // visible bell ----------------------------------------
  grp = new QGroupBox(i18n("Visible Bell"), bell);
  grp->setColumnLayout( 0, Qt::Horizontal );
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout(grp->layout(), KDialog::spacingHint());

  visibleBell = new QCheckBox(i18n("&Use visible bell"), grp);
  vvbox->addWidget(visibleBell);
  QWhatsThis::add( visibleBell, i18n("This option will turn on the \"visible bell\", i.e. a visible"
    " notification shown every time that normally just a bell would occur. This is especially useful"
    " for deaf people.") );

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  invertScreen = new QRadioButton(i18n("I&nvert screen"), grp);
  hbox->addWidget(invertScreen);
  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  QWhatsThis::add( invertScreen, i18n("All screen colors will be inverted for the amount of time specified below.") );
  hbox->addSpacing(24);
  flashScreen = new QRadioButton(i18n("F&lash screen"), grp);
  hbox->addWidget(flashScreen);
  QWhatsThis::add( flashScreen, i18n("The screen will turn to a custom color for the amount of time specified below.") );
  hbox->addSpacing(12);
  colorButton = new KColorButton(grp);
  colorButton->setFixedWidth(colorButton->sizeHint().height()*2);
  hbox->addWidget(colorButton);
  hbox->addStretch();
  QWhatsThis::add( colorButton, i18n("Click here to choose the color used for the \"flash screen\" visible bell.") );

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);

  durationSlider = new ExtendedIntNumInput(grp);
  durationSlider->setRange(100, 2000, 100);
  durationSlider->setLabel(i18n("Duration:"));
  durationSlider->setSuffix(i18n(" msec"));
  hbox->addWidget(durationSlider);
  QWhatsThis::add( durationSlider, i18n("Here you can customize the duration of the \"visible bell\" effect being shown.") );

  connect(invertScreen, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(flashScreen, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(visibleBell, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(visibleBell, SIGNAL(clicked()), this, SLOT(checkAccess()));
  connect(colorButton, SIGNAL(clicked()), this, SLOT(changeFlashScreenColor()));

  connect(invertScreen, SIGNAL(clicked()), this, SLOT(invertClicked()));
  connect(flashScreen, SIGNAL(clicked()), this, SLOT(flashClicked()));

  connect(durationSlider, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));

  vbox->addStretch();

  // -----------------------------------------------------

  tab->addTab(bell, i18n("&Bell"));


  // modifier key settings -------------------------------
  QWidget *modifiers = new QWidget(this);

  vbox = new QVBoxLayout(modifiers, KDialog::marginHint(), KDialog::spacingHint());

  grp = new QGroupBox(i18n("S&ticky Keys"), modifiers);
  grp->setColumnLayout( 0, Qt::Horizontal );
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout(grp->layout(), KDialog::spacingHint());

  stickyKeys = new QCheckBox(i18n("Use &sticky keys"), grp);
  vvbox->addWidget(stickyKeys);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  stickyKeysLock = new QCheckBox(i18n("&Lock sticky keys"), grp);
  hbox->addWidget(stickyKeysLock);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  stickyKeysAutoOff = new QCheckBox(i18n("Turn sticky keys off when two keys are pressed simultaneously"), grp);
  hbox->addWidget(stickyKeysAutoOff);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  stickyKeysBeep = new QCheckBox(i18n("Use system bell whenever a modifier gets latched, locked or unlocked"), grp);
  hbox->addWidget(stickyKeysBeep);

  grp = new QGroupBox(i18n("Locking Keys"), modifiers);
  grp->setColumnLayout( 0, Qt::Horizontal );
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout(grp->layout(), KDialog::spacingHint());

  toggleKeysBeep = new QCheckBox(i18n("Use system bell whenever a locking key gets activated or deactivated"), grp);
  vvbox->addWidget(toggleKeysBeep);

  kNotifyModifiers = new QCheckBox(i18n("Use KDE's system notification mechanism whenever a modifier or locking key changes its state"), grp);
  vvbox->addWidget(kNotifyModifiers);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addStretch(1);
  kNotifyModifiersButton = new QPushButton(i18n("Configure System Notification..."), grp);
  kNotifyModifiersButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  hbox->addWidget(kNotifyModifiersButton);

  connect(stickyKeys, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(stickyKeysLock, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(stickyKeysAutoOff, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(stickyKeys, SIGNAL(clicked()), this, SLOT(checkAccess()));

  connect(stickyKeysBeep, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(toggleKeysBeep, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(kNotifyModifiers, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(kNotifyModifiers, SIGNAL(clicked()), this, SLOT(checkAccess()));
  connect(kNotifyModifiersButton, SIGNAL(clicked()), this, SLOT(configureKNotify()));

  vbox->addStretch();

  tab->addTab(modifiers, i18n("&Modifier Keys"));

  // key filter settings ---------------------------------
  QWidget *filters = new QWidget(this);

  vbox = new QVBoxLayout(filters, KDialog::marginHint(), KDialog::spacingHint());

  grp = new QGroupBox(i18n("Slo&w Keys"), filters);
  grp->setColumnLayout( 0, Qt::Horizontal );
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout(grp->layout(), KDialog::spacingHint());

  slowKeys = new QCheckBox(i18n("&Use slow keys"), grp);
  vvbox->addWidget(slowKeys);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  slowKeysDelay = new ExtendedIntNumInput(grp);
  slowKeysDelay->setSuffix(i18n(" msec"));
  slowKeysDelay->setRange(50, 10000, 100);
  slowKeysDelay->setLabel(i18n("Acceptance dela&y:"));
  hbox->addWidget(slowKeysDelay);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  slowKeysPressBeep = new QCheckBox(i18n("&Use system bell whenever a key is pressed"), grp);
  hbox->addWidget(slowKeysPressBeep);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  slowKeysAcceptBeep = new QCheckBox(i18n("&Use system bell whenever a key is accepted"), grp);
  hbox->addWidget(slowKeysAcceptBeep);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  slowKeysRejectBeep = new QCheckBox(i18n("&Use system bell whenever a key is rejected"), grp);
  hbox->addWidget(slowKeysRejectBeep);

  grp = new QGroupBox(i18n("Bounce Keys"), filters);
  grp->setColumnLayout( 0, Qt::Horizontal );
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout(grp->layout(), KDialog::spacingHint());

  bounceKeys = new QCheckBox(i18n("Use bou&nce keys"), grp);
  vvbox->addWidget(bounceKeys);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  bounceKeysDelay = new ExtendedIntNumInput(grp);
  bounceKeysDelay->setSuffix(i18n(" msec"));
  bounceKeysDelay->setRange(100, 5000, 100);
  bounceKeysDelay->setLabel(i18n("D&ebounce time:"));
  hbox->addWidget(bounceKeysDelay);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  bounceKeysRejectBeep = new QCheckBox(i18n("Use the system bell whenever a key is rejected"), grp);
  hbox->addWidget(bounceKeysRejectBeep);

  connect(slowKeysDelay, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
  connect(slowKeys, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(slowKeys, SIGNAL(clicked()), this, SLOT(checkAccess()));

  connect(slowKeysPressBeep, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(slowKeysAcceptBeep, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(slowKeysRejectBeep, SIGNAL(clicked()), this, SLOT(configChanged()));

  connect(bounceKeysDelay, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
  connect(bounceKeys, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(bounceKeysRejectBeep, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(bounceKeys, SIGNAL(clicked()), this, SLOT(checkAccess()));

  vbox->addStretch();

  tab->addTab(filters, i18n("&Keyboard Filters"));

  // gestures --------------------------------------------
  QWidget *features = new QWidget(this);

  vbox = new QVBoxLayout(features, KDialog::marginHint(), KDialog::spacingHint());

  grp = new QGroupBox(i18n("Activation Gestures"), features);
  grp->setColumnLayout( 0, Qt::Horizontal );
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout(grp->layout(), KDialog::spacingHint());

  gestures = new QCheckBox(i18n("Use gestures for activating sticky keys and slow keys"), grp);
  vvbox->addWidget(gestures);
  QString shortcut = mouseKeysShortcut(this->x11Display());
  if (shortcut.isEmpty())
	  QWhatsThis::add (gestures, i18n("Here you can activate keyboard gestures that turn on the following features: \n"
			  "Sticky keys: Press Shift key 5 consecutive times\n"
					  "Slow keys: Hold down Shift for 8 seconds"));
  else
	  QWhatsThis::add (gestures, i18n("Here you can activate keyboard gestures that turn on the following features: \n"
			  "Mouse Keys: %1\n"
					  "Sticky keys: Press Shift key 5 consecutive times\n"
					  "Slow keys: Hold down Shift for 8 seconds").arg(shortcut));

  timeout = new QCheckBox(i18n("Turn sticky keys and slow keys off after a certain period of inactivity"), grp);
  vvbox->addWidget(timeout);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  timeoutDelay = new KIntNumInput(grp);
  timeoutDelay->setSuffix(i18n(" min"));
  timeoutDelay->setRange(1, 30, 4);
  timeoutDelay->setLabel(i18n("Timeout:"));
  hbox->addWidget(timeoutDelay);

  grp = new QGroupBox(i18n("Notification"), features);
  grp->setColumnLayout( 0, Qt::Horizontal );
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout(grp->layout(), KDialog::spacingHint());

  accessxBeep = new QCheckBox(i18n("Use the system bell whenever a gesture is used to turn an accessibility feature on or off"), grp);
  vvbox->addWidget(accessxBeep);

  gestureConfirmation = new QCheckBox(i18n("Show a confirmation dialog whenever a keyboard accessibility feature is turned on or off"), grp);
  vvbox->addWidget(gestureConfirmation);
  QWhatsThis::add (gestureConfirmation, i18n("If this option is checked, KDE will show a confirmation dialog whenever a keyboard accessibility feature is turned on or off.\nBe sure you know what you are doing if you uncheck it, as the keyboard accessibility settings will then always be applied without confirmation.") );

  kNotifyAccessX = new QCheckBox(i18n("Use KDE's system notification mechanism whenever a keyboard accessibility feature is turned on or off"), grp);
  vvbox->addWidget(kNotifyAccessX);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addStretch(1);
  kNotifyAccessXButton = new QPushButton(i18n("Configure System Notification..."), grp);
  kNotifyAccessXButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  hbox->addWidget(kNotifyAccessXButton);

  connect(gestures, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(timeout, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(timeout, SIGNAL(clicked()), this, SLOT(checkAccess()));
  connect(timeoutDelay, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
  connect(accessxBeep, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(gestureConfirmation, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(kNotifyAccessX, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(kNotifyAccessX, SIGNAL(clicked()), this, SLOT(checkAccess()));
  connect(kNotifyAccessXButton, SIGNAL(clicked()), this, SLOT(configureKNotify()));

  vbox->addStretch();

  tab->addTab(features, i18n("Activation Gestures"));

  load();
}


KAccessConfig::~KAccessConfig()
{
}

void KAccessConfig::configureKNotify()
{
	KAboutData about(I18N_NOOP("kaccess"),
						  I18N_NOOP("KDE Accessibility Tool"),
						  0);
	KNotifyDialog::configure (this, 0, &about);
}

void KAccessConfig::changeFlashScreenColor()
{
	invertScreen->setChecked(false);
	flashScreen->setChecked(true);
	configChanged();
}

void KAccessConfig::selectSound()
{
  QStringList list = KGlobal::dirs()->findDirs("sound", "");
  QString start;
  if (list.count()>0)
    start = list[0];
  // TODO: Why only wav's? How can I find out what artsd supports?
  QString fname = KFileDialog::getOpenFileName(start, i18n("*.wav|WAV Files"));
  if (!fname.isEmpty())
    soundEdit->setText(fname);
}


void KAccessConfig::configChanged()
{
  emit changed(true);
}


void KAccessConfig::load()
{
  load( false );
}

void KAccessConfig::load( bool useDefaults )
{
  KConfig *config = new KConfig("kaccessrc", true, false);

  config->setGroup("Bell");
  config->setReadDefaults( useDefaults );

  systemBell->setChecked(config->readBoolEntry("SystemBell", true));
  customBell->setChecked(config->readBoolEntry("ArtsBell", false));
  soundEdit->setText(config->readPathEntry("ArtsBellFile"));

  visibleBell->setChecked(config->readBoolEntry("VisibleBell", false));
  invertScreen->setChecked(config->readBoolEntry("VisibleBellInvert", true));
  flashScreen->setChecked(!invertScreen->isChecked());
  QColor def(Qt::red);
  colorButton->setColor(config->readColorEntry("VisibleBellColor", &def));

  durationSlider->setValue(config->readNumEntry("VisibleBellPause", 500));


  config->setGroup("Keyboard");

  stickyKeys->setChecked(config->readBoolEntry("StickyKeys", false));
  stickyKeysLock->setChecked(config->readBoolEntry("StickyKeysLatch", true));
  stickyKeysAutoOff->setChecked(config->readBoolEntry("StickyKeysAutoOff", false));
  stickyKeysBeep->setChecked(config->readBoolEntry("StickyKeysBeep", true));
  toggleKeysBeep->setChecked(config->readBoolEntry("ToggleKeysBeep", false));
  kNotifyModifiers->setChecked(config->readBoolEntry("kNotifyModifiers", false));

  slowKeys->setChecked(config->readBoolEntry("SlowKeys", false));
  slowKeysDelay->setValue(config->readNumEntry("SlowKeysDelay", 500));
  slowKeysPressBeep->setChecked(config->readBoolEntry("SlowKeysPressBeep", true));
  slowKeysAcceptBeep->setChecked(config->readBoolEntry("SlowKeysAcceptBeep", true));
  slowKeysRejectBeep->setChecked(config->readBoolEntry("SlowKeysRejectBeep", true));

  bounceKeys->setChecked(config->readBoolEntry("BounceKeys", false));
  bounceKeysDelay->setValue(config->readNumEntry("BounceKeysDelay", 500));
  bounceKeysRejectBeep->setChecked(config->readBoolEntry("BounceKeysRejectBeep", true));

  gestures->setChecked(config->readBoolEntry("Gestures", true));
  timeout->setChecked(config->readBoolEntry("AccessXTimeout", false));
  timeoutDelay->setValue(config->readNumEntry("AccessXTimeoutDelay", 30));

  accessxBeep->setChecked(config->readBoolEntry("AccessXBeep", true));
  gestureConfirmation->setChecked(config->readBoolEntry("GestureConfirmation", false));
  kNotifyAccessX->setChecked(config->readBoolEntry("kNotifyAccessX", false));

  delete config;

  checkAccess();

  emit changed(useDefaults);
}


void KAccessConfig::save()
{
  KConfig *config= new KConfig("kaccessrc", false);

  config->setGroup("Bell");

  config->writeEntry("SystemBell", systemBell->isChecked());
  config->writeEntry("ArtsBell", customBell->isChecked());
  config->writePathEntry("ArtsBellFile", soundEdit->text());

  config->writeEntry("VisibleBell", visibleBell->isChecked());
  config->writeEntry("VisibleBellInvert", invertScreen->isChecked());
  config->writeEntry("VisibleBellColor", colorButton->color());

  config->writeEntry("VisibleBellPause", durationSlider->value());


  config->setGroup("Keyboard");

  config->writeEntry("StickyKeys", stickyKeys->isChecked());
  config->writeEntry("StickyKeysLatch", stickyKeysLock->isChecked());
  config->writeEntry("StickyKeysAutoOff", stickyKeysAutoOff->isChecked());
  config->writeEntry("StickyKeysBeep", stickyKeysBeep->isChecked());
  config->writeEntry("ToggleKeysBeep", toggleKeysBeep->isChecked());
  config->writeEntry("kNotifyModifiers", kNotifyModifiers->isChecked());

  config->writeEntry("SlowKeys", slowKeys->isChecked());
  config->writeEntry("SlowKeysDelay", slowKeysDelay->value());
  config->writeEntry("SlowKeysPressBeep", slowKeysPressBeep->isChecked());
  config->writeEntry("SlowKeysAcceptBeep", slowKeysAcceptBeep->isChecked());
  config->writeEntry("SlowKeysRejectBeep", slowKeysRejectBeep->isChecked());


  config->writeEntry("BounceKeys", bounceKeys->isChecked());
  config->writeEntry("BounceKeysDelay", bounceKeysDelay->value());
  config->writeEntry("BounceKeysRejectBeep", bounceKeysRejectBeep->isChecked());

  config->writeEntry("Gestures", gestures->isChecked());
  config->writeEntry("AccessXTimeout", timeout->isChecked());
  config->writeEntry("AccessXTimeoutDelay", timeoutDelay->value());

  config->writeEntry("AccessXBeep", accessxBeep->isChecked());
  config->writeEntry("GestureConfirmation", gestureConfirmation->isChecked());
  config->writeEntry("kNotifyAccessX", kNotifyAccessX->isChecked());


  config->sync();

  if (systemBell->isChecked() ||
      customBell->isChecked() ||
      visibleBell->isChecked())
  {
    KConfig cfg("kdeglobals", false, false);
    cfg.setGroup("General");
    cfg.writeEntry("UseSystemBell", true);
    cfg.sync();
  }

  // make kaccess reread the configuration
  // When turning things off, it needs to be done by kaccess,
  // so don't actually kill it *shrug*.
  if ( true /*needToRunKAccessDaemon( config )*/ )
      kapp->startServiceByDesktopName("kaccess");

  else // don't need it -> kill it
  {
      DCOPRef kaccess( "kaccess", "qt/kaccess" );
      kaccess.send( "quit" );
  }

  delete config;

  emit changed(false);
}


void KAccessConfig::defaults()
{
	load( true );
}


void KAccessConfig::invertClicked()
{
  flashScreen->setChecked(false);
}


void KAccessConfig::flashClicked()
{
  invertScreen->setChecked(false);
}


void KAccessConfig::checkAccess()
{
  bool custom = customBell->isChecked();
  soundEdit->setEnabled(custom);
  soundButton->setEnabled(custom);
  soundLabel->setEnabled(custom);

  bool visible = visibleBell->isChecked();
  invertScreen->setEnabled(visible);
  flashScreen->setEnabled(visible);
  colorButton->setEnabled(visible);
  durationSlider->setEnabled(visible);

  bool sticky = stickyKeys->isChecked();
  stickyKeysLock->setEnabled(sticky);
  stickyKeysAutoOff->setEnabled(sticky);
  stickyKeysBeep->setEnabled(sticky);

  bool slow = slowKeys->isChecked();
  slowKeysDelay->setEnabled(slow);
  slowKeysPressBeep->setEnabled(slow);
  slowKeysAcceptBeep->setEnabled(slow);
  slowKeysRejectBeep->setEnabled(slow);

  bool bounce = bounceKeys->isChecked();
  bounceKeysDelay->setEnabled(bounce);
  bounceKeysRejectBeep->setEnabled(bounce);

  bool useTimeout = timeout->isChecked();
  timeoutDelay->setEnabled(useTimeout);
}

extern "C"
{
  KDE_EXPORT KCModule *create_access(QWidget *parent, const char *name)
  {
    return new KAccessConfig(parent, name);
  }

  /* This one gets called by kcminit

   */
  KDE_EXPORT void init_access()
  {
    KConfig *config = new KConfig("kaccessrc", true, false);
    bool run = needToRunKAccessDaemon( config );

    delete config;
    if (run)
      kapp->startServiceByDesktopName("kaccess");
  }
}


