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
#include <string.h>
#include <new>
// TODO: bug 0: required string.h or memory.h library to use memsest function and new to use new - perhaps, not a bug

enum PAGE_COLOR {
    // TODO: bug 1: change enum numeration, now it starts with 0
    PG_COLOR_GREEN, /* page may be released without high overhead */
    PG_COLOR_YELLOW, /* nice to have */
    PG_COLOR_RED, /* page is actively used */
    PAGE_COLOR_SIZE // TODO: bug 2: added ENUM SIZE constant to make assert
};

/**
 * UINT Key of a page in hash-table (prepared from color and address)
 */
union PageKey {
    struct {
        UINT cColor: 8; // TODO: bug 3: change data type: it's old bytefield sized 8 wasn't good with CHAR
        UINT cAddr: 8*sizeof(UINT) - 8; // TODO: bug 4: calculate field size to reach predictible behavior
    };

    UINT uKey;
};


/* Prepare from 2 chars the key of the same configuration as in PageKey */
#define CALC_PAGE_KEY(Addr, Color)  do { ((Color) + ((Addr) << 8)) } while (0)
// TODO: bug 5: added brackets to fix operator priority


/**
 * Descriptor of a single guest physical page
 */
struct PageDesc {
    PageKey uKey;

    /* list support */
    PageDesc *next, *prev;
};

// TODO: bug 6: fixed define internals. It should be conluded in do {} while (0) construction
#define PAGE_INIT(Desc, Addr, Color)                    \
do {                                                    \
    (Desc).uKey.uKey = CALC_PAGE_KEY((UINT)Addr, Color ); \
    (Desc).next = (Desc).prev = nullptr;                \
} while (0)
// TODO: bug 7: (Desc).uKey was only struct PageKey


/* storage for pages of all colors */
static PageDesc *PageStrg[PAGE_COLOR_SIZE]; // TODO: bug 2: scalable declaration

void PageStrgInit() {
    memset(PageStrg, 0,PAGE_COLOR_SIZE * sizeof(*PageStrg));        //TODO: bug 8 fix sizeof() argument
}

PageDesc *PageFind(void *ptr, UINT color) {
    if (color >= PAGE_COLOR_SIZE)
        return nullptr;
    // TODO: bug 9: preventing out of array reading

    // TODO: bug 10: extra ; after loop
    for (PageDesc *Pg = PageStrg[color]; Pg; Pg = Pg->next)
        if (Pg->uKey.uKey == CALC_PAGE_KEY((UINT)ptr, color))       // TODO: bug 11: ==; TODO: bug 12: CALC_PAGE_KEY argument bringing
            return Pg;

    return nullptr;
}

PageDesc *PageReclaim(UINT cnt) {
    UINT color = 0;
    PageDesc *Pg = PageStrg[0]; // TODO: bug 13: not inited pointer
    // TODO: bug 14: what if Pg already NULL?!
    while (cnt--) {
        if (!Pg) {
            if (++color >= PAGE_COLOR_SIZE)
                break;
            // TODO: bug 15: checked color out of range
            Pg = PageStrg[color];
        }
        if (Pg) {
            Pg = Pg->next;
            PageRemove(PageStrg[color]);
        }
    }
    return Pg; // TODO: bug 15: return page
}

PageDesc *PageInit(void *ptr, UINT color) {
    if (color >= PAGE_COLOR_SIZE)
        return nullptr;
    // TODO: bug 8: preventing out of array reading

    try {
        PageDesc *pg = new PageDesc;
        PAGE_INIT(*pg, ptr, color); // TODO: bug 15: chage & to *
        return pg;
    }
    catch (std::bad_alloc& ba) {
        fprintf(stderr, "bad_alloc caught: %s\n", ba.what());
        return nullptr;
    }
    // TODO: bug 16: new sends exeption, should catch it :(
}

/**
 * Print all mapped pages
 */
void PageDump() {
    UINT color = 0;
    #define PG_COLOR_NAME(clr) #clr
    char *PgColorName[] = {
                    PG_COLOR_NAME(PG_COLOR_GREEN),
                    PG_COLOR_NAME(PG_COLOR_YELLOW),
                    PG_COLOR_NAME(PG_COLOR_RED)};
    // TODO: bug 17: wrong color order

    while (color <= PG_COLOR_RED) {
        printf("PgStrg[(%s) %u] ********** \n", PgColorName[color], color);  // TODO: bug 18: wrong fmtstring arg order
        for (PageDesc *Pg = PageStrg[color++]; Pg != nullptr; Pg = Pg->next) {  // TODO: bug 17: pistfix ++
            if ((Pg->uKey).cAddr == nullptr)     // TODO: bug 18: == , not a =; TODO: bug 19: uAddr is not exists, so use (Pg->uKey).cAddr
                continue;

                printf("Pg :Key = 0x%x, addr %p\n", (UINT)Pg->uKey.uKey, (void *)(Pg->uKey).cAddr); // TODO: bug 19: uAddr is not exists, so use (Pg->uKey).cAddr
                // TODO: bug 20: argument bringing, as it's bit field
        }
    }
#undef PG_COLOR_NAME
}
