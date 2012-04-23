// $Id$

#ifndef ENVVAR_H
#define ENVVAR_H

/*
 * Description of environment variables. Note that the name of the variable
 * is given as the key in the QDict, so we don't repeat it here.
 */

class QListViewItem;

struct EnvVar {
    QString value;
    enum EnvVarStatus { EVclean, EVdirty, EVnew, EVdeleted };
    EnvVarStatus status;
    QListViewItem* item;
};

#endif // ENVVAR_H
