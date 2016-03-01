#ifndef HASHTABLE
#define HASHTABLE

#include <cstdlib>
#include <iostream>
#include <string>

// 32 bit integers
#ifdef _MSC_VER
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

using namespace std;

// default hash function values
const uint32_t Prime = 0x01000193; //   16777619
const uint32_t Seed  = 0x811C9DC5; // 2166136261


template <typename T>
class HashTable
{
private:
    unsigned static const defaultTableSize = 4;
    unsigned tableSize;
    struct item
    {
        unsigned int key;
        T value;
        item *next;
    };
    item **table;

    // hash a single byte
    inline uint32_t fnv1a(unsigned char oneByte, uint32_t hash = Seed)
    { return (oneByte ^ hash) * Prime; }

public:
    HashTable();
    ~HashTable();
    uint32_t hash(uint32_t key, uint32_t hash = Seed);
    int numberOfItemsInIndex(const uint32_t index);
    void printTable();
    bool addItem(const uint32_t newKey, const T newValue);
    void printItemsInIndex(const uint32_t index);
    bool findValue(const uint32_t key, T &value);
    bool removeItem(uint32_t key);
};



template <typename T> HashTable<T>::HashTable()
{
    tableSize = defaultTableSize;
    table = new item *[tableSize];
    for(unsigned int i = 0; i < tableSize; i++)
    {
        table[i] = NULL;
    }
}

template <typename T> HashTable<T>::~HashTable(){}

template <typename T> void HashTable<T>::printTable()
{
    for(unsigned int i = 0; i < tableSize; i++)
    {
        if(table[i] == NULL)
        {
            cout << "----------------\n";
            cout << "index = " << i << endl;
            cout << "key : empty" << endl;
            cout << "value : empty" << endl;
            cout << "----------------\n";
        }
        else
        {
            cout << "----------------\n";
            cout << "index = " << i << endl;
            cout << "key : " << table[i]->key << endl;
            cout << "value : " << table[i]->value << endl;
            cout << "# of items = " << numberOfItemsInIndex(i) << endl;
            cout << "----------------\n";
        }
    }
}

template <typename T> bool HashTable<T>::addItem(const uint32_t newKey, const T newValue)
{
    int index = hash(newKey);

    if(table[index] == NULL)
    {
        table[index] = new item;
        table[index]->key = newKey;
        table[index]->value = newValue;
        table[index]->next = NULL;
        return true;
    }
    else
    {
        item *ptr = table[index];
        while(ptr->next){ ptr = ptr->next; }

        ptr->next = new item;
        ptr = ptr->next;
        ptr->key = newKey;
        ptr->value = newValue;
        ptr->next = NULL;
        return 1;
    }
    return 0;
}


template <typename T> int HashTable<T>::numberOfItemsInIndex(const uint32_t index)
{
    if(index >= tableSize){ return -1; }
    else
    {
        item *ptr = table[index];
        uint32_t count = 0;

        while(ptr)
        {
            ptr = ptr->next;
            count++;
        }
        return count;
    }
}

template <typename T> void HashTable<T>::printItemsInIndex(const uint32_t index)
{
    if(index >= tableSize)
    {
        cout << "Index " << index << " is greater that size of the table" << endl;
        return;
    }
    else
    {
        if(!table[index])
        {
            cout << "Index " << index << " is empty" << endl;
            return;
        }
        else
        {
            item *ptr = table[index];
            cout << "index " << index << " contains the following item(s)" << endl;
            while(ptr)
            {
                cout << "----------------\n";
                cout << "key : " << ptr->key << endl;
                cout << "value : " << ptr->value << endl;
                cout << "----------------\n";
                ptr = ptr->next;
            }
        }
    }
}


template <typename T> bool HashTable<T>::findValue(const uint32_t key, T &value)
{
    uint32_t index = hash(key);
    item *ptr = table[index];

    while(ptr->key != key)
    {
        if(ptr->next == NULL){ return false; }
        ptr = ptr->next;
    }
    value = ptr->value;

    return true;
}


template <typename T> bool HashTable<T>::removeItem(uint32_t key)
{
    uint32_t index = hash(key);
    //Case 0 - bucket is empty
    if(table[index] == NULL){ return false; }

    //Case 1 - only 1 item contained in bucket
    //         and that item has matching name
    else if(table[index]->key == key &&
       table[index]->next == NULL)
    {
        delete table[index];
        return true;
    }

    //Case 2 - match is located in the first item in the
    //         bucket but there are more items in the bucket
    else if(table[index]->key == key)
    {
        item *delPtr = table[index];
        table[index] = table[index]->next;
        delete delPtr;
        return true;
    }

    //Case 3 - bucket contains items but first item is not a match
    else
    {
        item *ptr1 = table[index]->next;
        item *ptr2 = table[index];

        while(ptr1 != NULL && ptr1->key != key)
        {
            ptr2 = ptr1;
            ptr1 = ptr1->next;
        }

        //Case 3.1 : no match
        if(ptr1 == NULL){ return false; }

        //Case 3.2 : match is found
        else
        {
            ptr2->next = ptr1->next;
            delete ptr1;
            return true;
        }
    }
}


template <typename T> uint32_t HashTable<T>::hash(uint32_t key, uint32_t hash)
{
    const unsigned char* ptr = (const unsigned char*) &key;
    hash = fnv1a(*ptr++, hash);
    hash = fnv1a(*ptr++, hash);
    hash = fnv1a(*ptr++, hash);
    return fnv1a(*ptr  , hash) % tableSize;
}


#endif // HASHTABLE

