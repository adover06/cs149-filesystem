/*
 * main.c - command loop (Andrew Dover)
 * Reads one command per line and calls the functions in filesystem.h.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h"

static void error(const char *msg)
{
    printf("Error: %s\n", msg);
}

int main(void)
{
    char line[1024], cmd[16], arg1[16], arg2[16];

    if (!checkSizes()) {
        printf("Wrong structure sizes: dir entry %d (need 20), union %d (need 512)\n",
               (int)sizeof(sector[0].directory.dir[0]), (int)sizeof(union block));
        return 1;
    }
    initDisk();

    while (1) {
        printf("> ");
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL)
            break;
        line[strcspn(line, "\n")] = '\0';

        cmd[0] = arg1[0] = arg2[0] = '\0';
        sscanf(line, "%15s %15s %15s", cmd, arg1, arg2);
        if (cmd[0] == '\0')
            continue;

        if (strcmp(cmd, "CREATE") == 0) {
            if (strlen(arg2) == 0 || strlen(arg2) > NAME_LEN)
                error("usage: CREATE U|D name (name up to 8 chars)");
            else if (fsCreate(arg1[0], arg2) < 0)
                error("CREATE failed");

        } else if (strcmp(cmd, "DELETE") == 0) {
            if (fsDelete(arg1) < 0)
                error("DELETE failed");

        } else if (strcmp(cmd, "OPEN") == 0) {
            if (fsOpen(arg1[0], arg2) < 0)
                error("OPEN failed");

        } else if (strcmp(cmd, "CLOSE") == 0) {
            if (fsClose() < 0)
                error("CLOSE failed");

        } else if (strcmp(cmd, "READ") == 0) {
            int n = atoi(arg1);
            char *buf = malloc(n + 1);
            int got = fsRead(n, buf);
            if (got < 0) {
                error("READ failed");
            } else {
                buf[got] = '\0';
                printf("%s\n", buf);
            }
            free(buf);

        } else if (strcmp(cmd, "WRITE") == 0) {
            /* WRITE n data -- data is everything after "WRITE n ", spaces kept */
            char *p = line + strlen("WRITE");
            char *data;
            int n;
            while (*p == ' ')
                p++;
            n = (int)strtol(p, &data, 10);
            if (*data == ' ')
                data++;
            if (n <= 0 || (int)strlen(data) < n)
                error("usage: WRITE n data (need at least n bytes of data)");
            else if (fsWrite(n, data) < n)
                error("WRITE failed or disk full");

        } else if (strcmp(cmd, "SEEK") == 0) {
            if (fsSeek(atoi(arg1)) < 0)
                error("SEEK failed");

        } else if (strcmp(cmd, "DIR") == 0) {
            fsDir();

        } else if (strcmp(cmd, "CD") == 0) {
            if (fsCd(arg1) < 0)
                error("CD failed");

        } else if (strcmp(cmd, "ROOT") == 0) {
            fsRoot();

        } else if (strcmp(cmd, "EXIT") == 0) {
            break;

        } else {
            error("unknown command");
        }
    }

    freeAllNodes();
    return 0;
}
