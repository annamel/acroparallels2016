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
	PG_COLOR_GREEN = 0, /* page may be released without high overhead */
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
		UINT	cAddr: 24;
        CHAR	cColor: 8;
	};

	UINT	uKey;
};


/* Prepare from 2 chars the key of the same configuration as in PageKey */
#define CALC_PAGE_KEY( Addr, Color )	((Addr << 8) + Color)
//No idea how it should like, so i guess that will be like this
//Adress 0xAABBCC - 24bit Addr, 0xAA - 8bit Color, output - 32bit key
// 0xAABBCC << 8 -> 0xAABBCC00 + 0xAA -> 0xAABBCCAA


/**
 * Descriptor of a single guest physical page
 */
struct PageDesc
{
	PageKey			uKey;

	/* list support */
	PageDesc		*next, *prev;
};

#define PAGE_INIT( Desc, Addr, Color )              \
    do {                                            \
        (Desc).uKey = CALC_PAGE_KEY( Addr, Color ); \
        (Desc).next = (Desc).prev = NULL;           \
    } while(0)


/* storage for pages of all colors */
static PageDesc* PageStrg[ 3 ];

void PageStrgInit()
{
	memset( PageStrg, 0, sizeof(PageDesc*) * 3 );
}

PageDesc* PageFind( void* ptr, char color )
{
    if(color < PG_COLOR_GREEN || color > PG_COLOR_RED)
        return NULL;

	for( PageDesc* Pg = PageStrg[color]; Pg; Pg = Pg->next )
        if( Pg->uKey == (UINT)CALC_PAGE_KEY(ptr,color) )
           return Pg;

    return NULL;
}

PageDesc* PageReclaim( UINT cnt )
{
	UINT color = 0;
	PageDesc* Pg = PageStrg[0];

	while( cnt )
    {
        if( !Pg )
        {
            if( ++color > PG_COLOR_RED );
                return NULL; //No idea, what it should return as PageDesc* if not enough pages reclaimed

            Pg = PageStrg[ color ];
        }

		Pg = Pg->next;
		PageRemove( PageStrg[ color ] ); //Here I assume that PageRemove is smart enough to write next page adress
                                         //back into PageStrg[ color ], so it wouldn't crash(or have no result) if i call it twice
                                         //If so, why its not called RemoveTopPage or somelike? No idea
		cnt--;
	}


	return Pg;   //Since return was not stated and return type is PageDesc*
                 //I have ABSOLUTELY no idea, what should it return even in success case
                 //So it will return PG (Wow, such solution, who need not reclamed page adress after reclaiming?)
}

PageDesc* PageInit( void* ptr, UINT color )
{
    try
    {
        PageDesc* pg = new PageDesc;
    }
    catch(...) //No sure about sintax to catch any exception
    {
        return NULL;
    }

    if( pg )
        PAGE_INIT(*pg, ptr, color);
    else
        printf("Allocation has failed\n");
    return pg;
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
		printf("PgStrg[(%u) %s] ********** \n", (unsigned)color, PgColorName[color]);
        for( PageDesc* Pg = PageStrg[color++]; Pg != NULL; Pg = Pg->next )
        {
			if( Pg->uAddr == NULL )
				continue;

			printf("Pg :Key = 0x%x, addr %p\n", Pg->uKey, Pg->uAddr ); //Now idea if %x and printf can handle uKey. Assume it can.
		}
	}
	#undef PG_COLOR_NAME
}
