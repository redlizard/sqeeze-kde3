/***************************************************************************
*   Copyright (C) 2004-2006 by Thomas Fischer                             *
*   fischer@unix-ag.uni-kl.de                                             *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef SETTINGSFILEIO_H
#define SETTINGSFILEIO_H

#include <qwidget.h>

class QComboBox;
class QCheckBox;
class QLineEdit;

namespace KBibTeX
{
    class SettingsFileIO : public QWidget
    {
        Q_OBJECT
    public:
        SettingsFileIO( QWidget *parent = 0, const char *name = 0 );
        ~SettingsFileIO();

        void applyData();
        void readData();

    signals:
        void configChanged();

    protected slots:
        void slotConfigChanged();

    private:
        QComboBox *m_comboBoxEncoding;
        QComboBox *m_comboBoxStringDelimiters;
        QComboBox *m_comboBoxKeywordCasing;
        QCheckBox *m_checkBoxProtectCasing;
        QComboBox *m_comboBoxLanguage;
        QComboBox *m_comboBoxBibliographyStyle;
        QCheckBox *m_checkBoxEmbedFiles;
        QComboBox *m_comboBoxExportSystemHTML;

        void setupGUI();

    };
}

#endif
