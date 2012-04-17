/*
   Copyright (C) 2006 Michael Lentner <michaell@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

bool DominoStyle::eventFilter(QObject *obj, QEvent *ev)
{
	
// 	qDebug("className:   %s  --  name:  %s", obj->className(), obj->name());
	
	if(dynamic_cast<QComboBox*>(obj)) {
		
		switch(ev->type()) {
			case QEvent::Paint: {
				
				QComboBox* cb = dynamic_cast<QComboBox*>(obj);
				bool khtmlWidget = khtmlWidgets.contains(cb);
				const QColorGroup & g = cb->colorGroup();
				
				QPixmap buffer;
				QPainter p;
				if(khtmlWidget)
					p.begin(cb);
				else {
					buffer.resize(cb->width(), cb->height());
					p.begin(&buffer);
				}
				
				p.setFont(cb->font());
				
				
				if(!khtmlWidget && cb->parentWidget() && cb->parentWidget()->backgroundPixmap()){
					
					QPoint point = cb->backgroundOffset();
					p.drawTiledPixmap( 0, 0, cb->width(), cb->height(),
							*cb->parentWidget()->backgroundPixmap(),
							point.x(), point.y() );
				}
				
				
				QStyle::SFlags flags = QStyle::Style_Default;
				if (cb->isEnabled())
					flags |= QStyle::Style_Enabled;
				if (cb->hasFocus())
					flags |= QStyle::Style_HasFocus;
				
				if(!cb->listBox()) {
					
					drawComplexControl( QStyle::CC_ComboBox, &p, cb, cb->rect(), g,
							flags, (uint)QStyle::SC_All, /*(cb->arrowDown ? */QStyle::SC_ComboBoxArrow /*: QStyle::SC_None )*/);
					
					QRect re = querySubControlMetrics( QStyle::CC_ComboBox, cb, QStyle::SC_ComboBoxEditField );
					re = QStyle::visualRect(re, cb);
					p.setClipRect( re );
					
					QString str = 0;
					const QPixmap *pix = 0;
					if(cb->count() > 0) {
						str = cb->text(cb->currentItem());
						cb->pixmap(cb->currentItem());
					}
					if ( !str.isNull() ) {
// 						p.save();
// 						p.setFont(cb->font());
// 						QFontMetrics fm(cb->font());
// 						int x = re.x(), y = re.y() + fm.ascent();
// 						if( pix )
// 							x += pix->width() + 5;
// 						p.drawText( x, y, str );
// 						p.restore();
						dominoDrawItem( &p, re, AlignCenter, cb->colorGroup(),
								cb->isEnabled(), 0, str, -1,
								&cb->colorGroup().buttonText(), textEffectSettings.mode > 0 ? 1 : 0 );
					}
					if ( pix ) {
						p.fillRect( re.x(), re.y(), pix->width() + 4, re.height(),
								cb->colorGroup().brush( QColorGroup::Base ) );
						p.drawPixmap( re.x() + 2, re.y() +
								( re.height() - pix->height() ) / 2, *pix );
					}
				}
				else {
					drawComplexControl( QStyle::CC_ComboBox, &p, cb, cb->rect(), g,
							flags, (uint)QStyle::SC_All, /*(d->arrowDown ? */QStyle::SC_ComboBoxArrow/* : QStyle::SC_None )*/);
					QRect re = querySubControlMetrics( QStyle::CC_ComboBox, cb, QStyle::SC_ComboBoxEditField );
					re = QStyle::visualRect(re, cb);
					p.setClipRect( re );
					
					if ( !cb->editable() ) {
						
						QString str = 0;
						const QPixmap *pix = 0;
						if(cb->count() > 0) {
							str = cb->text(cb->currentItem());
							pix = cb->pixmap(cb->currentItem());
						}
						
						re.addCoords(+2, 0, 0, -1);
						if(pix) {
							drawItem( &p, QRect(re.left()+1, re.top(), re.width(), re.height()-1), AlignLeft|AlignVCenter, cb->colorGroup(),
									cb->isEnabled(), pix, 0, -1, &cb->colorGroup().buttonText() );
							re.setX(re.x()+pix->width()+2);
						}
						if(!str.isNull()) {
							dominoDrawItem( &p, QRect(re.left(), re.top(), re.width(), re.height()), AlignLeft|AlignVCenter, cb->colorGroup(),
									cb->isEnabled(), 0, str, -1, &cb->colorGroup().buttonText(), textEffectSettings.mode > 0 ? 1 : 0 );
						}
						else if(!pix) {
							DominoQListBoxItem * item = (DominoQListBoxItem*)cb->listBox()->item( cb->currentItem() );
							if ( item ) {
								int itemh = item->height( cb->listBox() );
								p.translate( re.x(), re.y() +(re.height()- itemh)/2);
								item->paint( &p );
							}
						}
					}
					else if ( cb->listBox() && cb->listBox()->item( cb->currentItem() ) ) {
						
						QListBoxItem * item = cb->listBox()->item( cb->currentItem() );
						const QPixmap *pix = item->pixmap();
						if ( pix ) {
							if(re.width() < pix->width())
								return true;
							QColor color = cb->isEnabled() ? cb->lineEdit()->paletteBackgroundColor() : cb->lineEdit()->palette().active().background();
							QPixmap* shadowPix = renderLineEditShadow(cb, QRect(0, 0, pix->width() + 3, re.height()), color, Draw_Left, QApplication::reverseLayout());
	
							if(!QApplication::reverseLayout()) {
								int x = re.x();
								int y = re.y();
								int w = pix->width() + 4;
								int h = re.height()+re.y();
								
								QRegion mask(x+3, y, w+3, h);
								mask += QRegion(x, y+3, 3, h-9);
								p.setClipRegion(mask);
							}
							bitBlt(shadowPix, 3, ( shadowPix->height() - pix->height() ) / 2, pix);
							p.drawPixmap(re.x(), re.y(), *shadowPix);
							delete shadowPix;
						}
					}
				}
				p.end();
				if(!khtmlWidget) {
					p.begin(cb);
					p.drawPixmap(0, 0, buffer);
				}
				
				return true;
				
			}
			case QEvent::Enter: {
				QButton* btn = static_cast<QButton*>(obj);
				if(btn->isEnabled()) {
					hoverWidget = btn;
					btn->repaint(false);
				}
				return false;
			}
			case QEvent::Leave: {
				QButton* btn = static_cast<QButton*>(obj);
				if(btn->isEnabled()) {
					hoverWidget = 0;
					btn->repaint(false);
				}
				return false;
			}
			case QEvent::PaletteChange: {
				setDominoButtonPalette(static_cast<QWidget*>(obj), Palette_Widget);
				return true;
			}
			case QEvent::ParentPaletteChange: {
				setDominoButtonPalette(dynamic_cast<QWidget*>(obj), Palette_Parent);
				return false;
			}
			case QEvent::ApplicationPaletteChange: {
				if(!khtmlWidgets.contains(static_cast<QWidget*>(obj)))
					setDominoButtonPalette(static_cast<QWidget*>(obj), Palette_Application);
				return true;
			}
			default:
				return false;
		}
	}

	// don't repaint the combobox if the mouse moves over the listbox.
	if(dynamic_cast<QListBox*>(obj) && dynamic_cast<QComboBox*>(obj->parent())) {
		
		if(dynamic_cast<QComboBox*>(obj->parent())) {
			if(ev->type() == QEvent::Show) {
				QComboBox* cb = dynamic_cast<QComboBox*>(obj->parent());
				cb->repaint(false);
				cb->setUpdatesEnabled(false);
				return false;
			}
			if(ev->type() == QEvent::Hide) {
				QComboBox* cb = dynamic_cast<QComboBox*>(obj->parent());
				cb->setUpdatesEnabled(true);
				cb->repaint(false);
				return false;
			}
		}
		// don't return here
	}
	
	if (dynamic_cast<QLineEdit*>(obj) || (dynamic_cast<QFrame*>(obj) && dynamic_cast<QFrame*>(obj)->frameShape() == QFrame::LineEditPanel) || obj->isA("QDateTimeEditor")) {
		
		QWidget* lineEdit = dynamic_cast<QWidget*>(obj);
		bool khtmlMode = khtmlWidgets.contains(lineEdit);
		bool isLineEdit = dynamic_cast<QLineEdit*>(obj);
		bool isReadOnly = isLineEdit && dynamic_cast<QLineEdit*>(obj)->isReadOnly();
		QFrame* frame = dynamic_cast<QFrame*>(obj);
		if((isReadOnly || khtmlMode) && frame->frameWidth() <= 0) {
			return false;
		}
		
		bool isEnabled = lineEdit->isEnabled();
		bool isSpinWidget = false;
		bool isComboBox = false;
		bool isDateTimeEditor = false;
		bool hasPixmap = false;
		bool noFrame = false;
		QRect r = lineEdit->rect();
		
		if(dynamic_cast<QComboBox*>(lineEdit->parentWidget())) {
			isComboBox = true;
			QComboBox* cb = dynamic_cast<QComboBox*>(lineEdit->parentWidget());
			if(cb->count() > 0 && cb->pixmap(cb->currentItem()))
				hasPixmap = true;
		}
		else if(obj->isA("QDateTimeEditor"))
			isDateTimeEditor = true;
		else if(dynamic_cast<QSpinWidget*>(lineEdit->parentWidget()))
			isSpinWidget = true;
		else if(isLineEdit && frame->frameShape() == QFrame::NoFrame)
			noFrame = true;
		
// 		if(isSpinWidget && !isComboBox && frame->frameShape() == QFrame::NoFrame)
// 			return false;
		
		int x, y, w, h;
		r.rect(&x, &y, &w, &h);

		switch(ev->type()) {
			case QEvent::Show:
				if(khtmlMode)
					return true;
			case QEvent::Resize: {
				if(noFrame) return false;
				
				if(isLineEdit) {
					QRect frameRect(r.x()+1, r.y(), r.width()-1, r.height());
					if(frame->frameRect() != frameRect)
						frame->setFrameRect(frameRect);
					
				}
				
				if(isSpinWidget || isComboBox) {
					
					if(!hasPixmap && !QApplication::reverseLayout()) {
						QRegion mask(x+3, y, w-3, h);
						mask += QRegion(x, y+3, 3, h-6);
						lineEdit->setMask(mask);
					}
					else if(QApplication::reverseLayout()) {
						QRegion mask(x, y, w-3, h);
						mask += QRegion(w-3, y+3, 3, h-6);
						lineEdit->setMask(mask);
					}
					else if(isComboBox)
						lineEdit->clearMask();
				}
				else if(isDateTimeEditor) {
					
					if(!QApplication::reverseLayout()) {
						QRegion mask(x+2, y, w-2, h);
						mask += QRegion(x+1, y+1, 1, h-2);
						mask += QRegion(x, y+2, 1, h-4);
						lineEdit->setMask(mask);
					}
					else {
						QRegion mask(x, y, w-2, h);
						mask += QRegion(w-2, y+1, 1, h-2);
						mask += QRegion(w-1, y+2, 1, h-4);
						lineEdit->setMask(mask);
					}
				}
				
				if(isLineEdit) {
					lineEdit->repaint(false); // not realy needed, but removes some flicker
					return true;
				}
				return false;
			}
			case QEvent::Paint: {
				
				if(noFrame)
					return false;
				
				const QColorGroup cg = lineEdit->colorGroup();
				const QColor bg = !isEnabled ? cg.background() : lineEdit->paletteBackgroundColor();
				QPainter p;
				
				if(isSpinWidget || isComboBox || isDateTimeEditor) {
					
					 // QDateTimeEditor doesn't respect background pixmaps :(
					if(w <1 || h <1 || isDateTimeEditor)
						return false;
					
					uint flags = hasPixmap ? Draw_Right : Draw_Left|Draw_Right;
					QPixmap* lineEditPix = renderLineEditShadow(lineEdit, lineEdit->rect(), bg, flags, QApplication::reverseLayout());
					lineEdit->setUpdatesEnabled(false);
					lineEdit->setErasePixmap(*lineEditPix);
					// update the gap which resulted from the altered frame rect...
					lineEdit->erase(QRect(x, y, 1, h));
					lineEdit->setUpdatesEnabled(true);
					delete lineEditPix;
					return false;
				}
				
				
				// normal lineEdit
				const QPixmap* parentBgPix = lineEdit->parentWidget() ? lineEdit->parentWidget()->paletteBackgroundPixmap() : 0;
				QPixmap* lineEditEdges = 0;
				if(khtmlMode || parentBgPix || !(lineEditEdges = QPixmapCache::find(QString::number(bg.pixel(), 16)+QString::number(cg.background().pixel(), 16)+"lEditEdges"))) {
					if(khtmlMode) {
						
						QPixmap insidePix = tintImage(qembed_findImage("lineedit4inside"), bg);
						bitBlt(lineEdit, x, y, &insidePix, 0, 0, 4, 4);
						bitBlt(lineEdit, w-4, y, &insidePix, 4, 0, 4, 4);
						bitBlt(lineEdit, x, h-4, &insidePix, 0, 4, 4, 4);
						bitBlt(lineEdit, w-4, h-4, &insidePix, 4, 4, 4, 4);
						
						QPixmap edgesPix = qembed_findImage("lineedit4edges");
						bitBlt(lineEdit, x, y, &edgesPix, 0, 0, 4, 4);
						bitBlt(lineEdit, w-4, y, &edgesPix, 4, 0, 4, 4);
						bitBlt(lineEdit, x, h-4, &edgesPix, 0, 4, 4, 4);
						bitBlt(lineEdit, w-4, h-4, &edgesPix, 4, 4, 4, 4);
					}
					else {
						
						lineEditEdges = new QPixmap(8, 8);
						if(parentBgPix) {
							QPoint offset = lineEdit->parentWidget()->backgroundOffset();
							offset += QPoint(lineEdit->geometry().x(), lineEdit->geometry().y());
							QPainter painter(lineEditEdges);
							painter.drawTiledPixmap(0, 0, 4, 4, *parentBgPix, offset.x(), offset.y());
							painter.drawTiledPixmap(4, 0, 4, 4, *parentBgPix, offset.x()+r.width()-4, offset.y());
							painter.drawTiledPixmap(0, 4, 4, 4, *parentBgPix, offset.x(), offset.y()+r.height()-4);
							painter.drawTiledPixmap(4, 4, 4, 4, *parentBgPix, offset.x()+r.width()-4, offset.y()+r.height()-4);
						}
						else
							lineEditEdges->fill(cg.background());
						
						QPixmap tmpPix = tintImage(qembed_findImage("lineedit4inside"), bg);
						bitBlt(lineEditEdges, 0, 0, &tmpPix, 0, 0, 8, 8);
						
						QPixmap tmpPix2 = qembed_findImage("lineedit4edges");
						bitBlt(lineEditEdges, 0, 0, &tmpPix2, 0, 0, 8, 8);
						
						if(!parentBgPix)
							QPixmapCache::insert(QString::number(bg.pixel(), 16)+QString::number(cg.background().pixel(), 16)+"lEditEdges", *lineEditEdges);
					}
				}
				
				
				QPixmap* lineEditPix = 0;
				
				if(khtmlMode)
					p.begin(lineEdit);
				else {
					lineEditPix = new QPixmap(w, h);
					p.begin(lineEditPix);
					
					bitBlt(lineEditPix, x, y, lineEditEdges, 0, 0, 4, 4);
					bitBlt(lineEditPix, w-4, y, lineEditEdges, 4, 0, 4, 4);
					bitBlt(lineEditPix, x, h-4, lineEditEdges, 0, 4, 4, 4);
					bitBlt(lineEditPix, w-4, h-4, lineEditEdges, 4, 4, 4, 4);
					
				}
				
				p.fillRect(QRect(x+4, y, w-8, h), bg);
				p.fillRect(QRect(x, y+4, 4, h-8), bg);
				p.fillRect(QRect(w-4, y+4, 4, h-8), bg);
				
				QColor top_1 = alphaBlendColors(QColor(26, 26, 26), bg, 125);
				QColor top_2 = alphaBlendColors(QColor(18, 18, 18), bg, 50);
				QColor top_3 = alphaBlendColors(QColor(16, 16, 16), bg, 6);
				QColor bottom = alphaBlendColors(QColor(0, 0, 0), bg, 30);
					
				QColor left_1 = alphaBlendColors(QColor(0, 0, 0), bg, 78);
				QColor left_2 = alphaBlendColors(QColor(0, 0, 0), bg, 10);
					
				p.setPen(top_1);
				p.drawLine(x+4, y, w-5, y);
				p.setPen(top_2);
				p.drawLine(x+4, y+1, w-5, y+1);
				p.setPen(top_3);
				p.drawLine(x+4, y+2, w-5, y+2);
				p.setPen(bottom);
				p.drawLine(x+4, h-1, w-5, h-1);
					
				p.setPen(left_1);
				p.drawLine(x, y+4, x, h-4);
				p.drawLine(w-1, y+4, w-1, h-4);
				p.setPen(left_2);
				p.drawLine(x+1, y+4, x+1, h-4);
				p.drawLine(w-2, y+4, w-2, h-4);
				p.end();
				
				
				if(!khtmlMode) {
					
					if(isLineEdit) {
						// simply drawing the pixmap would be faster, but the problem with the base color as erase color for disabled lineEdits would remain.
						XSetWindowBackgroundPixmap( lineEdit->x11Display(), lineEdit->winId(), lineEditPix->handle());
// 						// erase the frame
						lineEdit->erase(x, y, w, 2);
						lineEdit->erase(x, h-4, w, 4);
						lineEdit->erase(x, y+2, 3, h-4);
						lineEdit->erase(w-3, y+2, 3, h-4);
					}
					else {
						bitBlt(lineEdit, 0, 0, lineEditPix);
					}
					delete lineEditPix;
				}
				return false;
			}
			case QEvent::MouseButtonPress:
			case 6/*QEvent::KeyPress*/: {
				// it will be reseted in QLineEdit::mouseMoveEvent()
				if(isLineEdit && !khtmlMode && isEnabled && !isReadOnly && !isDateTimeEditor)
					static_cast<QWidget*>(obj)->setCursor(Qt::blankCursor);
				return false;
			}
			case QEvent::ApplicationPaletteChange:
				if(!khtmlMode) {
					lineEdit->setBackgroundMode(QWidget::PaletteBase);
					lineEdit->setPalette(qApp->palette());
				}
				return true;
			case QEvent::ParentPaletteChange:
				if(!khtmlMode) {
					lineEdit->setBackgroundMode(QWidget::PaletteBase);
					lineEdit->setPalette(lineEdit->parentWidget()->palette());
				}
				return true;
			case QEvent::PaletteChange:
				if(!khtmlMode) {
					lineEdit->setBackgroundMode(QWidget::PaletteBase);
					lineEdit->setPalette(lineEdit->palette());
				}
				return true;
			case QEvent::Wheel:
			case QEvent::Hide:
			case 7/*QEvent::KeyRelease*/:
				// jump to smoothscrolling
				if(_smoothScrolling)
					break;
			default:
				return false;
		}
	}
	
	
	
	
	// argb rubberBand
	if(!strcmp(obj->name(), "qt_viewport") && rubberBandType == ArgbRubber) {
		switch(ev->type()) {
			case QEvent::ChildRemoved:
			case QEvent::DragMove:
			case QEvent::DragEnter: {
				if(rubber) {
					viewPortPressed = false;
					oldRubberRect = QRect();
					rubber->destroy();
				}
				return false;
			}
			case QEvent::MouseButtonPress: {
				viewPortPressed = true;
				rubberViewPort = static_cast<const QWidget*>(obj);
				break;
			}
			case QEvent::MouseButtonRelease:
				if(rubber) {
					viewPortPressed = false;
					rubberViewPort = 0;
					oldRubberRect = QRect();
					if(dynamic_cast<QListView*>(obj->parent()) && rubber->window)
						ignoreNextFocusRect = true;
					rubber->destroy();
				}
				
#if KDE_VERSION >= 0x30506
			case QEvent::Show: {
				char selectionName[64];
				snprintf(selectionName, 64, "_NET_WM_CM_S%d", DefaultScreen(qt_xdisplay()));
				Atom compSelection = XInternAtom(qt_xdisplay(), selectionName, False);
				if(XGetSelectionOwner(qt_xdisplay(), compSelection)) {
					compositeManagerRunning = true;
				}
				else {
					compositeManagerRunning = false;
				}
			}
#endif
			default:
				break;
		}
	}
	
	
	// smoothscrolling
	if(dynamic_cast<QScrollView*>(obj) && _smoothScrolling) {
		switch(ev->type()) {
			case QEvent::Wheel: {
				QWheelEvent* e = (QWheelEvent*)ev;
				QScrollView* sv = dynamic_cast<QScrollView*>(obj);
				bool pageSteps = false;
				int pageStep = sv->verticalScrollBar()->pageStep();
				int lineStep = sv->verticalScrollBar()->lineStep();
				scrollVertical = e->orientation() == Qt::Vertical;
				
				QScrollBar* scrollBar = scrollVertical ? sv->verticalScrollBar() : sv->horizontalScrollBar();
				
				int step = QMIN( QApplication::wheelScrollLines()*lineStep, pageStep );
				if ( ( e->state() & ControlButton ) || ( e->state() & ShiftButton ) ) {
					step = pageStep;
					pageSteps = true;
				}
				int dy = (e->delta()*step)/120;
				
				e->accept();
				scrollWidget = sv;
				scrollDistance += dy;
				
				// don't accumulate to much
				if(scrollDistance < 0) {
					scrollDistance = pageSteps? QMAX(scrollDistance, pageStep*2-(pageStep*4)): scrollDistance;
					int minDistance = scrollBar->maxValue() - scrollBar->value();
					scrollDistance = QMAX(scrollDistance, minDistance-(minDistance*2));
				}
				else {
					scrollDistance = pageSteps? QMIN(scrollDistance, pageStep*2): scrollDistance;
					scrollDistance = QMIN(scrollDistance, scrollBar->value());
				}
				
				if(scrollDistance != 0) {
					scrollTimer->start(pageSteps? 4: 14);
				}
				
				return true;
			}
			case 7: { // ?? 7 = QEvent::KeyRelease
				QKeyEvent* ke = (QKeyEvent*)ev;
				if(scrollTimer->isActive() && (ke->key() == Qt::Key_Shift || ke->key() == Qt::Key_Control)) {
					scrollTimer->stop();
					scrollDistance = 0;
				}
				return false;
			}
			case QEvent::WindowActivate:
			case QEvent::WindowDeactivate:
				// fixes the scrollbar flicker
				return true;
			case QEvent::Hide:
			case QEvent::Show:
				scrollDistance = 0;
				// don't return here (because of konqframe and ?)
			default:
				break;
		}
	}


	if ( dynamic_cast<QScrollBar*>(obj) ) {
		
		switch(ev->type()) {
			case QEvent::WindowActivate: {
				QScrollBar* sb = dynamic_cast<QScrollBar*>(obj);
				if(sb->backgroundMode() == Qt::NoBackground && !sb->testWState(WState_HasMouse)) {
					sb->setBackgroundMode(Qt::PaletteButton);
				}
				return true;
			}
			case QEvent::Show: {
				// no flicker in konqi and konsole, hits maybe others, too.
				if(obj->parent()->parent() && (!strcmp(obj->parent()->parent()->name(), "KonqFrame") || !strcmp(obj->parent()->parent()->name(), "tab pages"))) {
					dynamic_cast<QScrollBar*>(obj)->setBackgroundMode(Qt::NoBackground);
				}
				scrollDistance = 0;
				return true;
			}
			case QEvent::Hide:
				scrollDistance = 0;
				return true;
			case QEvent::Resize: {
				QScrollBar* sb = dynamic_cast<QScrollBar*>(obj);
				if(sb->backgroundMode() != Qt::NoBackground && sb->isVisible()) {
					sb->setBackgroundMode(Qt::NoBackground);
				}
				return false;
			}
			case QEvent::ParentPaletteChange: {
				QScrollBar* sb = dynamic_cast<QScrollBar*>(obj);
				sb->setBackgroundMode(Qt::PaletteButton);
				sb->setPalette(sb->parentWidget()->palette());
				return true;
			}
			case QEvent::PaletteChange: {
				QScrollBar* sb = dynamic_cast<QScrollBar*>(obj);
				sb->setBackgroundMode(Qt::PaletteButton);
				sb->setPalette(sb->palette());
				return true;
			}
			case QEvent::ApplicationPaletteChange: { // ###
				QScrollBar* sb = dynamic_cast<QScrollBar*>(obj);
				removeCachedSbPix(sb->paletteBackgroundColor());
				sb->setBackgroundMode(Qt::PaletteButton);
// 				sb->update();
				return false;
			}
			case QEvent::Wheel: {
				if(!_smoothScrolling)
					return false;
				QWheelEvent* e = (QWheelEvent*)ev;
				QScrollBar* sb = dynamic_cast<QScrollBar*>(obj);
				if(!dynamic_cast<QScrollView*>(sb->parentWidget()))
					return false;
				
				bool pageSteps = false;
				int pageStep = sb->pageStep();
				int lineStep = sb->lineStep();
				
				int step = QMIN( QApplication::wheelScrollLines()*lineStep, pageStep );
				if ( ( e->state() & ControlButton ) || ( e->state() & ShiftButton ) ) {
					step = pageStep;
					pageSteps = true;
				}
				int dy = (e->delta()*step)/120;
				
				e->accept();
				scrollWidget = dynamic_cast<QScrollView*>(sb->parentWidget());
				scrollDistance += dy;
				
				// don't accumulate to much
				if(scrollDistance < 0) {
					scrollDistance = pageSteps? QMAX(scrollDistance, pageStep*2-(pageStep*4)): scrollDistance;
					int minDistance = sb->maxValue() - sb->value();
					scrollDistance = QMAX(scrollDistance, minDistance-(minDistance*2));
				}
				else {
					scrollDistance = pageSteps? QMIN(scrollDistance, pageStep*2): scrollDistance;
					scrollDistance = QMIN(scrollDistance, sb->value());
				}
				
				scrollVertical = sb->orientation() == Qt::Vertical;
				if(scrollDistance != 0) {
					scrollTimer->start(pageSteps? 4: 14);
				}
				
				return true;
			}
			case 7: { // ?? 7 = QEvent::KeyRelease
				QKeyEvent* ke = (QKeyEvent*)ev;
				if(scrollTimer->isActive() && (ke->key() == Qt::Key_Shift || ke->key() == Qt::Key_Control)) {
					scrollTimer->stop();
					scrollDistance = 0;
				}
				return false;
			}
			default:
				return false;
		}
	}


	if ( dynamic_cast<QProgressBar*>(obj) && _animateProgressBar)
	{
		switch(ev->type()) {
			case QEvent::Show:
				if(!animationTimer->isActive())
					animationTimer->start( 50, false );
				return true;
			case QEvent::Hide:
				if(animationTimer->isActive())
					animationTimer->stop();
				return true;
			case QEvent::ApplicationPaletteChange:
				setDominoButtonPalette(static_cast<QWidget*>(obj), Palette_Application);
				return true;
			case QEvent::ParentPaletteChange:
				setDominoButtonPalette(static_cast<QWidget*>(obj), Palette_Parent);
				return true;
			case QEvent::PaletteChange:
				setDominoButtonPalette(static_cast<QWidget*>(obj), Palette_Widget);
				return true;
			default:
				return false;
		}
	}
	
	if (dynamic_cast<QStatusBar*>(obj)) {
		if(ev->type() == QEvent::PaletteChange) {
			QStatusBar* sb = dynamic_cast<QStatusBar*>(obj);
			sb->setPaletteBackgroundColor(qApp->palette().active().background());
			sb->setPaletteForegroundColor(qApp->palette().active().foreground());
			return true;
		}
		return false;
	}
	
	if (dynamic_cast<QGroupBox*>(obj) && dynamic_cast<QFrame*>(obj)->frameShape() == QFrame::GroupBoxPanel) {
		QGroupBox* gb = dynamic_cast<QGroupBox*>(obj);
		QRect r = gb->rect();
		
		switch(ev->type()) {
			case QEvent::Paint: {
// 				if(y+16 > h || 16 > w) // don't overlap the pixmaps
// 					return false;
				renderGroupBox(obj, ev);
				return true;
			}
			case QEvent::Resize: {
				if(!gb->isFlat())
					renderGroupBox(obj, ev);
				return false;
			}
			case QEvent::ApplicationPaletteChange:
			case QEvent::PaletteChange:
			case QEvent::ParentPaletteChange: {
				
				if(gb->isFlat() || gb->lineWidth() <= 0) {
					return false;
				}
				
				QColor outsideColor = gb->parentWidget() ? gb->parentWidget()->paletteBackgroundColor() : gb->paletteBackgroundColor();
				QColor insideColor = groupBoxSettings.tintBackground ? getGroupBoxColor(gb) : gb->paletteBackgroundColor();
				
				QPalette pal(qApp->palette());
				pal.setColor(QPalette::Active, QColorGroup::Background, insideColor);
				pal.setColor(QPalette::Inactive, QColorGroup::Background, insideColor);
				pal.setColor(QPalette::Disabled, QColorGroup::Background, insideColor);
				pal.setColor(QPalette::Disabled, QColorGroup::Text, alphaBlendColors(pal.active().foreground(), insideColor, 40));
				gb->setPalette(pal);
				
				
				if(gb->isCheckable()) {
					((QCheckBox*)gb->child("qt_groupbox_checkbox", 0, false))->setPaletteBackgroundColor(outsideColor);
				}
				
				if(groupBoxPixmaps.contains(gb)) {
					delete groupBoxPixmaps[gb];
					groupBoxPixmaps.remove(gb);
				}
				return false;
			}
			default:
				return false;
		}
	}

	if(dynamic_cast<QSpinWidget*>(obj)) {
		
		switch(ev->type()) {
			case QEvent::Paint: {
				
				QSpinWidget* sw = static_cast<QSpinWidget*>(obj);
				QPixmap buffer(sw->size());
				QPainter p(&buffer);
				
				if(sw->parentWidget() && sw->parentWidget()->parentWidget() && sw->parentWidget()->parentWidget()->backgroundPixmap()) {
					QPoint point = sw->backgroundOffset();
					p.drawTiledPixmap( 0, 0, sw->width(), sw->height(),
							*sw->parentWidget()->parentWidget()->backgroundPixmap(), point.x(), point.y() );
				}
				
				
				
				///////
				QStyle::SFlags flags = QStyle::Style_Default;
				if (sw->isEnabled())
					flags |= QStyle::Style_Enabled;
				if (sw->hasFocus() || sw->focusProxy() && sw->focusProxy()->hasFocus())
					flags |= QStyle::Style_HasFocus;
				if(spinWidgetDown && sw->hasMouse())
					flags |= QStyle::Style_Down;
				QRect fr = QStyle::visualRect(querySubControlMetrics( QStyle::CC_SpinWidget, sw, QStyle::SC_SpinWidgetFrame ), sw );
				drawComplexControl( QStyle::CC_SpinWidget, &p, sw, sw->rect(), sw->colorGroup(), flags, (uint)QStyle::SC_All, QStyle::SC_None );
				p.end();
				p.begin(sw);
				p.drawPixmap(0,0,buffer);
				return true;
			}
			case QEvent::MouseButtonRelease:
			case QEvent::MouseButtonPress:
			case QEvent::MouseButtonDblClick: {
				QSpinWidget* sw = static_cast<QSpinWidget*>(obj);
				QPainter p(sw);
				
				QStyle::SFlags flags = QStyle::Style_Default;
				if (sw->isEnabled())
					flags |= QStyle::Style_Enabled;
				if (sw->hasFocus() || sw->focusProxy() && sw->focusProxy()->hasFocus())
					flags |= QStyle::Style_HasFocus;
				if(ev->type() != QEvent::MouseButtonRelease && dynamic_cast<QSpinBox*>(sw->parentWidget())) {
					QSpinBox* sb = dynamic_cast<QSpinBox*>(sw->parentWidget());
					QMouseEvent* e = (QMouseEvent*)ev;
					
					if(sb->downRect().contains(e->pos()) && sb->value() != sb->minValue()) {
						flags |= QStyle::Style_Down;
						spinWidgetDown = true;
					}
					if(sb->upRect().contains(e->pos()) && sb->value() != sb->maxValue()) {
						flags |= QStyle::Style_Down;
						spinWidgetDown = true;
					}
					if(spinWidgetDown) {
						if(sw->paletteBackgroundPixmap())
							sw->erase();
						drawComplexControl( QStyle::CC_SpinWidget, &p, sw, sw->rect(), sw->colorGroup(), flags, (uint)QStyle::SC_All, QStyle::SC_None);
					}
					return false;
					
				}
				else
					spinWidgetDown = false;
				
				if(sw->paletteBackgroundPixmap())
					sw->erase();
				drawComplexControl( QStyle::CC_SpinWidget, &p, sw, sw->rect(), sw->colorGroup(), flags, (uint)QStyle::SC_All, QStyle::SC_None);
				return false;
			}
			case QEvent::Enter: {
				QPushButton* btn = static_cast<QPushButton*>(obj);
				if(!btn->isEnabled())
					return false;
				hoverWidget = btn;
				btn->repaint(btn->paletteBackgroundPixmap());
				return false;
			}
			case QEvent::Leave: {
				spinWidgetDown = false;
				QPushButton* btn = static_cast<QPushButton*>(obj);
				if(!btn->isEnabled())
					return false;
				hoverWidget = 0;
				btn->repaint(btn->paletteBackgroundPixmap());
				return false;
			}
			case QEvent::ApplicationPaletteChange:
				setDominoButtonPalette(dynamic_cast<QWidget*>(obj), Palette_Application);
				return true;
			case QEvent::ParentPaletteChange:
				setDominoButtonPalette(dynamic_cast<QWidget*>(obj), Palette_Parent);
				return true;
			case QEvent::PaletteChange:
				setDominoButtonPalette(dynamic_cast<QWidget*>(obj), Palette_Parent);
				return true;
			default:
				break;
		}
		return false;
	}

	
	if (dynamic_cast<QTabBar*>(obj)) {
		
		switch(ev->type()) {
			case QEvent::Wheel: {
				QWheelEvent* e = (QWheelEvent*)ev;
				QTabWidget* tw = dynamic_cast<QTabWidget*>(obj->parent());
				if(!tw)
					return false;
				
				int current = tw->currentPageIndex();
				int count = tw->count();
				
				if(count < 2)
					return false;
				
				current = e->delta() > 0 ? current - 1 : current + 1;
				
				if(current > count - 1)
					current = 0;
				else if(current < 0)
					current = count - 1;
				if(tw->page(current)->isEnabled())
					tw->setCurrentPage(current );
				return true;
			}
			case QEvent::ApplicationPaletteChange:
				setDominoButtonPalette(dynamic_cast<QWidget*>(obj), Palette_Application);
				return true;
			case QEvent::ParentPaletteChange:
				setDominoButtonPalette(dynamic_cast<QWidget*>(obj), Palette_Parent);
				return true;
			case QEvent::PaletteChange:
				setDominoButtonPalette(dynamic_cast<QWidget*>(obj), Palette_Widget);
				return true;
			default:
				return false;
		}
	}
	
	if (dynamic_cast<QTabWidget*>(obj)) {
		
		QTabWidget* tw = dynamic_cast<QTabWidget*>(obj);
		if(tw->isA("KonqFrameTabs")) {
			
			QColorGroup cg = qApp->palette().active();
			switch (ev->type()) {
				case QEvent::Paint: {
					QRect r = tw->rect();
					int top, height;
					if(tw->tabPosition() == QTabWidget::Top) {
						top = r.top();
						height =  r.height()-tw->currentPage()->height();
					}
					else {
						top = r.top()+tw->currentPage()->height();
						height = r.height();
					}
					
					QPainter p(tw);
					p.setPen(cg.background());
					p.drawLine(r.left(), top, r.width(), top);
					
					p.setPen(cg.background().dark(120));
					p.drawLine(r.left(), top+1, r.width(), top+1);
					
					p.setPen(cg.background().dark(konqTabBarContrast+20));
					p.drawLine(r.left(), top+2, r.width(), top+2);
					return true;
				}
				case QEvent::Show:
				case QEvent::PaletteChange:
				case QEvent::ApplicationPaletteChange: {
					if(tw->eraseColor() != cg.background().dark(konqTabBarContrast))
						tw->setEraseColor(cg.background().dark(konqTabBarContrast));
					return true;
				}
				default:
					return false;
			}
		}
		else if(ev->type() == QEvent::Wheel) {
			if(!tw)
				return false;
			
			QRect r = tw->rect();
			QRect re  = ((DominoQTabWidget*)tw)->tabBar()->rect();
			int top;
			if(tw->tabPosition() == QTabWidget::Top) {
				top = r.top();
			}
			else {
				top = r.top() + tw->height()-re.height() - 2;
			}
			
			QWheelEvent* e = (QWheelEvent*)ev;
			if(QRect(re.x(), top, r.width(), re.height()).contains(e->pos())) {
				int current = tw->currentPageIndex();
				int count = tw->count();
				if(count < 2)
					return false;
				
				current = e->delta() > 0 ? current - 1 : current + 1;
				
				if(current > count - 1)
					current = 0;
				else if(current < 0)
					current = count - 1;
				if(tw->page(current)->isEnabled())
					tw->setCurrentPage(current);
				return true;
			}
		}
		if(ev->type() == QEvent::LayoutHint) {
			if(!tw)
				return false;
			QWidget* w;
			if((w = tw->cornerWidget(Qt::TopRight))) {
				if(!w->width() != 25 || w->height() != 23)
					w->setFixedSize(25, 23);
			}
			if((w = tw->cornerWidget(Qt::TopLeft))) {
				if(!w->width() != 25 || w->height() != 23)
					w->setFixedSize(25, 23);
			}
			if((w = tw->cornerWidget(Qt::BottomRight))) {
				if(!w->width() != 25 || w->height() != 23)
					w->setFixedSize(25, 23);
			}
			if((w = tw->cornerWidget(Qt::BottomLeft))) {
				if(!w->width() != 25 || w->height() != 23)
					w->setFixedSize(25, 23);
			}
			
			return false;
		}
		return false;
 	}
	if(::qt_cast<KToolBarButton*>(obj)) {
		switch(ev->type()) {
			case QEvent::Paint: {
				KToolBar *bar = dynamic_cast<KToolBar*>(obj->parent());
				if(!bar) return false;  // KMyMoney2
				KToolBarButton* btn = dynamic_cast<KToolBarButton*>(obj);
				int index = bar->itemIndex(btn->id());
				
				
				if(_toolBtnAsBtn && sideRepaint == false) {
					sideRepaint = true;
					KToolBarButton* sideBtn;
					if((sideBtn = bar->getButton(bar->idAt(index-1)))) {
						sideBtn->repaint(false);
					}
					if((sideBtn = bar->getButton(bar->idAt(index+1)))) {
						sideBtn->repaint(false);
					}
					sideRepaint = false;
				}
				
				
				SFlags flags = QStyle::Style_Default;
				SCFlags active = QStyle::SC_None;
				if(btn->isOn()) flags |= QStyle::Style_On;
				if (btn->isDown()) {
					flags  |= QStyle::Style_Down;
					active |= QStyle::SC_ToolButton;
				}
				if (btn->isEnabled())  flags |= QStyle::Style_Enabled;
				if (btn->isEnabled() && btn->hasMouse())    flags |= QStyle::Style_Raised;
				if (btn->hasFocus())   flags |= QStyle::Style_HasFocus;
				//SCFlags controls,
				
				QPainter p(btn);
				
				if(btn->isDown()/* || btn->isOn()*/)
					buttonContour->setState(Contour_Pressed);
				else if(btn == hoverWidget && btn->isEnabled())
					buttonContour->setState(Contour_MouseOver);
				
				drawComplexControl(QStyle::CC_ToolButton, &p, btn, btn->rect(), btn->colorGroup(), flags, QStyle::SC_ToolButton, active, QStyleOption());
				buttonContour->reset();
				return true;
			}
			case QEvent::MouseButtonPress: {
				if(((QMouseEvent*)ev)->button() == Qt::RightButton)
					return false;
				KToolBarButton* btn = dynamic_cast<KToolBarButton*>(obj);
				btn->setUpdatesEnabled(false);
				if(!btn->isOn()) btn->setOn(true);
				btn->setUpdatesEnabled(true);
				hoverWidget = 0;
				return false;
			}
			case QEvent::MouseButtonRelease: {
				if(((QMouseEvent*)ev)->button() == Qt::RightButton)
					return false;
				KToolBarButton* btn = dynamic_cast<KToolBarButton*>(obj);
				btn->setUpdatesEnabled(false);
				if(btn->isOn())  btn->setOn(false);
				btn->setUpdatesEnabled(true);
				if(btn->hasMouse())
					hoverWidget = btn;
				return false;
			}
			case QEvent::MouseButtonDblClick: {
				if(((QMouseEvent*)ev)->button() == Qt::RightButton)
					return false;
				KToolBarButton* btn = dynamic_cast<KToolBarButton*>(obj);
				if(!btn->isOn()) btn->setOn(true);
				hoverWidget = 0;
				return false;
			}
			case QEvent::Enter: {
				KToolBarButton* btn = dynamic_cast<KToolBarButton*>(obj);
				if(btn->isEnabled()) {
					hoverWidget = btn;
					btn->repaint(false);
				}
				return false;
			}
			case QEvent::Leave: {
				KToolBarButton* btn = dynamic_cast<KToolBarButton*>(obj);
				if(btn == hoverWidget) {
					hoverWidget = 0;
					btn->repaint(false);
				}
				return false;
			}
			case QEvent::Move: {
				if(_toolBtnAsBtn) {
					KToolBarButton* btn = dynamic_cast<KToolBarButton*>(obj);
					btn->repaint(false);
				}
				return false;
			}
			case QEvent::ApplicationPaletteChange: {
				setDominoButtonPalette(static_cast<QWidget*>(obj), Palette_Application);
				return true;
			}
			default:
				return false;
		}
	}

	if(dynamic_cast<QRadioButton*>(obj) && dynamic_cast<QRadioButton*>(obj)->paletteBackgroundPixmap()) {
		if (ev->type() == QEvent::Paint) {
			QRadioButton* rb = static_cast<QRadioButton*>(obj);
			
			rb->removeEventFilter(this);
			QApplication::sendEvent(rb, (QPaintEvent*)ev);
			rb->installEventFilter(this);
			
			QStyle::SFlags flags = QStyle::Style_Default;
			if ( rb->isEnabled() )
				flags |= QStyle::Style_Enabled;
			if ( rb->hasFocus() )
				flags |= QStyle::Style_HasFocus;
			if ( rb->isDown() )
				flags |= QStyle::Style_Down;
			if ( rb->hasMouse() )
				flags |= QStyle::Style_MouseOver;
			if ( rb->state() == QButton::On )
				flags |= QStyle::Style_On;
			else if ( rb->state() == QButton::Off )
				flags |= QStyle::Style_Off;
			
			QPainter p(rb);
			drawControl(CE_RadioButton, &p, rb, QRect(0,0,17,17), rb->palette().active(), flags);
			return true;
		}
		if (ev->type() == QEvent::Show && !strcmp(obj->name(), "__khtml")) {
			QRadioButton* rb = static_cast<QRadioButton*>(obj);
			QBitmap bm(rb->size());
			bm.fill(color0);
			rb->setMask(bm);
		}
		return false;
	}
	
	if (obj->isA("KonqCheckBox")) {
		
		if (ev->type() == QEvent::Show || ev->type() == QEvent::ApplicationPaletteChange) { // at least doesn't flicker in the unchecked state ;-)
			QButton* btn = dynamic_cast<QButton*>(obj);
			QPixmap pix(btn->size());
			pix.fill(qApp->palette().active().background());
			QPainter p(&pix);
			drawControl( CE_CheckBox, &p, btn, QRect(0,0,18,19), qApp->palette().active(), Style_Default|Style_Off, QStyleOption::Default);
			btn->setErasePixmap(pix);
			return true;
		}
		if (ev->type() == QEvent::Paint) {
			QButton* btn = dynamic_cast<QButton*>(obj);
			btn->erase();
			SFlags flags = Style_Default;
			if ( btn->isDown() )
				flags |= Style_Down;
			if ( btn->state() == QButton::On )
				flags |= Style_On;
			else if ( btn->state() == QButton::Off )
				flags |= Style_Off;
			if ( btn->isEnabled() )
				flags |= Style_Enabled;
			QPainter p(btn);
			drawPrimitive(PE_CheckMark, &p, QRect(0,0,16,17), qApp->palette().active(), flags);
			return true;
		}
		return false;
	}
	
	if(dynamic_cast<QCheckBox*>(obj)) {
		if (ev->type() == QEvent::Paint) {
			QCheckBox* cb = static_cast<QCheckBox*>(obj);
			
			bool khtmlWidget = !strcmp(obj->name(), "__khtml");
			if(khtmlWidget || cb->paletteBackgroundPixmap()) {
				if(khtmlWidget) {
					// this prevents the checkbox been erased before the rest of the html page
					QBitmap bm(cb->size());
					bm.fill(color0);
					cb->setMask(bm);
				}
				else {
					// draws a masked checkbox
					cb->removeEventFilter(this);
					QApplication::sendEvent(cb, (QPaintEvent*)ev);
					cb->installEventFilter(this);
				}
				// draws the missing alpha part
				QStyle::SFlags flags = QStyle::Style_Default;
				if ( cb->isEnabled() )
					flags |= QStyle::Style_Enabled;
				if ( cb->hasFocus() )
					flags |= QStyle::Style_HasFocus;
				if ( cb->isDown() )
					flags |= QStyle::Style_Down;
				if ( cb->hasMouse() )
					flags |= QStyle::Style_MouseOver;
				if ( cb->state() == QButton::On )
					flags |= QStyle::Style_On;
				else if ( cb->state() == QButton::Off )
					flags |= QStyle::Style_Off;
				else if ( cb->state() == QButton::NoChange )
					flags |= QStyle::Style_NoChange;
				
				QPainter p(cb);
				drawControl(CE_CheckBox, &p, cb, QRect(0,0,18,19), cb->palette().active(), flags);
				return true;
			}
			return false;
		}
		return false;
	}
	
	if(dynamic_cast<QPopupMenu*>(obj)) {
		switch(ev->type()) {
			case QEvent::Resize:
			case QEvent::Show: {
				QWidget* w = static_cast<QWidget*>(obj);
				QRect r = w->rect();
				QRegion mask(r.x()+2, r.y(), r.width()-4, r.height());
				mask += QRegion(r.x(), r.y()+2, r.width(), r.height()-4);
				mask += QRegion(r.x()+1, r.y()+1, r.width()-2, r.height()-2);
				w->setMask(mask);
				return false;
			}
			case QEvent::Hide:
				static_cast<QWidget*>(obj)->setBackgroundMode(NoBackground);
				return false;
			case QEvent::ApplicationPaletteChange:
				if(obj->isA("PanelKMenu")) {
					QWidget* w = static_cast<QWidget*>(obj);
					QPalette pal = qApp->palette();
					pal.setColor(QColorGroup::ButtonText, alphaBlendColors(_popupMenuColor, pal.active().foreground(), 150));
					w->setPalette(pal);
					return true;
				}
			default:
				return false;
		}
	}
	
	
	if(dynamic_cast<QPushButton*>(obj) && !obj->isA("KDockButton_Private") || dynamic_cast<QToolButton*>(obj) ) {
		QWidget* w = static_cast<QWidget*>(obj);

		switch(ev->type()) {
			case QEvent::Enter: {
				if(!w->isEnabled()) return false;
				hoverWidget = w;
				w->repaint(false);
				return false;
			}
			case QEvent::Leave: {
				if(!w->isEnabled()) return false;
				hoverWidget = 0;
				w->repaint(false);
				return false;
			}
			case QEvent::MouseButtonPress: {
				if(dynamic_cast<QToolButton*>(obj)) {
					QMouseEvent* e = (QMouseEvent*)ev;
					if(e->button() == Qt::LeftButton)
						popupMousePressPos = e->pos();
				}
				return false;
			}
			case QEvent::MouseMove: {
				QMouseEvent* e = (QMouseEvent*)ev;
				if(dynamic_cast<QToolButton*>(obj) && popupTimer->isActive() && ((e->pos()-popupMousePressPos).manhattanLength() > 4 /*KGlobalSettings::dndEventDelay()*/))
					popupToolMenu();
				return false;
			}
			case QEvent::ParentPaletteChange: {
				if(obj->isA("QToolButton")) {
					
					if(w->parentWidget() && !khtmlWidgets.contains(w)) {
						w->setUpdatesEnabled(false);
						if(w->parentWidget()->paletteBackgroundPixmap()) {
							w->setBackgroundOrigin(setPixmapOrigin(w->parentWidget()->backgroundOrigin()));
							setDominoButtonPalette(w, Palette_Parent);
						}
						else {
							w->setBackgroundMode(PaletteBackground);
							w->setBackgroundOrigin(QWidget::WidgetOrigin);
							setDominoButtonPalette(w, Palette_Parent);
						}
						w->setUpdatesEnabled(true);
						w->repaint(true);
					}
					return true;
				}
				else
					setDominoButtonPalette(w, Palette_Parent);
				return true;
				break;
			}
			case QEvent::PaletteChange: {
				setDominoButtonPalette(w, Palette_Widget);
				return true;
			}
			case QEvent::ApplicationPaletteChange: {
				return true;
			}
			default:
				return false;
		}
	}
	if(obj->isA("QDockWindowHandle") || obj->isA("AppletHandleDrag"))  {
		if ((ev->type() == QEvent::Enter) &&  static_cast<QWidget*>(obj)->isEnabled()) {
			QWidget* w = static_cast<QWidget*>(obj);
			hoverWidget = w;
			w->repaint(true);
			return false;
		}
		if (ev->type() == QEvent::Leave && static_cast<QWidget*>(obj) == hoverWidget) {
			hoverWidget = 0;
			static_cast<QWidget*>(obj)->repaint(true);
			return false;
		}
		return false;
	}
	
	if(obj->isA("KDockButton_Private")) {
		if (ev->type() == QEvent::Paint) {
			QPixmap* btnPix = 0;
			QButton* btn = static_cast<QButton*>(obj);
			if(!(btnPix = QPixmapCache::find(QString::number(btn->paletteBackgroundColor().pixel(), 16)+btn->name()))) {
				QPixmap* tmpPix = new QPixmap(qembed_findImage(btn->name()));
				if(!tmpPix)
					return false;
				btnPix = new QPixmap(12, 24);
				btnPix->fill(btn->paletteBackgroundColor());
				bitBlt(btnPix, 0, 0, tmpPix);
				QPixmapCache::insert(QString::number(btn->paletteBackgroundColor().pixel(), 16)+btn->name(), btnPix);
			}
			
			if(btn->isOn() || btn->isDown())
				bitBlt(btn, 0, 1, btnPix, 0, 12, 12, 12);
			else
				bitBlt(btn, 0, 1, btnPix, 0, 0, 12, 12);
			return true;
		}
		return false;
	}
	
	if(dynamic_cast<KPopupTitle*>(obj)) {
		
		switch(ev->type()) {
			case QEvent::Paint: {
				
				KPopupTitle* pt = static_cast<KPopupTitle*>(obj);
				QRect r = pt->rect();
				QPainter p(pt);
				
				if(pt->parentWidget() && pt->parentWidget()->paletteBackgroundPixmap()) {
					QPoint point = QPoint(pt->geometry().x(), pt->geometry().y());
					p.drawTiledPixmap( 0, 0, pt->width(), pt->height(),
							*pt->parentWidget()->backgroundPixmap(),
							point.x(), point.y() );
				}
				
				QFont f = p.font();
				f.setBold(true);
				p.setFont(f);
				
				if(!pt->icon().isNull()) {
					int titleWidth = p.fontMetrics().width(pt->title());
					p.drawPixmap((r.width()-titleWidth)/2-4-pt->icon().width(), (r.height()-pt->icon().height())/2, pt->icon());
				}
				if (!pt->title().isNull()) {
					p.setPen(alphaBlendColors(_popupMenuColor, pt->paletteForegroundColor(), 150));
					p.drawText(0, 0, r.width(), r.height(), AlignCenter | SingleLine, pt->title());
				}
				return true;
			}
			default:
				return false;
		}
	}
	if(dynamic_cast<QButton*>(obj) && obj->parent() && obj->parent()->isA("QToolBox")) {
		QWidget* w = dynamic_cast<QWidget*>(obj);
		if(!w->isEnabled())
			return false;
		if(ev->type() == QEvent::Enter) {
			hoveredToolBoxTab = true;
			w->repaint(false);
			return false;
		}
		if(ev->type() == QEvent::Leave) {
			hoveredToolBoxTab = false;
			w->repaint(false);
			return false;
		}
		if(ev->type() == QEvent::Move) {
			hoveredToolBoxTab = false;
			return false;
		}
		return false;
	}
	if(dynamic_cast<QHeader*>(obj)) {
		switch(ev->type()) {
			case QEvent::ApplicationPaletteChange:
				static_cast<QHeader*>(obj)->setBackgroundMode(Qt::PaletteBackground);
				setDominoButtonPalette(static_cast<QWidget*>(obj), Palette_Application);
				return true;
			case QEvent::ParentPaletteChange:
				setDominoButtonPalette(static_cast<QWidget*>(obj), Palette_Parent);
				return true;
			case QEvent::PaletteChange:
				static_cast<QHeader*>(obj)->setBackgroundMode(Qt::PaletteBackground);
				setDominoButtonPalette(static_cast<QWidget*>(obj), Palette_Widget);
				return true;
			default:
				return false;
		}
		return false;
	}
	
	if(dynamic_cast<QDockWindow*>(obj)) {
		switch(ev->type()) {
			case QEvent::LayoutHint: {
				QDockWindow* dw = dynamic_cast<QDockWindow*>(obj);
				if(!dw->widget())
					return false;
				if(!dw->area()) {
					dw->setFrameStyle(QFrame::Panel|QFrame::Sunken);
					dw->setLineWidth(1);
					QRect r = dw->rect();
					QRegion mask(r);
					mask -= QRegion(0, 0, 1, 1);
					mask -= QRegion(r.width()-1, 0, 1, 1);
					mask -= QRegion(0, r.height()-1, 1, 1);
					mask -= QRegion(r.width()-1, r.height()-1, 1, 1);
					dw->setMask(mask);
				}
				else if(dw->frameStyle() == QFrame::Panel|QFrame::Sunken) {
					dw->setFrameStyle(QFrame::StyledPanel|QFrame::Sunken);
					dw->setLineWidth(2);
					dw->clearMask();
				}
			}
			default:
				break;
				
		}
	}
	else if(obj->isA("QWorkspaceChild")) {
		switch(ev->type()) {
			case QEvent::Show:
			case QEvent::Resize: {
				QWidget* w = static_cast<QWidget*>(obj);
				QRect r = w->rect();
				QRegion mask(r);
				mask -= QRegion(0, 0, 1, 1);
				mask -= QRegion(r.width()-1, 0, 1, 1);
				mask -= QRegion(0, r.height()-1, 1, 1);
				mask -= QRegion(r.width()-1, r.height()-1, 1, 1);
				w->setMask(mask);
			}
			default:
				break;
		}
	}
		
	if(obj->isA("QDockWindowResizeHandle")) {
		if(ev->type() == QEvent::PaletteChange)
			static_cast<QWidget*>(obj)->setBackgroundMode(Qt::PaletteBackground);
		return false;
	}
		
	if(obj->isA("QWhatsThat")) { // QWhatsThis
		
		if (ev->type() == QEvent::Show) {
			QWidget* w = dynamic_cast<QWidget*>(obj);
			w->setBackgroundMode(Qt::NoBackground);
			QRect r = w->rect();
			QRegion mask(r.x()+3, r.y(), r.width()-6, r.height());
			mask += QRegion(r.x()+2, r.y()+1, r.width()-4, r.height()-2);
			mask += QRegion(r.x()+1, r.y()+2, r.width()-2, r.height()-4);
			mask += QRegion(r.x(), r.y()+3, r.width(), r.height()-6);
			w->setMask(mask);
			return true;
		}
		
		if (ev->type() == QEvent::Paint) {
			DominoQWhatsThat* dwt = (DominoQWhatsThat*)obj;
			QString text = dwt->text;
			QRect r = dwt->rect();
			
			int x = r.x();
			int y = r.y();
			int w = r.width();
			int h = r.height();
			
			QRegion mask(x+1, y+4, w-2, h-8);
			mask += QRegion(x+2, y+3, w-4, h-6);
			mask += QRegion(x+3, y+2, w-6, h-4);
			mask += QRegion(x+4, y+1, w-8, h-2);
			
			QPainter p(dwt);
			p.setClipRegion(mask);
			renderGradient(&p, QRect(r.left(), r.top(), r.width(), r.height()), _toolTipColor.light(110), _toolTipColor.dark(105), true);
			p.setClipping(false);
			
			QColor tabContour2 = qApp->palette().active().background().dark(150);
			
			p.save();
			p.setPen(tabContour2);
			p.drawLine(x+7,y, w-8,y); // top
			p.drawLine(x+7,h-1, w-8,h-1); // bottom
			p.drawLine(x,y+7, x,h-8); // left
			p.drawLine(w-1,y+7, w-1,h-8); // right
			p.restore();
			
			bitBlt(dwt, x, y, border1, 0, 0, 7, 7);
			bitBlt(dwt, w-7, y, border1, 7, 0, 7, 7);
			bitBlt(dwt, x, h-7, border1, 0, 7, 7, 7);
			bitBlt(dwt, w-7, h-7, border1, 7, 7, 7, 7);
			
			r.addCoords( 12, 8, -12, -8 );
			
			if(QStyleSheet::mightBeRichText( text )) {
				QSimpleRichText* srt = new QSimpleRichText( text, dwt->font());
				srt->draw(&p, r.x(), r.y(), r, qApp->palette().active(), 0);
			}
			else {
				r.addCoords( 2, 3, 0, 0 );
				p.drawText( r, AlignAuto + AlignTop + WordBreak + ExpandTabs, text );
			}
			
			return true;
		}
		return false;
	}
	
	
	if(obj->isA("KonqFileTip")) {
		
		if (ev->type() == QEvent::Show || ev->type() == QEvent::Paint) {
			
			QWidget* widget = dynamic_cast<QWidget*>(obj);
			QRect r = widget->rect();
			
			QPixmap pix(r.size());
			QPainter p(&pix);
			renderGradient(&p, QRect(r.left(), r.top(), r.width(), r.height()), _toolTipColor.light(110), _toolTipColor.dark(105), true);
			
			if(ev->type() == QEvent::Show) {
				dynamic_cast<QFrame*>(widget)->setFrameStyle(QFrame::NoFrame);
				widget->setBackgroundMode(Qt::NoBackground);
				
				QRegion mask(r.x()+3, r.y(), r.width()-6, r.height());
				mask += QRegion(r.x()+2, r.y()+1, r.width()-4, r.height()-2);
				mask += QRegion(r.x()+1, r.y()+2, r.width()-2, r.height()-4);
				mask += QRegion(r.x(), r.y()+3, r.width(), r.height()-6);
				dynamic_cast<QFrame*>(widget)->setMask(mask);
				
				QObjectList *list = const_cast<QObjectList *>(widget->children());
				QObjectListIt it( *list );
				QObject * o;
				while ( (o = it.current()) != 0 ) {
					++it;
					if(dynamic_cast<QLabel*>(o)) {
						QLabel* l = dynamic_cast<QLabel*>(o);
						l->setBackgroundOrigin(QWidget::ParentOrigin);
						l->setPaletteBackgroundPixmap(pix);
					}
				}
			}
			p.end();
			
			if(ev->type() == QEvent::Paint) {
				
				int x = r.x();
				int y = r.y();
				int w = r.width();
				int h = r.height();
				
				QRegion mask(x+1, y+4, w-2, h-8);
				mask += QRegion(x+2, y+3, w-4, h-6);
				mask += QRegion(x+3, y+2, w-6, h-4);
				mask += QRegion(x+4, y+1, w-8, h-2);
				
				p.begin(widget);
				p.setClipRegion(mask);
				p.drawPixmap(0, 0, pix);
				p.setClipping(false);
				
				QColor tabContour2 = qApp->palette().active().background().dark(150);
				p.setPen(tabContour2);
				p.drawLine(x+7,y, w-8,y); // top
				p.drawLine(x+7,h-1, w-8,h-1); // bottom
				p.drawLine(x,y+7, x,h-8); // left
				p.drawLine(w-1,y+7, w-1,h-8); // right
				
				bitBlt(widget, x, y, border1, 0, 0, 7, 7);
				bitBlt(widget, w-7, y, border1, 7, 0, 7, 7);
				bitBlt(widget, x, h-7, border1, 0, 7, 7, 7);
				bitBlt(widget, w-7, h-7, border1, 7, 7, 7, 7);
				
			}
			
			return true;
		}
		return false;
	}
	
	if(obj->isA("QTipLabel")) {
		
		if (ev->type() == QEvent::Show) {
			QLabel* l = dynamic_cast<QLabel*>(obj);
			l->resize( l->width()+2, l->height()+2);
			l->setBackgroundMode(Qt::NoBackground);
			int x, y, w, h;
			l->rect().rect(&x, &y, &w, &h);
			
			QRegion mask(x, y+3, w, h-6);
			mask += QRegion(x+1, y+2, w-2, h-4);
			mask += QRegion(x+2, y+1, w-4, h-2);
			mask += QRegion(x+3, y, w-6, h);
			l->setMask(mask);
			return true;
		}
		if (ev->type() == QEvent::Paint) {
			
			QLabel* label = dynamic_cast<QLabel*>(obj);
			QRect r = label->rect();
			
			int x = r.x();
			int y = r.y();
			int w = r.width();
			int h = r.height();
			
			// one pixel smaler as the mask
			QRegion mask(x+1, y+4, w-2, h-8);
			mask += QRegion(x+2, y+3, w-4, h-6);
			mask += QRegion(x+3, y+2, w-6, h-4);
			mask += QRegion(x+4, y+1, w-8, h-2);
			
			QPainter p(label);
			p.setClipRegion(mask);
			
			renderGradient(&p, QRect(r.left(), r.top(), r.width(), r.height()), _toolTipColor.light(110), _toolTipColor.dark(105), true);
			p.setClipping(false);
			
			QColor tabContour2 = qApp->palette().active().background().dark(150);
			p.setPen(tabContour2);
			p.drawLine(x+7,y, w-8,y); // top
			p.drawLine(x+7,h-1, w-8,h-1); // bottom
			p.drawLine(x,y+7, x,h-8); // left
			p.drawLine(w-1,y+7, w-1,h-8); // right
			
			bitBlt(label, x, y, border1, 0, 0, 7, 7);
			bitBlt(label, w-7, y, border1, 7, 0, 7, 7);
			bitBlt(label, x, h-7, border1, 0, 7, 7, 7);
			bitBlt(label, w-7, h-7, border1, 7, 7, 7, 7);
			
			QSimpleRichText* srt = new QSimpleRichText(label->text(), label->font());
			srt->setWidth(r.width()-5);
			srt->draw(&p, r.x()+4, r.y(), r, qApp->palette().active(),0);
			delete srt;
		}
		return true;
	}
	if(obj->isA("QViewportWidget") && dynamic_cast<QTextEdit*>(obj->parent())) {
		switch(ev->type()) {
			case QEvent::MouseButtonPress:
			case 6/*QEvent::KeyPress*/: {
				QTextEdit* te = dynamic_cast<QTextEdit*>(obj->parent());
				if(te->isEnabled() && !te->isReadOnly())
					static_cast<QWidget*>(obj)->setCursor(Qt::blankCursor);
				return false;
			}
			default:
				return false;
		}
	}

	if (dynamic_cast<QFrame*>(obj)) { // at last because other widgets may include a frame.
		
		QFrame* f = dynamic_cast<QFrame*>(obj);
		const QRect r = f->rect();
		
		if (ev->type() == QEvent::Paint) {
			
			switch(f->frameShape()){
				case QFrame::HLine: {
					QPainter p(f);
// 					p.setPen(f->paletteBackgroundColor().dark(120));
					p.setPen(alphaBlendColors(f->paletteForegroundColor(), f->paletteBackgroundColor(), 40));
					p.drawLine(0,0,f->frameRect().width(),0);
					return true;
				}
				case QFrame::VLine: {
					QPainter p(f);
					p.setPen(alphaBlendColors(f->paletteForegroundColor(), f->paletteBackgroundColor(), 40));
					p.drawLine(0,0,0,f->frameRect().height());
					return true;
				}
				case QFrame::WinPanel:
				case QFrame::Panel: {
					if(f->parentWidget() && f->parentWidget()->isA("KDockSplitter")) {
						
						if(!f->erasePixmap()) {
							bool horizontal = (r.width() > r.height());
							QPixmap pix(r.size());
							QPainter p(&pix);
							renderGradient(&p, r, lightenColor(f->paletteBackgroundColor(), 40), darkenColor(f->paletteBackgroundColor(), 30), horizontal);
							f->setErasePixmap(pix);
						}
						return true;
					}
					
					QPainter p(f);
					QColor frameColor;
					if(f->frameShape() == QFrame::Panel && f->frameShadow() == QFrame::Plain) // used as a selection in kspread
						frameColor = f->paletteBackgroundColor().dark(160);
					else if(dynamic_cast<QListBox*>(obj) && dynamic_cast<QComboBox*>(obj->parent()))
						frameColor = f->paletteBackgroundColor().dark(190);
					else
						frameColor = f->paletteBackgroundColor().dark(120);
					
						
					p.setPen(frameColor);
					p.drawRect(f->rect());
					
					QPaintEvent* e = (QPaintEvent*)ev;
					p.setClipRegion(e->region().intersect(f->contentsRect()));
					DominoQFrame* df = (DominoQFrame*)f;
					df->drawContents(&p);
					
					DominoQWidget* dw = (DominoQWidget*)f;
					QPaintEvent* event = new QPaintEvent(e->region().intersect(f->contentsRect()));
					dw->paintEvent(event);
					return true;
				}
				case QFrame::ToolBarPanel: {
					if(f->isA("KonqSidebarTree")) {
						QPainter p(f);
						p.setPen(qApp->palette().active().background().dark(120));
						p.drawRect(f->rect());
						return true;
					}
					return false;
				}
				case QFrame::Box: {
					if(f->parentWidget() && !strcmp(f->parentWidget()->name(), "KonqFrame")) {
						QPainter p(f);
						p.fillRect(r.width()-16, r.height()-16, 15, 15, qApp->palette().active().background()); // 15x15 scrollview cornerwidget gap
						return true;
					}
					else if(konquerorMode && f->isA("KHTMLView")) {
						QPainter p(f);
						p.setPen(qApp->palette().active().background().dark(120));
						p.drawRect(f->rect());
						return true;
					}
				}
				default:
					return false;
			}
		}
		
		if (ev->type() == QEvent::Show || ev->type() == QEvent::ApplicationPaletteChange) { // ApplicationPaletteChange for style changes
			
			switch(f->frameShape()){
				case QFrame::TabWidgetPanel:
					if(ev->type() == QEvent::ApplicationPaletteChange) {
						
						if(f->lineWidth() != 4)
							f->setLineWidth(4);
						if(f->frameRect() != QRect(0, 0, f->width(), f->height()-1))
							f->setFrameRect(QRect(0, 0, f->width(), f->height()-1));
						if(f->margin() != 0)
							f->setMargin(0);
						
						QWidget* w = f->parentWidget();
						for(int i=0; i<3; i++) { // the third parentcheck is for the tabWidget on kcm khtml_plugins
							if(!w) break;
							if(dynamic_cast<QGroupBox*>(w)) {
								f->setPaletteBackgroundColor(qApp->palette().active().background());
								break;
							}
							w = w->parentWidget();
						}
					}
					return true;
				case QFrame::Box:
					if(f->parentWidget() && !strcmp(f->parentWidget()->name(), "KonqFrame")) {
						f->setEraseColor(qApp->palette().active().background().dark(120));
						f->erase();
						return true;
					}
				case QFrame::Panel:
					if(ev->type() == QEvent::ApplicationPaletteChange && f->parentWidget() && f->parentWidget()->isA("KDockSplitter")) {
						f->setBackgroundMode(Qt::PaletteBackground);
						return true;
					}
				default:
					return false;
			}
			return false;
		}
		if(ev->type() == QEvent::LayoutHint && f->frameShape() == QFrame::TabWidgetPanel) { // don't look broken in kcm stylePreview
			if(f->lineWidth() != 4)
				f->setLineWidth(4);
			if(f->frameRect() != QRect(0, 0, f->width(), f->height()-1))
				f->setFrameRect(QRect(0, 0, f->width(), f->height()-1));
			if(f->margin() != 0)
				f->setMargin(0);
			
			return false;
		}
	}
	
	// nothing matched
	return false;
	
}
