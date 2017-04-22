#ifndef _USER_H
#define _USER_H

enum class Color;
class Hex;

class User{
	Color color;
public:
	virtual void make_move(Hex& hex_game)=0;
	void set_color(Color c);
	Color get_color();
};

#endif