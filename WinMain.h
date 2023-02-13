#ifndef __WINMAIN_H_
#define __WINMAIN_H_

#include <easydraw.h>
#include "game.h"

LRESULT CALLBACK MessageHandler(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
int FirstEmptyInFriendlyArray(); // returns the first empty index number from ScreenElements[]
int FirstEmptyInEnemyArray();

extern EASYDRAW *DDraw;
extern Gameptr game;
extern GameElement *FriendlyArray[GameElement::MAXELEMENTS+1];
extern GameElement *EnemyArray[GameElement::MAXENEMIES];

#endif