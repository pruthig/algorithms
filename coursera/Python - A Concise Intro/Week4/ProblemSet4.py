# - ProblemSet4.py *- coding: utf-8 -*-
"""
Problem 4_1:
Write a function that will sort an alphabetic list (or list of words) into 
alphabetical order. Make it sort independently of whether the letters are 
capital or lowercase. First print out the wordlist, then sort and print out
the sorted list.
Here is my run on the list firstline below (note that the wrapping was added 
when I pasted it into the file -- this is really two lines in the output).

problem4_1(firstline)
['Happy', 'families', 'are', 'all', 'alike;', 'every', 'unhappy', 'family',
 'is', 'unhappy', 'in', 'its', 'own', 'way.', 'Leo Tolstoy', 'Anna Karenina']
['alike;', 'all', 'Anna Karenina', 'are', 'every', 'families', 'family',
'Happy', 'in', 'is', 'its', 'Leo Tolstoy', 'own', 'unhappy', 'unhappy', 'way.']

"""
#%%
firstline = ["Happy", "families", "are", "all", "alike;", "every", \
              "unhappy", "family", "is", "unhappy", "in", "its", "own", \
              "way.", "Leo Tolstoy", "Anna Karenina"] 
#%%
def problem4_1(wordlist):
    """ Takes a word list prints it, sorts it, and prints the sorted list """
    print(wordlist)
    wordlist.sort(key=str.lower)
    print(wordlist)
    
#%%
"""
Problem 4_2:
Write a function that will compute and print the mean and standard deviation 
of a list of real numbers (like the following).  Of course, the length of the
list could be different. Don't forget to import any libraries that you might
need.
Here is my run on the list of 25 floats create below:

problem4_2(numList)
51.528
30.81215290541488

"""
#%%
import random
import statistics

numList = []
random.seed(150)
for i in range(0,25):
    numList.append(round(100*random.random(),1))
#%%   
def problem4_2(ran_list):
    """ Compute the mean and standard deviation of a list of floats """
    print(statistics.mean(ran_list))
    print(statistics.stdev(ran_list))
#%%
"""
Problem 4_3:
Write a function problem4_3(product, cost) so that you can enter the product
and its cost and it will print out nicely. Specifically, allow 25 characters
for the product name and left-justify it in that space; allow 6 characters for 
the cost and right justify it in that space with 2 decimal places. Precede the
cost with a dollar-sign.  There should be no other spaces in the output.

Here is how one of my runs looks:
problem4_3("toothbrush",2.6)
toothbrush               $  2.60

"""
#%%
def problem4_3(product, cost):
    """ Prints the product name in a space of 25 characters, left-justified
        and the price in a space of 6 characters, right-justified"""
    print("{0:<25}${1:>6.2f}".format(product, cost))


#%%    
"""
Problem 4_4:
This problem is to build on phones.py.  You add a new menu item
  r) Reorder
This will reorder the names/numbers in the phone list alphabetically by name. 
This may sound difficult at first thought, but it really is straight forward.  
You need to add two lines to the main_loop and one line to menu_choice to print 
out the new menu option (and add 'r' to the list of acceptable choices).  In 
addition you need to add a new function to do the reordering: I called mine
reorder_phones(). Here is a start for this very short function:

def reorder_phones():
    global phones       # this insures that we use the one at the top
    pass # replace this pass (a do-nothing) statement with your code


Note: The auto-grader will run your program, choose menu items s, r, s, and q
in that order.  It will provide an unsorted CSV file and see if your program
reorders it appropriately. The grader will provide a version of myphones.csv
that has a different set of names in it from the ones we used in the lesson. 
This difference in data will, of course, not matter with a well coded program.
Below the result of this added function is shown using the names used in class.
Note that name is a single field.  Reorder by that field, don't try to separate
first and last name and reorder by one or the other --- just treat name as a 
single field that you re-order by.  Also, in this case upper/lower case won't
matter.

TIP: phones[] is a list of lists (each sublist is a [name, phone].  It looks 
complicated to sort.  Just pretend that each sublist is a single name item and
code it accordingly.  It will work.  This is a beginner course and this sort
function requires only one line and no fancy outside material to make it work.)
The main thrust of this problem is to add in the various pieces to make a new
menu entry.

Before:
Choice: s
     Name                     Phone Number 
  1  Jerry Seinfeld          (212) 842-2527
  2  George Costanza         (212) 452-8145
  3  Elaine Benes            (212) 452-8723
After:
Choice: s
     Name                     Phone Number 
  1  Elaine Benes            (212) 452-8723
  2  George Costanza         (212) 452-8145
  3  Jerry Seinfeld          (212) 842-2527

"""

import os
import csv


phones = []
name_pos = 0
phone_pos = 1
phone_header = [ 'Name', 'Phone Number']

def proper_menu_choice(which):
    if not which.isdigit():
        print ("'" + which + "' needs to be the number of a phone!")
        return False
    which = int(which)
    if which < 1 or which > len(phones):
        print ("'" + str(which) + "' needs to be the number of a phone!")
        return False
    return True
    
def delete_phone(which):
    if not proper_menu_choice(which):
        return
    which = int(which)

    del phones[which-1]
    print( "Deleted phone #", which)

def edit_phone(which):
    if not proper_menu_choice(which):
        return
    which = int(which)
        
    phone = phones[which-1]
    print("Enter the data for a new phone. Press <enter> to leave unchanged.")
    
    print(phone[name_pos])
    newname = input("Enter phone name to change or press return: ")
    if newname == "":
        newname = phone[name_pos]
        
    print(phone[phone_pos])    
    newphone_num = input("Enter new phone number to change or press return: ")
    if newphone_num == "":
        newphone_num = phone[phone_pos]
            
    phone = [newname, newphone_num]
    phones[which-1] = phone

  
def save_phone_list():

    f = open("myphones.csv", 'w', newline='')
    for item in phones:
        csv.writer(f).writerow(item)
    f.close()
  
def load_phone_list():
    if os.access("myphones.csv",os.F_OK):
        f = open("myphones.csv")
        for row in csv.reader(f):
            phones.append(row)
        f.close()

     
def reorder_phones():
    global phones
    phones.sort(key=lambda phone: phone[0].lower())

    
    
def show_phones():
    show_phone(phone_header, "")
    index = 1
    for phone in phones:
        show_phone(phone, index)
        index = index + 1
    print()

def show_phone(phone, index):
    outputstr = "{0:>3}  {1:<20}  {2:>16}"
    print(outputstr.format(index, phone[name_pos], phone[phone_pos]))

def create_phone():
    print("Enter the data for a new phone:")
    newname = input("Enter name: ")
    newphone_num = input("Enter phone number: ")
    phone = [newname,newphone_num]
    phones.append(phone)
    
def menu_choice():
    """ Find out what the user wants to do next. """
    print("Choose one of the following options?")
    print("   s) Show")
    print("   n) New")
    print("   r) Reorder")
    print("   d) Delete")
    print("   e) Edit")
    print("   q) Quit")
    choice = input("Choice: ")    
    if choice.lower() in ['n','d', 'r', 's','e', 'q']:
        return choice.lower()
    else:
        print(choice +"?")
        print("Invalid option")
        return None


def main_loop():
    
    load_phone_list()
    
    while True:
        choice = menu_choice()
        if choice == None:
            continue
        if choice == 'q':
            print( "Exiting...")
            break     # jump out of while loop
        elif choice == 'n':
            create_phone()
        elif choice == 'r':
            reorder_phones()
        elif choice == 'd':
            which = input("Which item do you want to delete? ")
            print("which is ", which)
            delete_phone(which)
        elif choice == 's':
            show_phones()
        elif choice == 's':
            reorder_phones()
        elif choice == 'e':
            which = input("Which item do you want to edit? ")
            print("which is ", which)
            edit_phone(which)
        else:
            print("Invalid choice.")
            
    save_phone_list()
    

