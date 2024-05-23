/** user_test.c
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 163840
static const char * const task_state_array[] = {// mainde for dongusunde kullandım.
    "R","P","D","T","X","t","Z","I","S",
};

void read_proc_file(const char *file_path) {
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytesRead);
    }

    close(fd);
}

void write_proc_file(const char *file_path, const char *data) {
    int fd = open(file_path, O_WRONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    size_t len = strlen(data);
    ssize_t bytesWritten = write(fd, data, len);

    if (bytesWritten != len) {
        perror("Error writing to file");
        exit(EXIT_FAILURE);
    }

    close(fd);
}

int main() {
    const char *file_path = "/proc/mytaskinfo";

    int i =0;
    for(i=0;i<8;i++){
      write_proc_file(file_path, task_state_array[i]);
      read_proc_file(file_path);
      }
    return 0;
}
