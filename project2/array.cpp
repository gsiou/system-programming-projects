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
        capacity_ *= 2;
        T *bigArr = new T[capacity_];

        // Copy old array contents
        memcpy(bigArr, arr_, size_ * sizeof(T));

        // Delete old array
        delete [] arr_;

        // Set bigArr as our array
        arr_ = bigArr;
    }

    // Add elem at the end of the array
    arr_[size_] = elem;
    size_++;
}

template <class T>
void
Array<T>::set(int index, T elem){
    if(index < size_){
        arr_[index] = elem;
    }
    else{
        throw std::out_of_range("Out of array bounds");
    }
}

template <class T>
void
Array<T>::deleteAt(int index){
    if(size_ == 0){
        return ; // Nothing to delete.
    }
    else if(size_ == 1){
        // ????
    }
}

template <class T>
T
Array<T>::get(int index) const{
    if(index < size_){
        return arr_[index];
    }
    else{
        throw std::out_of_range("Out of array bounds");
    }
}

