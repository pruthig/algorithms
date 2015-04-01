class laplace : public strategy{
public:
void call(){
cout<<"Laplace transform called\n";
}
};

class bezier : public strategy{
public:
void call(){
cout<<"Bezier curve algo called\n";
}
};

class client{
strategy st;
public:
strategy getStrategy(){
return st;
}
};

