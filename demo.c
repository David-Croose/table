#include <stdio.h>
#include "table.h"

int main(void) {
#define FAIL_IF(x)      if ((x) != 0) { goto error; }
    table_handle_t h;
    static char buf[100];
    static char readbuf[sizeof(buf) * 3];
    uint32_t reallen;
    int i;
    static const char *str[] = {
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
        "11111",
        "222",
        "234R",
        "sadfhi",
        "9845+",
        "+234:",
        "asvb-3",
        "zzhfg",
    };

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
