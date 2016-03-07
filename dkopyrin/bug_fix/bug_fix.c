/**
* @brief
*		Find errors and decrease probability of getting errors of the same kind in the future
*		This piece of code won't compile and it doesn't describe an entire algorithm: just part of some page storage
*
* @author
*		AnnaM
*/

//#include <Windows.h>
#include <new>

#include <stdio.h>

enum PAGE_COLOR
{
	PG_COLOR_GREEN,  /* page may be released without high overhead */
	PG_COLOR_YELLOW, /* nice to have */
	PG_COLOR_RED,	 /* page is actively used */
	PG_COLOR_END	 /* added for scalability */
};


/**
 * UINT Key of a page in hash-table (prepared from color and address)
 */
union PageKey
{
	struct
	{
		UINT	cColor: 8;
		UINT	cAddr: 8*sizeof(UINT) - 8;
	};

	UINT	uKey;
};


/* Prepare from 2 chars the key of the same configuration as in PageKey */
#define CALC_PAGE_KEY( Addr, Color )	((Color) + ((Addr)<< 8 ))


/**
 * Descriptor of a single guest physical page
 */
struct PageDesc
{
	union PageKey			uKey;

	/* list support */
	struct PageDesc		*next, *prev;
};

#define PAGE_INIT( Desc, Addr, Color )                   		\
	do								\
	{                                             			\
		(Desc).uKey.uKey = CALC_PAGE_KEY( Addr, Color );  	\
		(Desc).next      = (Desc).prev = NULL;            	\
	}								\
	while(0);


/* storage for pages of all colors */
static struct PageDesc* PageStrg[PG_COLOR_END];

void PageStrgInit()
{
	memset(PageStrg, 0, sizeof(PageStrg));
}

struct PageDesc* PageFind( void* ptr, UINT color )
{
	if (ptr == NULL || color >= PG_COLOR_END)
		return NULL;

	for( struct PageDesc* Pg = PageStrg[color]; Pg; Pg = Pg->next )
		if( Pg->uKey.uKey == CALC_PAGE_KEY((UINT) ptr, color) )
			return Pg;

	return NULL;
}

PageDesc* PageReclaim( UINT cnt )
{
	UINT color = PG_COLOR_GREEN;
	PageDesc* Pg = PageStrg[color];
	while( cnt )
	{
		if( Pg == NULL )
		{
			if (color < PG_COLOR_END - 1)
				color++;
			Pg = PageStrg[ color ];
		}

		Pg = Pg->next;
		PageRemove( PageStrg[ color ] );
		cnt--;
	}

	return Pg;
}

PageDesc* PageInit( void* ptr, UINT color )
{
	if (ptr == NULL || color < 0 || color >= PG_COLOR_END)
		return NULL;

  	try
  	{
		PageDesc* pg = new PageDesc;
		PAGE_INIT(*pg, (UINT) ptr, color);
		return pg;
	}
	catch(std::bad_alloc& ba)
	{
		printf("Allocation has failed\n");
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
		PG_COLOR_NAME(PG_COLOR_YELLOW),
		PG_COLOR_NAME(PG_COLOR_RED)
	};

	while( color <= PG_COLOR_RED )
	{
		printf("PgStrg[(%s) %u] ********** \n", PgColorName[color], color );
		for( PageDesc* Pg = PageStrg[color++]; Pg != NULL; Pg = Pg->next )
		{
			if( Pg->uAddr == NULL )
				continue;

			printf("Pg :Key = 0x%x, addr %p\n", (Pg->uKey).uKey, (void *)(Pg->uKey).cAddr );
		}
	}
	#undef PG_COLOR_NAME
}
