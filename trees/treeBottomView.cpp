//program prints out the bottom view of the tree using LOT
#include<iostream>
#include<map>


using namespace std;

typedef struct treeStruct{
int element;
int grade;
struct treeStruct *left;
struct treeStruct *right;
}treeStruct;


struct treeStruct* newNode(int data)
{
struct treeStruct *newElement = new(struct treeStruct);
newElement->left = NULL;
newElement->right = NULL;
newElement->grade = 0;
newElement->element = data;
return newElement;
}

std::map<int, int> mapB;

struct treeStruct* treeCreator(){
//create the main pointer
struct treeStruct *mainPtr;
mainPtr = newNode(10);
mainPtr->left = newNode(6);
mainPtr->right = newNode(12);

mainPtr->left->left = newNode(2);
mainPtr->left->right = newNode(9);

mainPtr->right->left = newNode(11);
mainPtr->right->right = newNode(18);

mainPtr->right->right->left = newNode(15);
mainPtr->right->right->right = newNode(20);

return mainPtr;
}

int counter = 0;


void printBottomTree(){


	int p = 0;
	cout<<"Printing the map\n";

	for(map<int, int>::iterator itr = mapB.begin(); itr != mapB.end(); ++itr)
		cout<<itr->second<<"  "<<itr->first<<endl;
}
void printNodes(struct treeStruct *ptr,  int g){

if(ptr == NULL)
	return;


if(ptr != NULL){
	ptr->grade = g;
	map<int, int>::iterator itr;
	if((itr = mapB.find(ptr->grade)) != mapB.end())
		mapB.erase(itr);

	mapB.insert(std::pair<int, int>(ptr->grade ,ptr->element) );
}
printNodes(ptr->left, g-1);
printNodes(ptr->right, g+1);

}

int main(){
int lvl = 0;
struct treeStruct *rootPtr = treeCreator();
printNodes(rootPtr, 0);
printBottomTree();
return 0;
}



