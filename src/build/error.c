
#include "../all.h"

#include <stdarg.h>

void build_err(Build *b, char *msg) {
    printf("# Error: %s\n", msg);
    exit(1);
}

void parse_err(Parser *p, int start, char *msg, ...) {
    printf("# Parse error\n");
    Build *b = p->b;
    Chunk* chunk = p->chunk;
    char *content = chunk->content;
    // Trace
    Chunk* chunks = p->chunks;
    int chunkc = p->chunk_index + 1;
    if (chunkc > 1) {
        int x = chunkc;
        printf("------------------------------\n");
        while (--x >= 0) {
            Chunk *ch = &chunks[x];
            ChunkPos* pos = chunk_token_pos(b, ch, ch->i);
            printf("=> line: %d | col: %d | file: %s\n", pos->line, pos->col, ch->fc ? ch->fc->path : "(generated code)");
        }
        printf("------------------------------\n");
    }
    // Position
    ChunkPos *pos = chunk_token_pos(b, chunk, chunk->i);
    ChunkPos *start_pos = pos;
    if (start > -1) {
        start_pos = chunk_token_pos(b, chunk, start);
    }
    int line = pos->line;
    int col = pos->col;
    int i = 0;

    va_list args;
    va_start(args, msg);
    char error[2048];
    vsprintf(error, msg, args);
    va_end(args);

    printf("# File: %s\n", chunk->fc ? chunk->fc->path : "(generated code)");
    printf("# Line: %d | Col: %d\n", line, col);
    printf("# Error: %s\n", error);

    exit(1);
}

void lex_err(Build* b, Chunk *ch, int content_i, char *msg, ...) {

    char* content = ch->content;

    va_list args;
    va_start(args, msg);
    char error[2048];
    vsprintf(error, msg, args);
    va_end(args);

    int line, col;
    get_content_line_col(content, content_i, &line, &col);

    printf("# SYNTAX ERROR\n");
    printf("# File: %s\n", ch->fc ? ch->fc->path : "(generated code)");
    printf("# Line: %d | Col: %d\n", line, col);
    printf("# Error: %s\n", error);

    exit(1);
}


void get_content_line_col(char* content, int target_i, int* _line, int* _col) {
    int i = 0;
    int line = 0;
    int col = 0;
    while (i < target_i) {
        char ch = content[i++];
        if(ch == 0)
            break;
        col++;
        if(ch == '\n'){
            col = 0;
            line++;
        }
    }
    *_line = line;
    *_col = col;
}