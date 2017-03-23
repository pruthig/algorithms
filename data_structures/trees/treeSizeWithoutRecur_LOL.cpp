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

int sizeWithoutRecur(struct treeStruct *ptr)
{
	queue<treeStruct *> q;
	int size = 0;

	if(ptr != NULL)
	{
		q.push(ptr);
		++size;
	}

  	else
		return 0;	

	while(!q.size() == 0)
	{
		struct treeStruct *tmp = q.front();
		q.pop();

		if(tmp->left) 
		{
			q.push(tmp->left);
			++size;
		}
		if(tmp->right)
		{
			q.push(tmp->right);
			++size;
		}
	}
	return size;
}

int main()
{
	struct treeStruct *rootPtr = treeCreator();
	int count = sizeWithoutRecur(rootPtr);
	cout<<"Number of  nodes in tree : "<<count<<endl;
	return 0;
}



