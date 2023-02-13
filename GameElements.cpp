#include "defines.h"
#include "directions.h"
#include "winmain.h"
#include "powerup.h"

int GameElement::KillCount = 0; // number of enemies shot down

void GameElement::ShootRocket()
{
	if( game->FriendlyCount >= MAXELEMENTS - 1 ) // rockets cannot be fired
		return;

	int index = FirstEmptyInFriendlyArray(); // a rocket belongs in the friendly array
	FriendlyArray[index] = new Rocket(x, y); // and not in the enemy
	FriendlyArray[index]->identifier = index;
	game->FriendlyCount++;
}

void GameElement::Explosion()
{
	if( game->FriendlyCount >= MAXELEMENTS )
		return;

	int index = FirstEmptyInFriendlyArray();

	FriendlyArray[index] = new Explosion1(x, y);
	FriendlyArray[index]->identifier = index;
	game->FriendlyCount++;
}

//****************************************************************

Fighter::Fighter()
{
	x = 512, y = 720;
	state = ALIVE, which = 0;	

	std::string file = "data/firebird.bmp";
	surface = new Animation(0, 0, FIGHTERWIDTH, FIGHTERHEIGHT, 3, file, DDraw);

	for( int i = 0; i < 3; i++ )
		DDraw->SetColorKey(0, surface->GetFrame(i));	
}

void Fighter::Move(int direction)
{
	switch( direction )
	{
		case WEST :
			if( x <= FIGHTERSPEED )
				break;
			else
				x-=FIGHTERSPEED;
			break;

		case EAST :
			if( x >= 976 ) // 40 for the fighterheight
				break;
			else
				x+=FIGHTERSPEED;
			break;

		case NORTH :
			if( y <= FIGHTERSPEED )
				break;
			else
				y -= FIGHTERSPEED;
			break;

		case SOUTH :
			if( y >= 720 ) // 40 for the fighterwidth
				break;
			else
				y += FIGHTERSPEED;
			break;
	}
}

void Fighter::Shoot()
{
	if( this->state == SHOOTING || game->FriendlyCount >= MAXELEMENTS )
		return;
	
	int index = FirstEmptyInFriendlyArray();

	FriendlyArray[index] = new FighterRocket(x+20, y);
	FriendlyArray[index]->identifier = index;	// the index of the rocket	

	this->state = SHOOTING;

	game->PlaySound(game->rocket);	

	game->FriendlyCount++;
}

bool __fastcall Fighter::CheckHit(RECT *rect) // does the fighter touch this rectangle?
{
	bool res = false;

	if( x + FIGHTERWIDTH >= rect->left && x <= rect->right && 
		y + FIGHTERHEIGHT >= rect->top && y <= rect->bottom  )
		res = true;

	return res;
}

void Fighter::Explode()
{
	FriendlyArray[FIGHTER]->hits--;	

	if( FriendlyArray[FIGHTER]->hits == 0 ) // game over, start a new game
	{		
		HINSTANCE hInst = (HINSTANCE)GetWindowLong(DDraw->GetWindowHandle(), GWL_HINSTANCE);
		game->SetGameCounter(0);
		FriendlyArray[FIGHTER]->hits = 5;
		return;
	}

	if( game->NeutralCount < MAXNEUTRALS )
	{
		std::string s = "Lives -1";
		int index = game->FirstEmptyInNeutralArray();
		game->NeutralArray[index] = new FloatText(x, y, s);
		game->NeutralArray[index]->identifier = index;
	}	

	int x_heading = rand() % 1024;
	int y_heading = rand() % 768;

	Fighter *fighter = (Fighter *)FriendlyArray[FIGHTER];

	while( x != x_heading && y != y_heading )
	{		
		if( x < x_heading ) 
			x++;
		else if( x > x_heading )
			x--;

		if( y < y_heading )
			y++;
		else if( y > y_heading )
			y--;
		
		DDraw->DrawOnPrimary(fighter->x, fighter->y, fighter->surface->GetFrame(0), DDBLT_KEYSRC, NULL);		
	}

	game->PlaySound(game->hurt);	
}

//**************************************************************

ImageID Bullet::surface = -1;

Bullet::Bullet(int x, int y, int x_heading, int y_heading)
{
	this->x = x, this->y = y;
	speed = 10;

	this->x_heading = x_heading, this->y_heading = y_heading;

	if( x_heading > x && y_heading > y )
		direction = SOUTHEAST;
	else if( x_heading < x && y_heading > y )
		direction = SOUTHWEST;
	else if( x_heading > x && y_heading < y )
		direction = NORTHEAST;
	else
		direction = SOUTH;

	if( surface == -1 )
	{
		surface = DDraw->CreateImage(BULLETWIDTH, BULLETHEIGHT, 0);
		if( !DDraw->LoadBitmap(this->surface, "data/bullet.bmp") )
			DDraw->ErrorMessage("couldn't load bullet");

		DDraw->SetColorKey(0, this->surface);
	}	
	game->PlaySound(game->bullet);
}

void Bullet::Move()
{
	if( direction == NORTHEAST )
		x += speed, y -= speed;
	else if( direction == SOUTHEAST )
		x += speed, y += speed;
	else if( direction == SOUTHWEST )
		x -= speed, y += speed;
	else
		y += speed;

	DDraw->DrawImage(x, y, surface, DDBLT_KEYSRC, NULL);
	
	if( abs(x - x_heading) <= speed || abs(y - y_heading) <= speed )
		Remove();
	else if( y > 768 || y < 0 || x < 0 || x > 1024 )
		Remove();
	else 
	{
		RECT rect = {x, y, x + ROCKETWIDTH, y + ROCKETHEIGHT};

		if( FriendlyArray[FIGHTER]->CheckHit(&rect) )
		{
			this->Remove();		
			FriendlyArray[FIGHTER]->Explode();
		}
	}
}

void Bullet::Remove()
{
	int index = this->identifier; // identifier is the index in the friendlyarray
	
	delete FriendlyArray[index];
	FriendlyArray[index] = NULL;		

	game->FriendlyCount--;
}

ImageID Rocket::surface = -1;

Rocket::Rocket(int x, int y)
{
	static bool side = false; // this one depends where the rocket comes from

	state = ALIVE;

	if( side )
	{
		this->x = x + 40; // x and y represent the startposition
		this->y = y + 100;
	}
	else
	{
		this->x = x + 210;
		this->y = y + 100;
	}

	side = !side;

	int x_co = FriendlyArray[FIGHTER]->x + 30;
	int y_co = FriendlyArray[FIGHTER]->y + 30;

	x_heading = x_co;
	y_heading = y_co;

	if( y_heading >= y )
		direction = SOUTH;
	else
		direction = NORTH;

	if( surface == -1 )
	{
		this->surface = DDraw->CreateImage(10, 20, 0);

		if( !DDraw->LoadBitmap(this->surface, "data/rocket.bmp") )
			DDraw->ErrorMessage("couldn't load rocket");

		DDraw->SetColorKey(0, this->surface);
	}
}

void Rocket::Move()
{
	static int i = 0;	

	if( state == EXPLODING )
	{
		if( i < 6 )
		    DDraw->DrawImage(x, y, game->RocketExplosion->GetFrame(0), DDBLT_KEYSRC, NULL);
		else if( i >= 6 && i < 12 )
			DDraw->DrawImage(x, y, game->RocketExplosion->GetFrame(1), DDBLT_KEYSRC, NULL);
		else if( i >= 12 && i < 18 )
			DDraw->DrawImage(x, y, game->RocketExplosion->GetFrame(2), DDBLT_KEYSRC, NULL);
		else
			DDraw->DrawImage(x, y, game->RocketExplosion->GetFrame(3), DDBLT_KEYSRC, NULL);

		i++;
		if( i > 24 )
		{
			this->Remove();
			i = 0;
		}

		return; // no more actions are required
	}

	if( x_heading == x && (y_heading >= y - 3 && y_heading <= y + 3 ))
	{		
		this->Explode();
		return;
	}

	if( x_heading > x ) 							
		x++;
	else if( x_heading < x )
		x--;

	if( y_heading > y && direction == SOUTH )
		y+=4;
	else if( y_heading < y && direction == NORTH )
		y-=4;
	else if( direction == SOUTH && y_heading <= y )
		this->Explode();
	else if( direction == NORTH && y_heading >= y )
		this->Explode();

	if( direction == NORTH )
	{
		DDBLTFX fx;
		ZeroMemory(&fx, sizeof(DDBLTFX));
		fx.dwSize = sizeof(fx);
		fx.dwDDFX = DDBLTFX_MIRRORUPDOWN;
		DDraw->DrawImage(x, y, this->surface, DDBLT_DDFX | DDBLT_KEYSRC, &fx);
	}
	else
		DDraw->DrawImage(x, y, this->surface, DDBLT_KEYSRC, NULL);

	RECT rect = {x, y, x + ROCKETWIDTH, y + ROCKETHEIGHT};

	if( FriendlyArray[FIGHTER]->CheckHit(&rect) )
	{
		if( state == ALIVE )
			this->Explode();		
		FriendlyArray[FIGHTER]->Explode();
	}
}

void Rocket::Explode()
{	
	state = EXPLODING;
}

void Rocket::Remove()
{		
	int index = this->identifier; // rockets are in the friendlyarray

	FriendlyArray[index] = NULL;
	delete FriendlyArray[index];
	game->FriendlyCount--;
}
//********************************************************************

// a fastrocket is fired by Boss2
Animation *FastRocket::surfaces = NULL;
int FastRocket::speed = 8;

FastRocket::FastRocket(int x, int y, int direction)
{
	if (surfaces == NULL)
	{
		std::string file = "data/fastrocket.bmp";
		surfaces = new Animation(0, 0, 33, 30, 6, file, DDraw);
		for( int i = 0; i < 6; i++ )
			DDraw->SetColorKey(0, surfaces->GetFrame(i));
	}
	state = ALIVE;
	step = 0;
	this->direction = direction;	
	this->x = x;
	this->y = y;

	game->FriendlyCount++;
}

void FastRocket::Move()
{
	static int i = 0;	

	if( state == EXPLODING )
	{
		if( i < 6 )
		    DDraw->DrawImage(x, y, game->RocketExplosion->GetFrame(0), DDBLT_KEYSRC, NULL);
		else if( i >= 6 && i < 12 )
			DDraw->DrawImage(x, y, game->RocketExplosion->GetFrame(0), DDBLT_KEYSRC, NULL);
		else if( i >= 12 && i < 18 )
			DDraw->DrawImage(x, y, game->RocketExplosion->GetFrame(0), DDBLT_KEYSRC, NULL);
		else
			DDraw->DrawImage(x, y, game->RocketExplosion->GetFrame(0), DDBLT_KEYSRC, NULL);

		i++;
		if( i > 24 )
		{
			this->Remove();
			i = 0;
		}

		return; // no more actions are required
	}

	if( direction == SOUTH )
	{
		DDraw->DrawImage(x, y, surfaces->GetFrame(4), DDBLT_KEYSRC, NULL);
		y += speed;
	}
	else if( direction == SOUTHEAST )
	{
		DDraw->DrawImage(x, y, surfaces->GetFrame(3), DDBLT_KEYSRC, NULL);
		y+=speed, x+=speed;
	}
	else // SOUTHWEST
	{
		DDraw->DrawImage(x, y, surfaces->GetFrame(5), DDBLT_KEYSRC, NULL);
		y+=speed, x-=speed;
	}

	RECT rect = {x, y, x + ROCKETWIDTH, y + ROCKETHEIGHT};

	if( y > 750 )
		Explode();
	else if( FriendlyArray[FIGHTER]->CheckHit(&rect) )
	{
		if( state == ALIVE )
			this->Explode();		
		FriendlyArray[FIGHTER]->Explode();
	}
}

void FastRocket::Remove()
{
	int index = this->identifier; // identifier is the index in the friendlyarray

	delete FriendlyArray[index];	
	FriendlyArray[index] = NULL;	

	game->FriendlyCount--;
}

void FastRocket::Explode()
{
	state = EXPLODING;
}

//********************************************************************

ImageID FighterRocket::surface = -1;

FighterRocket::FighterRocket(int x, int y)
{		
	this->InitialX = this->x = x + 15;
	this->InitialY = this->y = y;

	state = ALIVE;

	if( surface == -1 )
	{		
		this->surface = DDraw->CreateImage(ROCKETWIDTH, ROCKETHEIGHT, 0, "data/f_rocket.bmp");

		DDraw->SetColorKey(0, this->surface);
	}	
}

void FighterRocket::Move()
{
	int index;
		
	y-=6;

	DDraw->DrawImage(x, y, this->surface, DDBLT_KEYSRC, NULL);

	if( y <= 4 ) // top of the screen
	{						
		this->Remove();
	}
	else if( this->CheckHit(index) ) // index is passed by reference
	{
		this->Remove(); // remove the rocket

		EnemyArray[index]->hits--;
		if( EnemyArray[index]->hits <= 0 )
		{
			if( game->NeutralCount < MAXNEUTRALS )
			{
				std::string s = "Gotcha!";
				int arrIndex = game->FirstEmptyInNeutralArray();
				game->NeutralArray[arrIndex] = new FloatText(x, y, s);
				game->NeutralArray[arrIndex]->identifier = arrIndex;
			}	

			rand() & 1 ? game->PlaySound(game->die) : game->PlaySound(game->die2);
			EnemyArray[index]->Remove();
			KillCount++; // static var in gameelement
		}
	}	
}

bool FC FighterRocket::CheckHit(int &index)
{
	for( int i = 0; i < MAXENEMIES; i++ )
	{
		if( EnemyArray[i] )
		{
			if( y >= EnemyArray[i]->y && y <= EnemyArray[i]->y + EnemyArray[i]->height &&
				x >= EnemyArray[i]->x && x <= EnemyArray[i]->x + EnemyArray[i]->width )
			{
				index = i; // passed by reference
				return true;
			}
		}
	}
	return false;
}

void FighterRocket::Remove()
{	
	int index = this->identifier; // identifier is the index in the friendlyarray

	FriendlyArray[index] = NULL;
	delete FriendlyArray[index];	

	game->FriendlyCount--;
}

//***************************************************************************

// static var that keeps track of the number of instances
int Enemy1::Enemy1Count = 0;

Enemy1::Enemy1()
{
	if( Enemy1Count == 0 )
	{
		this->x = 200;
		this->y = 0;
	}
	else if( Enemy1Count == 1 )
	{
		this->x = 824;
		this->y = 0;
	}
	else if( Enemy1Count == 2 )
	{
		this->x = 0;
		this->y = 300;
	}
	else
	{
		this->x = 1024 - ENEMYWIDTH;
		this->y = 300;
	}

	this->type = Enemy1Count;
	this->hits = 1;
	this->height = ENEMYHEIGHT;
	this->width = ENEMYWIDTH;
	Enemy1Count++;

	this->surface = DDraw->CreateImage(ENEMYWIDTH, ENEMYHEIGHT, 0);

	if( !DDraw->LoadBitmap(this->surface, "data/enemy1.bmp") )
		DDraw->ErrorMessage("couldn't load enemy1");

	DDraw->SetColorKey(0, DDraw->GetImage(this->surface));
	game->EnemyCount++;
}

void Enemy1::Move()
{
	if( type == 0 )
		x+=3, y+=3;
	else if( type == 1 )
		x-=3, y+=3;
	else if( type == 2 )
		x+=4;
	else
		x-=4;

	DDraw->DrawImage(x, y, this->surface, DDBLT_KEYSRC, NULL);

	if( type == 0  || type == 1 )
	{
		if( y > 768 )
			this->Remove();
	}
	else if( type == 2 )
	{
		if( x > 1024 )
			this->Remove();
	}
	else // type == 3
	{
		if( x < 0 - ENEMYWIDTH )
			this->Remove();
	}

	if( x > FriendlyArray[FIGHTER]->x - 5 && x < FriendlyArray[FIGHTER]->x + 5 )
		ShootRocket(); // shoot at the fighter when the x-axis is the same

	RECT rect = { x, y + 5, x + ENEMYWIDTH, y + ENEMYHEIGHT };

	if( FriendlyArray[FIGHTER]->CheckHit(&rect) )
		FriendlyArray[FIGHTER]->Explode();
}

void Enemy1::Remove()
{
	int index = this->identifier;	

	Enemy1Count--;

	if( Enemy1Count == 0 ) // advance to the next stage in the game
	{
		KillCount = 0;		
		SetTimer(DDraw->GetWindowHandle(), GAMECLOCK, GAMESPEED, NULL);
	}

	DDraw->ReleaseSurface(this->surface);
	game->EnemyCount--;

	EnemyArray[index] = NULL;
	delete EnemyArray[index];	

	Explosion();
}

int Enemy2::Enemy2Count = 0; // static member

//*******************************************************************

Enemy2::Enemy2()
{
	if( Enemy2Count == 0 )
		x = 100, y = 768;
	else if( Enemy2Count == 1 )
		x = 924, y = 768;
	else if( Enemy2Count == 2 )
		x = 300, y = 0;
	else
		x = 724, y = 0;

	this->type = Enemy2Count;
	this->hits = 1;
	this->height = ENEMYHEIGHT;
	this->width = ENEMYWIDTH;
	Enemy2Count++;

	this->surface = DDraw->CreateImage(ENEMYWIDTH, ENEMYHEIGHT, 0);

	if( !DDraw->LoadBitmap(this->surface, "data/enemy2.bmp") )
		DDraw->ErrorMessage("couldn't create enemy2");

	DDraw->SetColorKey(0, DDraw->GetImage(this->surface));	
	game->EnemyCount++;
}

void Enemy2::Move()
{
	if( type == 0 )
		y-=2;
	else if( type == 1 )
		y-=2;
	else if( type == 2 )
		y+=3;
	else
		y+=3;

	DDraw->DrawImage(x, y, this->surface, DDBLT_KEYSRC, NULL);

	if( type == 0 || type == 1 )
	{
		if( y < 0 - ENEMYHEIGHT )
			this->Remove();
	}
	else if( type == 2 || type == 3 )
	{
		if( y > 768 )
			this->Remove();
	}

	RECT rect = { x + 5, y + 5, x + (ENEMYWIDTH - 5), y + (ENEMYHEIGHT -  5) };

	if( FriendlyArray[FIGHTER]->CheckHit(&rect) )
		FriendlyArray[FIGHTER]->Explode();
}

void Enemy2::Remove()
{
	int index = this->identifier;

	Enemy2Count--;

	if( Enemy2Count == 0 ) // advance to the next stage in the game
	{
		SetTimer(DDraw->GetWindowHandle(), GAMECLOCK, GAMESPEED, NULL);		
		if( KillCount >= 4 )
		{			
			int index = FirstEmptyInEnemyArray();
			EnemyArray[index] = new Pulse(x, y); // a pulse is not really an enemy, but you need to be able to pick it up
			EnemyArray[index]->identifier = index;
			game->EnemyCount++;
		}
		KillCount = 0;
	}

	DDraw->ReleaseSurface(this->surface);
	game->EnemyCount--;

	EnemyArray[index] = NULL;
	delete EnemyArray[index];

	Explosion();
}

//*********************************************************************

int Enemy3::Enemy3Count = 0; // number of enemies on screen

Enemy3::Enemy3()
{
	if( Enemy3Count == 0 )
		x = 0, y = 768;
	else if( Enemy3Count == 1 )
		x = 0, y = 0;
	else if( Enemy3Count == 2 )
		x = 1024, y = 768;
	else
		x = 1024, y = 0;

	this->type = Enemy3Count;
	this->hits = 1;
	this->height = ENEMYHEIGHT;
	this->width = ENEMYWIDTH;
	Enemy3Count++;

	this->surface = DDraw->CreateImage(ENEMYWIDTH, ENEMYHEIGHT, 0);

	if( !DDraw->LoadBitmap(this->surface, "data/enemy3.bmp") )
		DDraw->ErrorMessage("couldn't create enemy3");

	DDraw->SetColorKey(0, DDraw->GetImage(this->surface));	
	game->EnemyCount++;
}

void Enemy3::Move()
{
	if( type == 0 )
		y-=2, x+=2;
	else if( type == 1 )
		y+=2, x+=2;
	else if( type == 2 )
		y-=3, x-=2;
	else
		y+=3, x-=2;

	DDraw->DrawImage(x, y, this->surface, DDBLT_KEYSRC, NULL);

	if( type == 0 || type == 2 )
	{
		if( y < 0 - ENEMYHEIGHT )
			this->Remove();
	}
	else if( type == 1 || type == 3 )
	{
		if( y > 768 )
			this->Remove();
	}

	RECT rect = { x + 10, y + 10, x + (ENEMYWIDTH - 15), y + (ENEMYHEIGHT - 15)};

	if( FriendlyArray[FIGHTER]->CheckHit(&rect) )
		FriendlyArray[FIGHTER]->Explode();
}

void Enemy3::Remove()
{
	int index = this->identifier;

	Enemy3Count--;

	if( Enemy3Count == 0 ) // advance to the next stage in the game
	{		
		SetTimer(DDraw->GetWindowHandle(), GAMECLOCK, GAMESPEED, NULL);
		if( KillCount >= 4 )
		{			
			int i = FirstEmptyInEnemyArray();
			EnemyArray[i] = new Pulse(x, y);
			EnemyArray[i]->identifier = i;
			game->EnemyCount++;
		}
		KillCount = 0;
	}

	DDraw->ReleaseSurface(this->surface);
	game->EnemyCount--;

	EnemyArray[index] = NULL;
	delete EnemyArray[index];

	Explosion();
}

//***********************************************************

int Enemy4::Enemy4Count = 0;

Enemy4::Enemy4()
{
	if (Enemy4Count == 0)
	{
		x = 100; 
		y = 0;
	}
	else if(Enemy4Count == 1)
	{
		x = 200; 
		y = 768;
	}
	else if(Enemy4Count == 2)
	{
		x = 300; 
		y = 0;
	}
	else if(Enemy4Count == 3)
	{
		x = 400;
		y = 768;
	}
	else if(Enemy4Count == 4)
	{
		x = 500;
		y = 0;
	}
	else if(Enemy4Count == 5)
	{
		x = 600;
		y = 768;
	}
	else if(Enemy4Count == 6)
	{
		x = 700;
		y = 0;
	}
	else if(Enemy4Count == 7)
	{
		x = 800;
		y = 768;
	}
	else if(Enemy4Count == 8)
	{
		x = 900;
		y = 0;
	}
	else if(Enemy4Count == 9)
	{
		x = 1000;
		y = 768;
	}
	
	this->type = Enemy4Count;
	this->hits = 30;
	this->height = ENEMYHEIGHT;
	this->width = ENEMYWIDTH;
	Enemy4Count++;

	this->surface = DDraw->CreateImage(ENEMYWIDTH, ENEMYHEIGHT, 0);

	if( !DDraw->LoadBitmap(this->surface, "data/enemy4.bmp") )
		DDraw->ErrorMessage("couldn't create enemy2");

	DDraw->SetColorKey(0, DDraw->GetImage(this->surface));	
	game->EnemyCount++;
}

void Enemy4::Move()
{
	if( type == 0 || type == 2 || type == 4 || type == 6 || type == 8 )
		y+=2;
	else
		y-=2;

	DDraw->DrawImage(x, y, this->surface, DDBLT_KEYSRC, NULL);

	RECT rect = { x + 5, y + 5, x + (ENEMYWIDTH - 5), y + (ENEMYHEIGHT - 5) };

	if( FriendlyArray[FIGHTER]->CheckHit(&rect) )
	{
		this->Remove();
		FriendlyArray[FIGHTER]->Explode();
		return;
	}

	if( type == 0 || type == 2 || type == 4 || type == 6 || type == 8 )
	{
		if( y >= 768 )
			this->Remove();
	}
	else
	{
		if( y <= 0 )
			this->Remove();
	}
}

void Enemy4::Remove()
{
	int index = this->identifier;	

	Enemy4Count--;

	if( Enemy4Count == 0 ) // advance to the next stage in the game
	{
		KillCount = 0;
		SetTimer(DDraw->GetWindowHandle(), GAMECLOCK, GAMESPEED, NULL);		
	}

	DDraw->ReleaseSurface(this->surface);
	game->EnemyCount--;

	EnemyArray[index] = NULL;
	delete EnemyArray[index];	

	Explosion();
}

//*******************************************************

int Enemy5::Enemy5Count = 0;
Animation * Enemy5::surfaces = NULL;

Enemy5::Enemy5(int x, int y)
{
	this->x = x, this->y = y;
	this->type = Enemy5Count;
	hits = 3, which = 0;
	speed = 5;
	width = ENEMY5WIDTH, height = ENEMY5HEIGHT;
	
	if( surfaces == NULL )
	{
		std::string file = "data/enemy5.bmp";
		surfaces = new Animation(0, 0, ENEMY5WIDTH, ENEMY5HEIGHT, 2, file, DDraw);
		DDraw->SetColorKey(0, surfaces->GetFrame(0));
		DDraw->SetColorKey(0, surfaces->GetFrame(1));
	}

	game->EnemyCount++;
	Enemy5Count++;
}

void Enemy5::Move()
{
	y+=2;
	if( which <= 5 )
	{
		if( type & 1 )
			DDraw->DrawImage(x, y, surfaces->GetFrame(0), DDBLT_KEYSRC, NULL);
		else
		{
			DDraw->DrawMirrorImage(x, y, surfaces->GetFrame(0), DDBLT_KEYSRC);
		}
	}
	else
	{
		if( type & 1 )
			DDraw->DrawImage(x, y, surfaces->GetFrame(1), DDBLT_KEYSRC, NULL);
		else
		{
			DDraw->DrawMirrorImage(x, y, surfaces->GetFrame(1), DDBLT_KEYSRC);
		}
	}

	RECT rect = { x + 5, y + 5, x + (ENEMY5WIDTH - 5), y + (ENEMY5HEIGHT - 5) };

	if( FriendlyArray[FIGHTER]->CheckHit(&rect) )
	{
		this->Remove();
		FriendlyArray[FIGHTER]->Explode();		
		return;
	}

	which++;
	if( which > 10 ) which = 0;

	if( type & 1 )
	{
		y+=speed;
		x++;
		if( y > 768 ) this->Remove();
	}
	else
	{
		y-=speed;
		x++;

		if( y < 0 ) this->Remove();
	}
}

void Enemy5::Remove()
{
	int index = this->identifier;	

	Enemy5Count--;

	game->EnemyCount--;

	delete EnemyArray[index];	
	EnemyArray[index] = NULL;

	Explosion();
}

//*******************************************************

int Enemy6::Enemy6Count = 0;
Animation * Enemy6::surfaces = NULL;

Enemy6::Enemy6(int x, int y)
{
	this->x = x, this->y = y;
	this->width = ENEMY6WIDTH, this->height = ENEMY6HEIGHT;
	speed = 6;
	direction = NORTH;
	game->EnemyCount++;
	this->type = Enemy6Count;
	Enemy6Count++;
	hits = 3, which = 0;

	if( surfaces == NULL )
	{
		std::string file = "data/enemy6.bmp";
		surfaces = new Animation(0, 0, this->width, this->height, 3, file, DDraw);

		for( int i = 0; i < 3; i++ )
			DDraw->SetColorKey(0, surfaces->GetFrame(i));
	}
}

void Enemy6::Move()
{
	if( direction == NORTH )
	{
		if( type & 1 )
			y -= speed, x -= speed;
		else
			y -= speed, x += speed;

		if( y < 0 )
			direction = SOUTH;
	}
	else
	{
		if( type & 1 )
			y += speed, x += speed;
		else
			y += speed, x -= speed;

		if( y > 768 )
		{
			Remove();
			return;
		}
	}

	if( which >= 0 && which < 5 )
		DDraw->DrawImage(x, y, surfaces->GetFrame(0), DDBLT_KEYSRC, NULL);
	else if( which >= 5 && which < 10 )
		DDraw->DrawImage(x, y, surfaces->GetFrame(1), DDBLT_KEYSRC, NULL);
	else
		DDraw->DrawImage(x, y, surfaces->GetFrame(2), DDBLT_KEYSRC, NULL);

	which++;
	if( which > 15 )
		which = 0;

	RECT rect = { x, y + 5, x + ENEMY6WIDTH, y + ENEMY6HEIGHT };

	if( FriendlyArray[FIGHTER]->CheckHit(&rect) )
		FriendlyArray[FIGHTER]->Explode();
}

void Enemy6::Remove()
{
	int index = this->identifier;	

	Enemy6Count--;

	if( Enemy6Count == 0 ) // advance to the next stage in the game
	{
		KillCount = 0;
		SetTimer(DDraw->GetWindowHandle(), GAMECLOCK, GAMESPEED, NULL);
	}	

	game->EnemyCount--;
	
	delete EnemyArray[index];	
	EnemyArray[index] = NULL;

	EnemyArray[index] = new Pulse(x, y); // a pulse is not really an enemy, but you need to be able to pick it up
	EnemyArray[index]->identifier = index;
	game->EnemyCount++;

	Explosion();
}

//**************************************************************************

int Enemy7::Enemy7Count = 0;

Enemy7::Enemy7(int x, int y)
{
	this->x = x, this->y = y;
	speed = 4, hits = 2;
	width = ENEMY7WIDTH, height = ENEMY7HEIGHT;
	
	surface = DDraw->CreateImage(ENEMY7WIDTH, ENEMY7HEIGHT, 0);

	std::string file = "data/enemy7.bmp";
	DDraw->LoadBitmap(surface, (char *)file.c_str());
	DDraw->SetColorKey(0, surface);

	Enemy7Count++;
	game->EnemyCount++;
}

void Enemy7::Move()
{
	y += speed;

	DDraw->DrawImage(x, y, surface, DDBLT_KEYSRC, NULL);
	
	if( y > 100 && y <= 104 )
		Shoot();
	else if( y > 300 && y <= 300 + speed )
		Shoot();
	else if( y > 500 && y <= 500 + speed )
		Shoot();

	if( y > 768 )
		Remove();
}

void Enemy7::Shoot()
{
	if( game->FriendlyCount >= MAXELEMENTS - 4 ) // rockets cannot be fired
		return;

	int index;

	for( int i = 0; i < 3; i++ )
	{
		index = FirstEmptyInFriendlyArray(); // a rocket belongs in the friendly array

		if( i == 0 )	
			FriendlyArray[index] = new Bullet(x, y, x - 250, y + 250); // and not in the enemy
		else if( i == 1 )
			FriendlyArray[index] = new Bullet(x, y, x, y + 250); 
		else
			FriendlyArray[index] = new Bullet(x, y, x + 250, y + 250);

		FriendlyArray[index]->identifier = index;
		game->FriendlyCount++;		
	}
}

void Enemy7::Remove()
{
	DDraw->ReleaseSurface(surface);

	int index = this->identifier;

	delete EnemyArray[index];
	EnemyArray[index] = NULL;

	game->EnemyCount--;
	Enemy7Count--;

	if( Enemy7Count == 0 )
	{
		KillCount = 0;
		SetTimer(DDraw->GetWindowHandle(), GAMECLOCK, GAMESPEED, NULL);
	}

	Explosion();
}

//**************************************************************************

int Meteor::MeteorCount = 0;

Meteor::Meteor()
{
	int i;

	x = rand() % 1024;
	y = 0 + rand() % 50;
	this->width = ENEMYWIDTH;
	this->height = ENEMYHEIGHT;
	this->hits = 5;
	this->which = 0;
	MeteorCount++;

	for( i = 0; i < 3; i++ )
		this->surface[i] = DDraw->CreateImage(ENEMYWIDTH, ENEMYHEIGHT, 0);

	if( !DDraw->LoadBitmap(surface[0], "data/meteor1.bmp") )
		DDraw->ErrorMessage("couldn't load meteor1");

	if( !DDraw->LoadBitmap(surface[1], "data/meteor2.bmp") )
		DDraw->ErrorMessage("couldn't load meteor2");

	if( !DDraw->LoadBitmap(surface[2], "data/meteor3.bmp") )
		DDraw->ErrorMessage("couldn't load meteor3");

	for( i = 0; i < 3; i++ )
		DDraw->SetColorKey(DDraw->Convert16Bit(0, 0, 0), surface[i]);

	game->EnemyCount++;
}

void Meteor::Move()
{
	y+=2;
	
	if( which >= 0 && which <= 10 )
		DDraw->DrawImage(x, y, surface[0], DDBLT_KEYSRC, NULL);	
	else if( which > 10 && which <= 15 )
		DDraw->DrawImage(x, y, surface[1], DDBLT_KEYSRC, NULL);	
	else if( which > 15 && which <= 25 )
		DDraw->DrawImage(x, y, surface[2], DDBLT_KEYSRC, NULL);	

	which++;

	if( which > 25 )
		which = 0;

	if( y >= 768 )
		this->Remove();

	RECT rect = { x + 5, y + 5, x + (ENEMYWIDTH - 5), y + (ENEMYHEIGHT - 5) };

	if( FriendlyArray[FIGHTER]->CheckHit(&rect) )
	{
		FriendlyArray[FIGHTER]->Explode();
		this->Remove();
	}
}

void Meteor::Remove()
{
	int index = this->identifier;

	MeteorCount--;

	if( MeteorCount <= 2 )
		SetTimer(DDraw->GetWindowHandle(), GAMECLOCK, GAMESPEED, NULL);			

	for( int i = 0; i < 3; i++ )
		DDraw->ReleaseSurface(surface[i]);

	game->EnemyCount--;

	EnemyArray[index] = NULL;
	delete EnemyArray[index];

	Explosion();
}

//***************************************************************

Explosion1::Explosion1(int x, int y)
{
	this->x = x, this->y = y, which = 0;

	surface = new int[FRAMECOUNT]; // 6 frames so far
	int i;

	int totalpic = DDraw->CreateImage(FRAMECOUNT * 55, 54, 0);

	if( !DDraw->LoadBitmap(totalpic, "data/kaboom.bmp") )
		DDraw->ErrorMessage("Error in loadbitmap");

	for( i = 0; i < FRAMECOUNT; i++ )
	{
		surface[i] = DDraw->CreateImage(55, 54, 0);
		DDraw->SetColorKey(0, surface[i]);
	}

	for( i = 0; i < FRAMECOUNT; i++ )
		DDraw->CopyBitmap(surface[i], totalpic, i * 55, 0, 55, 54);
	
	DDraw->ReleaseSurface(totalpic);
}

void Explosion1::Move()
{
	if( which >= 0 && which < 5 )
		DDraw->DrawImage(x, y, surface[0], DDBLT_KEYSRC, NULL);
	else if( which >= 5 && which < 10 )
		DDraw->DrawImage(x, y, surface[1], DDBLT_KEYSRC, NULL);
	else if( which >= 10 && which < 15 )
		DDraw->DrawImage(x, y, surface[2], DDBLT_KEYSRC, NULL);
	else if( which >= 15 && which < 20 )
		DDraw->DrawImage(x, y, surface[3], DDBLT_KEYSRC, NULL);
	else if( which >= 20 && which < 25 )
		DDraw->DrawImage(x, y, surface[4], DDBLT_KEYSRC, NULL);
	else
		DDraw->DrawImage(x, y, surface[5], DDBLT_KEYSRC, NULL);

	which++;
	if( which == 30 )
		Remove();
}

void Explosion1::Remove()
{
	int index = this->identifier;

	for( int i = 0; i < FRAMECOUNT; i++ )
		DDraw->ReleaseSurface(surface[i]);

	delete[] surface;

	FriendlyArray[index] = NULL;
	delete FriendlyArray[index];
	game->FriendlyCount--;
}

//*******************************************************************

Cloud::Cloud(int y, bool lefttoright)
{
	this->y = y, LeftToRight = lefttoright;

	if( lefttoright )
		this->x = 0;
	else
		this->x = 930;

	surface = DDraw->CreateImage(CLOUDWIDTH, CLOUDHEIGHT, 0);

	char file[40];

	if( rand() & 1 )
		strcpy(file, "data/cloud.bmp");
	else
		strcpy(file, "data/cloud2.bmp");

	if( !DDraw->LoadBitmap(surface, file) )
		DDraw->ErrorMessage("couldn't load cloud");

	DDraw->SetColorKey(DDraw->Convert16Bit(0, 0, 0), surface);

	game->NeutralCount++;
}

void Cloud::Move()
{
	if( LeftToRight )
	{
		x++;
		if( x > 925 )
			this->Remove();
	}
	else
	{
		x--;
		if( x < 1 )
			this->Remove();
	}

	DDraw->DrawImage(x, y, surface, DDBLT_KEYSRC, NULL);
}

void Cloud::Remove()
{
	int index = this->identifier;

	DDraw->ReleaseSurface(surface);

	game->NeutralArray[index] = NULL;
	delete game->NeutralArray[index];
	game->NeutralCount--;
}

//***********************************************************************

int Tank::tankspassed = 0;

Tank::Tank(int x, int y)
{
	this->x = x, this->y = y;
	this->width = TANKWIDTH, this->height = TANKHEIGHT;
	hits = 2;

	tankspassed++;
	game->EnemyCount++;

	surface = DDraw->CreateImage(TANKWIDTH, TANKHEIGHT, 0, "data/tank.bmp");
	DDraw->SetColorKey(0, surface);

	if( tankspassed == 20 )
	{
		tankspassed = 0;
		KillTimer(DDraw->GetWindowHandle(), TANKINTERRUPT);
	}
}

void Tank::Move()
{
	y++;

	DDraw->DrawImage(x, y, surface, DDBLT_KEYSRC, NULL);

	if( y % 50 == 0 )
		Shoot();

	if( y > 768 )
		Remove();
}

void Tank::Remove()
{
	DDraw->ReleaseSurface(surface);

	int index = this->identifier;
	delete EnemyArray[index];
	EnemyArray[index] = NULL;
	game->EnemyCount--;

	Explosion();
}

void Tank::Shoot()
{
	if( game->FriendlyCount >= MAXELEMENTS - 1 ) // bullet cannot be fired
		return;

	int index = FirstEmptyInFriendlyArray(); // a rocket belongs in the friendly array

	Fighter *f = (Fighter *)FriendlyArray[FIGHTER];

	FriendlyArray[index] = new Bullet(x+25, y+25, f->x, f->y); // and not in the enemy
	FriendlyArray[index]->identifier = index;

	game->FriendlyCount++;	
}

//***********************************************************************
Pause::Pause(int speed) 
: speed(speed)
{
	this->x = 0;
}

void Pause::Move()
{
	x += speed;

	if( x > 1024 )
		Remove();
}

void Pause::Remove()
{
	SetTimer(DDraw->GetWindowHandle(), GAMECLOCK, GAMESPEED, NULL);

	int index = this->identifier;
	delete EnemyArray[index];
	EnemyArray[index] = NULL;
}