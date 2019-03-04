#include<iostream>
#include<string>
#include<memory>
#include<algorithm>

using namespace std;


int matrix_top_bottom_impl(int **arr, int i, int j, int n)
{
    if(i >= n || j >= n)
        return 0;
    if(arr[i][j] == 1)
        return 1;
    if(i == n-1 && j == n-1)
    {
        arr[i][j] = 1;
        return 1;
    }
    int right = matrix_top_bottom_impl(arr, i, j+1, n);
    int down =  matrix_top_bottom_impl(arr, i+1, j, n);

    return down + right;
}

// This program computes the ways we can reach from top->bottom by going only right or down ways
void matrix_top_bottom()
{
    int n;
    cout<<"Enter dimension of matrix\n";
    cin>>n;
    int **arr = (int**)(malloc(n*sizeof(int*)));
    for(int i = 0; i < n ; ++i)
        arr[i] = (int*)malloc(n*sizeof(int));

    // initialize
    for(int i =0; i < n ; ++i)
        for(int j = 0; j < n; ++j)
            arr[i][j] = 0;
    
    int res = matrix_top_bottom_impl(arr, 0, 0, n);
    cout<<"\nNumber of ways you can go to end via right or down is: "<<res<<endl;
}

int cal_num_of_tiles_impl(int n)
{
    if(n <= 0)
        return 0;
    else if(n == 1)
        return 1;
    else if(n == 2)
        return 2;
    else
    {
        return cal_num_of_tiles_impl(n-1) + cal_num_of_tiles_impl(n-2);
    }
}

int ways_to_score_with_3_5_10_impl(int n)
{
    if(n < 3)
        return 0;
    else if(n == 3 || n == 5)
        return 1;
    else if(n == 10)
        return 2;
    else
    {
        return ways_to_score_with_3_5_10_impl(n-3) + 
                ways_to_score_with_3_5_10_impl(n-5) + 
                 ways_to_score_with_3_5_10_impl(n-10);
    }
    
}

void edit_distance_impl(string s1, string s2, int **arr)
{
    int m = s1.size();  // source
    int n = s2.size();  // target to be changed(will be first row (2nd dimension))

    // fill first row
    if(s1[0] == s2[0])
        arr[0][0] = 0;
    else
    {
        arr[0][0] = 1;
    }
        
    for(int i = 1; i < n; ++i)
    {
        arr[0][i] = i; //arr[0][i-1] + 1;
    }

    // fill 1st column
    for(int j = 1; j < m; ++j)
    {
        arr[j][0] = j;//arr[j-1][0] + 1;
    }

    // main loop
    for(int i = 1; i < m ; ++i)
    {
        for(int j = 1; j < n ; ++j)
        {
            int min_ = std::min(min(arr[i-1][j], arr[i][j-1]), arr[i-1][j-1]);
            if(s2[j] == s1[i])
            {
                arr[i][j] = arr[i-1][j-1];
            }
            else
            {
                arr[i][j] = min_ + 1;
            }
        }
    }


}

void edit_distance()
{
    string s1, s2;
    cout<<"enter the source and target string in sequence\n";
    cin>>s1>>s2;
    int sz_1 = s1.size(), sz_2 = s2.size();
    int **ary = new int*[sz_1];
    for(int i = 0; i < sz_1; ++i) {
        ary[i] = new int[sz_2];
    }

    edit_distance_impl(s1, s2, ary);
    cout<<"minimum replacement required are: "<<ary[s1.size()-1][s2.size()-1]<<endl;
}

void ways_to_score_with_3_5_10()
{
    int n = 0;
    cout<<"Enter the total score:\n";
    cin>>n;
    int res = ways_to_score_with_3_5_10_impl(n);
    cout<<"Number of ways to score "<<n<<" with coins of 3, 5 and 10 is: "<<res<<endl;
}

void string_interleaving_impl(string s1, string s2, string res, int i, int j)
{
    string temp = res;
    if((i + j) == (s1.size() + s2.size()))
    {
        cout<<temp<<endl;
        return;
    }
    
    if(i < s1.size())
    {
        string_interleaving_impl(s1, s2, res + s1[i], i+1, j);
    }
    if(j < s2.size())
    {
        string_interleaving_impl(s1, s2, res + s2[j], i, j+1);
    }
}

void string_interleaving()
{
    string str1 = "", str2 = "";
    cout<<"Enter the 2 strings: \n";
    cin>>str1>>str2;
    string_interleaving_impl(str1, str2, "", 0, 0);
}

// program to count number of tiles
void cal_num_of_tiles()
{
    // this program will calculate the number of ways we can place tiles of size 2*1 in plot of 2*N
    int n;
    cout<<"Enter value of N\n";
    cin>>n;
    int res = cal_num_of_tiles_impl(n);
    cout<<"Number of ways to place tiles: "<<res<<endl;
}

int main()
{
    // matrix_top_bottom();
    // cal_num_of_tiles();
    // ways_to_score_with_3_5_10();
    // edit_distance();
    string_interleaving();
    return 0;
}