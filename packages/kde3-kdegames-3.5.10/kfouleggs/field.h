#ifndef FE_FIELD_H
#define FE_FIELD_H

#include "common/field.h"
#include "common/misc_ui.h"


class FEField : public Field
{
 Q_OBJECT
 public:
    FEField(QWidget *parent);

 private slots:
    virtual void removedUpdated();
    void settingsChanged();
};

#endif
