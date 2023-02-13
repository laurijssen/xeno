#include "winmain.h"
#include "defines.h"
#include "directions.h"
#include "resource.h"

// flag for debugging
#define __DEBUG__

Game::Game(void * hInstance)
{
	GameCounter = 0;
	NeutralCount = 0;
	EnemyCount = 0;
	FriendlyCount = 0;
	cheating = false;
	pause = false;

	Enemy1::Enemy1Count = 0;
	Enemy2::Enemy2Count = 0;
	Enemy3::Enemy3Count = 0;
	Enemy4::Enemy4Count = 0;
	Enemy5::Enemy5Count = 0;
	Enemy6::Enemy6Count = 0;
	Tank::tankspassed = 0;

	ShowCursor(false);

	if( (DDraw = new EASYDRAW(1024, 768, MessageHandler, "Xeno", IDI_APPICON)) == NULL )
		return;

	if( (DInput = new EASYINPUT(DDraw->GetWindowHandle())) == NULL )
		return;	
	
	if( (DSound = new EASYSOUND(DDraw->GetWindowHandle())) == NULL )
		return;	

	CreateUnits();	

	bgmusic = new Midi(DSound->GetSoundObject());
	bgmusic->LoadMidi("sound/bgmusic.mid");
	bgmusic->Play();
}


Game::~Game()
{
	KillTimers();
	ShowCursor(true);

	for( int i = 0; i < GameElement::MAXELEMENTS; i++ )
		if( FriendlyArray[i] != NULL )
			delete FriendlyArray[i];
	for( i = 0; i < GameElement::MAXENEMIES; i++ )
		if( EnemyArray[i] != NULL )
			delete EnemyArray[i];
	for( i = 0; i < GameElement::MAXNEUTRALS; i++ )
		if( NeutralArray[i] != NULL )
			delete NeutralArray[i];	

	FastRocket::surfaces = NULL;
	Enemy5::surfaces = NULL;
	Enemy6::surfaces = NULL;
	FighterRocket::surface = -1;
	Bullet::surface = -1;
	Rocket::surface = -1;

	delete bgmusic;
	delete RocketExplosion;
	delete DDraw;
	delete DSound;
	delete DInput;	
}

/* return true if the timer should keep running */
bool Game::AdvanceGame()
{
	int index, i;

	switch( GetGameProgress() )
	{
		case 0 : 			
			if( EnemyCount > GameElement::MAXENEMIES - 15 )
				return false;

			for( i = 0; i < 11; i++ )
			{
				index = FirstEmptyInEnemyArray();
				EnemyArray[index] = new Enemy4();
				EnemyArray[index]->identifier = index;				
			}
			return false;

		case 1 : case 2 : case 3 : case 4 : case 5 : 						
			for( i = 0; i < 4; i++ )
			{
				index = FirstEmptyInEnemyArray();
				EnemyArray[index] = new Meteor();
				EnemyArray[index]->identifier = index;
			}
			return true;			

		case 6 : case 7 : case 8 :
			for( i = 0; i < 4; i++ )
			{
				index = FirstEmptyInEnemyArray();
				EnemyArray[index] = new Enemy3();
				EnemyArray[index]->identifier = index;				
			}
			return false; // do not continue until these ships are destroyed

		case 9 : case 10 : 			
			for( i = 0; i < 4; i++ )
			{
				index = FirstEmptyInEnemyArray();
				EnemyArray[index] = new Enemy1();
				EnemyArray[index]->identifier = index;				
			}
			return false;

		case 11 : case 12 : case 13 : 
			for( i = 0; i < 4; i++ )
			{
				index = FirstEmptyInEnemyArray();
				EnemyArray[index] = new Enemy2();
				EnemyArray[index]->identifier = index;
			}
			return false;

		case 14 :
			EnemyArray[BOSS1] = new Boss1();
			return false;

		case 15 :
			delete bgmusic; // bgmusic is the background MIDI object
			bgmusic = new Midi(DSound->GetSoundObject());
			bgmusic->LoadMidi("sound/bgmusic2.mid");
			bgmusic->Play();

			LevelSwitch();
			if( !DDraw->LoadBitmap(bground, "data/clouds.bmp") )
				DDraw->ErrorMessage("couldn't create clouds");

			return true;

		case 16 :
			index = FirstEmptyInEnemyArray();
			EnemyArray[index] = new Pause(10);
			EnemyArray[index]->identifier = index;				
			return false;

		case 17 : case 18 :
			for( i = 0; i < 6; i++ )
			{
				index = FirstEmptyInEnemyArray();
				EnemyArray[index] = new Enemy7(i * 160 + 70, 0);
				EnemyArray[index]->identifier = index;				
			}
			return false;

		case 19 : case 20 :
			for( i = 0; i < 9; i++ )
			{
				index = FirstEmptyInEnemyArray();

				if( i & 1 )
					EnemyArray[index] = new Enemy6(1024 - i * 100, 768);
				else
					EnemyArray[index] = new Enemy6(i * 100, 768);

				EnemyArray[index]->identifier = index;				
			}
			return false;

		case 21 :
			SetTimer(DDraw->GetWindowHandle(), TANKINTERRUPT, 3000, NULL);
			EnemyArray[BOSS2] = new Boss2();
			return false;

		default :
			game->StartOver();
			SetTimer(DDraw->GetWindowHandle(), GAMECLOCK, GAMESPEED, NULL);			

			return true; // keep the timer running
	}
}

/* The fighter is created and the background */
void Game::CreateUnits()
{
	int i;

	for( i = 0; i < GameElement::MAXELEMENTS+1; i++ ) // +1 bevause of sentinel in the end
		FriendlyArray[i] = NULL;

	for( i = 0; i < GameElement::MAXENEMIES; i++ )
		EnemyArray[i] = NULL;

	for( i = 0; i < GameElement::MAXNEUTRALS; i++ )
		NeutralArray[i] = NULL;

	bground = DDraw->CreateImage(1024, 768, 0);
	if( !DDraw->LoadBitmap(bground, "data/gas.bmp") )
		DDraw->ErrorMessage("couldn't create background");

	FriendlyArray[FIGHTER] = new Fighter;
	FriendlyCount++;

	if( cheating )
		FriendlyArray[FIGHTER]->hits = 2000;	
	else
		FriendlyArray[FIGHTER]->hits = 4;	
	
	std::string file = "data/rocketexplosion.bmp";	
	RocketExplosion = new Animation(0, 0, 25, 25, 4, file, DDraw);
	for( i = 0; i < 4; i++ )
		DDraw->SetColorKey(0, RocketExplosion->GetFrame(i)); // black

	DDraw->ClearScreen();
	DDraw->Swap();
	DDraw->ClearScreen();

	// load the sounds using easysound	
	rocket = DSound->LoadSound("sound/rocket.wav");
	hurt   = DSound->LoadSound("sound/hurt.wav");
	pulse  = DSound->LoadSound("sound/pulse.wav");
	die    = DSound->LoadSound("sound/die.wav");
	die2   = DSound->LoadSound("sound/die2.wav");
	bang   = DSound->LoadSound("sound/bang.wav");
	bullet = DSound->LoadSound("sound/bullet.wav");
}

void Game::HandleKeys()
{
	if( DInput->KeyDown(DIK_LEFT) )
		FriendlyArray[FIGHTER]->Move(WEST);

	if( DInput->KeyDown(DIK_RIGHT) )
		FriendlyArray[FIGHTER]->Move(EAST);

	if( DInput->KeyDown(DIK_DOWN) )
		FriendlyArray[FIGHTER]->Move(SOUTH);

	if( DInput->KeyDown(DIK_UP) )
		FriendlyArray[FIGHTER]->Move(NORTH);

	if( DInput->KeyDown(DIK_SPACE) )
		FriendlyArray[FIGHTER]->Shoot();

	if( DInput->KeyDown(DIK_PAUSE) || DInput->KeyDown(DIK_P) )
	{
		pause = !pause;
		if( pause )
			KillTimers();
	}

	if( DInput->KeyDown(DIK_RCONTROL) && DInput->KeyDown(DIK_C) )
	{
		GameCounter = 15;
		cheating = !cheating;
		if( cheating )
			FriendlyArray[FIGHTER]->hits = 2000;
		else
			FriendlyArray[FIGHTER]->hits = 4;
	}

	if( DInput->KeyDown(DIK_ESCAPE) )
		PostQuitMessage(0);	
}

void Game::ScrollBackground()
{
	static int y = 0;

	Fighter *fighter = (Fighter *)FriendlyArray[FIGHTER];

	if( y >= 766 )
		y = 0;

	RECT destrect = {0, y, 1024, 768};
	RECT srcrect  = {0, 0, 1024, 768 - y};

	DDraw->DrawImage(&destrect, &srcrect, game->bground, 0, NULL);

	SetRect(&srcrect, 0, 768 - y, 1024, 768);
	SetRect(&destrect, 0, 0, 1024, y);
	DDraw->DrawImage(&destrect, &srcrect, game->bground, 0, NULL);

	y+=2;

	if( fighter->which >= 0 && fighter->which <= 1 )
		DDraw->DrawImage(fighter->x, fighter->y, fighter->surface->GetFrame(0), DDBLT_KEYSRC);
	else
		DDraw->DrawImage(fighter->x, fighter->y, fighter->surface->GetFrame(1), DDBLT_KEYSRC);

	fighter->which >= 10 ? fighter->which = 0 : fighter->which++;
}	

void Game::ShowStatistics()
{
	std::string buff = "Lives left : ";	
	char temp[10];

	itoa(FriendlyArray[FIGHTER]->hits, temp, 10);	
	buff += temp;

	DDraw->DrawString(10, 10, (char *)buff.c_str());

	if( cheating )
	{
		std::string buff2 = "Enemycount : ";
		itoa(EnemyCount, temp, 10);
		buff2 += temp;
		DDraw->DrawString(10, 20, buff2.c_str());

		buff2 = "";
		buff2 = "Friendlycount : ";
		itoa(FriendlyCount, temp, 10);
		buff2 += temp;
		DDraw->DrawString(10, 30, buff2.c_str());

		buff2 = "SurfaceCount : ";
		itoa(DDraw->GetOffsurfaceCount(), temp, 10);
		buff2 += temp;
		DDraw->DrawString(10, 50, buff2.c_str());
	}
}

void Game::AcquireKeyboard()
{
	DInput->Acquire();
}

int Game::FirstEmptyInNeutralArray()
{
	int res = 0;

	for( int i = 0; i < GameElement::MAXNEUTRALS; i++ )
	{
		if( NeutralArray[i] == NULL )
		{
			res = i;
			break;
		}
	}
	return res;
}

void Game::ExtraWave()
{
	if( EnemyCount > GameElement::MAXENEMIES )
		return;

	int index;

	for( int i = 0; i < 20; i+=2)
	{
		index = FirstEmptyInEnemyArray();
		EnemyArray[index] = new Enemy5(50 * i, 0); // x and y position
		EnemyArray[index]->identifier = index;		
	}

	for( i = 1; i < 20; i+=2)
	{
		index = FirstEmptyInEnemyArray();
		EnemyArray[index] = new Enemy5(50 * i, 768); // x and y position
		EnemyArray[index]->identifier = index;		
	}
}

void Game::LevelSwitch()
{	
	DDraw->ClearScreen(NULL);
	DDraw->Swap();
	DDraw->ClearScreen(NULL);	

	for( int i = 0; i < 100; i++ )
	{		
		DDraw->StartDrawing();	
		DDraw->SetColor(DDraw->Convert16Bit(rand() % 31, rand() % 31, rand() % 31));
		DDraw->FillRect(0, 0, 1024, 768);
		DDraw->StopDrawing();
		DDraw->SetColor(65535);
		DDraw->StartDrawing();	
		DDraw->DrawString(1024/3, 300, "Congratulations: Let's make it a little harder");
		DDraw->StopDrawing();
		DDraw->Swap();		
	}

	DDraw->StopDrawing();
	DDraw->SetColor(65535);
}

void Game::PlaySound(int which)
{
	DSound->Play(which);
}

EASYINPUT *Game::GetKeyboard()
{
	return DInput;
}

void Game::SetTimers()
{
	SetTimer(DDraw->GetWindowHandle(), FIGHTERINTERRUPT, 250, NULL); // this timer will send wm-timer msgs to winmain
	SetTimer(DDraw->GetWindowHandle(), GAMECLOCK, GAMESPEED, NULL);
	SetTimer(DDraw->GetWindowHandle(), CLOUDTIMER, 6000, NULL);			
	SetTimer(DDraw->GetWindowHandle(), EXTRAWAVE, 15000, NULL);	
}

void Game::KillTimers()
{
	KillTimer(DDraw->GetWindowHandle(), FIGHTERINTERRUPT);	
	KillTimer(DDraw->GetWindowHandle(), TANKINTERRUPT);
	KillTimer(DDraw->GetWindowHandle(), GAMECLOCK);
	KillTimer(DDraw->GetWindowHandle(), CLOUDTIMER);
	KillTimer(DDraw->GetWindowHandle(), EXTRAWAVE);
}

void Game::SetGameCounter(int i)
{
	GameCounter = i;
}

void Game::ShowMainMenu()
{
	ImageID letter_x = DDraw->CreateImage(223, 209, 0, "data/letter_x.bmp");
	ImageID letter_e = DDraw->CreateImage(72, 83, 0, "data/letter_e.bmp");
	ImageID letter_n = DDraw->CreateImage(85, 83, 0, "data/letter_n.bmp");
	ImageID letter_o = DDraw->CreateImage(80, 82, 0, "data/letter_o.bmp");
	U16 lowcolor = DDraw->Convert16Bit(0, 0, 0);
	DDraw->SetColorKey(lowcolor, DDraw->GetImage(letter_x));
	DDraw->SetColorKey(lowcolor, DDraw->GetImage(letter_e));
	DDraw->SetColorKey(lowcolor, DDraw->GetImage(letter_n));	
	DDraw->SetColorKey(lowcolor, DDraw->GetImage(letter_o));
	Fighter *f = (Fighter *)FriendlyArray[FIGHTER];

	int x = 362;
	int y = 0;

	while(y < 250) // letter x comes flying across the screen
	{
		f->Move(EAST);	
		DDraw->DrawImage(x, y, letter_x, DDBLT_KEYSRC);
		if( DInput->KeyDown(DIK_ESCAPE) )
			goto cleanup;
		Sleep(40);
		y+=8;
		DDraw->Swap();
		DDraw->ClearScreen();
	}

	x = 300, y = 768;
	while( y > 300 ) // the letter e is drawn
	{
		DDraw->DrawImage(362, 250, letter_x, DDBLT_KEYSRC);
		DDraw->DrawImage(x, y, letter_e, DDBLT_KEYSRC);
		if( DInput->KeyDown(DIK_ESCAPE) )
			goto cleanup;
		Sleep(40);
		y -= 12;
		DDraw->Swap();
		DDraw->ClearScreen();
	}

	x = 600, y = 0;
	while( y < 300 ) // the letter n is drawn
	{
		DDraw->DrawImage(362, 250, letter_x, DDBLT_KEYSRC);
		DDraw->DrawImage(300, 300, letter_e, DDBLT_KEYSRC);
		DDraw->DrawImage(x, y, letter_n, DDBLT_KEYSRC);
		if( DInput->KeyDown(DIK_ESCAPE) )
			goto cleanup;
		Sleep(40);
		y += 10;
		DDraw->Swap();
		DDraw->ClearScreen();
	}

	x = 700, y = 768;
	while( y > 350 ) // the letter o is drawn
	{
		DDraw->DrawImage(362, 250, letter_x, DDBLT_KEYSRC);
		DDraw->DrawImage(300, 300, letter_e, DDBLT_KEYSRC);
		DDraw->DrawImage(600, 300, letter_n, DDBLT_KEYSRC);
		DDraw->DrawImage(x, y, letter_o, DDBLT_KEYSRC);
		if( DInput->KeyDown(DIK_ESCAPE) )
			goto cleanup;
		Sleep(40);
		y -= 10;
		DDraw->Swap();
		DDraw->ClearScreen();
	}

	DDraw->ClearScreen(NULL, 65535);
	int i;
	for( i = 0; i < 17; i++ )
	{
		DDraw->Swap();
		Sleep(100);
	}

cleanup:
	Sleep(1000);
	DDraw->ReleaseSurface(letter_x);
	DDraw->ReleaseSurface(letter_e);
	DDraw->ReleaseSurface(letter_n);
	DDraw->ReleaseSurface(letter_o);

	SetTimers(); // turn on the timers to start the game. the menu is gone now
}