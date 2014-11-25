#include<iostream>
#include<sstream>
#include<string>
#include<fstream>
using namespace std;

typedef struct treeStruct{
int element;
struct treeStruct *left;
struct treeStruct *right;
}treeStruct;


int g_Val = 0;
int k = 0;
void findMedian(treeStruct *node, int k);
int n = 0;
int result = 0;

struct treeStruct* insertNode(struct treeStruct *ptr, int element);

struct treeStruct *rootPtr = NULL;

struct treeStruct* newNode(int data)
{
struct treeStruct *newElement = new(struct treeStruct);
newElement->left = NULL;
newElement->right = NULL;
newElement->element = data;
return newElement;
}

void printSortedTree(struct treeStruct *ptr){
if(ptr == NULL)
	return;

printSortedTree(ptr->left);
cout<<ptr->element<<",";
printSortedTree(ptr->right);

}

struct treeStruct* insertNode(struct treeStruct *ptr, int element){
if(ptr == NULL)
{
	return newNode(element);
}
if(element < ptr->element)
	ptr->left  = insertNode(ptr->left, element);
else
	ptr->right  = insertNode(ptr->right, element);
return ptr;
}

int main(){


ifstream ifs;
ifs.open("/home/pruthi/Desktop/Median.txt");
int i = 0;
std::string str = "";
stringstream ss;
int index = 1;

while (std::getline(ifs, str)) //Read a str 
{

	stringstream ss(str);
	ss>>i;
	
	rootPtr = insertNode(rootPtr, i);
	if(index%2 == 0)
		k = (index)/2;
	else
		k = (index+1)/2;

	n = 0;
	result = 0;
	findMedian(rootPtr, k);
	//cout<<"Med value "<<med<<endl;
	g_Val = g_Val + result;
	++index;
}

cout<<"Sum is "<<g_Val<<endl;
return 0;
}


void findMedian(treeStruct *ptr, int x){

	if(ptr == NULL)
		return ;

	findMedian(ptr->left, x);
	n = n+1;
	if(n == x){
		result = ptr->element;
		return ;
	}

	findMedian(ptr->right, x);
}


