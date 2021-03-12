#include <stdio.h>
#include <stdlib.h>

#include "../../code/mm_core.h"

struct code_snippet {
    const char* FileName;
    const char** Words;
    u32 WordCount;
    
};


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
GenerateCode(code_snippet* Snippets,
             u32 SnippetCount,
             const char* DestFileName,
             const char* CodeGuard = 0)
{
    
    FILE * DestFile = {};
    if(fopen_s(&DestFile, DestFileName, "w") != 0) {
        printf("Cannot open file: %s\n", DestFileName);
        return;
    }
    Defer { fclose(DestFile); };
    printf("Working on %s\n", DestFileName);  
    
    for (u32 SnippetIndex = 0; SnippetIndex < SnippetCount; ++SnippetIndex) {
        code_snippet Snippet = Snippets[SnippetIndex];
        Assert(Snippet.WordCount <= 9);
        FILE* SrcFile = {};
        if (fopen_s(&SrcFile, Snippet.FileName, "r") != 0) {
            printf("Cannot find file: %s\n", Snippet.FileName);
            return;
        }
        Defer { fclose(SrcFile); }; 
        
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
                    const char* Word = Snippet.Words[WordIndex];
                    fwrite(Word, 1, SiStrLen(Word), DestFile); 
                }
                else {
                    fwrite(&CurrentChar, 1, 1, DestFile);
                }
            } else if (CurrentChar == 0) {
                // Do nothing
            }else {
                fwrite(&CurrentChar, 1, 1, DestFile);
            }
            
        }
    }
    
}


int main(int argc, const char* argv[]) {
    code_snippet Snippets[2] = {};
    
    
    Snippets[0].FileName = "template_array";
    const char* Words0[] = { "array_int", "int", "ArrayInt_" };
    Snippets[0].Words = Words0;
    
    Snippets[1].FileName = "template_array_find";
    const char* Word1[] = { "array_int", "u32", "ArrayInt_", "CompareIntUI32" };
    Snippets[1].Words = Word1;
    
    GenerateCode(Snippets, ArrayCount(Snippets), "output.h");
    
    printf("Done!\n");
    return 0;
}
