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
mainPtr->left->right = newNode(9);

mainPtr->right->left = newNode(11);
mainPtr->right->right = newNode(18);

mainPtr->right->right->left = newNode(16);
mainPtr->right->right->right = newNode(20);

return mainPtr;
}


queue<treeStruct *> q;
treeStruct *temp;
void printDoubleTree(struct treeStruct *ptr)
{
	int size = 0, tmp = 0;

	if(ptr != NULL)
	{
		q.push(ptr);
		size++;
	}
	else
		return;
	while(1)
	{
		tmp = 0;
		while(size != 0)
		{
			ptr = q.front();
			q.pop();   size--;
			if(ptr == NULL)
				return;
		        cout<<ptr->element<<", ";		
			if(ptr->left != NULL)
				q.push(ptr->left); tmp++;	
			if(ptr->right != NULL)
				q.push(ptr->right); tmp++;
		
		}
		cout<<endl;
		size = tmp;
	}


}
void createDouble(treeStruct *ptr){
if(ptr == NULL)
	return;

createDouble(ptr->left);
temp = newNode(ptr->element);
temp->left = ptr->left;
ptr->left = temp;
createDouble(ptr->right);

return;
}


int main(){
treeStruct *rootPtr = treeCreator();
createDouble(rootPtr);
printDoubleTree(rootPtr);
return 0;
}



