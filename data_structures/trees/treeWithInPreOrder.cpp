

#include<iostream>



using namespace std;

typedef struct treeStruct{
char element;
struct treeStruct *left;
struct treeStruct *right;
}treeStruct;

int search(char *input, char p);
void print(struct treeStruct *d);

struct treeStruct* newNode(char data)
{
struct treeStruct *newElement = new(struct treeStruct);
newElement->left = NULL;
newElement->right = NULL;
newElement->element = data;
return newElement;
}

//IN = D B E A F C
//PR = A B D E C F
struct treeStruct*  buildTree(char *i, char *p, int s, int e){
if(s > e){
return NULL;
}


static int preIndex = 0;
char a = *(p+preIndex);
int iIndex = search(i, a);
++preIndex;

struct treeStruct *n = newNode(a);

if(s == e){
return newNode( *(i + s));
}
n->left = buildTree(i, p, s, iIndex-1 );
n->right = buildTree(i, p, iIndex+1, e);
return n;
}

int main(){
char *in = "DBEAFC";
char *pre = "ABDEFC";

struct treeStruct *root = buildTree(in, pre, 0, 5);
print(root);
return 0;
}

int search(char *input, char p){
for(int i=0;i<=5;i++)
	if( *(input+i) == p)
		return i;
}

void print(struct treeStruct *d){
if(d == NULL)
	return;

print(d->left);
cout<<d->element<<", ";
print(d->right);
}




