#ifndef __POWERUP_H_
#define __POWERUP_H_

#include "gameelements.h"

typedef class Pulse : public GameElement
{
public :
	int surface[2];
	int which; // determines which picture to draw
	int startY;

	Pulse(int x, int y);

	void Move();
	void Remove();
} Pulse, *Pulseptr, **PulseList;

//*******************************************************

typedef class FloatText : public GameElement
{
public:
	std::string text;
	int startY;

	FloatText(int x, int y, std::string &text);
	void Move();
	void Remove();
}FloatText, *FloatTextptr, **FloatTextList;

//*******************************************************

#endif