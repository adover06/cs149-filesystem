#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h"

int main(void)
{
    char line[1024], cmd[1024], arg1[1024], arg2[1024];
    char *buf;
    int n, i, got, len;

    if (!checkSizes()) {
        printf("Error: wrong structure sizes\n");
        return 1;
    }
    initDisk();

    while (1) {
        printf("> ");
        if (fgets(line, sizeof(line), stdin) == NULL)
            break;
        len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        cmd[0] = '\0';
        arg1[0] = '\0';
        arg2[0] = '\0';
        sscanf(line, "%s %s %s", cmd, arg1, arg2);
        if (cmd[0] == '\0')
            continue;

        if (strcmp(cmd, "CREATE") == 0) {
            if (strlen(arg2) == 0 || strlen(arg2) > NAME_LEN)
                printf("Error: bad name\n");
            else if (fsCreate(arg1[0], arg2) < 0)
                printf("Error: CREATE failed\n");

        } else if (strcmp(cmd, "DELETE") == 0) {
            if (fsDelete(arg1) < 0)
                printf("Error: DELETE failed\n");

        } else if (strcmp(cmd, "OPEN") == 0) {
            if (fsOpen(arg1[0], arg2) < 0)
                printf("Error: OPEN failed\n");

        } else if (strcmp(cmd, "CLOSE") == 0) {
            if (fsClose() < 0)
                printf("Error: CLOSE failed\n");

        } else if (strcmp(cmd, "READ") == 0) {
            n = atoi(arg1);
            if (n <= 0) {
                printf("Error: READ failed\n");
            } else {
                buf = malloc(n + 1);
                got = fsRead(n, buf);
                if (got < 0) {
                    printf("Error: READ failed\n");
                } else {
                    buf[got] = '\0';
                    printf("%s\n", buf);
                }
                free(buf);
            }

        } else if (strcmp(cmd, "WRITE") == 0) {
            n = atoi(arg1);
            i = 0;
            while (line[i] == ' ')
                i++;
            i = i + 5;
            while (line[i] == ' ')
                i++;
            while (line[i] >= '0' && line[i] <= '9')
                i++;
            if (line[i] == ' ')
                i++;
            if (n <= 0 || strlen(&line[i]) < n)
                printf("Error: not enough data\n");
            else if (fsWrite(n, &line[i]) < n)
                printf("Error: WRITE failed\n");

        } else if (strcmp(cmd, "SEEK") == 0) {
            if (fsSeek(atoi(arg1)) < 0)
                printf("Error: SEEK failed\n");

        } else if (strcmp(cmd, "DIR") == 0) {
            fsDir();

        } else if (strcmp(cmd, "CD") == 0) {
            if (fsCd(arg1) < 0)
                printf("Error: CD failed\n");

        } else if (strcmp(cmd, "ROOT") == 0) {
            fsRoot();

        } else if (strcmp(cmd, "EXIT") == 0) {
            break;

        } else {
            printf("Error: unknown command\n");
        }
    }

    freeAllNodes();
    return 0;
}
