// -*- c++ -*-

#ifndef KSYSTRAYCMD_H
#define KSYSTRAYCMD_H

#include <qlabel.h>
#include <kwin.h>

class KShellProcess;
class KWinModule;

/**
 * Provides a system tray icon for a normal window.
 *
 * @author Richard Moore, rich@kde.org
 */
class KSysTrayCmd : public QLabel
{
  Q_OBJECT
public:
  KSysTrayCmd();
  ~KSysTrayCmd();

  void setWindow( WId w ) { win = w; }
  void setCommand( const QString &cmd ) { command = cmd; }
  void setPattern( const QString &regexp ) { window = regexp; }
    void setStartOnShow( bool enable ) { lazyStart = enable; isVisible = !enable; }
  void setNoQuit( bool enable ) { noquit = enable; }
  void setQuitOnHide( bool enable ) { quitOnHide = enable; }
  void setOnTop( bool enable ) { onTop = enable; }
  void setOwnIcon( bool enable ) { ownIcon = enable; }
  void setDefaultTip( const QString &tip ) { tooltip = tip; }
  bool hasTargetWindow() const { return (win != 0); }
  bool hasRunningClient() const { return (client != 0); }
  const QString &errorMsg() const { return errStr; }

  bool start();

  static WId findRealWindow( WId w, int depth = 0 );

public slots:
  void refresh();

  void showWindow();
  void hideWindow();
  void toggleWindow() { if ( isVisible ) hideWindow(); else showWindow(); }

  void setTargetWindow( WId w );
  void execContextMenu( const QPoint &pos );

  void quit();
  void quitClient();

protected slots:
  void clientExited();

  void windowAdded(WId w);
  void windowChanged(WId w);

protected:
  bool startClient();
  void checkExistingWindows();
  void setTargetWindow( const KWin::WindowInfo &info );

  void mousePressEvent( QMouseEvent *e );

private:
  QString command;
  QString window;
  QString tooltip;
  bool isVisible;
  bool lazyStart;
  bool noquit;
  bool quitOnHide;
  bool onTop; ///< tells if window must stay on top or not
  bool ownIcon; ///< tells if the ksystraycmd icon must be used in systray

  WId win;
  KShellProcess *client;
  KWinModule *kwinmodule;
  QString errStr;

  /** Memorized 'top' position of the window*/
  int top;
  /** Memorized 'left' position of the window*/
  int left;
};

#endif // KSYSTRAYCMD_H
