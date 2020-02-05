#include <stdint.h>

#define CONFIG_MAXCOLLEN          (10)      // max colume length(in bytes)
#define CONFIG_MAXITEMLEN         (40)      // max item length(in bytes)

typedef struct {
    char *pool;
    uint32_t poollen;

    uint32_t totcol;
    uint32_t currcol;
    uint32_t colwidth[CONFIG_MAXCOLLEN];

    char end[3];                            // "\n" or "\r" or "\r\n"
} table_handle_t;

int32_t table_init(table_handle_t *h, uint8_t *mem, uint32_t memlen, int32_t totcol, const char *end);
int32_t table_add_seprow(table_handle_t *h, char sep);
int32_t table_write(table_handle_t *h, const char *fmt, ...);
int32_t table_read(table_handle_t *h, char *mem, uint32_t memlen, uint32_t *reallen);
void table_destroy(table_handle_t *h);

