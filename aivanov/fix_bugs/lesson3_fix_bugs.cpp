/**
* @brief
*		Find errors and decrease probability of getting errors of the same kind in the future
*		This piece of code won't compile and it doesn't describe an entire algorithm: just part of some page storage
*
* @author
*		AnnaM
*/


/*Илья Кротов <ilya.krotov@phystech.edu>,*/

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//typedef unsigned UINT;
//typedef char CHAR;


//FIXME: begin with null
enum PAGE_COLOR
{
	PG_COLOR_GREEN = 0, /* page may be released without high overhead */
	PG_COLOR_YELLOW, /* nice to have */
	PG_COLOR_RED,	/* page is actively used */
	PG_COLOR_END
};


/**
 * UINT Key of a page in hash-table (prepared from color and address)
 */
union PageKey
{
	struct
	{
		//FIXME:alignment!!!!!
        UINT	cColor: 8;
		UINT	cAddr: 24;
	};

	UINT	uKey; //FIXME: explicit size
};


/* Prepare from 2 chars the key of the same configuration as in PageKey */
//FIXME: big endian?

//FIXME:typecast
//FIXME operator priority, size check?
#define CALC_PAGE_KEY( Addr, Color )	(  (Color) + (((unsigned) Addr) << 8 )) 


/**
 * Descriptor of a single guest physical page
 */
struct PageDesc
{
	PageKey			uKey;	

	/* list support */
	PageDesc		*next, *prev;
};


/*FIXME: wrong union use*/
/*FIXME: Desc is a pointer*/
/*FIXME: Use with if-else*/
#define PAGE_INIT( Desc, Addr, Color )              \
do													\
{                                               \
    (Desc)->uKey.uKey = CALC_PAGE_KEY( Addr, Color );\
    (Desc)->next = (Desc)->prev = NULL;           \
}												\
while (0)
        

/* storage for pages of all colors */
static PageDesc* PageStrg[ 3 ];

void PageRemove(PageDesc*);

void PageStrgInit()
{
	//FIXME: sizeof PageStrg?
	memset( PageStrg, 0, sizeof PageStrg);
}

PageDesc* PageFind( void* ptr, UINT color )
{
	if (PG_COLOR_END >= PG_COLOR_END)
		return NULL;
		
//FIXME: extra semicolon 
	for( PageDesc* Pg = PageStrg[color]; Pg; Pg = Pg->next )
//FIXME: union use as integer
        if( Pg->uKey.uKey == CALC_PAGE_KEY(ptr,color) )
           return Pg;                                                                                                                                     
    return NULL;
}

PageDesc* PageReclaim( UINT cnt )
{
	UINT color = 0;
	//FIXME: init
	PageDesc* Pg = PageStrg[ PG_COLOR_GREEN ];
	while( cnt )
	{
		Pg = Pg->next;
		PageRemove( PageStrg[ color ] );
		cnt--;
		if( Pg == NULL )
		{
			color++;
			
			if (color >= PG_COLOR_END)
				return NULL;
			
			Pg = PageStrg[ color ];
		}
	}
	
	return Pg;
}
            
PageDesc* PageInit( void* ptr, UINT color )
{
	//most likely exceptions will be disabled anyway
	
	if (color >= PG_COLOR_END)
		return NULL;
	
	try
	{
   		PageDesc* pg = new PageDesc;
   	}
   	catch(...)
   	{
        printf("Allocation has failed (exception)\n");
	}
	
    if( pg )
    //FIXME: *, not &
        PAGE_INIT(pg, ptr, color);
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
	//FIXME: typecsast discarding const
	const char* PgColorName[] = 
	{
		PG_COLOR_NAME(PG_COLOR_GREEN),
		PG_COLOR_NAME(PG_COLOR_YELLOW),
		PG_COLOR_NAME(PG_COLOR_RED)
	};

	while( color < PG_COLOR_END )
	{
		//FIXME:order
		printf("PgStrg[(%s) %u] ********** \n", PgColorName[color], color );
		for( PageDesc* Pg = PageStrg[color++]; Pg != NULL; Pg = Pg->next )
		{
			//FIXME assignment-comparison
			//FIXME no such member
			if(! Pg->uKey.cAddr )
				continue;

			/*FIXME union parts*/
			printf("Pg :Key = 0x%x, addr %X\n", Pg->uKey.uKey, (unsigned) Pg->uKey.cAddr );
		}
	}
	#undef PG_COLOR_NAME
}

