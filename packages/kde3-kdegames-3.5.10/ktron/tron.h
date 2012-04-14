/* ********************************************************************************
  This file is part of the game 'KTron'

  Copyright (C) 1998-2000 by Matthias Kiefer <matthias.kiefer@gmx.de>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

  *******************************************************************************/  

#ifndef TRON_H
#define TRON_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <math.h>
#include <krandomsequence.h>

class KActionCollection;

#include "player.h"

enum Player{One,Two,Both,Nobody};
// Bits that defines the rect and which sides to draw
enum {BACKGROUND=0, PLAYER1=1,PLAYER2=2,TOP=4,BOTTOM=8,LEFT=16,RIGHT=32};

/**
* @short The playingfield
*/
class Tron : public QWidget
{
  Q_OBJECT

public:
  Tron(QWidget *parent=0, const char *name=0);
  ~Tron();
  void setActionCollection(KActionCollection*);
  void updatePixmap();
  void setBackgroundPix(QPixmap);
  void setComputerplayer(Player player, bool);
  bool isComputer(Player player);
  void setVelocity(int);
  void setRectSize(int newSize);

public slots:
  /** Starts a new game. The difference to reset is, that the players
  * points are set to zero. Emits gameEnds(Nobody).
  */
  void newGame();
  void togglePause();

  /**
  * checks if both players are computer and starts the game
  */
  void computerStart();

  void loadSettings();

signals:
  void gameEnds(Player loser);
  void gameReset();

protected:
  /** bitBlt�s the rect that has to be updated from the
  * bufferpixmap on the screen and writes eventually text
  */
  void paintEvent(QPaintEvent *);
  /** resets game and creates a new playingfield */
  void resizeEvent(QResizeEvent *);
  void keyPressEvent(QKeyEvent *);
  void keyReleaseEvent(QKeyEvent *);
  /** pauses game */
  void focusOutEvent(QFocusEvent *);

private:
  /** Stores key shortcuts */
  KActionCollection* actionCollection;
  /** Drawing buffer */
  QPixmap *pixmap;
  /** The playingfield */
  QMemArray<int> *playfield;
  /** game status flag */
  bool gamePaused;
  /** game status flag */
  bool gameEnded;
  /**  used for waiting after game ended */
  bool gameBlocked;
  /** flag, if a string should be drawn, how to start the game */
  bool beginHint;
  /** Height  of the playingfield in number of rects*/
  int fieldHeight;
  /** Width of the playingfield in number of rects*/
  int fieldWidth;
  QTimer *timer;
  player players[2];

  /** Backgroundpixmap **/
  QPixmap bgPix;

  /** time in ms between two moves */
  int velocity;
  /** size of the rects */
  int rectSize;

  /** The random sequence generator **/
  KRandomSequence random;

  // Options
  /** determines level of computerplayer */
  int lookForward;

  // Funktionen
  /** resets the game */
  void reset();
  /** starts the game timer */
  void startGame();
  /** stops the game timer */
  void stopGame();
  /** creates a new playfield and a bufferpixmap */
  void createNewPlayfield();
  /** paints players at current player coordinates */
  void paintPlayers();
  /** draws a rect in current TronStyle at position x,y of the playingfield */
  void drawRect(QPainter & p, int x, int y);
  /** emits gameEnds(Player) and displays the winner by changing color*/
  void showWinner(Player winner);

  /** calculates if player playerNr would crash
  * if he moves xInc in x-direction and yInc in y-direction
  */
  bool crashed(int playerNr,int xInc, int yInc) const;
  /** calculates if player playerNr should change direction */
  void think(int playerNr);
  void changeDirection(int playerNr,int dis_right,int dis_left);

  /** sets the direction of player playerNr to newDirection */
  void switchDir(int playerNr,Directions::Direction newDirection);
  /**
  * updates the the rect at current position to not draw a
  * border in the direction of the next step.
  * (only used in mode OLine)
  *
  * -1 means update both players
  */
  void updateDirections(int playerNr=-1);
  
private slots:
    /**
    * This is the main function of KTron.
    * It checkes if an accelerator is pressed and than moves this player
    * forward. Then it checkes if a crash occurred.
    * If no crash happen it moves both players forward and checks again
    * if someone crashed.
    */
    void doMove();
    void unblockGame();
    void showBeginHint();
};


#endif // TRON_H

