#include <stdint.h>

// max colume length(in bytes)
#define CONFIG_MAXCOLLEN          (10)

// max item length(in bytes)
#define CONFIG_ITEMSIZE           (30)

#define MINIMAL_ITEMSIZE          (5)

#if (CONFIG_ITEMSIZE < MINIMAL_ITEMSIZE)
#error 'CONFIG_ITEMSIZE' is wrong
#endif

typedef struct {
    char *pool;                             // the memory pool stores the string
    uint32_t poollen;                       // the length(in bytes) of @pool

    uint32_t totcol;                        // total column
    uint32_t currcol;                       // current column(range: [0, totcol - 1)
    uint32_t colwidth[CONFIG_MAXCOLLEN];    // the width(in bytes) of each column

    char end[3];                            // the line end character. can be: "\n", "\r" or "\r\n"

    int32_t title;                          // 1: title is active; 0: title is deactive
} table_handle_t;                           // the prototype of a @table instance

int32_t table_init(table_handle_t *h, char *mem, uint32_t memlen, int32_t totcol, const char *end, const char *title[]);
int32_t table_write(table_handle_t *h, const char *fmt, ...);
int32_t table_read(table_handle_t *h, char *mem, uint32_t memlen, uint32_t *reallen);
void table_destroy(table_handle_t *h);

