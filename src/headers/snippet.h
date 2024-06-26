
#ifndef _H_SNIP
#define _H_SNIP

#include "typedefs.h"

Scope* gen_snippet_ast(Allocator* alc, Parser* p, Snippet* snip, Map* idfs, Scope* scope_parent);

struct Snippet {
    Array* args;
    Chunk* chunk;
    Scope* fc_scope;
    Array* exports;
};
struct SnipArg {
    char* name;
    int type;
};

#endif
