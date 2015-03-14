//In exception handlng, no implicit conversion takes place
//Like below catch(...) executes but not catch(int h)    
//Compiler mandates to have a catch block after try block
//if both derived and base class are thrown as exceptions then derived must appear before base
//since basecan handle all kinnds of excption...
//Upcast is easy from derived to base thats why
//In C++ all exceptions are unchecked i.e they are not checked at compile time..
//All the objects created inside try block are deleted before control is transferred to catch block..
#include <iostream>

using namespace std;

int main()
{
   int x = -1;

   // Some code
   cout << "Before try \n";
   try {
      cout << "Inside try \n";
      if (x < 0)
      {
         throw 'a';
         cout << "After throw (Never executed) \n";
      }
   }
   catch (int x ) {
      cout << "Exception Caught \n";
   }
   catch(...){
		cout<<"Default exception handler\n";
	}

   return 0;
}
