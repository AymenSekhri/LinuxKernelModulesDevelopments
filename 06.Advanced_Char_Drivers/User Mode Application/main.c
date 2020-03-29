#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    int fd;
    char buf[128];

    int decission = 0;
    printf("\nWhat do you want ?\n[0] for Writing\n[1] for reading\nI want: ");
    scanf("%d", &decission);
    if (decission == 0) {
        //Wrinting
        fd = open("/dev/char0", O_WRONLY);
        if (fd == -1) {
            perror("File cannot be opened");
            return EXIT_FAILURE;
        }
        printf("Eneter what you want to write:");
        scanf(" %[^\n]s", buf);
        int wr_num = write(fd, buf, strlen(buf));
        if (wr_num == strlen(buf)) {
            printf("Written Completelly, %d bytes has been written.\n", wr_num);
        } else {
            printf("Partial Write, %d bytes has been written.\n", wr_num);
        }
        close(fd);
    } else if (decission == 1) {
        //Reading
        fd = open("/dev/char0", O_RDONLY);
        if (fd == -1) {
            perror("File cannot be opened");
            return EXIT_FAILURE;
        }
        int rd_num = read(fd, buf, 128);
        if (rd_num == 0) {
            printf("Nothing to read.\n");
        } else {
            printf("Returned %d bytes, buf= \"%s\"\n", rd_num, buf);
        }
        close(fd);
    }
    printf("Good Byte\n");
    return 0;
}
