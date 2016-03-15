/**
* @brief
*		Find errors and decrease probability of getting errors of the same kind in the future
*		This piece of code won't compile and it doesn't describe an entire algorithm: just part of some page storage
*
* @author
*		AnnaM
*/

// ERROR_FIX: some additional libraries are required
#include <Windows.h>
#include <stdio.h>
#include <string.h>     // for 'memset'
#include <new>          // for 'new'
#include <iostream>     // for 'cout'
#include <limits.h>     // for 'CHAR_BIT'
#include <stdint.h>     // for 'uintptr_t'
#include <inttypes.h>   // for 'PRIxPTR'

enum PAGE_COLOR
{
        // ERROR_FIX: begin with 0
	PG_COLOR_GREEN,         /* page may be released without high overhead */
	PG_COLOR_YELLOW,        /* nice to have */
	PG_COLOR_RED,	        /* page is actively used */
        PG_NUMB_OF_COLORS
        // ERROR_FIX: added 'NUMB_OF_COLORS' for convenience
};


/**
 * UINT Key of a page in hash-table (prepared from color and address)
 */
union PageKey
{
        // as I googled, for bit-field members of structure padding are off,
        //      unless they cross word boundaries (here crossing not presented)
        // http://www.catb.org/esr/structure-packing/#_bitfields
	struct
	{
        UINT        cColor: 8;      // ERROR_FIX: the standard suggests using 'integer' types for portability
	uintptr_t   cAddr: (sizeof (uintptr_t) * CHAR_BIT - 8); // ERROR_FIX: for support of x64 addresses;
	};                                                      //      'sizeof(char)' is always equal to 1,
                                                                //      'uintptr_t' is capable of storing any pointer
	uintptr_t   uKey;
};


/* Prepare from 2 chars the key of the same configuration as in PageKey */
// ERROR_FIX: parenthesis around <<
// ERROR_FIX: cast 'Addr' ptr to 'uintptr_t' to suppress an error
#define CALC_PAGE_KEY( Addr, Color )	( (uintptr_t)(Color) + ((uintptr_t)(Addr) << 8) )


/**
 * Descriptor of a single guest physical page
 */
struct PageDesc
{
// as this is C++, not C, there is no need for 'union' and 'struct' keywords before PageKey, PageDesc, etc.
// ERROR_FIX: uKey has a member uKey, renamed to 'pKey' to avoid ambiguity
	PageKey			pKey;

	/* list support */
	PageDesc		*next, *prev;
};

// ERROR_FIX: 'do {...} while (0)' wrapping
// ERROR_FIX: should be pKey.uKey
#define PAGE_INIT( Desc, Addr, Color )                          \
        do                                                      \
        {                                                       \
                (Desc).pKey.uKey = CALC_PAGE_KEY( Addr, Color );\
                (Desc).next      = (Desc).prev = NULL;          \
        }                                                       \
        while (0)


/* storage for pages of all colors */
// ERROR_FIX: changed 3 -> PG_NUMB_OF_COLORS
static PageDesc* PageStrg[ PG_NUMB_OF_COLORS ];

void PageStrgInit()
{
// as 'PageStrg' declared as const size array [],
//      sizeof(PageStrg) is equal to sum of sizes of all it's members
// ERROR_FIX: no '&' required
	memset( PageStrg, 0, sizeof(PageStrg) );
}

// ERROR_FIX: color type not 'char' but 'enum'
PageDesc* PageFind( void* ptr, PAGE_COLOR color )
// ERROR_FIX: C++ ensures compile time checking of enum parameters of function
//                   but for runtime checking let's do it manually
{
        if ((color < PG_COLOR_GREEN) || (PG_NUMB_OF_COLORS <= color))
                return NULL;    // MAYBE_ERROR: maybe not NULL but 'nullptr'?
                                // as everywhere in file NULL is used, let it be so

        // ERROR_FIX: deleted ';' after 'for'
	for( PageDesc* Pg = PageStrg[color]; Pg; Pg = Pg->next )
                // ERROR_FIX: should be pKey.uKey
                if( Pg->pKey.uKey == CALC_PAGE_KEY(ptr,color) )
                // ERROR_FIX: 'CALC_PAGE_KEY' macro fixed for x64 addresses
                        return Pg;
    return NULL;
}

// MAYBE_ERROR: no matter error happened or function succeeded, NULL is returned
PageDesc* PageReclaim( UINT cnt )
{
        // ERROR_FIX: not 'UINT' but 'enum', and not '0' but PG_COLOR_GREEN
	PAGE_COLOR color = PG_COLOR_GREEN;
        // ERROR_FIX: init Pg and check it's value
	PageDesc* Pg = PageStrg[color];
        if (Pg == NULL) return NULL;

	while( cnt )
	{
                // ERROR_FIX: should be 'Pg' (or maybe even '&Pg') rather than 'PageStrg[ color ]'
		PageRemove(Pg);
                // ERROR_FIX: changed order of operations: first remove, then ->next, then check
		Pg = Pg->next;
		cnt--;
		if( Pg == NULL )
		{
                        // ERROR_FIX: check boundaries
			if ((color + 1) < PG_NUMB_OF_COLORS)
                        {
                                color++;
                                Pg = PageStrg[ color ];
                        }
                        else
                                return Pg;
		}
	}
        // ERROR_FIX: 'return' statement added
        return Pg;
}

// ERROR_FIX: not 'UINT' but 'enum'
PageDesc* PageInit( void* ptr, PAGE_COLOR color )
// ERROR_FIX: check for 'color' boundaries
{
        if ((color < PG_COLOR_GREEN) || (PG_NUMB_OF_COLORS <= color))
                return NULL;

        // ERROR_FIX: 'new' throws exception
        try
        {
                PageDesc* pg = new PageDesc;
                PAGE_INIT(*pg, ptr, color);      // ERROR_FIX: '*' instead of '&' needed
                return pg;
        }
        catch (std::bad_alloc& ba)
        {
                std::cerr << "bad_alloc caught: " << ba.what() << '\n';
                return NULL;
        }
}

/**
 * Print all mapped pages
 */
void PageDump()
{
        // ERROR_FIX: not 'UINT' but 'enum', and not '0' but PG_COLOR_GREEN
	PAGE_COLOR color = PG_COLOR_GREEN;
	#define PG_COLOR_NAME(clr) #clr
	char* PgColorName[] =
	{
                // ERROR_FIX: corrected order of colors
		PG_COLOR_NAME(PG_COLOR_GREEN),
		PG_COLOR_NAME(PG_COLOR_YELLOW),
		PG_COLOR_NAME(PG_COLOR_RED)
	};

        // ERROR_FIX: as there is PG_NUMB_OF_COLORS exist, let's use it
	while( color < PG_NUMB_OF_COLORS )
	{
                // ERROR_FIX: corrected wrong order of 'printf' arguments
		printf("PgStrg[(%s) %u] ********** \n", PgColorName[color], (UINT)color );
                // ERROR_FIX: changed prefix to postfix incrementation
		for( PageDesc* Pg = PageStrg[color++]; Pg != NULL; Pg = Pg->next )
		{
                        // ERROR_FIX: == instead of =
                        // ERROR_FIX: pKey.uAddr
                        // ERROR_FIX: uAddr -> cAddr
                        // ERROR_FIX: .cAddr needs to be casted to ptr or compared to 0, not NULL
			if( Pg->pKey.cAddr == 0 )
				continue;

                        // ERROR_FIX: pKey.uKey & pKey.cAddr
                        // ERROR_FIX: because of using 'uintptr_t', in 'printf' we need to use 'PRIxPTR'
			printf("Pg :Key = 0x%" PRIxPTR ", addr %" PRIxPTR "\n", Pg->pKey.uKey, (uintptr_t)(Pg->pKey.cAddr) );
		}
	}
	#undef PG_COLOR_NAME
}
