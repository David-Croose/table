#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "table.h"

static int32_t strcat_safe2(char *p1, uint32_t p1len, const char *p2) {
    uint32_t i;
    uint32_t p2len;

    if (!p1 || !p1len || !p2) {
        return -1;
    }
    if ((p2len = strlen(p2)) == 0) {
        return 0;
    }

    for (i = 0; i < p1len - 1 && p1[i]; i++);
    if (i >= p1len - 1) {
        return -2;
    }

    if (p2len > p1len - 1 - i) {
        return -3;
    }
    memcpy(&p1[i], p2, p2len);
    return p2len;
}

/**
 * append the source string to destination string, every append with a NULL terminated.
 * e.g:
 *      char buf[10] = {0};
 *      strcat_safe(buf, sizeof(buf), "_123");   // buf="_123", 0
 *      strcat_safe(buf, sizeof(buf), "_456");   // buf="_123", 0, "_456", 0
 * @param p1: the destination string
 * @param p1len: the length(in bytes) of @p1
 * @param p2: the source string
 * @return: <0  : failure
 *          >=0 : the appended bytes
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
 * get a string(e.g: {'1', '2', '3', 0, '4', '5', '6', 0, '7', '8', '9', 0}) by
 * specified index
 * @param h: a @table instance
 * @param index: the sequence number of the string(start with 0) in @h
 * @return: !NULL : the address of the string
 *          NULL  : the string can not be found
 */
static char *getitem(table_handle_t *h, uint32_t index) {
    uint32_t i;
    uint32_t find;

    if (!h) {
        return NULL;
    }

    if (index == 0) {
        return (h->pool[0] ? &h->pool[0] : NULL);
    }
    if (index > h->poollen / 2) {   // if index > max index
        return NULL;
    }

    for (find = 0, i = 1; i < h->poollen - 1; i++) {
        if (h->pool[i] == 0 && ++find == index) {
            return (h->pool[i + 1] ? &h->pool[i + 1] : NULL);
        }
    }
    return NULL;
}

/**
 * append a string to a buffer, if the string is shorter than a specify value, padding it
 * @param p: the buffer to be appended to
 * @param plen: the length(in bytes) of @p
 * @param item: the string append to @p
 * @param itemlen: the length(in bytes) of @item
 * @return: <0  : failure
 *          >=0 : the wrote bytes
 */
static int32_t addstring_pad(char *p, uint32_t plen, const char *item, uint32_t itemlen) {
    uint32_t i;
    char buf[CONFIG_ITEMSIZE];
    uint32_t space;
    uint32_t _itemlen;

    if (!p || !plen || !item || (itemlen < MINIMAL_ITEMSIZE || itemlen > CONFIG_ITEMSIZE)) {
        return -1;
    }

    p[plen - 1] = 0;
    for (i = 0; i < plen - 1 && p[i]; i++);
    if (i >= plen - 1) {                // if can't find '\0', which means p is full
        return -2;
    }

    memset(buf, ' ', sizeof(buf));
    space = itemlen - 1;                // the last character can not be filled, it must be ' ' or others
    _itemlen = strlen(item);
    if (_itemlen <= space) {
        memcpy(buf, item, _itemlen);
    } else {
        memcpy(buf, item, space);
        buf[space - 1] = '.';
        buf[space - 2] = '.';
        buf[space - 3] = '.';
    }

    if (itemlen > plen - 1 - i) {
        return -3;
    }
    memcpy(&p[i], buf, itemlen);
    return itemlen;
}

int32_t table_init(table_handle_t *h, char *mem, uint32_t memlen, int32_t totcol, const char *end) {
    uint32_t i;

    if (!h || !mem || !memlen || (!totcol || totcol > CONFIG_MAXCOLLEN) || !end) {
        return -1;
    }

    memset(mem, 0, memlen);
    memset(h, 0, sizeof(*h));
    for (i = 0; i < totcol; i++) {
        h->colwidth[i] = MINIMAL_ITEMSIZE;
    }
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
    static char buf[CONFIG_ITEMSIZE];
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
    int32_t writelen;

    if (!h || !mem || !memlen || !reallen) {
        return -1;
    }

    memset(mem, 0, memlen);
    *reallen = 0;

    for (i = 0, col = 0; (p = getitem(h, i)) != NULL; i++) {
        if ((writelen = addstring_pad(mem, memlen, p, h->colwidth[col])) < 0) {
            return -2;
        }
        *reallen += writelen;

        if (++col >= h->totcol) {
            if ((writelen = strcat_safe2(mem, memlen, h->end)) < 0) {
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
#define FAIL_IF(x)      if ((x) != 0) { goto error; }

char *str[] = {
    "123",
    "4567",
    "89",
    "ABCDEFG",
    "_abcd",
    "__xxx",
    "xyz0",
    "~789cdef",
    "!gg",
    "hjklopq",
    "22334",
    "zxc_89",
};

/// int main() {
///     char buf[20] = {0};
///     int i;
/// 
///     for (i = 0; i < sizeof(str) / sizeof(str[0]); i++) {
///         addstring_pad(buf, sizeof(buf), str[i]);
///     }
/// 
/// 
///     return 0;
/// }

int main() {
    table_handle_t h;
    static char buf[100];
    static char readbuf[sizeof(buf) * 3];
    uint32_t reallen;
    int i;

    FAIL_IF(table_init(&h, buf, sizeof(buf), 3, "\n"));
    for (i = 0; i < sizeof(str) / sizeof(str[0]); i++) {
        if (table_write(&h, str[i])) {
            break;
        }
    }
    FAIL_IF(table_read(&h, readbuf, sizeof(readbuf), &reallen));
    printf("%s\n", readbuf);
    table_destroy(&h);

    printf("ok\n");
    return 0;

error:
    printf("error\n");
    return -1;
}
//===========================================================================================

