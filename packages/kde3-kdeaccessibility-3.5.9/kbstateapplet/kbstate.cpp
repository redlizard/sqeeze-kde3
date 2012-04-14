/*
 *  Copyright (c) 2004 Gunnar Schmi Dt <gunnar@schmi-dt.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */

#include <qpainter.h>
#include <qtooltip.h>
#include <qdrawutil.h>
#include <qcursor.h>
#include <qimage.h>
#include <qpopupmenu.h>

#include <kaboutapplication.h>
#include <kpopupmenu.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <knotifyclient.h>
#include <kshortcut.h>
#include <kkeynative.h>
#include <kdemacros.h>
#include <kprocess.h>
#include "kdeexportfix.h"
#include "kbstate.h"
#include "kbstate.moc"

extern "C"
{
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/keysymdef.h>
#include <X11/extensions/XKB.h>

    KDE_EXPORT KPanelApplet* init(QWidget *parent, const QString& configFile)
    {
        KGlobal::locale()->insertCatalogue("kbstateapplet");
        KbStateApplet *applet = new KbStateApplet(configFile, KPanelApplet::Normal, KPanelApplet::About, parent, "kbstateapplet");
        return applet;
    }
}

struct ModifierKey {
   const unsigned int mask;
   const KeySym keysym;
   const char *name;
   const char *icon;
   const char *text;
   const bool isModifier;
};

static ModifierKey modifierKeys[] = {
    { ShiftMask, 0, I18N_NOOP("Shift"), "shiftkey", "", true },
    { ControlMask, 0, I18N_NOOP("Control"), "controlkey", "", true },
    { 0, XK_Alt_L, I18N_NOOP("Alt"), "altkey", "", true },
    { 0, 0, I18N_NOOP("Win"), "superkey", "", true },
    { 0, XK_Meta_L, I18N_NOOP("Meta"), "metakey", "", true },
    { 0, XK_Super_L, I18N_NOOP("Super"), "superkey", "", true },
    { 0, XK_Hyper_L, I18N_NOOP("Hyper"), "hyperkey", "", true },
    { 0, 0, I18N_NOOP("Alt Graph"), "", I18N_NOOP("æ"), true },
	 { 0, XK_Num_Lock, I18N_NOOP("Num Lock"), "lockkey", I18N_NOOP("Num"), false },
    { LockMask, 0, I18N_NOOP("Caps Lock"), "capskey", "", false },
	 { 0, XK_Scroll_Lock, I18N_NOOP("Scroll Lock"), "lockkey", I18N_NOOP("Scroll"), false },
    { 0, 0, "", "", "", false }
};


/********************************************************************/

KbStateApplet::KbStateApplet(const QString& configFile, Type t, int actions,
                       QWidget *parent, const char *name)
  : KPanelApplet( configFile, t, actions, parent, name )
{
   for (int i = 0; i < 8; i++) {
      icons[i] = 0;
   }
   instance = new KInstance ("kbstateapplet");
   loadConfig();
	initMasks();
	mouse    = new MouseIcon (instance, this, "mouse");
	sticky   = new TimeoutIcon (instance, "", "kbstate_stickykeys", this, "sticky");
	slow   = new TimeoutIcon (instance, "", "kbstate_slowkeys", this, "slow");
	bounce = new TimeoutIcon (instance, "", "", this, "bounce");

	xkb = XkbGetMap(qt_xdisplay(), 0, XkbUseCoreKbd);

	if (xkb != 0) {
		XkbGetControls (qt_xdisplay(), XkbAllControlsMask, xkb);
		if (xkb->ctrls != 0)
			accessxFeatures = xkb->ctrls->enabled_ctrls;
		else
			accessxFeatures = 0;
	}
	else
		accessxFeatures = 0;

   //startTimer(100); // ten times a second
   connect(kapp, SIGNAL(kdisplayPaletteChanged()), SLOT(paletteChanged()));
   
   kapp->installX11EventFilter (this);
   int opcode_rtn, error_rtn;
   XkbQueryExtension (this->x11Display(), &opcode_rtn, &xkb_base_event_type,  &error_rtn, NULL, NULL);
   XkbSelectEvents (this->x11Display(), XkbUseCoreKbd, XkbAllEventsMask,
						  XkbAllEventsMask);
   
	buildPopupMenu();
}

KbStateApplet::~KbStateApplet() {
   kapp->removeX11EventFilter (this);
   setCustomMenu(0);
   delete instance;
   delete popup;
   delete sizePopup;
}
// Builds, connects _popup menu
void KbStateApplet::buildPopupMenu()
{
   sizePopup=new KPopupMenu(this);
   sizePopup->setCheckable( true );
   sizePopup->insertItem(i18n("Small"),  13);
   sizePopup->insertItem(i18n("Medium"), 20);
   sizePopup->insertItem(i18n("Large"),  26);
   connect(sizePopup,SIGNAL(activated(int)), this, SLOT(setIconDim(int)));

   showPopup=new KPopupMenu(this);
   showPopup->setCheckable( true );
   modifierItem=showPopup->insertItem(i18n("Modifier Keys"), this, SLOT(toggleModifier()));
	lockkeysItem=showPopup->insertItem(i18n("Lock Keys"),     this, SLOT(toggleLockkeys()));
	mouseItem=showPopup->insertItem(i18n("Mouse Status"), this, SLOT(toggleMouse()));
	accessxItem=showPopup->insertItem(i18n("AccessX Status"), this, SLOT(toggleAccessX()));

	popup = new KPopupMenu(this);
	popup->setCheckable( true );
   popup->insertTitle(0, i18n("Keyboard Status Applet"));
	popup->insertItem(i18n("Set Icon Size"),sizePopup);
	fillSpaceItem = popup->insertItem(i18n("Fill Available Space"),
												 this, SLOT(toggleFillSpace()));
   popup->insertItem(i18n("Show"),showPopup);
   popup->insertItem(i18n("Configure AccessX Features..."), this, SLOT(configureAccessX()));
   popup->insertItem(i18n("Configure Keyboard..."), this, SLOT(configureKeyboard()));
   popup->insertItem(i18n("Configure Mouse..."), this, SLOT(configureMouse()));
   popup->insertSeparator();
   popup->insertItem(i18n("About"), this, SLOT(about()));
   setCustomMenu(popup);
   updateMenu();
}

void KbStateApplet::updateMenu()
{  if (popup) {
		showPopup->setItemChecked (modifierItem, showModifiers);
		showPopup->setItemChecked (lockkeysItem, showLockkeys);
		showPopup->setItemChecked (mouseItem,    showMouse);
		showPopup->setItemChecked (accessxItem,  showAccessX);
		popup->setItemChecked (fillSpaceItem,  fillSpace);
      sizePopup->setItemChecked(13, size == 13);
      sizePopup->setItemChecked(20, size == 20);
      sizePopup->setItemChecked(26, size == 26);
   }
}

void calculateSizes (int space, int modifiers, int lockkeys, int accessx,
							bool showMouse, int &lines, int &length, int &size)
// Calculates the layout based on a given number of modifiers, lockkeys and
// accessx features.
// Output:
// lines:  number of lines
// length: number of icons per line
// size:   size of the icons
{
   // Calculate lines and length
	if (showMouse)
		++accessx;

	lines = space>=size ? space/size : 1;
	length = modifiers + lockkeys + accessx;

	if (length > 0 && lines >= 2) {
      length = (length + lines-1)/lines;
   
      // As we want to have some line breaks, we need to do some corrections:
      // Calculate the number of lines that are really needed:
		int linesNeeded = (modifiers+length-1)/length + (lockkeys+length-1)/length;
		int tmp1 = modifiers%length == 0 ? 0 : length - modifiers%length;
		int tmp2 = lockkeys%length == 0 ? 0 : length - lockkeys%length;
		if ((tmp1 + tmp2) < accessx)
			linesNeeded = (modifiers+lockkeys+accessx + length-1)/length;

      // If we need more lines than we have, try with more icons per line:
      while (linesNeeded > lines) {
         length++;
			linesNeeded = (modifiers+length-1)/length + (lockkeys+length-1)/length;
			int tmp1 = modifiers%length == 0 ? 0 : length - modifiers%length;
			int tmp2 = lockkeys%length == 0 ? 0 : length - lockkeys%length;
			if ((tmp1 + tmp2) < accessx)
				linesNeeded = (modifiers+lockkeys+accessx + length-1)/length;
      }
      lines = linesNeeded;
   }
}

int KbStateApplet::widthForHeight(int h) const {
   int lines, length;
   int size = this->size;

	int accessx = 0;
	if ((accessxFeatures & XkbStickyKeysMask) != 0)
		accessx++;
	if ((accessxFeatures & XkbSlowKeysMask) != 0)
		accessx++;
	if ((accessxFeatures & XkbBounceKeysMask) != 0)
		accessx++;

   calculateSizes (h, showModifiers?modifiers.count():0,
                      showLockkeys?lockkeys.count():0,
							 showAccessX?accessx:0,
							 showMouse, lines, length, size);

	if (fillSpace)
		size = h/lines;

   return length*size;
}

int KbStateApplet::heightForWidth(int w) const {
   int lines, length;
	int size = this->size;

	int accessx = 0;
	if ((accessxFeatures & XkbStickyKeysMask) != 0)
		accessx++;
	if ((accessxFeatures & XkbSlowKeysMask) != 0)
		accessx++;
	if ((accessxFeatures & XkbBounceKeysMask) != 0)
		accessx++;

   calculateSizes (w, showModifiers?modifiers.count():0,
                      showLockkeys?lockkeys.count():0,
							 showAccessX?accessx:0,
							 showMouse, lines, length, size);

	if (fillSpace)
		size = w/lines;

   return length*size;
}

void KbStateApplet::mousePressEvent(QMouseEvent *e) {
   if (e->button() == RightButton)
      popup->popup(e->globalPos());
}

void KbStateApplet::setIconDim (int size) {
   this->size = size;
   saveConfig();
   updateMenu();
   update();
   updateGeometry();
   emit updateLayout();
}

void KbStateApplet::toggleFillSpace() {
	fillSpace = !fillSpace;
	saveConfig();
	updateMenu();
	layout();
	updateGeometry();
	emit updateLayout();
}

void KbStateApplet::resizeEvent( QResizeEvent*e ) {
   QWidget::resizeEvent(e);
   layout();
}

void KbStateApplet::layout() {
	int size = this->size;

   int lines, length, x,y,dx,dy, ldx,ldy;
	int modifierCount = showModifiers?modifiers.count():0;
	int lockkeyCount  = showLockkeys?lockkeys.count():0;
	int accessxCount  = 0;

	if (showAccessX) {
		if ((accessxFeatures & XkbStickyKeysMask) != 0)
			accessxCount++;
		if ((accessxFeatures & XkbSlowKeysMask) != 0)
			accessxCount++;
		if ((accessxFeatures & XkbBounceKeysMask) != 0)
			accessxCount++;
	}

   if (orientation() == Vertical) {
		calculateSizes (width(), modifierCount, lockkeyCount, accessxCount,
							 showMouse, lines, length, size);
		
		if (fillSpace)
			size = width()/lines;

      x  = (width()-lines*size)/2;
      y  = 0;
      dx = 0;
      dy = size;
      ldx= size;
      ldy= 0;
   }
   else {
		calculateSizes (height(), modifierCount, lockkeyCount, accessxCount,
							 showMouse, lines, length, size);
		
		if (fillSpace)
			size = height()/lines;

      x  = 0;
      y  = (height()-lines*size)/2;
      dx = size;
      dy = 0;
      ldx= 0;
      ldy= size;
   }

   StatusIcon *icon;
   int item = 1;
   for (icon = modifiers.first(); icon; icon = modifiers.next()) {
      if (showModifiers) {
         icon->setGeometry (x, y, size, size);
         icon->show();
         icon->update();
         item++; x+=dx; y+=dy;
         if (item > length) {
            x = x - length*dx + ldx;
            y = y - length*dy + ldy;
            item = 1;
         }
      }
      else
         icon->hide();
   }

	int lockkeyLines = (lockkeyCount+length-1)/length;
	int accessxLines = lines - (modifierCount+length-1)/length - lockkeyLines;

	if (showMouse)
		++accessxCount;

	if (lockkeyLines*length + accessxLines*length
			>= lockkeyCount + accessxCount)
	{
		if (lines > 1 && item > 1) {
			x = x - (item-1)*dx + ldx;
			y = y - (item-1)*dy + ldy;
			item = 1;
		}
	}
	else {
		accessxLines++;
	}

	if (showMouse && showAccessX && accessxLines > 0) {
		mouse->setGeometry (x, y, size, size);
		mouse->show();
		mouse->update();
		accessxCount--;
		item++; x+=dx; y+=dy;
		if (item > length) {
			x = x - length*dx + ldx;
			y = y - length*dy + ldy;
			item = 1;
			accessxLines--;
		}
	}
	else
		mouse->hide();

	if ((accessxFeatures & XkbStickyKeysMask) != 0
			 && showAccessX && accessxLines > 0)
	{
		sticky->setGeometry (x, y, size, size);
		sticky->show();
		sticky->update();
		accessxCount--;
		item++; x+=dx; y+=dy;
		if (item > length) {
			x = x - length*dx + ldx;
			y = y - length*dy + ldy;
			item = 1;
			accessxLines--;
		}
	}
	else
		sticky->hide();

	if ((accessxFeatures & XkbSlowKeysMask) != 0
			&& showAccessX && accessxLines > 0)
	{
		slow->setGeometry (x, y, size, size);
		slow->show();
		slow->update();
		accessxCount--;
		item++; x+=dx; y+=dy;
		if (item > length) {
			x = x - length*dx + ldx;
			y = y - length*dy + ldy;
			item = 1;
			accessxLines--;
		}
   }
   else
		slow->hide();

	if ((accessxFeatures & XkbBounceKeysMask) != 0
			&& showAccessX && accessxLines > 0)
	{
		bounce->setGeometry (x, y, size, size);
		bounce->show();
		bounce->update();
		accessxCount--;
		item++; x+=dx; y+=dy;
		if (item > length) {
			x = x - length*dx + ldx;
			y = y - length*dy + ldy;
			item = 1;
			accessxLines--;
		}
	}
	else
		bounce->hide();

   if (lines > 1) {
      if (item != 1) {
         x = x - (item-1)*dx + ldx;
         y = y - (item-1)*dy + ldy;
      }
      item = 1;
   }
   for (icon = lockkeys.first(); icon; icon = lockkeys.next()) {
      if (showLockkeys) {
         icon->setGeometry (x, y, size, size);
         icon->show();
         icon->update();
         item++; x+=dx; y+=dy;
         if (item > length) {
            x = x - length*dx + ldx;
            y = y - length*dy + ldy;
            item = 1;
         }
      }
      else
         icon->hide();
	}

	if ((accessxFeatures & XkbBounceKeysMask) != 0
			 && showAccessX && accessxCount > 0)
	{
		bounce->setGeometry (x, y, size, size);
		bounce->show();
		bounce->update();
		item++; x+=dx; y+=dy;
		accessxCount--;
	}

	if ((accessxFeatures & XkbSlowKeysMask) != 0
			 && showAccessX && accessxCount > 0)
	{
		slow->setGeometry (x, y, size, size);
		slow->show();
		slow->update();
		item++; x+=dx; y+=dy;
		accessxCount--;
	}

	if ((accessxFeatures & XkbStickyKeysMask) != 0
			 && showAccessX && accessxCount > 0)
	{
		sticky->setGeometry (x, y, size, size);
		sticky->show();
		sticky->update();
		item++; x+=dx; y+=dy;
		accessxCount--;
	}

	if (showMouse && accessxCount > 0)
	{
		mouse->setGeometry (x, y, size, size);
		mouse->show();
		mouse->update();
		item++; x+=dx; y+=dy;
		accessxCount--;
	}
}

void KbStateApplet::paletteChanged() {
   for (int i = 0; i < 8; i++) {
      if (icons[i])
         icons[i]->updateImages();
	}
	mouse->update();
	sticky->update();
	slow->update();
	bounce->update();
}

void KbStateApplet::initMasks() {
   for (int i = 0; i < 8; i++) {
      if (icons[i])
         delete icons[i];
      icons[i] = 0;
   }
   state = 0;
   
   for (int i = 0; strcmp(modifierKeys[i].name, "") != 0; i++) {
      int mask = modifierKeys[i].mask;
      if (mask == 0)
         if (modifierKeys[i].keysym != 0)
            mask = XkbKeysymToModifiers (this->x11Display(), modifierKeys[i].keysym);
         else if (!strcmp(modifierKeys[i].name, "Win"))
            mask = KKeyNative::modX(KKey::WIN);
         else
            mask = XkbKeysymToModifiers (this->x11Display(), XK_Mode_switch)
                 | XkbKeysymToModifiers (this->x11Display(), XK_ISO_Level3_Shift)
                 | XkbKeysymToModifiers (this->x11Display(), XK_ISO_Level3_Latch)
                 | XkbKeysymToModifiers (this->x11Display(), XK_ISO_Level3_Lock);
      
      int map = 0;
      for (map = 0; map < 8; map++) {
         if ((mask & (1 << map)) != 0)
            break;
      }
      if ((map <= 7) && !(icons[map])) {
         icons[map] = new KeyIcon (i, instance, this, modifierKeys[i].name);
         QToolTip::add (icons[map], i18n (modifierKeys[i].name));
         connect (icons[map], SIGNAL(stateChangeRequest (KeyIcon*,bool,bool)),
                                SLOT(stateChangeRequest (KeyIcon*,bool,bool)));
         if (modifierKeys[i].isModifier)
            modifiers.append(icons[map]);
         else
            lockkeys.append(icons[map]);
      }
   }
}

bool KbStateApplet::x11Event (XEvent *evt) {
   if (evt->type == xkb_base_event_type + XkbEventCode) {
      XkbEvent *kbevt = (XkbEvent *)evt;
      switch (kbevt->any.xkb_type) {
      case XkbStateNotify:
         timerEvent (0);
			
			mouse->setState (kbevt->state.ptr_buttons);
         break;
      case XkbAccessXNotify:
         switch (kbevt->accessx.detail) {
         case XkbAXN_SKPress:
				slow->setGlyth(i18n("a (the first letter in the alphabet)", "a"));
				slow->setImage("unlatched");
            break;
         case XkbAXN_SKAccept:
            slow->setImage("keypressok");
            break;
         case XkbAXN_SKRelease:
            slow->setGlyth(" ");
            slow->setImage("kbstate_slowkeys");
            break;
         case XkbAXN_SKReject:
            slow->setImage("keypressno", kbevt->accessx.sk_delay>150?kbevt->accessx.sk_delay:150);
            break;
				case XkbAXN_BKAccept:
					slow->setGlyth(i18n("a (the first letter in the alphabet)", "a"));
					bounce->setImage("keypressok", kbevt->accessx.sk_delay>150?kbevt->accessx.sk_delay:150);
            break;
				case XkbAXN_BKReject:
					slow->setGlyth(i18n("a (the first letter in the alphabet)", "a"));
					bounce->setImage("keypressno", kbevt->accessx.sk_delay>150?kbevt->accessx.sk_delay:150);
            break;
         }
         break;
			case XkbControlsNotify: {
				XkbControlsNotifyEvent* event = (XkbControlsNotifyEvent*)evt;
				accessxFeatures = event->enabled_ctrls;
				
				if ((accessxFeatures & XkbMouseKeysMask) != 0) {
					XkbGetControls (qt_xdisplay(), XkbMouseKeysMask, xkb);
					if (xkb->ctrls->mk_dflt_btn < 1)
						mouse->setActiveKey (1);
					else if (xkb->ctrls->mk_dflt_btn > 3)
						mouse->setActiveKey (1);
					else
						mouse->setActiveKey (xkb->ctrls->mk_dflt_btn);
				}
				else
					mouse->setActiveKey (0);

				layout();
				updateGeometry();
				emit updateLayout();
				break;
			}
      case XkbExtensionDeviceNotify:
         /* This is a hack around the fact that XFree86's XKB doesn't give AltGr notifications */
         break;
      default:
         break;
      }
   }
   return false;
}

void KbStateApplet::timerEvent(QTimerEvent*) {
   XkbStateRec state_return;
   XkbGetState (this->x11Display(), XkbUseCoreKbd, &state_return);
   unsigned char latched = XkbStateMods (&state_return);
   unsigned char locked  = XkbModLocks  (&state_return);
   int mods = ((int)latched)<<8 | locked;
   
   if (state != mods) {
      state = mods;
      for (int i = 0; i < 8; i++) {
         if (icons[i])
            icons[i]->setState ((latched&(1<<i)) != 0, (locked&(1<<i)) != 0);
      }
	}
}

void KbStateApplet::stateChangeRequest (KeyIcon *source, bool latched, bool locked) {
   for (int i = 0; i < 8; i++) {
      if (icons[i] == source) {
         if (locked)
            XkbLockModifiers (this->x11Display(), XkbUseCoreKbd, 1<<i, 1<<i);
         else if (latched) {
            XkbLockModifiers (this->x11Display(), XkbUseCoreKbd, 1<<i, 0);
            XkbLatchModifiers (this->x11Display(), XkbUseCoreKbd, 1<<i, 1<<i);
         }
         else {
            XkbLockModifiers (this->x11Display(), XkbUseCoreKbd, 1<<i, 0);
            XkbLatchModifiers (this->x11Display(), XkbUseCoreKbd, 1<<i, 0);
         }
      }
   }
}


void KbStateApplet::toggleModifier() {
   showModifiers = !showModifiers;
   updateMenu();
   layout();
   updateGeometry();
   saveConfig();
   emit updateLayout();
}

void KbStateApplet::toggleLockkeys() {
   showLockkeys = !showLockkeys;
   updateMenu();
   layout();
   updateGeometry();
   saveConfig();
   emit updateLayout();
}

void KbStateApplet::toggleMouse() {
   showMouse = !showMouse;
   updateMenu();
   layout();
   updateGeometry();
   saveConfig();
   emit updateLayout();
}

void KbStateApplet::toggleAccessX() {
	showAccessX = !showAccessX;
	updateMenu();
	layout();
	updateGeometry();
	saveConfig();
	emit updateLayout();
}

void KbStateApplet::configureAccessX() {
   kapp->startServiceByDesktopName("kcmaccess");
}

void KbStateApplet::configureKeyboard() {
   // The modulename "keyboard" is ambiguous on SuSE systems
   // as there is also a YaST-module called "keyboard".
   KProcess proc;
   proc << "kcmshell";
   proc << "kde/keyboard";
   proc.start(KProcess::DontCare);
   proc.detach();
}

void KbStateApplet::configureMouse() {
   kapp->startServiceByDesktopName("mouse");
}

void KbStateApplet::about() {
   KAboutData about("kbstateapplet", I18N_NOOP("Keyboard Status Applet"), "0.2",
                    I18N_NOOP("Panel applet that shows the state of the modifier keys"), KAboutData::License_GPL_V2, "(C) 2004 Gunnar Schmi Dt");
   KAboutApplication a(&about, this);
   a.exec();
}

void KbStateApplet::loadConfig()
{
   KConfig *c = config();
   c->setGroup("General");
   size = c->readNumEntry("IconDim", 20);
	fillSpace    = c->readBoolEntry("fill space", true);
	showModifiers = c->readBoolEntry("Modifierkeys visible",true);
	showLockkeys  = c->readBoolEntry("Lockkeys visible",true);
	showMouse    = c->readBoolEntry("Mouse status visible",true);
	showAccessX  = c->readBoolEntry("Slowkeys status visible",true);
	showAccessX  = c->readBoolEntry("AccessX status visible",showAccessX);
}

void KbStateApplet::saveConfig()
{
   KConfig *c = config();
   c->setGroup("General");
	c->writeEntry("IconDim", size);
	c->writeEntry("fill space", fillSpace);
   c->writeEntry("Modifierkeys visible",    showModifiers);
   c->writeEntry("Lockkeys visible",        showLockkeys);
	c->writeEntry("Mouse status visible",    showMouse);
	c->writeEntry("AccessX status visible",  showAccessX);
   c->sync();
}

/********************************************************************/

KeyIcon::KeyIcon (int keyId, KInstance *instance,
                  QWidget *parent, const char *name)
 : StatusIcon (modifierKeys[keyId].name, parent, name) {
   this->instance = instance;
   this->keyId = keyId;
   this->tristate = (modifierKeys[keyId].isModifier);
   isLocked  = false;
   isLatched = false;
   updateImages ();
   connect (this, SIGNAL(clicked()), SLOT(clickedSlot()));
}

KeyIcon::~KeyIcon () {
}

void KeyIcon::setState (bool latched, bool locked) {
   latched = latched | locked;

   isLatched = latched;
   isLocked  = locked;
   update();
}

void KeyIcon::clickedSlot () {
   if (tristate)
      emit stateChangeRequest (this, !isLocked, isLatched&!isLocked);
   else
      emit stateChangeRequest (this, false, !isLocked);
}


void KeyIcon::resizeEvent( QResizeEvent*e )
{
   QWidget::resizeEvent(e);
   updateImages();
}

void KeyIcon::updateImages () {
   int size = width()<height() ? width() : height();

   locked    = instance->iconLoader()->loadIcon("lock_overlay", KIcon::Panel, size-4);
	if (strcmp(modifierKeys[keyId].icon, "")) {
	   latched   = instance->iconLoader()->loadIcon(modifierKeys[keyId].icon, KIcon::NoGroup, size-4);
   	unlatched = instance->iconLoader()->loadIcon(modifierKeys[keyId].icon, KIcon::NoGroup, size-4);
		
		QImage img = latched.convertToImage();
		KIconEffect::colorize(img,  KGlobalSettings::highlightedTextColor(), 1.0);
		latched.convertFromImage (img);
		
		img = unlatched.convertToImage();
		KIconEffect::colorize(img, KGlobalSettings::textColor(), 1.0);
		unlatched.convertFromImage (img);
	}

   update();
}

void KeyIcon::drawButton (QPainter *p) {
   QColor black;

   int x = (width()-locked.width())/2;
   int y = (height()-locked.height())/2;
   int o = 0;
   if (isLocked || isLatched) {
      qDrawShadePanel (p, 0, 0, width(), height(), colorGroup(), true, 1, NULL);
      p->fillRect (1,1,width()-2,height()-2, KGlobalSettings::highlightColor());
		if (strcmp(modifierKeys[keyId].icon, ""))
			p->drawPixmap (x+1,y+1, latched);
      black = KGlobalSettings::highlightedTextColor();
      o = 1;
   }
   else {
      qDrawShadePanel (p, 0, 0, width(), height(), colorGroup(), false, 1, NULL);
		if (strcmp(modifierKeys[keyId].icon, ""))
			p->drawPixmap (x,y, unlatched);
      black = KGlobalSettings::textColor();
   }

   QString text = i18n(modifierKeys[keyId].text);
   if (!text.isEmpty() && !text.isNull()) {
      QFont font = KGlobalSettings::generalFont();
      font.setWeight(QFont::Black);
      QFontMetrics metrics(font);
      QRect rect = metrics.boundingRect (text);
      int size;
      if (!strcmp(modifierKeys[keyId].name, "Alt Graph"))
         size = rect.width()>rect.height()?rect.width():rect.height();
      else
         size = rect.width()>12*rect.height()/5?rect.width():12*rect.height()/5;

      if (font.pixelSize() != -1)
         font.setPixelSize (font.pixelSize()*width()*19/size/32);
      else
         font.setPointSizeFloat (font.pointSizeFloat()*width()*19/size/32);

      p->setPen (black);
      p->setFont (font);
      if (!strcmp(modifierKeys[keyId].name, "Alt Graph"))
      p->drawText (o,o, width(), height(), Qt::AlignCenter, text);
      else
         p->drawText (o,o, width(), height()*(251)/384, Qt::AlignCenter, text);
   }
   if (tristate && isLocked) {
      p->drawPixmap(x+o,y+o, locked);
   }
}

/********************************************************************/

MouseIcon::MouseIcon (KInstance *instance, QWidget *parent, const char *name)
	: StatusIcon ("", parent, name)
{
	this->instance = instance;
	state = 0;
	activekey = 0;
	updateImages ();
	connect (this, SIGNAL(clicked()), SLOT(clickedSlot()));
}

MouseIcon::~MouseIcon () {
}

void MouseIcon::setState (int state) {
	this->state = state;

	update();
}

void MouseIcon::setActiveKey (int activekey) {
	this->activekey = activekey;

	update();
}

void MouseIcon::resizeEvent( QResizeEvent*e )
{
	QWidget::resizeEvent(e);
	updateImages();
}

QPixmap loadIcon(KInstance *instance, int size, QColor color, QString name) {
	KIconLoader *loader = instance->iconLoader();
	QPixmap result = loader->loadIcon(name, KIcon::NoGroup, size);

	QImage img = result.convertToImage();
	KIconEffect::colorize(img, color, 1.0);
	result.convertFromImage (img);
	
	return result;
}

void MouseIcon::updateImages () {
	int size = width()<height() ? width() : height();

	QColor textcolor = KGlobalSettings::textColor();
	QColor basecolor = KGlobalSettings::baseColor();
	mouse = loadIcon (instance, size, textcolor, "kbstate_mouse");
	leftSelected   = loadIcon (instance, size, textcolor,
										"kbstate_mouse_left_selected");
	middleSelected = loadIcon (instance, size, textcolor,
										"kbstate_mouse_mid_selected");
	rightSelected  = loadIcon (instance, size, textcolor,
										"kbstate_mouse_right_selected");
	leftDot   = loadIcon (instance, size, textcolor, "kbstate_mouse_left");
	middleDot = loadIcon (instance, size, textcolor, "kbstate_mouse_mid");
	rightDot  = loadIcon (instance, size, textcolor, "kbstate_mouse_right");
	leftDotSelected   = loadIcon (instance, size, basecolor,
											"kbstate_mouse_left");
	middleDotSelected = loadIcon (instance, size, basecolor,
											"kbstate_mouse_mid");
	rightDotSelected  = loadIcon (instance, size, basecolor,
											"kbstate_mouse_right");

	update();
}

void MouseIcon::drawButton (QPainter *p) {
	p->drawPixmap(0,0, mouse);
	if ((state & Button1Mask) != 0)
		p->drawPixmap(0,0, leftSelected);
	if ((state & Button2Mask) != 0)
		p->drawPixmap(0,0, middleSelected);
	if ((state & Button3Mask) != 0)
		p->drawPixmap(0,0, rightSelected);
	switch (activekey) {
		case 0:
			break;
		case 1:
			if ((state & Button1Mask) != 0)
				p->drawPixmap(0,0, leftDotSelected);
			else
				p->drawPixmap(0,0, leftDot);
			break;
		case 2:
			if ((state & Button2Mask) != 0)
				p->drawPixmap(0,0, middleDotSelected);
			else
				p->drawPixmap(0,0, middleDot);
			break;
		case 3:
			if ((state & Button3Mask) != 0)
				p->drawPixmap(0,0, rightDotSelected);
			else
				p->drawPixmap(0,0, rightDot);
			break;
	}
}

/********************************************************************/

TimeoutIcon::TimeoutIcon (KInstance *instance, const QString &text,
								  const QString &featurename,
								  QWidget *parent, const char *name)
 : StatusIcon (text, parent, name) {
	this->instance = instance;
	this->featurename = featurename;
	glyth = " ";
	setImage (featurename);
   connect (&timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

TimeoutIcon::~TimeoutIcon () {
}

void TimeoutIcon::update () {
   int size = width()<height() ? width() : height();
   if (pixmap.width() != size)
		pixmap = instance->iconLoader()->loadIcon(iconname, KIcon::NoGroup, size);

   QImage img = pixmap.convertToImage();
   KIconEffect::colorize(img, KGlobalSettings::textColor(), 1.0);
   pixmap.convertFromImage (img);

   image = pixmap;
   QWidget::update();
}

void TimeoutIcon::setGlyth (const QString &glyth) {
   timer.stop();
   this->glyth = glyth;

   QImage img = pixmap.convertToImage();
   KIconEffect::colorize(img, KGlobalSettings::textColor(), 1.0);
   pixmap.convertFromImage (img);

   image = pixmap;
   update();
}

void TimeoutIcon::setImage (const QString &name, int timeout) {
   timer.stop();
   iconname = name;
	if (!name.isNull() && !name.isEmpty()) {
	   int size = width()<height() ? width() : height();
		pixmap = instance->iconLoader()->loadIcon(iconname, KIcon::NoGroup, size);

   	QImage img = pixmap.convertToImage();
   	KIconEffect::colorize(img, KGlobalSettings::textColor(), 1.0);
   	pixmap.convertFromImage (img);

   	image = pixmap;
	}
   update();
   if (timeout > 0)
      timer.start (timeout, true);
}

void TimeoutIcon::timeout () {
	setGlyth (" ");
	setImage(featurename);
}


void TimeoutIcon::drawButton (QPainter *p) {
	QString text = glyth;
	int count  = 1;
	int factor = 19;

	if (!iconname.isNull() && !iconname.isEmpty())
	   p->drawPixmap(0,0, image);
	else if (glyth == " ") {
		text = i18n("a (the first letter in the alphabet)", "a");
		count = 3;
		factor = 64;
	}

   QFont font = KGlobalSettings::generalFont();
   font.setWeight(QFont::Black);
   QFontMetrics metrics(font);
   QRect rect = metrics.boundingRect (text);
   int size = count*rect.width() > rect.height()
			? count*rect.width() : rect.height();
   if (font.pixelSize() != -1)
      font.setPixelSize (font.pixelSize()*width()*factor/size/64);
   else
      font.setPointSizeFloat (font.pointSizeFloat()*width()*factor/size/64);

   p->setFont (font);
	if (count == 1) {
		p->setPen (KGlobalSettings::textColor());
	   p->drawText (0,0, width()/2, height()/2, Qt::AlignCenter, text);
	}
	else {
		QColor t = KGlobalSettings::textColor();
		QColor b = KGlobalSettings::baseColor();
		p->setPen (QColor ((2*t.red()+3*b.red())/5,
					  (2*t.green()+3*b.green())/5,
					  (2*t.blue()+3*b.blue())/5));
		p->drawText (width()/2,0, width()/2, height(), Qt::AlignCenter, text);
		p->setPen (QColor ((2*t.red()+b.red())/3,
					  (2*t.green()+b.green())/3,
					  (2*t.blue()+b.blue())/3));
		p->drawText (0,0, width(), height(), Qt::AlignCenter, text);
		p->setPen (KGlobalSettings::textColor());
		p->drawText (0,0, width()/2, height(), Qt::AlignCenter, text);
	}
}

/********************************************************************/

StatusIcon::StatusIcon (const QString &text, QWidget *parent, const char *name)
 : QPushButton (text, parent, name) {
   setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
}

StatusIcon::~StatusIcon () {
}

QSize StatusIcon::minimumSizeHint () const {
   return QSize (0,0);
}
