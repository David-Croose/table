
#define CONFIG_MAXCOLLEN          (10)      // max colume length(in bytes)
#define CONFIG_MAXITEMLEN         (40)      // max item length(in bytes)


typedef struct {
    char *pool;
    uint32_t poollen;

    uint32_t totcol;
    uint32_t currcol;
    uint32_t colwidth[CONFIG_MAXCOLLEN];

    char end;                               // '\n' or '\r' or '\r\n'
} table_handle_t;


static int32_t strcat_safe(char *p1, uint32_t p1len, const char *p2) {
    uint32_t i;
    uint32_t space;
    uint32_t p2len;
    char *p;

    if (!p1 || !p1len || !p2 || !p2len) {
        return -1;
    }
    p1[p1len - 1] = 0;
    space = p1len - 1;
    p2len = strlen(p2);

    for (i = 0; i < space && p1[i]; i++);
    if (i >= space) {       // if can't find '\0', that means p1 is full
        return -2;
    }
    if (i == 0 || i == space - 1) {
        p = &p1[i];
    } else {
        p = &p1[i + 1];
    }
    p2len = space - i;
    memcpy(p, p2, p2len);
    *(p + p2len) = 0;

    return p2len;
}

static int32_t addstring(table_handle_t *h, uint32_t col, const char *buf, uint32_t buflen) {
    if (strcat_safe(h->pool, h->poolen, buf) < 0) {
        return -1;
    }
    if (buflen > h->colwidth[col]) {
        h->colwidth[col] = buflen;
    }
    return 0;
}

static char *getitem(table_handle_t *h, uint32_t index) {
    uint32_t i;
    uint32_t find;

    if (!h) {
        return NULL;
    }

    if (index == 0) {
        return &h->pool[0];
    }
    if (index > h->poollen / 2 - 1) {
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


}

int32_t table_init(table_handle_t *h, uint8_t *mem, uint32_t memlen, int32_t totcol, char end) {

}

int32_t table_add_seprow(table_handle_t *h, char sep) {


}


int32_t table_write(table_handle_t *h, const char *fmt, ...) {
    static char buf[CONFIG_MAXITEMLEN];
    int buflen;
    va_list args;

    memset(buf, 0, sizeof(buf));
    va_start(args, fmt);
    if ((buflen = vsnprintf(buf, sizeof(buf) - 1, fmt, args)) <= 0) {
        return -1;
    }
    va_end(args);

    if (++h->currcol >= h->totcol) {
        h->currcol = 0;
    }
    if (addstring(h, h->currcol, buf) < 0) {
        return -2;
    }
    return 0;
}


int32_t table_read(table_handle_t *h, char *mem, uint32_t memlen, uint32_t *reallen) {
    char *p;
    uint32_t i;
    uint32_t col;
    char end[2];
    int32_t writelen;

    if (!h || !mem || !memlen || !reallen) {
        return -1;
    }
    
    memset(mem, 0, memlen);
    *reallen = 0;
    end[0] = h->end;
    end[1] = 0;

    for (i = 0, col = 0; (p = getitem(h, i)) != NULL; i++) {
        if ((writelen = addstring_pad(mem, memlen, p, h->colwidth[col])) < 0) {
            return -2;
        }
        *reallen += writelen;

        if (++col >= h->totcol) {
            if ((writelen = strcat_safe(mem, memlen, end, sizeof(end))) < 0) {
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


