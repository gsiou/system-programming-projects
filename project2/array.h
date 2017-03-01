#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stdexcept>

template <class T>
class Array{
public:
    Array();
    ~Array();
    int  size() const;
    void add(T elem);
    void set(int index, T elem);
    T    get(int index) const;
private:
    T *arr_;
    int size_;
    int capacity_;
    static const int start_capacity_ = 100;  
};

#include "array.inl"

#endif
