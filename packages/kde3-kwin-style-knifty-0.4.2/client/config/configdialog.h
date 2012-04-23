/****************************************************************************
** Form interface generated from reading ui file './configdialog.ui'
**
** Created: Tue Dec 23 23:38:45 2003
**      by: The User Interface Compiler ()
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QButtonGroup;
class QRadioButton;
class QCheckBox;

class ConfigDialog : public QWidget
{
    Q_OBJECT

public:
    ConfigDialog( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~ConfigDialog();

    QButtonGroup* titleAlign;
    QRadioButton* AlignLeft;
    QRadioButton* AlignHCenter;
    QRadioButton* AlignRight;
    QCheckBox* highContrastButtons;
    QCheckBox* designatedTitlebarColor;
    QCheckBox* bordersBlendColor;
    QCheckBox* titleShadow;

protected:
    QHBoxLayout* ConfigDialogLayout;
    QGridLayout* layout1;
    QHBoxLayout* titleAlignLayout;

protected slots:
    virtual void languageChange();

};

#endif // CONFIGDIALOG_H
