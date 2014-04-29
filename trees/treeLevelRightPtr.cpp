//this program connects the nodes in the same level with the another node at
//same level directn being left->right

#include<iostream>
#include<queue>


using namespace std;

typedef struct treeStruct{
int element;
struct treeStruct *left;
struct treeStruct *right;
struct treeStruct *ctor;   //connector
}treeStruct;

queue<treeStruct*> q;

struct treeStruct* newNode(int data)
{
struct treeStruct *newElement = new(struct treeStruct);
newElement->left = NULL;
newElement->right = NULL;
newElement->ctor = NULL;
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

void nodeConnector(struct treeStruct *ptr)
{
	int local = 0;

	q.push(ptr);
	while(!q.empty())
	{
		local = q.size();
			
		for(int i=0; i <= local - 1; i++)
		{
			struct treeStruct *dummy = q.front();
			q.pop();

			if(!q.empty())
			{
				//Attaching both
		    	struct treeStruct *nxt = q.front();
				dummy->ctor = nxt;
			}
			if(dummy->left != NULL)
				q.push(dummy->left);
			if(dummy->right != NULL)
				q.push(dummy->right);
		}
	}
			
}

int main(){
int node;
struct treeStruct *rootPtr = treeCreator();
cout<<"Connecting the nodes with right pointer\n";
nodeConnector(rootPtr);
return 0;
}



