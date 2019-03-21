#include<iostream>
#include<memory>
#include<vector>

using namespace std;

class A{
    int x, y;
public:
    A()
    {
    }
};

template<class T>
class customAlloc //: public std::allocator<T>
{

public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    customAlloc(){}
    ~customAlloc(){}

    //template <class U> struct rebind { typedef customAlloc<U> other; };
    //template <class U> customAlloc(const customAlloc<U>&){}

    //pointer address(reference x) const {return &x;}
    //const_pointer address(const_reference x) const {return &x;}
    //size_type max_size() const throw() {return size_t(-1) / sizeof(value_type);}

    T* allocate(size_t n)
    {
        cout<<"Allocating here the size of: "<<n<<"\n";
        T *t = new T[n];
        return t;
    }
    void deallocate(T* ptr, size_t n)
    {
        cout<<"Deallocating here\n";
        delete ptr;
    }

};

int main()
{
    vector<A, customAlloc<A>> vec(10, customAlloc<A>());
    A b = vec.at(0);
    return 0;
}