//the program counts the node at a given level in a tree using Q data structure

#include<iostream>
#include<queue>


using namespace std;

typedef struct treeStruct{
int element;
struct treeStruct *left;
struct treeStruct *right;
}treeStruct;

queue<treeStruct*> q;

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
mainPtr->left->right = newNode(9);

mainPtr->right->left = newNode(11);
mainPtr->right->right = newNode(18);

mainPtr->right->right->left = newNode(15);
mainPtr->right->right->right = newNode(20);

return mainPtr;
}

int maxNodeLevel(struct treeStruct *ptr)
{
	int width = 0, local = 0;
	q.push(ptr);
	while(!q.empty())
	{
		local = q.size();
		if(local > width)
			width = local;
		
		for(int i=0; i <= local - 1; i++)
		{
			struct treeStruct *dummy = q.front();
			q.pop();
			if(dummy->left != NULL)
				q.push(dummy->left);
			if(dummy->right != NULL)
				q.push(dummy->right);
		}
	}
			
	return width;
}

int main(){
struct treeStruct *rootPtr = treeCreator();
int count = maxNodeLevel(rootPtr);
cout<<"Max Width of tree : "<<count<<endl;
return 0;
}



