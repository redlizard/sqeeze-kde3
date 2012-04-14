/**
   File : qxsldbgdoc.h
   Author : Keith Isdale
   Date : 19th April 2002
   Description : The document to handle source and breakpoints
 */


#ifndef QXSLDBGDOC_H
#define QXSLDBGDOC_H

#include <kio/job.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qguardedptr.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

class QXsldbgDoc : public QObject {

Q_OBJECT

 public:
  QXsldbgDoc(QWidget *parent=0, KURL url=QString::null);
  ~QXsldbgDoc();

  void enableBreakPoint(uint lineNumber, bool state);
  void addBreakPoint(uint lineNumber);
  void deleteBreakPoint(uint lineNumber);
  void selectBreakPoint(uint lineNumberbool);
  bool isSelected(uint lineNumberbool);

  KURL url() const;
  KTextEditor::Document *kateDoc() {return kDoc;};
  KTextEditor::View *kateView() {return kView;};
 

  bool isLocked() {return locked;};
  void refresh();

signals:
    void docChanged();

private slots:
    void slotResult( KIO::Job *job );
    void lockDoc();
    void unlockDoc();

 private:
  QGuardedPtr<KTextEditor::Document> kDoc;
  QGuardedPtr<KTextEditor::View> kView;
  bool locked;
};

#endif
