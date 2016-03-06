/**
* @brief
*		Find errors and decrease probability of getting errors of the same kind in the future
*		This piece of code won't compile and it doesn't describe an entire algorithm: just part of some page storage
*
* @author
*		AnnaM
*/

#include <Windows.h>
#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <new>

enum PAGE_COLOR
{
	PG_COLOR_GREEN, /* page may be released without high overhead */        // 1 bag: from 0
	PG_COLOR_YELLOW, /* nice to have */
	PG_COLOR_RED,	/* page is actively used */
    PG_QUANTITY_OF_COLORS                                                   // aded
};


/**
 * UINT Key of a page in hash-table (prepared from color and address)
 */
union PageKey
{
	struct
	{
        UINT	uColor: 8;                                                  // 2 bag: CHAR -> UINT
		UINT	uAddr: sizeof(UINT)*8 - 8;                                  // if UINT 64 bit
	};

	UINT	uKey;
};


/* Prepare from 2 chars the key of the same configuration as in PageKey */
#define CALC_PAGE_KEY( Addr, Color )	(  (Color) + (((UINT)(Addr)) << 8) )        // 3 bag: order operations


/**
 * Descriptor of a single guest physical page
 */
struct PageDesc
{
	PageKey			uKey;	

	/* list support */
	PageDesc		*next, *prev;
};

#define PAGE_INIT( Desc, Addr, Color )                   \
   do {                                                  \
        (Desc).uKey.uKey = CALC_PAGE_KEY( Addr, Color ); \
        (Desc).next = (Desc).prev = NULL;                \
    } while (0)                                                             // 4 bag - aded: do while
        

/* storage for pages of all colors */
static PageDesc* PageStrg[ PG_QUANTITY_OF_COLORS ];

void PageStrgInit()
{
	memset( PageStrg, 0, PG_QUANTITY_OF_COLORS * sizeof(*PageStrg) );       // 5 bag: in sizeof & -> *
}

PageDesc* PageFind( void* ptr, char color )
{
    assert(color < PG_QUANTITY_OF_COLORS);                                  // aded
	for( PageDesc* Pg = PageStrg[color]; Pg; Pg = Pg->next )                // 6 bag: don't need ;
        if( Pg->uKey.uKey == CALC_PAGE_KEY(ptr,color) )                    // 7 bag: void* -> UINT (in define)
           return Pg;                                                                                                                                     
    return NULL;
}

PageDesc* PageReclaim( UINT cnt )
{
	UINT color = 0;
	PageDesc* Pg = PageStrg[0];                                             // 8 bag: init
	while( cnt )
	{
        if( Pg == NULL )                                                    // 9 bag: if is up
        {
            color++;
            if (color == PG_QUANTITY_OF_COLORS)                             // aded
                return nullptr;
            Pg = PageStrg[ color ];
            continue;
        }
		Pg = Pg->next;
		PageRemove( PageStrg[ color ] );
		cnt--;
	}
    return Pg;                                                              // 10 bag: there is no "return"
}
            
PageDesc* PageInit( void* ptr, UINT color )
{
    assert(color < PG_QUANTITY_OF_COLORS);
    try {
        PageDesc* pg = new PageDesc;
        if( pg )
            PAGE_INIT(*pg, ptr, color);                                     // 11 bag: & -> *
        else
            printf("Allocation has failed\n");                              // 12 bag: exception - aded "try catch"
        return pg;
    }
    catch (std::bad_alloc& ba) {
        std::cerr << "bad_alloc caught: "<< ba.what() << endl;
        return NULL;
    }
}

/**
 * Print all mapped pages
 */
void PageDump()
{
	UINT color = 0;
	#define PG_COLOR_NAME(clr) #clr
	char* PgColorName[] = 
	{
        PG_COLOR_NAME(PG_COLOR_GREEN),
		PG_COLOR_NAME(PG_COLOR_YELLOW),                                     // 13 bag: wrong order
        PG_COLOR_NAME(PG_COLOR_RED)
	};

	while( color <= PG_COLOR_RED )
	{
		printf("PgStrg[(%s) %u] ********** \n", PgColorName[color], color ); // 14 bag: wrong order
		for( PageDesc* Pg = PageStrg[color++]; Pg != NULL; Pg = Pg->next )   // 15 bag: ++ should be postfix
		{
			if( Pg->uKey.uAddr == NULL )                                     // 16 bag: = -> ==
				continue;

			printf("Pg :Key = 0x%x, addr %p\n", Pg->uKey.uKey, Pg->uKey.uAddr );
		}
	}
	#undef PG_COLOR_NAME
}
