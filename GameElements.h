#ifndef __GAMEELEMENT_H_
#define __GAMEELEMENT_H_

#include <easydraw.h>

typedef class GameElement
{
public :
	static int KillCount;
	enum MAXELEMENTS {MAXELEMENTS = 30, MAXENEMIES = 40, MAXNEUTRALS = 10};
	enum STATE {DEAD, ALIVE, EXPLODING, SHOOTING, FARAWAY};

	int width;
	int height;
	int x; // place on the screen
	int y;
	int direction;
	U32 hits;	
	int identifier; // the index in EnemyArray[]
	int state;
	
	void ShootRocket();
	void Explosion();

	virtual bool __fastcall CheckHit(RECT *rect) { return true; }	
	virtual void Explode() {};
	virtual void Move(int direction) {};
	virtual void Move() {};
	virtual void Shoot() {};
	virtual void Remove() {};
} GameElement, *GameElementptr, **GameElementList;

typedef class Boss1 : public GameElement
{
public :
	int laser;
	int step;
	int which;
	int surface[2];

	Boss1();	
	void Move();
	void ShootLaser();
	//void ShootRocket();
	void Remove();
} Boss1, *Boss1ptr, **Boss1List;

typedef class Boss2 : public GameElement
{
public:
	int step;
	ImageID surface;

	Boss2();
	void Move();
	void Remove();
	void Shoot();
} Boss2, *Boss2ptr, **Boss2List;

typedef class Fighter : public GameElement
{
public :
	enum {FIGHTERSPEED = 5};

	int hits;	
	Animation *surface; // 3 surfaces for animation
	int which; // determines which picture to draw

	Fighter();
	bool __fastcall CheckHit(RECT *rect);
	void Move(int direction);
	void Explode();
	void Shoot();
} Fighter, *Fighterptr, **FighterList;

typedef class Bullet : public GameElement
{
public:
	static ImageID surface;	

	Bullet(int x, int y, int x_heading, int y_heading);
	void Move();
	void Remove();

private:
	int speed;
	int type;
	int x_heading;
	int y_heading;

} Bullet, *Bulletptr, **BulletList;

typedef class Rocket : public GameElement
{
public :
	static ImageID surface;
	int x_heading;
	int y_heading;	

	Rocket(int x, int y);
	void Move();
	void Explode();
	void Remove();
} Rocket, *Rocketptr, **RocketList;

typedef class FastRocket : public GameElement
{
public:
	static Animation *surfaces;
	static int speed;
	int step;

	FastRocket(int x, int y, int direction);
	void Move();
	void Explode();
	void Remove();
} FastRocket, *FastRocketptr, **FastRocketList;

typedef class FighterRocket : public GameElement
{
public :
	static ImageID surface;
	int InitialX;
	int InitialY;

	FighterRocket(int x, int y);
	bool FC CheckHit(int &index);
	void Move();
	void Remove();
} FighterRocket, *FighterRocketptr, **FighterRocketList;

typedef class Enemy1 : public GameElement
{
public :
	static int Enemy1Count;
	ImageID surface;

	U32 type;

	Enemy1();

	void Move();
	void Remove();
} Enemy1, *Enemy1ptr, **Enemy1List;

typedef class Enemy2 : public GameElement
{
public :
	static int Enemy2Count;
	ImageID surface;

	U32 type;

	Enemy2();

	void Move();
	void Remove();
} Enemy2, *Enemy2ptr, **Enemy2List;

typedef class Enemy3 : public GameElement
{
public :
	static int Enemy3Count;
	ImageID surface;

	U32 type;

	Enemy3();

	void Move();
	void Remove();
} Enemy3, *Enemy3ptr, **Enemy3List;

/* the mines that will be flying over the screen */
typedef class Enemy4 : public GameElement
{
public :
	static int Enemy4Count;
	ImageID surface;

	U32 type;

	Enemy4();
	void Move();
	void Remove();
} Enemy4, *Enemy4ptr, **Enemy4List;

typedef class Enemy5 : public GameElement
{
public:
	static int Enemy5Count;
	static Animation *surfaces;

	U32 type;	
	int which;
	int speed;

	Enemy5(int x, int y);

	void Move();
	void Remove();
} Enemy5, *Enemy5ptr, **Enemy5List;

typedef class Enemy6 : public GameElement
{
public:
	static int Enemy6Count;
	static Animation *surfaces;

	U32 type;	
	int which;
	int speed;

	Enemy6(int x, int y);

	void Move();
	void Remove();
} Enemy6, *Enemy6ptr, **Enemy6List;

typedef class Enemy7 : public GameElement
{
public:
	static int Enemy7Count;
	ImageID surface;	

	Enemy7(int x, int y);

	void Move();
	void Shoot();
	void Remove();

private:
	int speed;
} Enemy7, *Enemy7ptr, **Enemy7List;

typedef class Meteor : public GameElement
{
public:
	static int MeteorCount;

	int surface[3]; // 3 frames for animation
	U32 type;
	int which;

	Meteor();
	void Move();
	void Remove();
} Meteor, *Metoerptr, **MeteorList;

typedef class Explosion1 : public GameElement
{
public:
	enum { FRAMECOUNT = 6 };
	int *surface; // 6 frames for animation
	int which;

	Explosion1(int x, int y);
	void Move();
	void Remove();
}Explosion1, *Explosion1ptr, **Explosion1List;

typedef class Cloud : public GameElement
{
public:
	bool LeftToRight;
	ImageID surface;

	Cloud(int y, bool lefttoright);
	void Move();
	void Remove();
} Cloud, *Cloudptr, **CloudList;

typedef class Tank : public GameElement
{
public:
	static int tankspassed;

	Tank(int x, int y);

	void Move();
	void Remove();
	void Shoot();
private:
	int surface;
} Tank, *Tankptr, **TankList;

typedef class Pause : public GameElement
{
public:	
	Pause(int speed);
	void Move();
	void Remove();

private:
	int speed;
} Pause, *Pauseptr, **PauseList;

#endif