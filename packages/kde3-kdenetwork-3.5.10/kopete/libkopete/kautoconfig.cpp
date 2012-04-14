/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Benjamin C Meyer (ben+kdelibs at meyerhome dot net)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include "kautoconfig.h"

#include <kglobal.h>
#include <qsqlpropertymap.h>
#include <qobjectlist.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kdeversion.h>

/**
 * Macro function to warn developers when they are making calls
 * that can never return anything of value
 */ 
#ifndef NDEBUG
#include "kdebug.h"
#define functionCallPreOrderCheck(functionName, returnValue) \
  if(!d->retrievedSettings){ \
      kdDebug(180) << "KAutoConfig::"functionName"() was called before " \
      "KAutoConfig::retrieveSettings().  This should NEVER happen because " \
      "it will do nothing.  Please Fix." << endl; \
    return returnValue; \
  }

#define functionCallPostOrderCheck(functionName, returnValue) \
  if(d->retrievedSettings){ \
      kdDebug(180) << "KAutoConfig::"functionName"() was called after " \
      "KAutoConfig::retrieveSettings().  This should NEVER happen because " \
      "it will do nothing.  Please Fix." << endl; \
    return returnValue; \
  }
#else
#define functionCallPostOrderCheck(functionName, returnValue)
#define functionCallPreOrderCheck(functionName, returnValue)
#endif

class KAutoConfig::KAutoConfigPrivate {

public:
  KAutoConfigPrivate() : changed(false)
#ifndef NDEBUG
    , retrievedSettings(false)
#endif
  { init(); }

  // Widgets to parse
  QPtrList<QWidget> widgets;
  // Name of the group that KConfig should be set to for each widget.
  QMap<QWidget*, QString> groups;

  // Child widgets of widgets to ignore
  QPtrList<QWidget> ignore;

  // Reset to false after saveSettings returns true.
  bool changed;

#ifndef NDEBUG
  // Many functions require this to be true to be of any value.
  bool retrievedSettings;
#endif

  // Known widgets that can be configured
  QMap<QWidget*, QPtrList<QWidget> > autoWidgets;
  // Default values for the widgets.
  QMap<QWidget*, QVariant> defaultValues;
  // Widgets to not get properties on (QLabel etc)
  QAsciiDict<int> ignoreTheseWidgets;

  void init(){
    ignoreTheseWidgets.insert("QLabel", new int(1));
    ignoreTheseWidgets.insert("QFrame", new int(2));
    ignoreTheseWidgets.insert("QGroupBox", new int(3));
    ignoreTheseWidgets.insert("QButtonGroup", new int(4));
    ignoreTheseWidgets.insert("QWidget", new int(5));
    ignoreTheseWidgets.setAutoDelete(true);

    static bool defaultKDEPropertyMapInstalled = false;
    if ( !defaultKDEPropertyMapInstalled && kapp ) {
      kapp->installKDEPropertyMap();
      defaultKDEPropertyMapInstalled = true;
    }
  }
};

KAutoConfig::KAutoConfig(KConfig *kconfig, QObject *parent,
    const char *name) : QObject(parent, name), config(kconfig) {
  d = new KAutoConfigPrivate();
}

KAutoConfig::KAutoConfig(QObject *parent, const char *name) :
    QObject(parent, name), config(KGlobal::config()) {
  d = new KAutoConfigPrivate();
}

KAutoConfig::~KAutoConfig(){
  delete d;
}

void KAutoConfig::addWidget(QWidget *widget, const QString &group){
  functionCallPostOrderCheck("addWidget",);
  d->groups.insert(widget, group);
  d->widgets.append(widget);
  QPtrList<QWidget> newAutoConfigWidget;
  d->autoWidgets.insert(widget, newAutoConfigWidget );
}

void KAutoConfig::ignoreSubWidget(QWidget *widget){
  functionCallPostOrderCheck("ignoreSubWidget",);
  d->ignore.append(widget);
}

bool KAutoConfig::retrieveSettings(bool trackChanges){
#ifndef NDEBUG
  if(d->retrievedSettings){
      kdDebug(180) << "This should not happen.  Function "
       "KAutoConfig::retrieveSettings() was called more then once, returning "
       "false.  Please fix." << endl;
    return false;
  }
  d->retrievedSettings = true;
#endif
  
  if(trackChanges){
    // QT
    changedMap.insert(QString::fromLatin1("QButton"), SIGNAL(stateChanged(int)));
    changedMap.insert(QString::fromLatin1("QCheckBox"), SIGNAL(stateChanged(int)));
    changedMap.insert(QString::fromLatin1("QPushButton"), SIGNAL(stateChanged(int)));
    changedMap.insert(QString::fromLatin1("QRadioButton"), SIGNAL(stateChanged(int)));
    changedMap.insert(QString::fromLatin1("QComboBox"), SIGNAL(activated (int)));
    //qsqlproperty map doesn't store the text, but the value!
    //changedMap.insert(QString::fromLatin1("QComboBox"), SIGNAL(textChanged(const QString &)));
    changedMap.insert(QString::fromLatin1("QDateEdit"), SIGNAL(valueChanged(const QDate &)));
    changedMap.insert(QString::fromLatin1("QDateTimeEdit"), SIGNAL(valueChanged(const QDateTime &)));
    changedMap.insert(QString::fromLatin1("QDial"), SIGNAL(valueChanged (int)));
    changedMap.insert(QString::fromLatin1("QLineEdit"), SIGNAL(textChanged(const QString &)));
    changedMap.insert(QString::fromLatin1("QSlider"), SIGNAL(valueChanged(int)));
    changedMap.insert(QString::fromLatin1("QSpinBox"), SIGNAL(valueChanged(int)));
    changedMap.insert(QString::fromLatin1("QTimeEdit"), SIGNAL(valueChanged(const QTime &)));
    changedMap.insert(QString::fromLatin1("QTextEdit"), SIGNAL(textChanged()));
    changedMap.insert(QString::fromLatin1("QTextBrowser"), SIGNAL(sourceChanged(const QString &)));
    changedMap.insert(QString::fromLatin1("QMultiLineEdit"), SIGNAL(textChanged()));
    changedMap.insert(QString::fromLatin1("QListBox"), SIGNAL(selectionChanged()));
    changedMap.insert(QString::fromLatin1("QTabWidget"), SIGNAL(currentChanged(QWidget *)));

    // KDE
    changedMap.insert( QString::fromLatin1("KComboBox"), SIGNAL(activated (int)));
    changedMap.insert( QString::fromLatin1("KFontCombo"), SIGNAL(activated (int)));
    changedMap.insert( QString::fromLatin1("KFontRequester"), SIGNAL(fontSelected(const QFont &)));
    changedMap.insert( QString::fromLatin1("KFontChooser"),  SIGNAL(fontSelected(const QFont &)));
    changedMap.insert( QString::fromLatin1("KHistoryCombo"), SIGNAL(activated (int)));

    changedMap.insert( QString::fromLatin1("KColorButton"), SIGNAL(changed(const QColor &)));
    changedMap.insert( QString::fromLatin1("KDatePicker"), SIGNAL(dateSelected (QDate)));
    changedMap.insert( QString::fromLatin1("KEditListBox"), SIGNAL(changed()));
    changedMap.insert( QString::fromLatin1("KListBox"), SIGNAL(selectionChanged()));
    changedMap.insert( QString::fromLatin1("KLineEdit"), SIGNAL(textChanged(const QString &)));
    changedMap.insert( QString::fromLatin1("KPasswordEdit"), SIGNAL(textChanged(const QString &)));
    changedMap.insert( QString::fromLatin1("KRestrictedLine"), SIGNAL(textChanged(const QString &)));
    changedMap.insert( QString::fromLatin1("KTextBrowser"), SIGNAL(sourceChanged(const QString &)));
    changedMap.insert( QString::fromLatin1("KTextEdit"), SIGNAL(textChanged()));
    changedMap.insert( QString::fromLatin1("KURLRequester"),  SIGNAL(textChanged (const QString& )));
    changedMap.insert( QString::fromLatin1("KIntNumInput"), SIGNAL(valueChanged (int)));
    changedMap.insert( QString::fromLatin1("KIntSpinBox"), SIGNAL(valueChanged (int)));
    changedMap.insert( QString::fromLatin1("KDoubleNumInput"), SIGNAL(valueChanged (double)));
  }

  // Go through all of the children of the widgets and find all known widgets
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *widget;
  bool usingDefaultValues = false;
  while ( (widget = it.current()) != 0 ) {
    ++it;
    config->setGroup(d->groups[widget]);
    usingDefaultValues |= parseChildren(widget, d->autoWidgets[widget], trackChanges);
  }
  return usingDefaultValues;
}

bool KAutoConfig::saveSettings() {
  functionCallPreOrderCheck("saveSettings", false);

  QSqlPropertyMap *propertyMap = QSqlPropertyMap::defaultMap();
  // Go through all of the widgets
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *widget;
  while ( (widget = it.current()) != 0 ) {
    ++it;
    config->setGroup(d->groups[widget]);

    // Go through the known autowidgets of this widget and save
    QPtrListIterator<QWidget> it( d->autoWidgets[widget] );
    QWidget *groupWidget;
    bool widgetChanged = false;
    while ( (groupWidget = it.current()) != 0 ){
      ++it;
      QVariant defaultValue = d->defaultValues[groupWidget];
      QVariant currentValue = propertyMap->property(groupWidget);
#if KDE_IS_VERSION( 3, 1, 90 )
      if(!config->hasDefault(QString::fromLatin1(groupWidget->name())) && currentValue == defaultValue){
        config->revertToDefault(QString::fromLatin1(groupWidget->name()));
        widgetChanged = true;
      }
      else{
#endif
        QVariant savedValue = config->readPropertyEntry(groupWidget->name(),
                                                             defaultValue);
        if(savedValue != currentValue){
          config->writeEntry(groupWidget->name(), currentValue);
          widgetChanged = true;
        }
#if KDE_IS_VERSION( 3, 1, 90 )
      }
#endif
    }
    d->changed |= widgetChanged;
    if(widgetChanged)
      emit( settingsChanged(widget) );
  }

  if(d->changed){
    emit( settingsChanged() );
    d->changed = false;
    config->sync();
    return true;
  }
  return false;
}

bool KAutoConfig::hasChanged() const {
  functionCallPreOrderCheck("hasChanged", false);

  QSqlPropertyMap *propertyMap = QSqlPropertyMap::defaultMap();
  // Go through all of the widgets
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *widget;
  while ( (widget = it.current()) != 0 ) {
    ++it;
    config->setGroup(d->groups[widget]);
    // Go through the known autowidgets of this widget and save
    QPtrListIterator<QWidget> it( d->autoWidgets[widget] );
    QWidget *groupWidget;
    while ( (groupWidget = it.current()) != 0 ){
      ++it;
      QVariant defaultValue = d->defaultValues[groupWidget];
      QVariant currentValue = propertyMap->property(groupWidget);
      QVariant savedValue = config->readPropertyEntry(groupWidget->name(),
      defaultValue);

      // Return once just one item is found to have changed.
      if((currentValue == defaultValue && savedValue != currentValue) ||
         (savedValue != currentValue))
        return true;
    }
  }
  return false;
}

bool KAutoConfig::isDefault() const {
  functionCallPreOrderCheck("isDefault", false);

  QSqlPropertyMap *propertyMap = QSqlPropertyMap::defaultMap();
  // Go through all of the widgets
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *widget;
  while ( (widget = it.current()) != 0 ) {
    ++it;
    config->setGroup(d->groups[widget]);
    // Go through the known autowidgets of this widget and save
    QPtrListIterator<QWidget> it( d->autoWidgets[widget] );
    QWidget *groupWidget;
    while ( (groupWidget = it.current()) != 0 ){
      ++it;
      QVariant defaultValue = d->defaultValues[groupWidget];
      QVariant currentValue = propertyMap->property(groupWidget);
      if(currentValue != defaultValue){
        //qDebug("groupWidget %s, has changed: default: %s new: %s", groupWidget->name(), defaultValue.toString().latin1(), currentValue.toString().latin1());
        return false;
      }
    }
  }
  return true;
}

void KAutoConfig::resetSettings() const {
  functionCallPreOrderCheck("resetSettings",);

  QSqlPropertyMap *propertyMap = QSqlPropertyMap::defaultMap();
  // Go through all of the widgets
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *widget;
  while ( (widget = it.current()) != 0 ) {
    ++it;
    config->setGroup(d->groups[widget]);

    // Go through the known autowidgets of this widget and save
    QPtrListIterator<QWidget> it( d->autoWidgets[widget] );
    QWidget *groupWidget;
    while ( (groupWidget = it.current()) != 0 ){
      ++it;
      QVariant defaultValue = d->defaultValues[groupWidget];
      if(defaultValue != propertyMap->property(groupWidget)){
        propertyMap->setProperty(groupWidget, defaultValue);
        d->changed = true;
      }
    }
  }
}

void KAutoConfig::reloadSettings() const {
  functionCallPreOrderCheck("reloadSettings", );

  QSqlPropertyMap *propertyMap = QSqlPropertyMap::defaultMap();
  // Go through all of the widgets
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *pageWidget;
  while ( (pageWidget = it.current()) != 0 ) {
    ++it;
    config->setGroup(d->groups[pageWidget]);

    // Go through the known widgets of this page and reload
    QPtrListIterator<QWidget> it( d->autoWidgets[pageWidget] );
    QWidget *widget;
    while ( (widget = it.current()) != 0 ){
      ++it;
      QVariant defaultSetting = d->defaultValues[widget];
      QVariant setting = 
        config->readPropertyEntry(widget->name(), defaultSetting);
      propertyMap->setProperty(widget, setting);
    }
  }
  d->changed = false;
}

bool KAutoConfig::parseChildren(const QWidget *widget,
    QPtrList<QWidget>& currentGroup, bool trackChanges){
  bool valueChanged = false;
  const QPtrList<QObject> *listOfChildren = widget->children();
  if(!listOfChildren)
    return valueChanged;

  QSqlPropertyMap *propertyMap = QSqlPropertyMap::defaultMap();
  QPtrListIterator<QObject> it( *listOfChildren );
  QObject *object;
  while ( (object = it.current()) != 0 )
  {
    ++it;
    if(!object->isWidgetType()){
      continue;
    }
    QWidget *childWidget = (QWidget *)object;
    if(d->ignore.containsRef(childWidget)){
      continue;
    }

    bool parseTheChildren = true;
#ifndef NDEBUG
    if(d->ignoreTheseWidgets[childWidget->className()] == 0 &&
		    childWidget->name(0) == NULL){
      // Without a name the widget is just skipped over.
      kdDebug(180) << "KAutoConfig::retrieveSettings, widget with "
        "NULL name.  className: " << childWidget->className() << endl;
    }
#endif
    
    
    if( d->ignoreTheseWidgets[childWidget->className()] == 0 &&  
      childWidget->name(0) != NULL )
    {
      QVariant defaultSetting = propertyMap->property(childWidget);
      if(defaultSetting.isValid())
      {
        parseTheChildren = false;
        // Disable the widget if it is immutable?
        if(config->entryIsImmutable( QString::fromLatin1(childWidget->name())))
          childWidget->setEnabled(false);
        else
        {
          // FOR THOSE WHO ARE LOOKING
          // Here is the code were the widget is actually marked to watch.
          //qDebug("KAutoConfig: Adding widget(%s)",childWidget->name()); 
          currentGroup.append(childWidget);
          d->defaultValues.insert(childWidget, defaultSetting);
        }
        // Get/Set settings and connect up the changed signal
        QVariant setting =
         config->readPropertyEntry(childWidget->name(), defaultSetting);
        if(setting != defaultSetting)
        {
          propertyMap->setProperty(childWidget, setting);
          valueChanged = true;
        }
        if(trackChanges && changedMap.find(QString::fromLatin1(childWidget->className())) !=
            changedMap.end())
        {
          connect(childWidget, changedMap[QString::fromLatin1(childWidget->className())],
                  this, SIGNAL(widgetModified()));
        }
#ifndef NDEBUG
        else if(trackChanges &&
          changedMap.find(QString::fromLatin1(childWidget->className())) == changedMap.end())
        {
          // Without a signal kautoconfigdialog could incorectly 
	  // enable/disable the buttons
	  kdDebug(180) << "KAutoConfig::retrieveSettings, Unknown changed "
           "signal for widget:" << childWidget->className() << endl;
        }
#endif

      }
#ifndef NDEBUG
      else
      { 
        // If kautoconfig doesn't know how to get/set the widget's value 
	// nothing can be done to it and it is skipped. 
	kdDebug(180) << "KAutoConfig::retrieveSettings, Unknown widget:" 
          << childWidget->className() << endl;
      }
#endif
    }
    if(parseTheChildren)
    {
      // this widget is not known as something we can store.
      // Maybe we can store one of its children.
      valueChanged |= parseChildren(childWidget, currentGroup, trackChanges);
    }
  }
  return valueChanged;
}

#include "kautoconfig.moc"

