#ifndef __GAME_H_
#define __GAME_H_

#include <easyinput.h>
#include <easysound.h>
#include "gameelements.h"

// hold some game administration vars
typedef class Game
{
public:			
	Animation *RocketExplosion;
	int NeutralCount, FriendlyCount, EnemyCount;
	int bground; // background identifier
	GameElement *NeutralArray[GameElement::MAXNEUTRALS];
	bool pause;

	EASYSOUND *DSound;

	// identifiers for playing sounds
	int rocket, hurt, pulse, die, die2, bang, bullet;

	Game(void * hInstance);	
	~Game();	
	bool AdvanceGame();
	void HandleKeys();
	void ScrollBackground();
	void PlaySound(int which);
	void ShowStatistics();
	int FirstEmptyInNeutralArray();
	void AcquireKeyboard();
	void StartOver() { GameCounter = 0; }
	void ExtraWave();	
	EASYINPUT *GetKeyboard();
	unsigned int GetGameProgress() { return GameCounter++; } // also advance the game by one step
	void SetTimers();
	void KillTimers();	
	void SetGameCounter(int i);
	void ShowMainMenu();

private:	
	bool cheating; // oh my god, say it ain't so!
	Midi *bgmusic;
	unsigned int GameCounter;
	EASYINPUT *DInput;
	Overlay *overlay;

	void CreateUnits();
	void LevelSwitch();
} Game, *Gameptr, **GameList;

#endif