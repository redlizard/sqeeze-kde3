#include <unistd.h>
#include <stdlib.h>

#include <qtimer.h>
#include <qpainter.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qlabel.h>

#include <kdialogbase.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kaudioplayer.h>
#include <knotifyclient.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <netwm.h>
#include <kshortcut.h>
#include <kkeynative.h>
#include <kwin.h>

#include <X11/XKBlib.h>
#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/keysymdef.h>


#include "kaccess.moc"

struct ModifierKey {
   const unsigned int mask;
   const KeySym keysym;
   const char *name;
   const char *lockedText;
   const char *latchedText;
   const char *unlatchedText;
};

static ModifierKey modifierKeys[] = {
    { ShiftMask, 0, "Shift",
      I18N_NOOP("The Shift key has been locked and is now active for all of the following keypresses."),
      I18N_NOOP("The Shift key is now active."),
      I18N_NOOP("The Shift key is now inactive.") },
    { ControlMask, 0, "Control",
      I18N_NOOP("The Ctrl key has been locked and is now active for all of the following keypresses."),
      I18N_NOOP("The Ctrl key is now active."),
      I18N_NOOP("The Ctrl key is now inactive.") },
    { 0, XK_Alt_L, "Alt",
      I18N_NOOP("The Alt key has been locked and is now active for all of the following keypresses."),
      I18N_NOOP("The Alt key is now active."),
      I18N_NOOP("The Alt key is now inactive.") },
    { 0, 0, "Win",
      I18N_NOOP("The Win key has been locked and is now active for all of the following keypresses."),
      I18N_NOOP("The Win key is now active."),
      I18N_NOOP("The Win key is now inactive.") },
    { 0, XK_Meta_L, "Meta",
      I18N_NOOP("The Meta key has been locked and is now active for all of the following keypresses."),
      I18N_NOOP("The Meta key is now active."),
      I18N_NOOP("The Meta key is now inactive.") },
    { 0, XK_Super_L, "Super",
      I18N_NOOP("The Super key has been locked and is now active for all of the following keypresses."),
      I18N_NOOP("The Super key is now active."),
      I18N_NOOP("The Super key is now inactive.") },
    { 0, XK_Hyper_L, "Hyper",
      I18N_NOOP("The Hyper key has been locked and is now active for all of the following keypresses."),
      I18N_NOOP("The Hyper key is now active."),
      I18N_NOOP("The Hyper key is now inactive.") },
    { 0, 0, "Alt Graph",
      I18N_NOOP("The Alt Gr key has been locked and is now active for all of the following keypresses."),
      I18N_NOOP("The Alt Gr key is now active."),
      I18N_NOOP("The Alt Gr key is now inactive.") },
    { 0, XK_Num_Lock, "Num Lock",
      I18N_NOOP("The Num Lock key has been activated."),
      "",
      I18N_NOOP("The Num Lock key is now inactive.") },
    { LockMask, 0, "Caps Lock",
      I18N_NOOP("The Caps Lock key has been activated."),
      "",
      I18N_NOOP("The Caps Lock key is now inactive.") },
    { 0, XK_Scroll_Lock, "Scroll Lock",
      I18N_NOOP("The Scroll Lock key has been activated."),
      "",
      I18N_NOOP("The Scroll Lock key is now inactive.") },
    { 0, 0, "", "", "", "" }
};


/********************************************************************/


KAccessApp::KAccessApp(bool allowStyles, bool GUIenabled)
  : KUniqueApplication(allowStyles, GUIenabled), _artsBellBlocked(false),
                                                 overlay(0), wm(0, KWinModule::INFO_DESKTOP)
{
  _activeWindow = wm.activeWindow();
  connect(&wm, SIGNAL(activeWindowChanged(WId)), this, SLOT(activeWindowChanged(WId)));

  artsBellTimer = new QTimer( this );
  connect( artsBellTimer, SIGNAL( timeout() ), SLOT( slotArtsBellTimeout() ));

  features = 0;
  requestedFeatures = 0;
  dialog = 0;

  initMasks();
  XkbStateRec state_return;
  XkbGetState (qt_xdisplay(), XkbUseCoreKbd, &state_return);
  unsigned char latched = XkbStateMods (&state_return);
  unsigned char locked  = XkbModLocks  (&state_return);
  state = ((int)locked)<<8 | latched;
}

int KAccessApp::newInstance()
{
  KGlobal::config()->reparseConfiguration();
  readSettings();
  return 0;
}

void KAccessApp::readSettings()
{
  KConfig *config = KGlobal::config();

  // bell ---------------------------------------------------------------

  config->setGroup("Bell");
  _systemBell = config->readBoolEntry("SystemBell", true);
  _artsBell = config->readBoolEntry("ArtsBell", false);
  _artsBellFile = config->readPathEntry("ArtsBellFile");
  _visibleBell = config->readBoolEntry("VisibleBell", false);
  _visibleBellInvert = config->readBoolEntry("VisibleBellInvert", false);
  QColor def(Qt::red);
  _visibleBellColor = config->readColorEntry("VisibleBellColor", &def);
  _visibleBellPause = config->readNumEntry("VisibleBellPause", 500);

  // select bell events if we need them
  int state = (_artsBell || _visibleBell) ? XkbBellNotifyMask : 0;
  XkbSelectEvents(qt_xdisplay(), XkbUseCoreKbd, XkbBellNotifyMask, state);

  // deactivate system bell if not needed
  if (!_systemBell)
    XkbChangeEnabledControls(qt_xdisplay(), XkbUseCoreKbd, XkbAudibleBellMask, 0);
  else
    XkbChangeEnabledControls(qt_xdisplay(), XkbUseCoreKbd, XkbAudibleBellMask, XkbAudibleBellMask);

  // keyboard -------------------------------------------------------------

  config->setGroup("Keyboard");

  // get keyboard state
  XkbDescPtr xkb = XkbGetMap(qt_xdisplay(), 0, XkbUseCoreKbd);
  if (!xkb)
    return;
  if (XkbGetControls(qt_xdisplay(), XkbAllControlsMask, xkb) != Success)
    return;

  // sticky keys
  if (config->readBoolEntry("StickyKeys", false))
    {
      if (config->readBoolEntry("StickyKeysLatch", true))
        xkb->ctrls->ax_options |= XkbAX_LatchToLockMask;
      else
        xkb->ctrls->ax_options &= ~XkbAX_LatchToLockMask;
      if (config->readBoolEntry("StickyKeysAutoOff", false))
         xkb->ctrls->ax_options |= XkbAX_TwoKeysMask;
      else
         xkb->ctrls->ax_options &= ~XkbAX_TwoKeysMask;
      if (config->readBoolEntry("StickyKeysBeep", false))
         xkb->ctrls->ax_options |= XkbAX_StickyKeysFBMask;
      else
         xkb->ctrls->ax_options &= ~XkbAX_StickyKeysFBMask;
      xkb->ctrls->enabled_ctrls |= XkbStickyKeysMask;
    }
  else
    xkb->ctrls->enabled_ctrls &= ~XkbStickyKeysMask;

  // toggle keys
  if (config->readBoolEntry("ToggleKeysBeep", false))
     xkb->ctrls->ax_options |= XkbAX_IndicatorFBMask;
  else
     xkb->ctrls->ax_options &= ~XkbAX_IndicatorFBMask;

  // slow keys
  if (config->readBoolEntry("SlowKeys", false)) {
      if (config->readBoolEntry("SlowKeysPressBeep", false))
         xkb->ctrls->ax_options |= XkbAX_SKPressFBMask;
      else
         xkb->ctrls->ax_options &= ~XkbAX_SKPressFBMask;
      if (config->readBoolEntry("SlowKeysAcceptBeep", false))
         xkb->ctrls->ax_options |= XkbAX_SKAcceptFBMask;
      else
         xkb->ctrls->ax_options &= ~XkbAX_SKAcceptFBMask;
      if (config->readBoolEntry("SlowKeysRejectBeep", false))
         xkb->ctrls->ax_options |= XkbAX_SKRejectFBMask;
      else
         xkb->ctrls->ax_options &= ~XkbAX_SKRejectFBMask;
      xkb->ctrls->enabled_ctrls |= XkbSlowKeysMask;
    }
  else
      xkb->ctrls->enabled_ctrls &= ~XkbSlowKeysMask;
  xkb->ctrls->slow_keys_delay = config->readNumEntry("SlowKeysDelay", 500);

  // bounce keys
  if (config->readBoolEntry("BounceKeys", false)) {
      if (config->readBoolEntry("BounceKeysRejectBeep", false))
         xkb->ctrls->ax_options |= XkbAX_BKRejectFBMask;
      else
         xkb->ctrls->ax_options &= ~XkbAX_BKRejectFBMask;
      xkb->ctrls->enabled_ctrls |= XkbBounceKeysMask;
    }
  else
      xkb->ctrls->enabled_ctrls &= ~XkbBounceKeysMask;
  xkb->ctrls->debounce_delay = config->readNumEntry("BounceKeysDelay", 500);

  // gestures for enabling the other features
  _gestures = config->readBoolEntry("Gestures", true);
  if (_gestures)
      xkb->ctrls->enabled_ctrls |= XkbAccessXKeysMask;
  else
      xkb->ctrls->enabled_ctrls &= ~XkbAccessXKeysMask;

  // timeout
  if (config->readBoolEntry("AccessXTimeout", false))
    {
      xkb->ctrls->ax_timeout = config->readNumEntry("AccessXTimeoutDelay", 30)*60;
      xkb->ctrls->axt_opts_mask = 0;
      xkb->ctrls->axt_opts_values = 0;
      xkb->ctrls->axt_ctrls_mask = XkbStickyKeysMask | XkbSlowKeysMask;
      xkb->ctrls->axt_ctrls_values = 0;
      xkb->ctrls->enabled_ctrls |= XkbAccessXTimeoutMask;
    }
  else
    xkb->ctrls->enabled_ctrls &= ~XkbAccessXTimeoutMask;

  // gestures for enabling the other features
  if (_gestures && config->readBoolEntry("AccessXBeep", true))
     xkb->ctrls->ax_options |= XkbAX_FeatureFBMask | XkbAX_SlowWarnFBMask;
  else
     xkb->ctrls->ax_options &= ~(XkbAX_FeatureFBMask | XkbAX_SlowWarnFBMask);

  _gestureConfirmation = config->readBoolEntry("GestureConfirmation", true);

  _kNotifyModifiers = config->readBoolEntry("kNotifyModifiers", false);
  _kNotifyAccessX = config->readBoolEntry("kNotifyAccessX", false);

  // mouse-by-keyboard ----------------------------------------------

  config->setGroup("Mouse");

  if (config->readBoolEntry("MouseKeys", false))
    {
      xkb->ctrls->mk_delay = config->readNumEntry("MKDelay", 160);

      // Default for initial velocity: 200 pixels/sec
      int interval = config->readNumEntry("MKInterval", 5);
      xkb->ctrls->mk_interval = interval;

      // Default time to reach maximum speed: 5000 msec
      xkb->ctrls->mk_time_to_max = config->readNumEntry("MKTimeToMax",
                                             (5000+interval/2)/interval);

      // Default maximum speed: 1000 pixels/sec
      //     (The old default maximum speed from KDE <= 3.4
      //     (100000 pixels/sec) was way too fast)
      xkb->ctrls->mk_max_speed = config->readNumEntry("MKMaxSpeed", interval);

      xkb->ctrls->mk_curve = config->readNumEntry("MKCurve", 0);
      xkb->ctrls->mk_dflt_btn = config->readNumEntry("MKDefaultButton", 0);

      xkb->ctrls->enabled_ctrls |= XkbMouseKeysMask;
    }
  else
    xkb->ctrls->enabled_ctrls &= ~XkbMouseKeysMask;

   features = xkb->ctrls->enabled_ctrls & (XkbSlowKeysMask | XkbBounceKeysMask | XkbStickyKeysMask | XkbMouseKeysMask);
   if (dialog == 0)
      requestedFeatures = features;
  // set state
  XkbSetControls(qt_xdisplay(), XkbControlsEnabledMask | XkbMouseKeysAccelMask | XkbStickyKeysMask | XkbSlowKeysMask | XkbBounceKeysMask | XkbAccessXKeysMask | XkbAccessXTimeoutMask, xkb);

  // select AccessX events
  XkbSelectEvents(qt_xdisplay(), XkbUseCoreKbd, XkbAllEventsMask, XkbAllEventsMask);

  if (!_artsBell && !_visibleBell && !_gestureConfirmation
      && !_kNotifyModifiers && !_kNotifyAccessX) {

     // We will exit, but the features need to stay configured
     uint ctrls = XkbStickyKeysMask | XkbSlowKeysMask | XkbBounceKeysMask | XkbMouseKeysMask | XkbAudibleBellMask | XkbControlsNotifyMask;
     uint values = xkb->ctrls->enabled_ctrls & ctrls;
     XkbSetAutoResetControls(qt_xdisplay(), ctrls, &ctrls, &values);
     exit(0);
  } else {
     // reset them after program exit
     uint ctrls = XkbStickyKeysMask | XkbSlowKeysMask | XkbBounceKeysMask | XkbMouseKeysMask | XkbAudibleBellMask | XkbControlsNotifyMask;
     uint values = XkbAudibleBellMask;
     XkbSetAutoResetControls(qt_xdisplay(), ctrls, &ctrls, &values);
  }

  delete overlay;
  overlay = 0;
}

static int maskToBit (int mask) {
   for (int i = 0; i < 8; i++)
      if (mask & (1 << i))
         return i;
   return -1;
}

void KAccessApp::initMasks() {
   for (int i = 0; i < 8; i++)
      keys [i] = -1;
   state = 0;

   for (int i = 0; strcmp (modifierKeys[i].name, "") != 0; i++) {
      int mask = modifierKeys[i].mask;
      if (mask == 0)
         if (modifierKeys[i].keysym != 0)
            mask = XkbKeysymToModifiers (qt_xdisplay(), modifierKeys[i].keysym);
         else if (!strcmp(modifierKeys[i].name, "Win"))
            mask = KKeyNative::modX(KKey::WIN);
         else
            mask = XkbKeysymToModifiers (qt_xdisplay(), XK_Mode_switch)
                 | XkbKeysymToModifiers (qt_xdisplay(), XK_ISO_Level3_Shift)
                 | XkbKeysymToModifiers (qt_xdisplay(), XK_ISO_Level3_Latch)
                 | XkbKeysymToModifiers (qt_xdisplay(), XK_ISO_Level3_Lock);

      int bit = maskToBit (mask);
      if (bit != -1 && keys[bit] == -1)
         keys[bit] = i;
   }
}


bool KAccessApp::x11EventFilter(XEvent *event)
{
  // handle XKB events
  if (event->type == xkb_opcode)
    {
      XkbAnyEvent *ev = (XkbAnyEvent*) event;

      switch (ev->xkb_type) {
      case XkbStateNotify:
         xkbStateNotify();
         break;
      case XkbBellNotify:
         xkbBellNotify((XkbBellNotifyEvent*)event);
         break;
      case XkbControlsNotify:
         xkbControlsNotify((XkbControlsNotifyEvent*)event);
         break;
      }
      return true;
    }

  // process other events as usual
  return KApplication::x11EventFilter(event);
}


void VisualBell::paintEvent(QPaintEvent *event)
{
  QWidget::paintEvent(event);
  QTimer::singleShot(_pause, this, SLOT(hide()));
}


void KAccessApp::activeWindowChanged(WId wid)
{
  _activeWindow = wid;
}


void KAccessApp::xkbStateNotify () {
   XkbStateRec state_return;
   XkbGetState (qt_xdisplay(), XkbUseCoreKbd, &state_return);
   unsigned char latched = XkbStateMods (&state_return);
   unsigned char locked  = XkbModLocks  (&state_return);
   int mods = ((int)locked)<<8 | latched;

   if (state != mods) {
      if (_kNotifyModifiers)
      for (int i = 0; i < 8; i++) {
         if (keys[i] != -1) {
            if (    (!*modifierKeys[keys[i]].latchedText)
                && ( (((mods >> i) & 0x101) != 0) != (((state >> i) & 0x101) != 0) ))
            {
               if ((mods >> i) & 1) {
                  KNotifyClient::event (0, "lockkey-locked", i18n(modifierKeys[keys[i]].lockedText));
               }
               else {
                  KNotifyClient::event (0, "lockkey-unlocked", i18n(modifierKeys[keys[i]].unlatchedText));
               }
            }
            else if ((*modifierKeys[keys[i]].latchedText)
                && ( ((mods >> i) & 0x101) != ((state >> i) & 0x101) ))
            {
               if ((mods >> i) & 0x100) {
                  KNotifyClient::event (0, "modifierkey-locked", i18n(modifierKeys[keys[i]].lockedText));
               }
               else if ((mods >> i) & 1) {
                  KNotifyClient::event (0, "modifierkey-latched", i18n(modifierKeys[keys[i]].latchedText));
               }
               else {
                  KNotifyClient::event (0, "modifierkey-unlatched", i18n(modifierKeys[keys[i]].unlatchedText));
               }
            }
         }
      }
      state = mods;
   }
}

void KAccessApp::xkbBellNotify(XkbBellNotifyEvent *event)
{
  // bail out if we should not really ring
  if (event->event_only)
    return;

  // flash the visible bell
  if (_visibleBell)
    {
      // create overlay widget
      if (!overlay)
        overlay = new VisualBell(_visibleBellPause);

      WId id = _activeWindow;

      NETRect frame, window;
      NETWinInfo net(qt_xdisplay(), id, desktop()->winId(), 0);

      net.kdeGeometry(frame, window);

      overlay->setGeometry(window.pos.x, window.pos.y, window.size.width, window.size.height);

      if (_visibleBellInvert)
        {
	  QPixmap screen = QPixmap::grabWindow(id, 0, 0, window.size.width, window.size.height);
	  QPixmap invert(window.size.width, window.size.height);
	  QPainter p(&invert);
	  p.setRasterOp(QPainter::NotCopyROP);
	  p.drawPixmap(0, 0, screen);
	  overlay->setBackgroundPixmap(invert);
	}
      else
	overlay->setBackgroundColor(_visibleBellColor);

      // flash the overlay widget
      overlay->raise();
      overlay->show();
      flushX();
    }

  // ask artsd to ring a nice bell
  if (_artsBell && !_artsBellBlocked ) {
    KAudioPlayer::play(_artsBellFile);
    _artsBellBlocked = true;
    artsBellTimer->start( 300, true );
  }
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
    keyname = i18n("Meta") + "+" + keyname;
  if ((modifiers & WinMask) != 0)
    keyname = KKey::modFlagLabel(KKey::WIN) + "+" + keyname;
  if ((modifiers & AltMask) != 0)
    keyname = KKey::modFlagLabel(KKey::ALT) + "+" + keyname;
  if ((modifiers & ControlMask) != 0)
    keyname = KKey::modFlagLabel(KKey::CTRL) + "+" + keyname;
  if ((modifiers & ShiftMask) != 0)
    keyname = KKey::modFlagLabel(KKey::SHIFT) + "+" + keyname;

  return keyname;
}

void KAccessApp::createDialogContents() {
   if (dialog == 0) {
      dialog = new KDialogBase(
            i18n("Warning"),
            KDialogBase::Yes | KDialogBase::No,
            KDialogBase::Yes, KDialogBase::Close,
            0, "AccessXWarning", true, true,
            KStdGuiItem::cont(), KStdGuiItem::cancel());

      QVBox *topcontents = new QVBox (dialog);
      topcontents->setSpacing(KDialog::spacingHint()*2);
      topcontents->setMargin(KDialog::marginHint());

      QWidget *contents = new QWidget(topcontents);
      QHBoxLayout * lay = new QHBoxLayout(contents);
      lay->setSpacing(KDialog::spacingHint());

      QLabel *label1 = new QLabel( contents);
      QPixmap pixmap = KApplication::kApplication()->iconLoader()->loadIcon("messagebox_warning", KIcon::NoGroup, KIcon::SizeMedium, KIcon::DefaultState, 0, true);
      if (pixmap.isNull())
         pixmap = QMessageBox::standardIcon(QMessageBox::Warning);
      label1->setPixmap(pixmap);

      lay->addWidget( label1, 0, Qt::AlignCenter );
      lay->addSpacing(KDialog::spacingHint());

      QVBoxLayout * vlay = new QVBoxLayout(lay);

      featuresLabel = new QLabel( "", contents );
      featuresLabel->setAlignment( WordBreak|AlignVCenter );
      vlay->addWidget( featuresLabel );
      vlay->addStretch();

      QHBoxLayout * hlay = new QHBoxLayout(vlay);

      QLabel *showModeLabel = new QLabel( i18n("&When a gesture was used:"), contents );
      hlay->addWidget( showModeLabel );

      showModeCombobox = new KComboBox (contents);
      hlay->addWidget( showModeCombobox );
      showModeLabel->setBuddy(showModeCombobox);
      showModeCombobox->insertItem ( i18n("Change Settings Without Asking"), 0);
      showModeCombobox->insertItem ( i18n("Show This Confirmation Dialog"), 1);
      showModeCombobox->insertItem ( i18n("Deactivate All AccessX Features & Gestures"), 2);
      showModeCombobox->setCurrentItem (1);

      dialog->setMainWidget(topcontents);
      dialog->enableButtonSeparator(false);

      connect (dialog, SIGNAL(yesClicked()), this, SLOT(yesClicked()));
      connect (dialog, SIGNAL(noClicked()), this, SLOT(noClicked()));
      connect (dialog, SIGNAL(closeClicked()), this, SLOT(dialogClosed()));
   }
}

void KAccessApp::xkbControlsNotify(XkbControlsNotifyEvent *event)
{
   unsigned int newFeatures = event->enabled_ctrls & (XkbSlowKeysMask | XkbBounceKeysMask | XkbStickyKeysMask | XkbMouseKeysMask);

   if (newFeatures != features) {
     unsigned int enabled  = newFeatures & ~features;
     unsigned int disabled = features & ~newFeatures;

     if (!_gestureConfirmation) {
        requestedFeatures = enabled | (requestedFeatures & ~disabled);
        notifyChanges();
        features = newFeatures;
     }
     else {
        // set the AccessX features back to what they were. We will
        // apply the changes later if the user allows us to do that.
        readSettings();

        requestedFeatures = enabled | (requestedFeatures & ~disabled);

        enabled  = requestedFeatures & ~features;
        disabled = features & ~requestedFeatures;

        QStringList enabledFeatures;
        QStringList disabledFeatures;

        if (enabled & XkbStickyKeysMask)
           enabledFeatures << i18n("Sticky keys");
        else if (disabled & XkbStickyKeysMask)
           disabledFeatures << i18n("Sticky keys");

        if (enabled & XkbSlowKeysMask)
           enabledFeatures << i18n("Slow keys");
        else if (disabled & XkbSlowKeysMask)
           disabledFeatures << i18n("Slow keys");

        if (enabled & XkbBounceKeysMask)
           enabledFeatures << i18n("Bounce keys");
        else if (disabled & XkbBounceKeysMask)
           disabledFeatures << i18n("Bounce keys");

        if (enabled & XkbMouseKeysMask)
           enabledFeatures << i18n("Mouse keys");
        else if (disabled & XkbMouseKeysMask)
           disabledFeatures << i18n("Mouse keys");

        QString question;
        switch (enabledFeatures.count()) {
           case 0: switch (disabledFeatures.count()) {
              case 1: question = i18n("Do you really want to deactivate \"%1\"?")
                    .arg(disabledFeatures[0]);
              break;
              case 2: question = i18n("Do you really want to deactivate \"%1\" and \"%2\"?")
                    .arg(disabledFeatures[0]).arg(disabledFeatures[1]);
              break;
              case  3: question = i18n("Do you really want to deactivate \"%1\", \"%2\" and \"%3\"?")
                    .arg(disabledFeatures[0]).arg(disabledFeatures[1])
                    .arg(disabledFeatures[2]);
              break;
              case 4: question = i18n("Do you really want to deactivate \"%1\", \"%2\", \"%3\" and \"%4\"?")
                    .arg(disabledFeatures[0]).arg(disabledFeatures[1])
                    .arg(disabledFeatures[2]).arg(disabledFeatures[3]);
              break;
           }
           break;
           case 1: switch (disabledFeatures.count()) {
              case 0: question = i18n("Do you really want to activate \"%1\"?")
                    .arg(enabledFeatures[0]);
              break;
              case 1: question = i18n("Do you really want to activate \"%1\" and to deactivate \"%2\"?")
                    .arg(enabledFeatures[0]).arg(disabledFeatures[0]);
              break;
              case 2: question = i18n("Do you really want to activate \"%1\" and to deactivate \"%2\" and \"%3\"?")
                    .arg(enabledFeatures[0]).arg(disabledFeatures[0])
                    .arg(disabledFeatures[1]);
              break;
              case 3: question = i18n("Do you really want to activate \"%1\" and to deactivate \"%2\", \"%3\" and \"%4\"?")
                    .arg(enabledFeatures[0]).arg(disabledFeatures[0])
                    .arg(disabledFeatures[1]).arg(disabledFeatures[2]);
              break;
           }
           break;
           case 2: switch (disabledFeatures.count()) {
              case 0: question = i18n("Do you really want to activate \"%1\" and \"%2\"?")
                    .arg(enabledFeatures[0]).arg(enabledFeatures[1]);
              break;
              case 1: question = i18n("Do you really want to activate \"%1\" and \"%2\" and to deactivate \"%3\"?")
                    .arg(enabledFeatures[0]).arg(enabledFeatures[1])
                    .arg(disabledFeatures[0]);
              break;
              case 2: question = i18n("Do you really want to activate \"%1\", and \"%2\" and to deactivate \"%3\" and \"%4\"?")
                    .arg(enabledFeatures[0]).arg(enabledFeatures[1])
                    .arg(enabledFeatures[0]).arg(disabledFeatures[1]);
              break;
           }
           break;
           case 3: switch (disabledFeatures.count()) {
              case 0: question = i18n("Do you really want to activate \"%1\", \"%2\" and \"%3\"?")
                    .arg(enabledFeatures[0]).arg(enabledFeatures[1])
                    .arg(enabledFeatures[2]);
              break;
              case 1: question = i18n("Do you really want to activate \"%1\", \"%2\" and \"%3\" and to deactivate \"%4\"?")
                    .arg(enabledFeatures[0]).arg(enabledFeatures[1])
                    .arg(enabledFeatures[2]).arg(disabledFeatures[0]);
              break;
           }
           break;
           case 4: question = i18n("Do you really want to activate \"%1\", \"%2\", \"%3\" and \"%4\"?")
                 .arg(enabledFeatures[0]).arg(enabledFeatures[1])
                 .arg(enabledFeatures[2]).arg(enabledFeatures[3]);
           break;
        }
        QString explanation;
        if (enabledFeatures.count()+disabledFeatures.count() == 1) {
           explanation = i18n("An application has requested to change this setting.");

           if (_gestures) {
              if ((enabled | disabled) == XkbSlowKeysMask)
                 explanation = i18n("You held down the Shift key for 8 seconds or an application has requested to change this setting.");
              else if ((enabled | disabled) == XkbStickyKeysMask)
                 explanation = i18n("You pressed the Shift key 5 consecutive times or an application has requested to change this setting.");
              else if ((enabled | disabled) == XkbMouseKeysMask) {
                 QString shortcut = mouseKeysShortcut(qt_xdisplay());
                 if (!shortcut.isEmpty() && !shortcut.isNull())
                    explanation = i18n("You pressed %1 or an application has requested to change this setting.").arg(shortcut);
              }
           }
        }
        else {
           if (_gestures)
              explanation = i18n("An application has requested to change these settings, or you used a combination of several keyboard gestures.");
           else
              explanation = i18n("An application has requested to change these settings.");
        }

        createDialogContents();
        featuresLabel->setText ( question+"\n\n"+explanation
              +" "+i18n("These AccessX settings are needed for some users with motion impairments and can be configured in the KDE Control Center. You can also turn them on and off with standardized keyboard gestures.\n\nIf you do not need them, you can select \"Deactivate all AccessX features and gestures\".") );

        KWin::setState( dialog->winId(), NET::KeepAbove );
        kapp->updateUserTimestamp();
        dialog->show();
     }
  }
}

void KAccessApp::notifyChanges() {
   if (!_kNotifyAccessX)
      return;

   unsigned int enabled  = requestedFeatures & ~features;
   unsigned int disabled = features & ~requestedFeatures;

   if (enabled & XkbSlowKeysMask)
      KNotifyClient::event (0, "slowkeys", i18n("Slow keys has been enabled. From now on, you need to press each key for a certain length of time before it is accepted."));
   else if (disabled & XkbSlowKeysMask)
      KNotifyClient::event (0, "slowkeys", i18n("Slow keys has been disabled."));

   if (enabled & XkbBounceKeysMask)
      KNotifyClient::event (0, "bouncekeys", i18n("Bounce keys has been enabled. From now on, each key will be blocked for a certain length of time after it is used."));
   else if (disabled & XkbBounceKeysMask)
      KNotifyClient::event (0, "bouncekeys", i18n("Bounce keys has been disabled."));

   if (enabled & XkbStickyKeysMask)
      KNotifyClient::event (0, "stickykeys", i18n("Sticky keys has been enabled. From now on, modifier keys will stay latched after you have released them."));
   else if (disabled & XkbStickyKeysMask)
      KNotifyClient::event (0, "stickykeys", i18n("Sticky keys has been disabled."));

   if (enabled & XkbMouseKeysMask)
      KNotifyClient::event (0, "mousekeys", i18n("Mouse keys has been enabled. From now on, you can use the number pad of your keyboard in order to control the mouse."));
   else if (disabled & XkbMouseKeysMask)
      KNotifyClient::event (0, "mousekeys", i18n("Mouse keys has been disabled."));
}

void KAccessApp::applyChanges() {
   notifyChanges();
   unsigned int enabled  = requestedFeatures & ~features;
   unsigned int disabled = features & ~requestedFeatures;

   KConfig *config = KGlobal::config();
   config->setGroup("Keyboard");

   if (enabled & XkbSlowKeysMask)
      config->writeEntry("SlowKeys", true);
   else if (disabled & XkbSlowKeysMask)
      config->writeEntry("SlowKeys", false);

   if (enabled & XkbBounceKeysMask)
      config->writeEntry("BounceKeys", true);
   else if (disabled & XkbBounceKeysMask)
      config->writeEntry("BounceKeys", false);

   if (enabled & XkbStickyKeysMask)
      config->writeEntry("StickyKeys", true);
   else if (disabled & XkbStickyKeysMask)
      config->writeEntry("StickyKeys", false);

   config->setGroup("Mouse");

   if (enabled & XkbMouseKeysMask)
      config->writeEntry("MouseKeys", true);
   else if (disabled & XkbMouseKeysMask)
      config->writeEntry("MouseKeys", false);

   config->sync();
}

void KAccessApp::yesClicked() {
   if (dialog != 0)
      dialog->deleteLater();
   dialog = 0;

   KConfig *config = KGlobal::config();
   config->setGroup("Keyboard");
   switch (showModeCombobox->currentItem()) {
      case 0:
         config->writeEntry("Gestures", true);
         config->writeEntry("GestureConfirmation", false);
         break;
      default:
         config->writeEntry("Gestures", true);
         config->writeEntry("GestureConfirmation", true);
         break;
      case 2:
         requestedFeatures = 0;
         config->writeEntry("Gestures", false);
         config->writeEntry("GestureConfirmation", false);
   }
   config->sync();

   if (features != requestedFeatures) {
      notifyChanges();
      applyChanges();
   }
   readSettings();
}

void KAccessApp::noClicked() {
   if (dialog != 0)
      dialog->deleteLater();
   dialog = 0;
   requestedFeatures = features;

   KConfig *config = KGlobal::config();
   config->setGroup("Keyboard");
   switch (showModeCombobox->currentItem()) {
      case 0:
         config->writeEntry("Gestures", true);
         config->writeEntry("GestureConfirmation", false);
         break;
      default:
         config->writeEntry("Gestures", true);
         config->writeEntry("GestureConfirmation", true);
         break;
      case 2:
         requestedFeatures = 0;
         config->writeEntry("Gestures", false);
         config->writeEntry("GestureConfirmation", true);
   }
   config->sync();

   if (features != requestedFeatures)
      applyChanges();
   readSettings();
}

void KAccessApp::dialogClosed() {
   if (dialog != 0)
      dialog->deleteLater();
   dialog = 0;

   requestedFeatures = features;
}

void KAccessApp::slotArtsBellTimeout()
{
  _artsBellBlocked = false;
}

void KAccessApp::setXkbOpcode(int opcode)
{
  xkb_opcode = opcode;
}
