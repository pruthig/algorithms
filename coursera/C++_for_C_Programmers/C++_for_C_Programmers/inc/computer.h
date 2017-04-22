#ifndef _COMPUTER_H
#define _COMPUTER_H

#include "user.h"

class Hex;

class Computer : public User {
public:
	void make_move(Hex& hex_game);

};

#endif