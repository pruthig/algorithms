//the program finds the level of a give node in the tree, if not is not there it informs accordingly

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

int nodeLevel(struct treeStruct *ptr, int nodeToFind)
{
	int width = 0, local = 0;
	int level = 0;
	if(ptr->element == nodeToFind){
		return 1;
	}
	q.push(ptr);
	while(!q.empty())
	{
		++level;
		local = q.size();
		if(local > width)
			width = local;
		
		for(int i=0; i <= local - 1; i++)
		{
			struct treeStruct *dummy = q.front();
			if(dummy->element == nodeToFind)
				return level;
			q.pop();
			
			if(dummy->left != NULL)
				q.push(dummy->left);
			if(dummy->right != NULL)
				q.push(dummy->right);
		}
	}
			
	return 0;
}

int main(){
int node;
struct treeStruct *rootPtr = treeCreator();
cout<<"Enter the node of which you wanna find the level\n";
cin>>node;
int count = nodeLevel(rootPtr, node);
if(!count)
	cout<<"Node not found\n";
else
	cout<<"Max Width of tree : "<<count<<endl;
return 0;
}



