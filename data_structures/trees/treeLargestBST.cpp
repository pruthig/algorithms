//Base tree ... totally bare
#include<iostream>
#include<cstdint>
#include<climits>

using namespace std;

struct TreeNode
{
	int data;
	struct TreeNode *left;
	struct TreeNode *right;
};


TreeNode* new_node(int data)
{
	TreeNode *newdata = new TreeNode;
	newdata->left = NULL;
	newdata->right = NULL;
	newdata->data = data;
	return newdata;
}

struct TreeNode* treeCreator()
{
	//create the main pointer
	struct TreeNode *mainPtr;
	mainPtr = new_node(0);
	mainPtr->left = new_node(6);
	mainPtr->right = new_node(12);

	mainPtr->left->left = new_node(2);
	mainPtr->left->right = new_node(9);

	mainPtr->right->left = new_node(11);
	mainPtr->right->right = new_node(18);

	mainPtr->right->right->left = new_node(16);
	mainPtr->right->right->right = new_node(20);

	return mainPtr;
}
/*
                                        10
                                      /    \
                                    6       2
                                   / \     / \
                                  2   9   11  18
                                              / \
                                            16   20
*/

size_t findlargestBST(TreeNode *node, int *min_n, int *max_n, int *max_sz)
{

    if(node == nullptr)
        return 0;

    int lmin = INT_MAX, lmax = INT_MIN, rmin = INT_MAX, rmax = INT_MIN;
    int *lmin_p = &lmin, *lmax_p = &lmax, *rmin_p = &rmin, *rmax_p = &rmax;
    int lsz = findlargestBST(node->left, lmin_p, lmax_p, max_sz);
    int rsz = findlargestBST(node->right, rmin_p, rmax_p, max_sz);
//min_n = INT_MAX;
//max_n = INT_MIN
    *min_n = min(node->data, min(*rmin_p, *lmin_p));
    *max_n = max(node->data, max(*rmax_p, *lmax_p));

    cout<<"\nNode data is: "<<node->data<<" and min/max are: "<<*min_n<<" and "<<*max_n<<endl;
    if((node->left == nullptr && node->right == nullptr) || (node->data > *lmax_p && node->data < *rmin_p))
    {
        int t_sz = lsz + rsz + 1;
        cout<<"\nlmax_p and rmin_p :"<<*lmax_p<<" "<<*rmin_p<<endl;
        cout<<"Size calculated: "<<t_sz<<" with given node data; "<<node->data<<endl;
        *max_sz = max(*max_sz, t_sz);
        return t_sz;
    }
    else
    {
        return 0;
    }
}

void traverseNodes(struct TreeNode *ptr)
{
	if(ptr == NULL)
		return;
	traverseNodes(ptr->left);
	cout<<ptr->data<<" ";
	traverseNodes(ptr->right);
}

int main()
{
    int min_n = INT_MAX, max_n = INT_MIN, max_sz = 0;
	TreeNode *rootPtr = treeCreator();
	traverseNodes(rootPtr);
	findlargestBST(rootPtr, &min_n, &max_n, &max_sz);
	cout<<"Max size is: "<<max_sz<<endl;
	return 0;
}



