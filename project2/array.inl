#include "array.h"
#include <cstring>

template <class T>
Array<T>::Array(){
    arr_ = new T[start_capacity_];
    size_ = 0;
    capacity_ = start_capacity_;
}

template <class T>
Array<T>::~Array<T>(){
    delete [] arr_;
}

template <class T>
int
Array<T>::size() const{
    return size_;
}

template <class T>
void
Array<T>::add(T elem){
    if(size_ == capacity_){
        // We have to resize our array

        // Create new array with double size
