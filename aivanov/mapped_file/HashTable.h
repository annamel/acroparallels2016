#ifndef CHASHTABLE
#define CHASHTABLE

#include <vector>
#include <cassert>

template<typename tKey, typename tData>
struct SCell
{
	tKey	key;
	tData	data;
};

template<typename tKey, typename tData>
class CHashTable
{
public:
	typedef unsigned (*tHashFunction)(const tKey&);
	typedef SCell<tKey, tData> tCell;
	typedef std::vector<tCell> tRow;

private:

	std::vector<tRow>	data_;
	tHashFunction		hashFunction_;

public:
	CHashTable(tHashFunction hashFunction, unsigned nRows);

	CHashTable(const CHashTable<tKey, tData>& other);
	CHashTable<tKey, tData>& operator = (const CHashTable<tKey, tData>& other);

	std::vector<tRow>& getData();
	tData& operator () (const tKey& key, bool mustExist = false);
	tData& operator [] (const tKey& key);
	tData* getElement(const tKey& key);

	void remove(const tKey& key);
	void clear();

};

template<typename tKey, typename tData>
CHashTable<tKey, tData>::CHashTable(tHashFunction hashFunction, unsigned nRows) :
	data_(nRows),
	hashFunction_(hashFunction)
{
}

template<typename tKey, typename tData>
std::vector<typename CHashTable<tKey, tData>::tRow>& CHashTable<tKey, tData>::getData()
{
	return data_;
}

template<typename tKey, typename tData>
tData& CHashTable<tKey, tData>::operator () (const tKey& key, bool mustExist)
{
	unsigned rowIndex = hashFunction_(key) % data_.size();
	std::vector<CHashTable::tCell>& row = data_[rowIndex];

	for (auto& cell : row)
		if (cell.key == key)
			return cell.data;

	assert(!mustExist);

	tCell cell;
	cell.data = tData();
	cell.key = key;
	row.push_back(cell);
	return row[row.size() - 1].data;
}

template<typename tKey, typename tData>
tData& CHashTable<tKey, tData>::operator [](const tKey& key)
{
	return (*this)(key);
}

template<typename tKey, typename tData>
tData* CHashTable<tKey, tData>::getElement(const tKey& key)
{
	unsigned rowIndex = hashFunction_(key) % data_.size();
	std::vector<tCell>& row = data_[rowIndex];

	for (auto& cell : row)
		if (cell.key == key)
		{
			return &cell.data;
		}
		
	return 0;
}

template<typename tKey, typename tData>
void CHashTable<tKey, tData>::remove(const tKey& key)
{
	unsigned rowIndex = hashFunction_(key) % data_.size();
	std::vector<tCell>& row = data_[rowIndex];
	bool entryFound = false;

	for (unsigned i = 0; i < row.size(); i++)
	{
		if (row[i].key == key)
		{
			row.erase(row.begin() + i);
			entryFound = true;
			break;
		}
	}
	assert(entryFound);
}

template<typename tKey, typename tData>
void CHashTable<tKey, tData>::clear()
{
	for (auto& row : data_)
		row.clear();
}

#endif
