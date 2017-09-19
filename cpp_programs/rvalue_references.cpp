#include<utility>
#include<iostream>
class X{
};
void g(X&& t){} // A
void g(X& t){}      // B

template<typename T>
void f(T&& t)
{
    g(std::forward<T>(t));
}

void h(X&& t)
{
    g(t);
}

int main()
{
    X x;
    f(x);   // 1
    f(X()); // 2
    //h(x);   <-- this will give error without perfect forwarding
    h(X()); // 3
}
