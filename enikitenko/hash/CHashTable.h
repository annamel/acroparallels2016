#ifndef CHASHTABLE
#define CHASHTABLE

#include "CArray.h"

#define Mod(a,b) ((a) % (b))

template<typename tKey, typename tData>
struct SCell
{
	tKey	Key;
	tData	Data;
};

template<typename tKey, typename tData>
class CHashTable
{
public:
	typedef int (*tHashFunction)(tKey&);
	typedef SCell<tKey, tData> tCell;
	typedef CArray<tCell> tRow;

private:

	CArray<tRow>	Data_;
	tHashFunction	HashFunction_;

public:
	
	CHashTable();
	CHashTable(tHashFunction HashFunction, int NRows);

	CHashTable(const CHashTable<tKey, tData>& Other);
	CHashTable<tKey, tData>& operator = (const CHashTable<tKey, tData>& Other);

	void Init (tHashFunction HashFunction, int NRows);
	CArray<tRow>& Data();
	tData& operator () (tKey& Key, bool MustExist = false);
	tData* Data(tKey& Key);

	void Remove(tKey Key);
	void Clear();

};

template<typename tKey, typename tData>
CHashTable<tKey, tData>::CHashTable() :
	Data_ (0)
{
}

template<typename tKey, typename tData>
CHashTable<tKey, tData>::CHashTable(tHashFunction HashFunction, int NRows) :
	Data_(NRows),
	HashFunction_(HashFunction)
{
	assert(NRows);
	for (int i = 0; i < NRows; i++)
		Data_.Insert();
}

template<typename tKey, typename tData>
void CHashTable<tKey, tData>::Init (tHashFunction HashFunction, int NRows)
{
	assert (NRows);

	Data_.Resize (NRows);
	HashFunction_ = HashFunction;

	for (int i = 0; i < NRows; i++)
		Data_.Insert();
}

template<typename tKey, typename tData>
CArray<typename CHashTable<tKey, tData>::tRow>& CHashTable<tKey, tData>::Data()
{
	return Data_;
}

template<typename tKey, typename tData>
tData& CHashTable<tKey, tData>::operator () (tKey& Key, bool MustExist)
{
	int RowIndex = Mod(HashFunction_(Key), Data_.Size());
	CArray<CHashTable::tCell>& Row = Data_[RowIndex];

	for (int i = 0; i < Row.Size(); i++)
	{
		tCell& Cell = Row[i];
		if (Cell.Key == Key)
			return Cell.Data;
	}

	assert(!MustExist);

	tCell Cell;
	Cell.Data = tData();
	Cell.Key = Key;
	Row.Insert(Cell, true);
	return Row[Row.Size() - 1].Data;
}

template<typename tKey, typename tData>
tData* CHashTable<tKey, tData>::Data(tKey& Key)
{
	int RowIndex = Mod(HashFunction_(Key), Data_.Size());
	CArray<tCell>& Row = Data_[RowIndex];

	for (int i = 0; i < Row.Size(); i++)
	{
		tCell& Cell = Row[i];
		if (Cell.Key == Key)
		{
			return &Cell.Data;
		}
	}
	return 0;
}

template<typename tKey, typename tData>
void CHashTable<tKey, tData>::Remove(tKey Key)
{
	int RowIndex = Mod(HashFunction_(Key), Data_.Size());
	CArray<tCell>& Row = Data_[RowIndex];
	bool EntryFound = false;

	for (int i = 0; i < Row.Size(); i++)
	{
		if (Row[i].Key == Key)
		{
			Row.Remove(i);
			EntryFound = true;
			break;
		}
	}
	assert(EntryFound);
}

template<typename tKey, typename tData>
void CHashTable<tKey, tData>::Clear()
{
	for (int i = 0; i < Data_.Size(); i++)
		Data_[i].Clear();
}

#endif
