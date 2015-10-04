#include<string>
#include<utility>
#include<algorithm>
#include<iostream>

using namespace std;


string addTwo(string rev1, string rev2);
string subTwo(string rev1, string rev2);
string divideByTwo(string number);


//Read apple count in the form ofstring from the sandard input and spi out
int main(){

string apples, difference;
string kApples, nApples;
//cout<<endl<<"Division is "<<divideByTwo(n1)<<" Add is : "<<addTwo(n1, n2)<<"Subtraction is :  "<<subTwo(n1, n2)<<endl;

for(int j=1; j <= 10; ++j)
{

    cin>>apples;
	cin>>difference;

		
	kApples = divideByTwo( addTwo(apples, difference) );
	//cout<<"Addition result is "<<addTwo(apples, difference)<<endl;
	nApples = subTwo(apples, kApples);
	cout<<kApples<<endl;
	cout<<nApples<<endl;
}

return 0;
}


string subTwo(string rev1, string rev2){

	int carry = 0;
	int largeNumber, smallNumber ;
	string smallString, largeString;
	string result = "";
	int len1 = rev1.length();
	int len2 = rev2.length();
	if( len1 == len2) {
		int r = rev1.compare(rev2);
		if(r == 0)
			return "0";
		smallNumber = (r>0)?len2:len1;
		largeNumber = (r>0)?len1:len2;
		smallString = (r>0)?rev2:rev1;
		largeString = (r>0)?rev1:rev2;
			
	}
	else {
		if(len2 < len1) {
			smallNumber = len2;
			largeNumber = len1;
			smallString = rev2;
			largeString = rev1;
		}
		else {
			smallNumber = len1;
			largeNumber = len2;
			smallString = rev1;
			largeString = rev2;
		}
	}
	string newString;
	newString.resize( largeNumber-smallNumber, '0');
	newString.append(smallString);
	smallString = newString;
	reverse(smallString.begin(), smallString.end());
	reverse(largeString.begin(), largeString.end());
	//Now both smallString and largeString are in reverse order

	int i  = 0;
	//cout<<"largeNumber is "<<largeNumber<<endl;
	//cout<<"Reversed first string :"<<largeString<<endl;
	//cout<<"Reversed second string :"<<smallString<<endl;
	while( i < largeNumber) {
		//find digits of both	
		int topDigit = 	largeString[i] - '0';
		int botDigit =  smallString[i] - '0';
		if(topDigit < botDigit) {
			if(topDigit == ':') {
				carry = 1;
				result.append( to_string(9 - botDigit));
				if(largeString[i+1] == '0')	
					largeString[i+1] = ':';
				++i;
			}
		
			else {
				result.append( to_string(10 + topDigit - botDigit));
					if(largeString[i+1] == 0) {
					largeString[i+1] = ':';
					carry = 1;
				}
				else {
					largeString[i+1]--;
					carry = 0;
				}
					++i;
			}
		}
		else  {
			if(carry == 1) {
				largeString[i]--;
			}
			else {
				result.append(to_string(topDigit - botDigit));
			}
				carry = 0;
			++i;
		}
	}//end of while loop and end of subtraction
		reverse(result.begin(), result.end());
		result.erase(0, result.find_first_not_of('0'));
		return result;
}



string addTwo(string rev1, string rev2){

int carry = 0;



	string smallString, largeString;
	int smallNumber, largeNumber;
    string result = "";
		int len1 = rev1.length();
		int len2 = rev2.length();

		if( len1 == len2) {
			int r = rev1.compare(rev2);
			smallNumber = (r>0)?len2:len1;
			largeNumber = (r>0)?len1:len2;
			smallString = (r>0)?rev2:rev1;
			largeString = (r>0)?rev1:rev2;

		}
		else {
			if(len2 < len1) {
				smallNumber = len2;
				largeNumber = len1;
				smallString = rev2;
				largeString = rev1;
			}
			else {
				smallNumber = len1;
				largeNumber = len2;
				smallString = rev1;
				largeString = rev2;
			}
		}
        if(smallString.size() != 1 || largeString.size() != 1) {
            string newString;
            newString.resize( largeNumber-smallNumber, '0');
            newString.append(smallString);
            smallString = newString;
            //cout<<"New String is "<<newString;
            reverse(largeString.begin(), largeString.end());
            reverse(smallString.begin(), smallString.end());
        }


int i  = 0;

while( i < largeNumber) {
	
    result.append(to_string (( ( largeString[i] - '0' )+( smallString[i] - '0') +carry )%10)) ;
	carry  =  (( ( largeString[i] - '0'  )+( smallString[i] - '0' ) +carry )/10)?1:0;
	++i;

}
    if(carry)
        result.append( to_string(carry) );

	reverse(result.begin(), result.end());
    //cout<<"Result is "<<result;
	return result;
}




string divideByTwo(string number){
	string result = "";
	int carry = 0;
	int length = number.length();
	int i  = 0;
	bool carryFromOne = false, negativeNumber = false;
	if(number[i] == '-') {
		negativeNumber = true;
		++i;
	}
	int testNum = number[i]-'0';
	while(true){
		if( i >= length )
			break;
	
		if( testNum%2 == 0){
			if(carry == 1){
				if( testNum%2 == 0) {
					if(carryFromOne == true) {
						result.append("0");
						carryFromOne = false;
					}
					carry = 0;
				}
				else {
					carry = 1;
				}

				result.append( to_string (testNum/2) );
				testNum = number[++i]-'0';
			}	
			else  {
					result.append( to_string(testNum/2 ));
					carry = 0;
					testNum = number[++i] - '0';
			}
		}
	
		else if( testNum == 1){
			if( i == length - 1 ){
				result.append("0");
				break;
			}
			else {
				testNum = 10 + ( number[++i] - '0') ;
				carry = 1;
				carryFromOne = true;
				continue;
			}
		}
		else {
			carry = 1;
			result.append( to_string (testNum/2 ));
			carryFromOne = false;
		
			testNum = 10 + ( number[++i] - '0' );
		}
	}
		result.erase(0, result.find_first_not_of('0'));
		if(negativeNumber)
			cout<<"-";
		return result;
}
