#ifndef __DOCUMENTATIONPART_H__
#define __DOCUMENTATIONPART_H__

#include <kdevhtmlpart.h>

/**
HTML documentation part.

Implements shell-dependent "duplicate" and "open in new window" actions of KDevHTMLPart.
*/
class HTMLDocumentationPart : public KDevHTMLPart
{
  Q_OBJECT

public:

    HTMLDocumentationPart();

protected slots:

    virtual void slotDuplicate();
    virtual void slotOpenInNewWindow(const KURL &url);
};

#endif
