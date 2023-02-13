#include <windows.h>

#define INITGUID // eithet this or add dxguid.lib to the project

#include "winmain.h"
#include "defines.h"

GameElement *FriendlyArray[GameElement::MAXELEMENTS+1];
GameElement *EnemyArray[GameElement::MAXENEMIES];

BOOL ActiveApp;

EASYDRAW *DDraw;
Gameptr game;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdSHow)
{
	char buff[40] = "Hits : ";
	MSG msg;
	int t, i;
	static long time = timeGetTime(); // the time for frame rate adjusting
		
	game = new Game(hInstance);	
	game->ShowMainMenu();

	while( 1 )
	{
		t = PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);
		if( t )
		{
			if( !GetMessage(&msg, NULL, 0, 0) )
			{
				delete game;
				return msg.wParam;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if( ActiveApp )
		{
			if( !game->pause ) // if pause was pressed
			{
				game->ScrollBackground();
				game->ShowStatistics();
				// i starts with 1 because number 0 is the fighter and it is handled by the player
				for( i = 1; i < GameElement::MAXELEMENTS; i++ )
				{
					if( FriendlyArray[i] != NULL )
						FriendlyArray[i]->Move();
				}

				for( i = 0; i < GameElement::MAXENEMIES; i++ )
				{
					if( EnemyArray[i] != NULL )
						EnemyArray[i]->Move();
				}

				for( i = 0; i < GameElement::MAXNEUTRALS; i++ )
				{
					if( game->NeutralArray[i] != NULL )
						game->NeutralArray[i]->Move();
				}

				game->HandleKeys();

				DDraw->Swap(); // flip the backbuffer and make all changes visible

				while( timeGetTime() - time < 25 ) ; // 40 fps (25 ms per frame)
				time = timeGetTime();
			}
			else if( game->GetKeyboard()->KeyDown(DIK_PAUSE) || 
					 game->GetKeyboard()->KeyDown(DIK_P)) // see if the game should be unpaused
			{
				game->SetTimers();
				game->pause = false;
			}
			else
			{
				DDraw->StartDrawing();
				DDraw->DrawString(490, 350, "Game Paused");
				DDraw->Swap();
				DDraw->StopDrawing();
			}
		}
		else
			WaitMessage();
	}		

	return 0;
}


LRESULT CALLBACK MessageHandler(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int index;

	switch( iMsg )
	{
		case WM_TIMER :
			switch( wParam )
			{
				case FIGHTERINTERRUPT : // defined in defines.h
					FriendlyArray[FIGHTER]->state = GameElement::ALIVE; // you can shoot again
					return 0;

				case GAMECLOCK : // defined in defines.h
					if( !game->AdvanceGame() )
						KillTimer(hwnd, GAMECLOCK);						
					return 0;

				case CLOUDTIMER :					
					index = game->FirstEmptyInNeutralArray();
					game->NeutralArray[index] = new Cloud(rand() % 768, rand() & 1 ? true : false);
					game->NeutralArray[index]->identifier = index;
					return 0;

				case EXTRAWAVE :
					game->ExtraWave();
					return 0;

				case TANKINTERRUPT :
					index = FirstEmptyInEnemyArray();			
					EnemyArray[index] = new Tank(rand() % 1024, 0);
					EnemyArray[index]->identifier = index;
					return 0;
			}
			return 0;

		case WM_ACTIVATEAPP :			
			ActiveApp = (BOOL)wParam;
			if( game  && ActiveApp )
			{
				game->AcquireKeyboard();				
			}
			if( DDraw != NULL )
				DDraw->RestoreImages();
			return 0;

		case WM_DESTROY :
			PostQuitMessage(0);			
			return 0;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

// utility function. retrieves the first empty cell in the FriendlyArray array
int FirstEmptyInFriendlyArray()
{
	int i = 1;

	while( FriendlyArray[i] != NULL )
		i++;

	return (i == GameElement::MAXELEMENTS) ? -1 : i;
}

int FirstEmptyInEnemyArray()
{
	int res;

	for( int i = 0; i < GameElement::MAXENEMIES ; i++ )
	{
		if( EnemyArray[i] == NULL )
		{
			res = i;
			break;
		}
	}
	return res;
}
