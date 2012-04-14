/*
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef KPOKER_H
#define KPOKER_H


// QT includes
#include <qwidget.h>
#include <qptrlist.h>

// KDE includes
#include <krandomsequence.h>

// own includes
#include "poker.h"


// QT classes
class QPushButton;
class QLineEdit;
class QLabel;
class QFrame;  
class QLineEdit;
class QFrame;
class QHBoxLayout;
class QVBoxLayout;

// KDE classes
class KConfig;


// own classes
class BetBox;
class CardWidget;
class OptionsDlg;
class NewGameDlg;
class PokerPlayer;
class PlayerBox;


// ================================================================
//                            Poker Game


enum PokerGameType {
  SinglePlayer,
  MultiPlayer
};


enum PokerGameState {
  StateStartRound=0,		// Before dealing. Deal cards to switch state.

  StateBet1,
  StateRaise1,

  StateExchangeCards,

  StateBet2,
  StateRaise2,

  StateSee
};


// A PokerGame would be the main class in any poker game.  It controls
// the game flow and has control over all the players.
//
// FIXME: This is very much in flux right now.  It is in the process
//        of being separated from class kpok.  A lot of things need to 
//        be done.  Among them are:
//          - Finish separation from kpok.

class PokerGame {
public:
  PokerGame(KRandomSequence *random);
  ~PokerGame();

  PokerGameType   getType() const                { return m_type; }
  void            setType(PokerGameType type)    { m_type = type; }

  // The state of the current round.
  PokerGameState  getState() const               { return m_state; }
  void            setState(PokerGameState state) { m_state = state; }

  // Money in the game.
  void            clearPot()                     { m_pot = 0; }
  int             getPot() const                 { return m_pot; }
  void            bet(int amount)                { m_pot += amount; }
  void            bet(PokerPlayer * player, int amount);

  int             getMinBet() const              { return m_minBet; }
  int             getMaxBet() const              { return m_maxBet; }
  void            setBettingLimits(int minBet, int maxBet)
    { m_minBet = minBet;  m_maxBet = maxBet; }

  // Players
  int             getNumPlayers() const          { return m_numPlayers; }
  int             getNumActivePlayers() const    { return m_activePlayers.count(); }
  int             getNumInactivePlayers() const  { return m_removedPlayers.count(); }
  PokerPlayer *   getActivePlayer(unsigned int nr)  { return m_activePlayers.at(nr); }
  void            activatePlayer(PokerPlayer *player);
  void            inactivatePlayer(PokerPlayer *player);
  bool            isActivePlayer(PokerPlayer *player) const { return m_activePlayers.contains(player); }

  // Misc
  // FIXME: clearDirty should only be called by a save method
  //        The isDirty flag should only be set internally.
  void            setDirty()                     { m_isDirty = true; }
  void            clearDirty()                   { m_isDirty = false; }
  bool            isDirty() const                { return m_isDirty; }

  // Some more complex methods.  FIXME: These must be expanded!
  void  newGame(PokerGameType type, 
		int numPlayers, PokerPlayer *players,
		int minBet, int MaxBet);
  void  newRound();

  void  dealCards(PokerPlayer *player, bool skip[]);


 private:

  // ----------------------------------------------------------------
  // Properties of the entire game, not just one round:

  PokerGameType   m_type;	// The current type of game
  unsigned int    m_numPlayers; // Used for constructing and deleting only
  PokerPlayer    *m_players;	// The players (owned by kpok)

  int             m_minBet; // the money the player will bet if he wants or not
  int             m_maxBet; // max total bet including minBet.

  // True if we need to save before exiting.
  // This is the case if the cash has changed for any of the players.
  bool            m_isDirty;

  // ----------------------------------------------------------------
  // Properties of the current round:

  PokerGameState  m_state;	// The current phase of the game round
  CardDeck        m_deck;	// The card deck we are using
  int             m_pot; 	// The amount of money people have bet.

  // The players in the game.
  QPtrList<PokerPlayer>   m_activePlayers;  // players still in the round
  QPtrList<PokerPlayer>   m_removedPlayers; // players out of this round
};


// ================================================================
//                          Poker Game View


class kpok : public QWidget
{
  Q_OBJECT

 public:
  kpok(QWidget * parent = 0, const char *name = 0);
  virtual ~kpok();
	
  QString getName (int playerNr); 
  void paintCash();

  bool isDirty() const                          { return m_game.isDirty(); }
	
  void setBlinking(bool bl)			{ m_blinking = bl; }
  void setAdjust(bool ad);
  void setSound(bool s);
	
  void updateLHLabel();//temporary function, only called once

  bool getSound() const				{ return sound; }
  bool getBlinking() const			{ return m_blinking; }
  bool getAdjust() const			{ return adjust; }

 signals:
  void changeLastHand(const QString &newHand, bool lastHand = true);
  void showClickToHold(bool show);
  void statusBarMessage(QString);
  void clearStatusBar();

 protected:
  void initWindow(); // called only once 
  void readOptions();
  void drawCards(PokerPlayer* p, bool skip[]);
  void newRound();
  void noMoney();
  void paintEvent( QPaintEvent * );	
  void playSound(const QString &filename);
  void setBetButtonEnabled(bool enabled);
  void setHand(const QString& newHand, bool lastHand = true);
  void setLastWinner(const QString& lastWinner);
  void startBlinking();
  void stopBlinking();
  void stopDrawing();
  void result();
  void winner();
	
  void bet();

  void displayWin(const QString& hand, int cashWon);

  /**
   * Displays the winner, adds the pot to his money
   *
   * othersPassed = true means all the other players didn't bet
   **/
  void displayWinner_Computer(PokerPlayer* winner, bool othersPassed);
	
  void removePlayerFromRound(PokerPlayer* removePlayer);
  void switchToOnePlayerRules();

  /**
   * @return The human player if he is in the game
   **/
  PokerPlayer* findHumanPlayer();

  /**
   * This method first reads the config file then starts a
   * newGame Dialog if it wasn't forbidden in config
   * 
   * After all options and defaults were set the method starts
   * @ref initPoker() with only the number of the players as an
   * argument or with a complete player class, depending on the
   * options the player chose @return True if successful false
   * if player clicked 'cancel' in the new game dialog
   **/
  bool readEntriesAndInitPoker();

  /**
   * This should only be done in the see phase and shows the
   * cards of the computer players
   **/
  void showComputerCards();

  /**
   * The main method for starting the game
   *
   * It constructs the players if only the number of players
   * are given or uses an existing array Then all player boxes
   * are being constructed as well as some smaller things like
   * the pot
   * @param players the number of the players 
   * @param ownAllPlayers This is used if there is already an array of players existing e.g. from @ref readEntriesAndInitPoker()
   **/
  void initPoker(unsigned int numPlayers);

  /**
   * Gives all players the deck as a card
   **/
  void drawAllDecks();


 public slots:
  void slotCardDeck();
  void toggleSound();
  void toggleAdjust();
  void toggleBlinking();
  void slotPreferences();
	
  bool initSound();

  /**
   * Just as the name says: This method/slot saves the current
   * game (to the config file)
   *
   * The game can be loaded on startup by activating the button
   * 'read from config'
   **/
  void saveGame(KConfig* conf);

  bool loadGame(KConfig* conf);
  bool loadGame();
  //	void commandCallback(int id);
  void newGame();
  void saveGame();

  void exchangeCard1();
  void exchangeCard2();
  void exchangeCard3();
  void exchangeCard4();
  void exchangeCard5();
  void drawClick();

  protected slots:
    void bTimerEvent();
  void drawCardsEvent();
  void waveTimerEvent();

  void betChange(int);
  void adjustBet();
  void out();

  void startWave();
  void stopWave();
  void toggleHeld(); // play a sound

 private:
  // The "document" - the game itself
  PokerGame       m_game;       // The game that this widget is showing.
  unsigned int    m_numPlayers;
  PokerPlayer    *m_players;	// The players

  int             cashPerRound;   // single player game: the ante
  int             currentMustBet; // the minimum bet amount
  int             oldBet_raise;   // used for raising only

  int             drawDelay;

  // Graphical layout.
  QVBoxLayout  *topLayout;
  QHBoxLayout  *inputLayout;
  QLabel       *potLabel;
  BetBox       *betBox;
  QPushButton  *drawButton;    // the main Button
  QLabel       *wonLabel;      // the winner
  QLabel       *clickToHold; 
  QWidget      *mWonWidget;

  PlayerBox   **playerBox;     //one box per player

  // Dialogs
  OptionsDlg* mOptions;
	
  // Other stuff
  KRandomSequence  m_random;

  QTimer       *blinkTimer; // the winning cards will blink 
  QTimer       *drawTimer;  // delay between drawing of the cards 
  QTimer       *waveTimer;  // for displaying of the win (if winner == human)

  bool          adjust;     // allow user to adjust the bet.
  int           drawStat;   // status of drawing (which card already was drawn etc.

  bool          sound;

  bool          m_blinking;	// True if card should blink when winning.
  int           m_blinkStat;	// status of blinking
  int           m_blinkingBox;	// box of winning player
   
  bool          waveActive;
  int           fCount;

  QString lastHandText;
};

#endif
