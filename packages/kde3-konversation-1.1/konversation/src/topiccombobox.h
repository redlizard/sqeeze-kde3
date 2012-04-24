/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  begin:     Don Nov 21 2002
  copyright: (C) 2002 by Dario Abatianni
  email:     eisfuchs@tigress.com
*/

#ifndef TOPICCOMBOBOX_H
#define TOPICCOMBOBOX_H

#include <kcombobox.h>


class TopicComboBox : public KComboBox
{
    Q_OBJECT

    public:
        explicit TopicComboBox(QWidget* parent);
        ~TopicComboBox();
        void insertStringList(const QStringList& list);

    protected:
        void wheelEvent(QWheelEvent *ev);

        signals:
        void topicChanged(const QString& newTopic);

    protected slots:
        void topicActivated(const QString& newTopic);
        void topicActivated(int index);
};
#endif
