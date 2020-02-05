#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "table.h"

/**
 * append the source string to destination string, every append with a NULL terminated.
 * e.g:
 *      char buf[10] = {0};
 *      strcat_safe(buf, sizeof(buf), "_123");   // buf="_123", 0
 *      strcat_safe(buf, sizeof(buf), "_456");   // buf="_123", 0, "_456", 0
 * @param: p1. the destination string
 * @param: p1len. the length(in bytes) of @p1
 * @param: p2. the source string
 * @return: <0 : failure
 *          >=0: the appended bytes
 */
static int32_t strcat_safe(char *p1, uint32_t p1len, const char *p2) {
    uint32_t i, j;
    uint32_t space;
    uint32_t p2len;

    if (!p1 || !p1len || !p2) {
        return -1;
    }

    p1[p1len - 1] = 0;
    if (p1[0] == 0) {
        j = 0;
    } else {
        for (i = 1; i < p1len - 2 && (p1[i] || p1[i + 1]); i++);      // find "00"
        j = i + 1;
    }
    space = p1len - 1 - j;

    p2len = strlen(p2);
    p2len = (p2len > space ? space : p2len);
    memcpy(&p1[j], p2, p2len);
    p1[j + p2len] = 0;
    return p2len;
}

static int32_t addstring(table_handle_t *h, uint32_t col, const char *buf) {
    uint32_t buflen = strlen(buf);

    if (strcat_safe(h->pool, h->poollen, buf) < 0) {
        return -1;
    }
    if (buflen > h->colwidth[col]) {
        h->colwidth[col] = buflen;
    }
    return 0;
}

/**
 * get a string from the specify string
 *
 */
static char *getitem(table_handle_t *h, uint32_t index) {
    uint32_t i;
    uint32_t find;

    if (!h) {
        return NULL;
    }

    if (index == 0) {
        return h->pool;
    }
    if (index > h->poollen / 2) {
        return NULL;
    }

    for (find = 0, i = 1; i < h->poollen - 1; i++) {
        if (h->pool[i] == 0) {
            if (++find == index) {
                return &h->pool[i + 1];
            }
        }
    }
    return NULL;
}

static int32_t addstring_pad(char *p, uint32_t plen, const char *item, uint32_t itemlen) {
    uint32_t i;
    char buf[CONFIG_MAXITEMLEN];
    uint32_t space;
    uint32_t _itemlen;

    if (!p || !plen || !item || !itemlen) {
        return -1;
    }

    p[plen - 1] = 0;
    for (i = 0; i < plen - 1 && p[i]; i++);
    if (i >= plen - 1) {                // if can't find '\0', which means p is full
        return -2;
    }

    memset(buf, ' ', sizeof(buf));
    space = itemlen - 1;
    _itemlen = strlen(item);
    if (_itemlen <= space) {
        memcpy(&p[i], item, _itemlen);
    } else {
        memcpy(&p[i], item, space);
        p[space - 1] = '.';
        p[space - 2] = '.';
        p[space - 3] = '.';
    }

    return 0;
}

int32_t table_init(table_handle_t *h, uint8_t *mem, uint32_t memlen, int32_t totcol, const char *end) {
    if (!h || !mem || !memlen || !totcol || !end) {
        return -1;
    }

    memset(mem, 0, memlen);
    memset(h, 0, sizeof(*h));
    h->pool = (char *)mem;
    h->poollen = memlen;
    h->totcol = totcol;
    memcpy(h->end, end, 2);
    return 0;
}

int32_t table_add_seprow(table_handle_t *h, char sep) {
    uint32_t i, j;
    uint32_t sum;
    uint32_t space = h->poollen - 1;
    char *p;

    for (p = h->pool, i = 0; i < space && p[i]; i++);
    if (i >= space) {       // if can't find '\0', that means p is full
        return -1;
    }

    for (sum = 0, j = 0; j < h->totcol; j++) {
        sum += h->colwidth[j];
    }
    if (i + sum + strlen(h->end) > space) {
        return -2;
    }

    for (j = 0; j < sum; j++) {
        h->pool[i + j] = sep;
    }
    strcat(h->pool, h->end);
    return 0;
}

int32_t table_write(table_handle_t *h, const char *fmt, ...) {
    static char buf[CONFIG_MAXITEMLEN];
    va_list args;

    memset(buf, 0, sizeof(buf));
    va_start(args, fmt);
    if (vsnprintf(buf, sizeof(buf) - 1, fmt, args) <= 0) {
        return -1;
    }
    va_end(args);

    if (addstring(h, h->currcol, buf) < 0) {
        return -2;
    }
    if (++h->currcol >= h->totcol) {
        h->currcol = 0;
    }
    return 0;
}

int32_t table_read(table_handle_t *h, char *mem, uint32_t memlen, uint32_t *reallen) {
    char *p;
    uint32_t i;
    uint32_t col;
    char end[3];
    int32_t writelen;

    if (!h || !mem || !memlen || !reallen) {
        return -1;
    }

    memset(mem, 0, memlen);
    *reallen = 0;
    memcpy(end, h->end, sizeof(end));

    for (i = 0, col = 0; (p = getitem(h, i)) != NULL; i++) {
        if ((writelen = addstring_pad(mem, memlen, p, h->colwidth[col])) < 0) {
            return -2;
        }
        *reallen += writelen;

        if (++col >= h->totcol) {
            if ((writelen = strcat_safe(mem, memlen, end)) < 0) {
                return -3;
            }
            *reallen += writelen;
            col = 0;
        }
    }

    return 0;
}

void table_destroy(table_handle_t *h) {
    if (!h) {
        return;
    }

    memset(h->pool, 0, h->poollen);
    memset(h, 0, sizeof(*h));
}

//===========================================================================================
int main() {
    char buf[20] = {0};
    int i;
    int res;

    for (i = 0; i < 20; i++) {
        res = strcat_safe(buf, sizeof(buf), "_");
        printf("buf=%s, res=%d\n", buf, res);
    }

    return 0;
}
//===========================================================================================

