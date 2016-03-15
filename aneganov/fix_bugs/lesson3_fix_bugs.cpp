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
#include <stdint.h> /* for uintptr_t -- @neganovalexey */

/* fixed: enum may start with 0 -- @neganovalexey */
enum PAGE_COLOR
{
	PG_COLOR_GREEN, /* page may be released without high overhead */
	PG_COLOR_YELLOW, /* nice to have */
	PG_COLOR_RED	/* page is actively used */
};


/**
 * UINT Key of a page in hash-table (prepared from color and address)
 */
union PageKey
{
	struct
	{
        UINT cColor: 8; /*fixed: the field may be UINT because of alignment -- @neganovalexey */
	UINT cAddr: 24;
	};

	UINT uKey;
};


/* Prepare from 2 chars the key of the same configuration as in PageKey */
/*fixed: parentheses -- @neganovalexey */
#define CALC_PAGE_KEY( Addr, Color )   ( (Color) +  ((Addr) << 8) ) 


/**
 * Descriptor of a single guest physical page
 */
struct PageDesc
{
	PageKey			uKey;	

	/* list support */
	PageDesc		*next, *prev;
};

/* fixed: multiple statements may be grouped into a single one there -- @neganovalexey */
/* fixed: operands of '=' -- @neganovalexey */
#define PAGE_INIT( Desc, Addr, Color )                   \
    do {                                                 \
        (Desc).uKey.uKey = CALC_PAGE_KEY( Addr, Color ); \ 
        (Desc).next      = (Desc).prev = NULL;           \
    }                                                    \
    while(0)
        

/* storage for pages of all colors */
static PageDesc* PageStrg[ 3 ];

void PageStrgInit()
{
	memset( PageStrg, 0, sizeof(PageStrg) ); /* fixed: unexpected ampersand -- @neganovalexey */
}

PageDesc* PageFind( void* ptr, char color )
{
	if(color < PG_COLOR_GREEN || color > PG_COLOR_RED) /* fixed: unchecked parameter -- @neganovalexey */
		return NULL;
	for( PageDesc* Pg = PageStrg[color]; Pg; Pg = Pg->next ) /*fixed: empty cycle -- @neganovalexey */
	{
        if( (Pg->uKey).uKey == CALC_PAGE_KEY( (UINT)ptr, (UINT)color) ) /* fixed: 1. operands of '==' 2. types -- @neganovalexey */
           return Pg;                                                                                                                                     
	}
    return NULL;
}

PageDesc* PageReclaim( UINT cnt )
{
	UINT color = PG_COLOR_GREEN; /* PG_COLOR_GREEN looks better than 0 -- @neganovalexey */
	PageDesc* Pg = PageStrg[color]; /* fixed: initialization -- @neganovalexey */
	if(Pg == NULL) return Pg;
	while( cnt-- ) /* fixed: possible dereferencing null pointer in cycle -- @neganovalexey */
	{
		if( Pg == NULL )
		{
			if (color < PG_COLOR_RED) /* fixed: overflow -- @neganovalexey */
			{
				color++;
				Pg = PageStrg[ color ];
			}
			else break;
		}

		Pg = Pg->next;
		PageRemove( PageStrg[ color ] );
	}

	return Pg; /* fixed: return from non-void function -- @neganovalexey */
}
            
PageDesc* PageInit( void* ptr, UINT color )
{
    try /* fixed: proper handling of an exception added -- @neganovalexey */
    {
    	PageDesc* pg = new PageDesc;
    	PAGE_INIT(*pg, ptr, color); /* fixed: star instead of ampersand -- @neganovalexey */
    	return pg;
    }
    catch( std::bad_alloc& ba )
    {
        fprintf(stderr, "Allocation has failed : %s\n", ba.what());
        return NULL;
    }
}

/**
 * Print all mapped pages
 */
void PageDump()
{
	UINT color = PG_COLOR_GREEN;
	#define PG_COLOR_NAME(clr) #clr
	char* PgColorName[] = 
	{
		PG_COLOR_NAME(PG_COLOR_GREEN), /*fixed: order -- @neganovalexey */
		PG_COLOR_NAME(PG_COLOR_YELLOW),
		PG_COLOR_NAME(PG_COLOR_RED)
	};

	while( color <= PG_COLOR_RED )
	{
		printf("PgStrg[(%s) %u] ********** \n", PgColorName[color], color); /*fixed: order -- @neganovalexey */
		for( PageDesc* Pg = PageStrg[color++]; Pg != NULL; Pg = Pg->next ) /*fixed: a postfix increment may be there -- @neganovalexey */
		{
			if( (Pg->uKey).cAddr == NULL ) /* fixed: 1. '==' instead of '='; 2. 'uKey.cAddr' instead of 'uAddr'  -- @neganovalexey */
				continue;

			printf("Pg :Key = 0x%x, addr %p\n", (Pg->uKey).uKey, (void *)(uintptr_t)(Pg->uKey).cAddr ); /* fixed: 1. 'uKey.cAddr' instead of 'cAddr' 2. explicit typecast to void * -- @neganovalexey */
		}
	}
	#undef PG_COLOR_NAME
}
