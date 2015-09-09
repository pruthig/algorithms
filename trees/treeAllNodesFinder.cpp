//All nodes finder will find all the nodes having right and left childern
//It uses level order traversal to do the same
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


int allNodeFinder(struct treeStruct *ptr)
{
	queue<treeStruct *> q;
	treeStruct *tmp;

	if(ptr == NULL)
		cout<<"it is null";
	else
		tmp = ptr;


	q.push(tmp);
	int count = 0;	

	while(q.size())
	{
		tmp = q.front();
		q.pop();
		if(tmp->left != NULL && tmp->right != NULL)
			count++;
		if(tmp->left)
			q.push(tmp->left);
		if(tmp->right)
			q.push(tmp->right);
 

		
	}
	return count;
}

int main()
{
	struct treeStruct *rootPtr = treeCreator();
	int count = allNodeFinder(rootPtr);
	cout<<"Count of full nodes "<<count<<endl;
	return 0;
}



