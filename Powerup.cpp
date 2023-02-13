#include "powerup.h"
#include "defines.h"
#include "winmain.h"

Pulse::Pulse(int x, int y)
{
	this->x = x, this->y = y, which = 0, hits = 1;
	startY = y;
	height = POWERUPHEIGHT, width = POWERUPWIDTH;

	surface[0] = DDraw->CreateImage(POWERUPWIDTH, POWERUPHEIGHT, 0, "data/pulse.bmp");
	surface[1] = DDraw->CreateImage(POWERUPWIDTH, POWERUPHEIGHT, 0, "data/pulse2.bmp");
}

void Pulse::Move()
{	
	x++, y++;

	if( which >= 0 && which < 5 )
		DDraw->DrawImage(x, y, surface[0], 0, NULL);
	else
		DDraw->DrawImage(x, y, surface[1], 0, NULL);

	which++;

	if( which == 10 )
		which = 0;

	RECT rect = { x, y, x + POWERUPWIDTH, y + POWERUPHEIGHT };

	if( FriendlyArray[FIGHTER]->CheckHit(&rect) )
	{
		game->PlaySound(game->pulse);		
		FriendlyArray[FIGHTER]->hits++;
		this->Remove();
	}
	else if( y > startY + 200 )
		this->Remove();
}

void Pulse::Remove()
{		
	int index = this->identifier;

	DDraw->ReleaseSurface(surface[0]);	
	DDraw->ReleaseSurface(surface[1]);	
	delete EnemyArray[index];
	EnemyArray[index] = NULL;
	game->EnemyCount--;
}

//***************************************************************

FloatText::FloatText(int x, int y, std::string &text)
{
	this->x = x, this->y = y, startY = y;
	this->text = text;

	game->NeutralCount++;
}

void FloatText::Move()
{
	DDraw->SetColor(DDraw->Convert16Bit(rand() % 255, rand() % 255, rand() % 255));

	if( y > 10 && x > 1 )
		DDraw->DrawString(x, y, text.c_str());

	y--;
	if( y < startY - 40 )
		Remove();

	DDraw->SetColor(65535);	
}

void FloatText::Remove()
{
	int index = this->identifier;

	if( game->NeutralArray[index] != NULL )
	{
		game->NeutralArray[index] = NULL;
		delete game->NeutralArray[index];	
		game->NeutralCount--;
	}
}

//**********************************************************
