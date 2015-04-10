//knuth-morris-pratt
//string searching made easy
#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<string>
#include<cstring>

using namespace std;

//Global temporary array..
int *p;
//string str = "";
//string pattern= "";





bool knuthMorrisPratt(string str, string pat){
int m;
int n = str.length();
int cur_index = 0;


while(true){

//Length of string to be matched is exceeding out of bounds
8 - cur_ < 3
if(n - cur_index < pat.length())
	break;

if( a[cur_index] == pat[0] ){
	//tmp is pointing to the cur_index in absolute
	int tmp = cur_index;
	m = 0;
	for( ; m < pat.length() && a[tmp] == pat[m]  ; ++m, ++tmp );
	if( m == pat.length() )
		return true;
	else if(m != 1){
		cur_index = cur_index + m - lps[m-1];
		continue;
	}

	//anyways we have to increment the current index when we dont have even a single match
	else
		++cur_index;
}
return false;
}

}//End of function
	
void printTemp(string s){
cout<<endl;
for(int i = 0; i <= s.size()-1; ++i)
	cout<<"["<<p[i]<<"]";
}


//LPS array taken from geeksforgeeks.org

void computeLPSArray(char *pat, int M, int *lps)
{
    int len = 0;  // lenght of the previous longest prefix suffix
    int i;
 
    lps[0] = 0; // lps[0] is always 0
    i = 1;
 
    // the loop calculates lps[i] for i = 1 to M-1
    while (i < M)
    {
       if (pat[i] == pat[len])
       {
         len++;
         lps[i] = len;
         i++;
       }
       else // (pat[i] != pat[len])
       {
         if (len != 0)
         {
           // This is tricky. Consider the example AAACAAAA and i = 7.
           len = lps[len-1];
 
           // Also, note that we do not increment i here
         }
         else // if (len == 0)
         {
           lps[i] = 0;
           i++;
         }
       }
    }
}


int main(){
string str; 
cout<<"Enter the main string"<<endl;
cin>>str;
cout<<endl;
cout<<"Now ener the pattern";
cin>>pat;

lps = new int[pat.size()];
memset(lps, 0, sizeof(pat)-1);
computeLPSArray(pat, pat.length(), lps);
bool find = knuthMorrisPratt(str, pat);
if(find)
	cout<<"There exists a substring\n";
else
	cout<<"Sorry, no match found";

printTemp(str);
return 0;
}

	
