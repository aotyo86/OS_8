#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(int ac,char *av[]) {
    int from = -1 ;
    int to = -1 ;
    caddr_t from_mmap;
    off_t i;
    off_t size;
    struct stat sb;

    if(ac!=3)  {
        fprintf(stderr,"Usage: %s from_file to_file\n",av[0]);
        exit(EXIT_FAILURE);
    }

    if((from=open(av[1], O_RDONLY)) < 0)  {
        perror("Can't open from file");
        exit(EXIT_FAILURE);
    }

    if(fstat(from, &sb) < 0) {
        perror("Can't fstat from file");
        exit(EXIT_FAILURE);
    }

    size = sb.st_size;

    if((to=open(av[2], O_CREAT|O_RDWR, 0666)) < 0)  {
        perror("Can't open to file");
        exit(EXIT_FAILURE);
    }

    if(lseek(to, size-sizeof(int), SEEK_SET) < 0) {
        perror("Error seeking to file position");
        exit(EXIT_FAILURE);
    }

    if(write(to, &i, sizeof(int)) < 0) {
        perror("Error writing to file");
        exit(EXIT_FAILURE);
    }

    from_mmap = mmap(NULL, size, PROT_READ, MAP_PRIVATE, from, 0);
    if(from_mmap == MAP_FAILED) {
        perror("Can't mmap from file");
        exit(EXIT_FAILURE);
    }

    if(write(to, from_mmap, size) < 0) {
        perror("Error writing to file");
        exit(EXIT_FAILURE);
    }

    if(munmap(from_mmap, size) < 0) {
        perror("Error unmapping from file");
        exit(EXIT_FAILURE);
    }
    if(madvise(from_mmap, size, MADV_NORMAL) < 0) {
        perror("Error setting madvise");
        exit(EXIT_FAILURE);
    }
    if(ftruncate(to, size) < 0) {
        perror("Error truncating file");
        exit(EXIT_FAILURE);
    }

    close(from);
    close(to);

    return EXIT_SUCCESS;
}
