#include<iostream>
#include<cstdlib>
using namespace std;


template<class T>
class customVec
{
    typedef T* pointer;
    pointer ptr;
    int size_;

public:
    customVec()
    {
        ptr = nullptr;
        size_ = 0;
    }
    customVec(const T& t)
    {
        ptr = (T*)malloc(t.size());
        for(int i = 0; i < size_; ++i)
        {
            ptr[i] = t.at(i);
        }
        this->size_ = t.size();
    }

    int size()
    {
        return size_;
    }
    T& operator=(T& t)
    {
        ptr = (T*)malloc(t.size());
        for(int i = 0; i < size; ++i)
        {
            ptr[i] = t.at(i);
        }
        this->size_ = t.size();
        return *this;
    }
    void push_back(T element)
    {
        if(size_+1 >= size_/2)
        {
            // allocate memory
            ptr = (T*)realloc(ptr, size_*2);
        }
        ptr[size_] = element;
        ++size_;
    }
    void pop_back()
    {
        if(size_ != 0)
            --size_;
    }
    void clear()
    {
        free(ptr);
    }
    T front()
    {
        if(size_ >= 1)
            return *(ptr);
    }
    T back()
    {
        return ptr[size_-1];
    }
    T at(int i)
    {
        return *(ptr+i);
    }

};

int main()
{
    customVec<int> cv;
    cv.push_back(1);
    cv.push_back(2);
    customVec<int> cv2 = cv;
    for(int i = 0; i < cv2.size(); ++i)
    {
        cout<<cv2.at(i);
    }
    return 0;
}