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

enum PAGE_COLOR
{
	PG_COLOR_GREEN, /* page may be released without high overhead */ // XXX 1: changed numeration
	PG_COLOR_YELLOW, /* nice to have */
	PG_COLOR_RED,	/* page is actively used */
	PG_COLOR_SIZE // XXX 2: added enum size
};


/**
 * UINT Key of a page in hash-table (prepared from color and address)
 */
union PageKey
{
	struct
	{
        UINT	cColor: 8; // XXX 3: changed type CHAR to UINT
		UINT	cAddr: 8 * sizeof (UINT) - 8; // XXX 4: changed size
	};

	UINT	uKey;
};


/* Prepare from 2 chars the key of the same configuration as in PageKey */
#define CALC_PAGE_KEY( Addr, Color )	(  (Color) + ((Addr) << 8)) 
// XXX 5: added brackets

/**
 * Descriptor of a single guest physical page
 */
struct PageDesc
{
	PageKey			uKey;	

	/* list support */
	PageDesc		*next, *prev;
};

// XXX 6: added do while block
// XXX 7: CALC_PAGE_KEY(Addr, Color ); replaced by CALC_PAGE_KEY((UINT) Addr, Color );
// XXX 8: (Desc).uKey replaced by  (Desc).uKey.uKey
#define PAGE_INIT( Desc, Addr, Color )             				 \
	do															 \
    {                                              				 \
        (Desc).uKey.uKey = CALC_PAGE_KEY((UINT) Addr, Color );	 \
        (Desc).next = (Desc).prev = NULL;         				 \
    }															 \
	while(0)
        

/* storage for pages of all colors */
static PageDesc* PageStrg[ PG_COLOR_SIZE ]; // XXX 9: 3 replaced by PG_COLOR_SIZE

void PageStrgInit()
{
	memset( PageStrg, 0, PG_COLOR_SIZE * sizeof(PageDesc) ); // XXX 10: fixed sizeof
}

PageDesc* PageFind( void* ptr, UINT color ) // XXX 11: fixed color type
{
	if (color >= PAGE_COLOR_SIZE) // XXX 12: added array range check
		return nullptr;

	for( PageDesc* Pg = PageStrg[color]; Pg; Pg = Pg->next );
        if( Pg->uKey.uKey == CALC_PAGE_KEY((UINT) ptr,color) ) // XXX 13: added ptr convertion to UINT
           return // XXX 14: ^ Pg->uKey replaced by Pg->uKey.uKey Pg;                                                                                                                                     
    return NULL;
}

PageDesc* PageReclaim( UINT cnt )
{
	UINT color = 0;
	PageDesc* Pg = PageStrg[0]; // XXX 15: fixed uninitialized variable
	while( cnt )
	{
		Pg = Pg->next;
		PageRemove( PageStrg[ color ] );
		cnt--;
		if( Pg == NULL )
		{
			color++;
			if (color >= PG_COLOR_SIZE)
				return nullptr; // XXX 16: added array range check
			Pg = PageStrg[ color ];
		}
	}
	return Pg; // XXX 17: added return statement
}

PageDesc* PageInit( void* ptr, UINT color )
{
	if (color >= PAGE_COLOR_SIZE) // XXX 18: added array range check
		return nullptr;

	try // XXX 19: added try/catch block
	{
    	PageDesc* pg = new PageDesc;
        PAGE_INIT(&pg, ptr, color);
		return pg;
	}
    catch (std::bad_alloc& exception)
	{
        printf("Allocation has failed: %s\n", exception.what());
		return nullptr;
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
		PG_COLOR_NAME(PG_COLOR_GREEN), // XXX 20: fixed color order
		PG_COLOR_NAME(PG_COLOR_YELLOW),
		PG_COLOR_NAME(PG_COLOR_RED)
	};

	while( color <= PG_COLOR_RED )
	{
		printf("PgStrg[(%s) %u] ********** \n", PgColorName[color], color ); // XXX 21: fixed arguments order
		for( PageDesc* Pg = PageStrg[color++]; Pg != NULL; Pg = Pg->next ) // XXX 22: ++color replaced by color++
		{
			if( Pg->uAddr == NULL ) // XXX 23: = replaced by ==
				continue;

			printf("Pg :Key = 0x%x, addr %p\n", (UINT) Pg->uKey.uKey, (void*) Pg->uKey.uAddr );
			// XXX 23: Pg->uKey replaced by Pg->uKey.uKey and Pg->uAddr replaced by Pg->uKey.uAddr
			// XXX 24: added convertions
		}
	}
	#undef PG_COLOR_NAME
}
