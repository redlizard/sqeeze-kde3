#ifndef __OPENWITHPART_H__
#define __OPENWITHPART_H__

#include <kurl.h>

#include "kdevplugin.h"

class QPopupMenu;
class Context;


class OpenWithPart : public KDevPlugin
{
	Q_OBJECT

public:

	OpenWithPart(QObject *parent, const char *name, const QStringList &);
	~OpenWithPart();


private slots:

	void fillContextMenu(QPopupMenu *popup, const Context *context);

	void openWithService();
	void openWithDialog();
	void openAsEncoding(int id);
	void openAsUtf8();

private:

  KURL::List m_urls;
};

#endif
