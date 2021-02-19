#include <stdio.h>
#include <stdlib.h>

#include "mm_core.h"

// For now, we can only replace $0 to $9 in the input file.
// Should be more than enough for most use cases. 
//
// Expected usage: 
//  # replaces $0 and $1 with u32 and U32 respectively
//  const char* Words[] = { "u32", "U32" }; 
//  GenerateCode("template_array.cpp",   // input file 
//               "gen_tpl_array_u32.h",  // output file
//               Words,                  
//               ArrayCount(Words));
//              
static inline void
GenerateCode(const char* SrcFileName,
             const char* DestFileName,
             const char* Words[],
             u32 WordCount) 
{
    Assert(WordCount <= 9);

    FILE* SrcFile = {};
    if (fopen_s(&SrcFile, SrcFileName, "r") != 0) {
        printf("Cannot find file: %s\n", SrcFileName);
        return;
    }
    Defer { fclose(SrcFile); }; 
    
    FILE * DestFile = {};
    if(fopen_s(&DestFile, DestFileName, "w") != 0) {
        printf("Cannot open file: %s\n", DestFileName);
        return;
    }
    Defer { fclose(DestFile); };
   
    printf("Working on %s\n", DestFileName);  
    while (!feof(SrcFile)) {
        char CurrentChar = 0;
        fread(&CurrentChar, 1, 1, SrcFile); 

        if (CurrentChar == '$') {
            char WordIndex = 0;
            if (fread(&WordIndex, 1, 1, SrcFile)) {
                WordIndex -= '0';
                if (WordIndex > 9) {
                    printf("Invalid WordIndex: %d\n", WordIndex);
                    return;
                }
                const char* Word = Words[WordIndex];
                fwrite(Word, 1, SiStrLen(Word), DestFile); 
            }
            else {
                fwrite(&CurrentChar, 1, 1, DestFile);
            }
        } else {
            fwrite(&CurrentChar, 1, 1, DestFile);
        }

    }

}

const char* Help = "Expected usage:\n\
\t$ tool_codegen <input_file> <output_file> <$1_replacement> <$2_replacement>...\n";

int main(int argc, const char* argv[]) {
    if (argc <= 3) {
        printf("%s", Help);
        return 0;
    }

    u32 WordCount = argc - 3; 

    GenerateCode(argv[1], 
                 argv[2], 
                 &argv[3], 
                 WordCount); 
    return 0;
}
