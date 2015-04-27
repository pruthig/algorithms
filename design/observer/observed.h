#ifndef SHOP_H
#define SHOP_H

#include<string>

class shop{
std::string name;
int price;
public:
shop(std::string s, int p);
shop();
shop(const shop&);
void notify(int c);
void changePrice(int p);
};

#endif
