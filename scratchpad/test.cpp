#include <stdio.h>
#include <stdlib.h>

#define NULL 0

// NOTE(Momo): Really naive version of readfile which will allocate memory
char* ReadFile(const char* path) {
    FILE* file = fopen(path, "r");
    if (file == nullptr) {
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long int filesize = ftell(file);
    
    char* ret = (char*)malloc(filesize);
    if (ret == NULL) {
        fclose(file);
        return NULL;
    }
    
    // Write into memory
    fseek(file, 0, SEEK_SET);
    size_t readAmt = fread(ret, 1, filesize, file);
    if (readAmt+1 != filesize-1) {
        printf("%lld vs %ld", readAmt, filesize);
        
        fclose(file);
        free(ret);
        return NULL;
    }
    
    fclose(file);
    return ret;
}

int main() {
    char* test = ReadFile("hello.txt");
    if (!test) {
        printf("died\n");
        return 1;
    }
    printf("%s", test);
    
    
    free(test);
    return 0;
}

