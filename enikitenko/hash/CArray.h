#ifndef CARRAY
#define CARRAY

#include <stdio.h>
#include <assert.h>
#include <algorithm>

template<typename T>
class CArray
{
public:
	
	CArray(const CArray& Other);
	CArray<T>& operator = (const CArray& Other);

	CArray(int MaxSize = 0);
	~CArray();
	
	T& operator [] (int i);
	void Resize(int MaxSize);
	void SetStatic();
	int Size();
	int MaxSize();
	T* Data();
	int Insert(T Element = T(), bool Resizable = false);
	void Remove(int i);
	void Clear();
	void Append(CArray& Other, bool Resizable = false);
	T Pop();
	T Top();
	void Sort();
	
private:

	T*	 Data_;
	int  Size_;
	int  MaxSize_;
	bool Static_;

};

template<typename T>
CArray<T>::CArray(int MaxSize) :
	Data_(MaxSize ? new T[MaxSize] : 0),
	Size_(0),
	MaxSize_(MaxSize),
	Static_(false)
{
}

template<typename T>
CArray<T>::~CArray()
{
	delete[] Data_;
	Data_ = NULL;
	MaxSize_ = 0;
}

template<typename T>
T& CArray<T>::operator [](int i)
{
	assert(i >= 0);
	assert(i < Size_);
	return Data_[i];
}

template<typename T>
void CArray<T>::Resize(int MaxSize)
{
	if (MaxSize_ == MaxSize)
		return;
	assert(MaxSize >= Size_);

	T* OldData = Data_;
	Data_ = MaxSize ? new T[MaxSize] : NULL;
	MaxSize_ = MaxSize;

	for (int i = 0; i < Size_; i++)
	{
		Data_[i] = OldData[i];
	}

	if (Static_)
		Size_ = MaxSize_;

	delete[] OldData;
}

template<typename T>
int CArray<T>::Size()
{
	return Size_;
}

template<typename T>
int CArray<T>::MaxSize()
{
	return MaxSize_;
}

template<typename T>
T* CArray<T>::Data()
{
	return Data_;
}

template<typename T>
int CArray<T>::Insert(T Element, bool Resizable)
{
	assert(!Static_);

	if (!Resizable)
	{
		assert(Size_ < MaxSize_);
	}
	else if (Size_ == MaxSize_)
		Resize(MaxSize_ + 1);

	Data_[Size_] = Element;
	Size_++;
	return Size_ - 1;
}

template<typename T>
void CArray<T>::Remove(int i)
{
	assert(!Static_);
	assert(Size_);

	Size_--;
	Data_[i] = Data_[Size_];
}

template<typename T>
void CArray<T>::Append(CArray& Other, bool Resizable)
{
	assert(!Static_);

	if (Size_ + Other.Size() > MaxSize_)
	{
		assert (Resizable);
		Resize (Size_ + Other.Size());
	}

	for (int i = 0; i < Other.Size(); i++)
		Insert (Other[i], false);
}

template<typename T>
void CArray<T>::Clear()
{
	assert(!Static_);
	Size_ = 0;
}

template<typename T>
T CArray<T>::Pop()
{
	assert(!Static_);

	assert(Size_);
	Size_--;
	return Data_[Size_];
}

template<typename T>
T CArray<T>::Top()
{
	assert(!Static_);
	assert(Size_);
	return Data_[Size_ - 1];
}

template<typename T>
void CArray<T>::SetStatic()
{
	Static_ = true;
	Size_   = MaxSize_;
}

template<typename T>
void CArray<T>::Sort()
{
	std::sort(Data_, Data_ + Size_);
}

template<typename T>
CArray<T>::CArray(const CArray<T>& Other)
{
	Data_ = NULL;
	*this = Other;
}

template<typename T>
CArray<T>& CArray<T>::operator = (const CArray<T>& Other)
{
	if (!Data_ || MaxSize_ != Other.MaxSize_)
	{
		delete[] Data_;
		MaxSize_ = Other.MaxSize_;
		if (MaxSize_)
			Data_ = new T[MaxSize_];
		else
			Data_ = NULL;
	}

	Size_ = Other.Size_;
	for (int i = 0; i < Size_; i++)
	{
		Data_[i] = Other.Data_[i];
	}

	return *this;
}

#endif
