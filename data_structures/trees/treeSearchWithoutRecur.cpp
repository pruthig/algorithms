#include<iostream>
#include<queue>


using namespace std;

typedef struct treeStruct{
int element;
struct treeStruct *left;
struct treeStruct *right;
}treeStruct;


struct treeStruct* newNode(int data)
{
struct treeStruct *newElement = new(struct treeStruct);
newElement->left = NULL;
newElement->right = NULL;
newElement->element = data;
return newElement;
}

struct treeStruct* treeCreator(){
//create the main pointer
struct treeStruct *mainPtr;
mainPtr = newNode(10);
mainPtr->left = newNode(6);
mainPtr->right = newNode(12);

mainPtr->left->left = newNode(2);
mainPtr->left->right = newNode(10);

mainPtr->right->left = newNode(4);
mainPtr->right->right = newNode(18);

mainPtr->right->right->left = newNode(0);
mainPtr->right->right->right = newNode(20);

return mainPtr;
}

int counter = 0;

void searchWithoutRecur(struct treeStruct *ptr, int m)
{
	while(1)
	{
		if(ptr == NULL){
			cout<<"Not found\n";
			return;
		}
		else if(ptr->element == m){
			cout<<"Element found\n";
			return;
		}
		else if(m < ptr->element){
			ptr = ptr->left;
			continue;
		}
		else{
			ptr = ptr->right;
			continue;
		}
	}
}

int main()
{
	struct treeStruct *rootPtr = treeCreator();
	int count;
	cout<<"Enter to find\n";
	cin>>count;
	searchWithoutRecur(rootPtr, count);
	return 0;
}



