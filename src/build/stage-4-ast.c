
#include "../all.h"

void stage_ast(Fc *fc);

void stage_4_ast(Fc *fc) {
    if (fc->is_header)
        return;

    Build *b = fc->b;

    if (b->verbose > 2)
        printf("Stage 4 | Parse AST: %s\n", fc->path);

    fc->alc_ast = b->alc_ast;

    usize start = microtime();
    stage_ast(fc);
    b->time_parse += microtime() - start;

    stage_4_ir(fc);
}

void stage_ast(Fc *fc) {

    usize start = microtime();

    Array *funcs = fc->funcs;
    for (int i = 0; i < funcs->length; i++) {
        Func *func = array_get_index(funcs, i);
        *fc->chunk_parse = *func->chunk_body;
        read_ast(fc, func->scope, false);
    }

    fc->b->time_parse += microtime() - start;
}

void read_ast(Fc *fc, Scope *scope, bool single_line) {
    Allocator *alc = fc->alc_ast;
    Build *b = fc->b;
    Chunk *chunk = fc->chunk_parse;

    if (!scope->ast)
        scope->ast = array_make(alc, 200);

    bool first = false;

    while (true) {

        if (single_line && first)
            return;

        char *tkn = tok(fc, true, true, true);
        int t = chunk->token;

        if (tkn[0] == ';')
            continue;
        if (t == tok_scope_close)
            break;

        first = true;
        //

        if (t == tok_id) {
            if (str_is(tkn, "let")){
                char* name = tok(fc, true, false, true);
                if(!is_valid_varname(tkn)) {
                    sprintf(b->char_buf, "Invalid variable name: '%s'", name);
                    parse_err(fc->chunk_parse, b->char_buf);
                }
                char* tkn = tok(fc, true, true, true);
                Type* type = NULL;
                if(str_is(tkn, ":")) {
                    type = read_type(fc, alc, scope, true);
                    tkn = tok(fc, true, true, true);
                }
                if(!str_is(tkn, "=")) {
                    sprintf(b->char_buf, "Expected '=' here, found: '%s'", tkn);
                    parse_err(fc->chunk_parse, b->char_buf);
                }

                Value* val = read_value(alc, fc, scope, true, 0);
                if(type) {
                    type_check(fc->chunk_parse, type, val->rett);
                } else {
                    type = val->rett;
                }

                Decl* decl = decl_make(alc, type, false);
                array_push(scope->ast, tgen_declare(alc, scope, decl, val));
                continue;
            }
            if (str_is(tkn, "return")){
                Value* val = NULL;
                if(scope->rett) {
                    val = read_value(alc, fc, scope, false, 0);
                    type_check(fc->chunk_parse, scope->rett, val->rett);
                }
                array_push(scope->ast, tgen_return(alc, val));
                scope->did_return = true;
                continue;
            }
        }

        tok_back(fc);

        Value* left = read_value(alc, fc, scope, true, 0);

        t = tok_id_next(fc);
        if((t == tok_op1 || t == tok_op2)) {
            tkn = tok(fc, true, true, true);
            if(str_in(tkn, ",=,+=,-=,*=,/=,")) {
                if (!value_is_assignable(left)) {
                    parse_err(chunk, "Cannot assign to left side");
                }

                Value *right = read_value(alc, fc, scope, true, 0);

                // TODO type check

                array_push(scope->ast, tgen_assign(alc, left, right));
                continue;
            }
            tok_back(fc);
        }

        //
        array_push(scope->ast, token_make(alc, t_statement, left));
    }
}
