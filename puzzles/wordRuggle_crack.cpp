#include<iostream>
#include<cstring>
#include<fstream>
#include<vector>
#include<algorithm>
#include<sys/time.h>

using namespace std;

void stringPrinter(char a[][4], int i, int j, string s);
void searchDict(string s, int start, int end);

ifstream inputFile("american-english");
ofstream outputFile("crack.txt");

bool traverse[4][4] = { {false, false, false, false},
			{false, false, false, false},
			{false, false, false, false},
			{false, false, false, false}
		      };
vector<string> resultVector_5, resultVector_3, resultVector_4, resultVector_x ;


string s = "";
string line;
string inputString;
char a[4][4];

string strArray[99155];
clock_t start, end;

int main(){
//read file in memory
int index=0;
char inputArray[16];
while(inputFile.good() && index<99155){
	getline(inputFile, line);
	for(int i=0;i<line.length();i++)
		if(line.at(i) == ' ')
	        {
		    cout<<"Dictionary invalid, please select some other one"<<endl	;
	            return -1;
		}
        strArray[index] = line;
	index++;

}

cout<<"Loaded words in memory\n";
cout<<"Please enter the input string"<<endl;
cin>>inputString;
if(inputString.size() == 16)
    strncpy((char*)a, inputString.c_str(), 16);
//    a = inputString.toArray();
else{
    cout<<"Invalid input size"<<endl;
    return -1;
}

start=clock();
for(int i=0; i<=3 ; i++){
for(int j=0; j<=3; j++){
s = "";
stringPrinter(a, i, j , s);
}
}

sort(resultVector_x.begin() ,  resultVector_x.end());
resultVector_x.erase( unique(resultVector_x.begin(), resultVector_x.end() ), resultVector_x.end());

sort(resultVector_5.begin() ,  resultVector_5.end());
resultVector_5.erase( unique(resultVector_5.begin(), resultVector_5.end() ), resultVector_5.end());

sort(resultVector_4.begin() ,  resultVector_4.end());
resultVector_4.erase( unique(resultVector_4.begin(), resultVector_4.end() ), resultVector_4.end());

sort(resultVector_3.begin() ,  resultVector_3.end());
resultVector_3.erase( unique(resultVector_3.begin(), resultVector_3.end() ), resultVector_3.end());

for (vector<string>::iterator it = resultVector_x.begin() ; it != resultVector_x.end(); ++it)
    cout << "Word found : "<< *it << endl;
for (vector<string>::iterator it = resultVector_5.begin() ; it != resultVector_5.end(); ++it)
    cout << "Word found : "<< *it << endl;

for (vector<string>::iterator it = resultVector_4.begin() ; it != resultVector_4.end(); ++it)
    cout << "Word found : "<< *it << endl;

for (vector<string>::iterator it = resultVector_3.begin() ; it != resultVector_3.end(); ++it)
    cout << "Word found : "<< *it << endl;

end = clock();
cout<<"Search completed.. :) "<<"time taken :"<<1.0*(end-start)/CLOCKS_PER_SEC<<" seconds"<<endl;
return 0;
}

void stringPrinter(char a[][4], int i, int j, string s){
if(i<0 || j<0 || i>3 || j>3 || traverse[i][j] == true)
	return;
traverse[i][j] = true;
s += a[i][j];
stringPrinter(a, i-1, j-1, s);
stringPrinter(a, i-1, j, s);
stringPrinter(a, i-1, j+1, s);
stringPrinter(a, i, j-1, s);
stringPrinter(a, i, j+1, s);
stringPrinter(a, i+1, j-1, s);
stringPrinter(a, i+1, j, s);
stringPrinter(a, i+1, j+1, s);
	searchDict(s, 0, 99154);

traverse[i][j] = false;
}

void searchDict(string s, int start, int end){

    if(start>end)
        return;

    int mid = (start+end)/2;
    if(s.compare(strArray[mid]) > 0)
    {
        start=mid+1;
        searchDict(s, start, end);
    }
    else if(s.compare(strArray[mid]) < 0)
    {
        end=mid-1;
        searchDict(s, start, end);
    }
    else
    {
	switch(s.size())
	{
	    case 3:
		resultVector_3.push_back(s);
		break;
	    case 4:
		resultVector_4.push_back(s);
		break;
	    case 5:
		resultVector_5.push_back(s);
		break;
	    case 6:
        case 7:
        case 8:
        case 9:
        case 10:
	    case 11:
	    case 12:
	    case 13:
		resultVector_x.push_back(s); 
	}
    }
}






