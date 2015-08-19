#include <bits/stdc++.h>

template <int n>
class print{
public:
print(){
std::cout<<" "<<n<<" "<<std::endl;
print<n+1> p;
}
};

template <>
class print<100>{
public:
print(){
std::cout<<"100"<<std::endl;
}
};

int main(){
	print<1> p;
}

