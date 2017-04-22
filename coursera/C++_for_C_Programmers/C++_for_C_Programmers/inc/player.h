#ifndef _PLAYER_H
#define _PLAYER_H

#include "user.h"

class Hex;

class Player : public User{
public:
	void make_move(Hex& hex_game);
};

#endif