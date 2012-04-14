
#ifndef XSLDBGDOC
#define XSLDBGDOC

#include <qstring.h>

class XsldbgDoc {
 public:
  XsldbgDoc();
  XsldbgDoc(const QString &fileName, const QString &text);
  ~XsldbgDoc();

  QString getText() const;
  QString getFileName() const;

  void setRow(int row);
  int getRow();
  void setColumn(int column);
  int getColumn();

 private:
  QString text;
  QString fileName;
  int row, column;

};

#endif
