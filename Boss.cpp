#include "directions.h"
#include "gameelements.h"
#include "defines.h"
#include "winmain.h"

Boss1::Boss1()
{
	x = step = which = 0;
	y = 10;
	state = ALIVE;
	direction = SOUTHEAST;
	hits = 50;
	width = BOSS1WIDTH, height = BOSS1HEIGHT;	

	this->surface[0] = DDraw->CreateImage(BOSS1WIDTH, BOSS1HEIGHT, 0, "data/insectboss1.bmp");
	this->surface[1] = DDraw->CreateImage(BOSS1WIDTH, BOSS1HEIGHT, 0, "data/insectboss2.bmp");

	DDraw->SetColorKey(0, this->surface[0]); // black colorkey
	DDraw->SetColorKey(0, this->surface[1]); // black colorkey

	laser = DDraw->CreateImage(5, 282, 0, "data/laser.bmp");	

	game->EnemyCount++;
}

void Boss1::Move()
{
	static int i = 0;	

	if( direction == SOUTHEAST )
	{
		x+=3, y+=3;
		if( y >= 384 )
			direction = NOWHERE;		
	}
	else if( direction == NORTHEAST )
	{
		x+=3, y-=3;
		if( y <= 10 )
			direction = WEST;				
	}
	else if( direction == WEST )
	{
		x-=3;
		if( x <= 0 )
			direction = SOUTHEAST;			
	}
	else // direction == NOWHERE, so shoot the laser
	{		
		if( i < 100 ) // shoot the laser 100 times
		{			
			this->ShootLaser();

			RECT rect = {473, 485, 540, 768}; // these are the coordinates of the laser
			if( FriendlyArray[FIGHTER]->CheckHit(&rect) ) // hit?
			{
				FriendlyArray[FIGHTER]->Explode();
				i = 100;
			}

			i++;
		}
		else // i >= 200
		{
			direction = NORTHEAST, i = 0;
		}
	}
	if( which >= 0 && which < 5 )
		DDraw->DrawImage(x, y, this->surface[0], DDBLT_KEYSRC);
	else
		DDraw->DrawImage(x, y, this->surface[1], DDBLT_KEYSRC);

	which++;
	if( which > 10 )
		which = 0;

	DDraw->SetColor(DDraw->Convert16Bit(0, 63, 0));
	DDraw->StartDrawing();
	DDraw->FrameRect(x, y - 4, x + BOSS1WIDTH, y);
	DDraw->FillRect(x, y - 4, x + hits * (BOSS1WIDTH / 50) , y);
	DDraw->SetColor(65535);
	DDraw->StopDrawing();

	// check if the boss touches the fighter and if so destroy the fighter
	RECT rect = { x + 30, y + 20, x + 220, y + 130 };

	if( FriendlyArray[FIGHTER]->CheckHit(&rect) )
		FriendlyArray[FIGHTER]->Explode();

	if( rand() % 15 == 0 )
		ShootRocket();
}

void Boss1::ShootLaser()
{	
	DDraw->DrawImage(505, 485, laser, 0, NULL); // middle laser
	DDraw->DrawImage(473, 485, laser, 0, NULL); // left laser
	DDraw->DrawImage(542, 485, laser, 0, NULL); // right laser
}

void Boss1::Remove()
{
	DDraw->ReleaseSurface(laser); 

	DDraw->ReleaseSurface(surface[0]);
	DDraw->ReleaseSurface(surface[1]);

	delete EnemyArray[BOSS1];
	EnemyArray[BOSS1] = NULL;
	SetTimer(DDraw->GetWindowHandle(), GAMECLOCK, GAMESPEED, NULL);
	game->EnemyCount--;
	KillCount = 0;
}

//**************************************************************************

Boss2::Boss2()
{
	y = step = 0;
	x = 400;
	state = ALIVE;
	hits = 75;
	direction = SOUTHWEST;

	width = BOSS2WIDTH, height = BOSS2HEIGHT;	

	this->surface = DDraw->CreateImage(BOSS2WIDTH, BOSS1HEIGHT, 0, "data/mechboss.bmp");

	DDraw->SetColorKey(0, this->surface); // black colorkey

	game->EnemyCount++;
}

void Boss2::Move()
{
	static int i = 0;

	if (direction == SOUTHWEST)
	{
		x -= 8, i++;

		if( i > 5 )
		{
			y += 4;
			i = 0;
		}

		if( x < 250 )
		{
			direction = SOUTHEAST;
			Shoot();
		}
	}		
	else if( direction == SOUTHEAST )
	{
		x+=8, i++;
		if( i > 5 )
		{
			y += 4;
			i = 0;
		}

		if( x > 550 )
		{
			direction = SOUTHWEST;
			Shoot();
		}
	}
	else if( direction == NORTH )
	{
		y -= 12;
		if( y < 5 )
		{
			direction = SOUTHWEST;
			Shoot();
		}
	}
	
	if( y > 600 )	
		direction = NORTH;

	DDraw->DrawImage(x, y, this->surface, DDBLT_KEYSRC);
	DDraw->SetColor(DDraw->Convert16Bit(0, 63, 0));
	DDraw->StartDrawing();
	DDraw->FrameRect(x, max(0, y - 5), x + BOSS2WIDTH-25, y);
	DDraw->FillRect(x, max(0, y - 5), x + hits * (BOSS2WIDTH / 75) , y);
	DDraw->SetColor(65535);
	DDraw->StopDrawing();
}

void Boss2::Shoot()
{
	for (int i = 0; i < 3; i++)
	{
		if( game->FriendlyCount >= MAXELEMENTS - 1 ) // rockets cannot be fired
			break;

		int index = FirstEmptyInFriendlyArray(); // a rocket belongs in the friendly array
		if (index == -1)
			return;

		if (i == 0)
			FriendlyArray[index] = new FastRocket(x + (BOSS2WIDTH>>1), y + (BOSS2HEIGHT>>1), SOUTH); // and not in the enemy
		else if( i == 1 )
			FriendlyArray[index] = new FastRocket(x + (BOSS2WIDTH>>1), y + (BOSS2HEIGHT>>1), SOUTHEAST);
		else
			FriendlyArray[index] = new FastRocket(x + (BOSS2WIDTH>>1), y + (BOSS2HEIGHT>>1), SOUTHWEST);

		FriendlyArray[index]->identifier = index;				

	}
	game->PlaySound(game->bang);
}

void Boss2::Remove()
{
	DDraw->ReleaseSurface(surface); 

	delete EnemyArray[BOSS1];
	EnemyArray[BOSS2] = NULL;
	SetTimer(DDraw->GetWindowHandle(), GAMECLOCK, GAMESPEED, NULL);	
	game->EnemyCount--;
	KillCount = 0;
}