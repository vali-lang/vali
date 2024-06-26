
#include "../all.h"

Value* coro_generate(Allocator* alc, Parser* p, Value* vfcall) {
    Build* b = p->b;
    Unit* u = p->unit;
    Scope* scope = p->scope;

    VFuncCall *fcall = vfcall->item;
    TypeFuncInfo *fi = fcall->on->rett->func_info;
    Array *types = fi->args;
    Type *rett = fi->rett;
    Array *rett_types = rett_types_of(alc, rett);

    bool has_arg = false;
    bool has_gc_arg = false;
    int gc_rett_count = 0;
    loop(types, i) {
        Type* type = array_get_index(types, i); 
        if(type_is_gc(type)) {
            has_gc_arg = true;
        } else {
            has_arg = true;
        }
    }
    bool has_args = has_arg || has_gc_arg;

    // Generate handler function
    Scope* parent = p->func->scope->parent;
    char name[256];
    sprintf(name, "VALK_CORO_%d", b->coro_count++);
    Func *func = func_make(b->alc, u, parent, dups(b->alc, name), NULL);

    Str* code = b->str_buf;
    str_clear(code);

    Idf *idf = idf_make(b->alc, idf_class, get_valk_class(b, "core", "Coro"));
    scope_set_idf(func->scope, "CORO_CLASS", idf, NULL);

    if (rett_types) {
        loop(rett_types, i) {
            char buf[16];
            sprintf(buf, "RETT%d", i);
            Type *rett = array_get_index(rett_types, i);
            if (type_is_gc(rett)) {
                gc_rett_count++;
            }
            idf = idf_make(b->alc, idf_type, rett);
            scope_set_idf(func->scope, buf, idf, NULL);
        }
    }

    // Handler type
    Type *ht = type_make(b->alc, type_func);
    ht->func_info = type_clone_function_info(b->alc, fi);
    ht->size = b->ptr_size;
    ht->is_pointer = true;
    idf = idf_make(b->alc, idf_type, ht);
    scope_set_idf(func->scope, "HANDLER_TYPE", idf, NULL);

    // Coro start function code
    str_flat(code, "(coro: CORO_CLASS) {\n");
    if(has_arg) {
        str_flat(code, "let args = coro.args\n");
    }
    if(has_gc_arg) {
        str_flat(code, "let gc_args = coro.gc_stack.base\n");
    }
    // Load args
    loop(types, i) {
        Type* type = array_get_index(types, i);
        char nr[32];
        char tnr[33];
        sprintf(nr, "%d", i);
        sprintf(tnr, "T%d", i);
        Idf *idf = idf_make(b->alc, idf_type, type_clone(b->alc, type));
        scope_set_idf(func->scope, tnr, idf, NULL);

        str_flat(code, "let arg");
        str_add(code, nr);
        str_flat(code, " = @ptrv(");
        str_add(code, type_is_gc(type) ? "gc_args" : "args");
        str_flat(code, ", ");
        str_add(code, tnr);
        str_flat(code, ", 0)\n");
        str_add(code, type_is_gc(type) ? "gc_args" : "args");
        str_flat(code, " += sizeof(");
        str_add(code, tnr);
        str_flat(code, ")\n");
    }
    // Call handler
    if(rett_types) {
        str_flat(code, "let ");
        loop(rett_types, i) {
            if(i > 0)
                str_flat(code, ", ");
            str_flat(code, "res");
            char nr[16];
            itos(i, nr, 10);
            str_add(code, nr);
        }
        str_flat(code, " = ");
    }
    str_flat(code, "(coro.handler @as HANDLER_TYPE)(");
    // Args
    loop(types, i) {
        char argname[36];
        sprintf(argname, "arg%d", i);
        if(i > 0)
            str_flat(code, ", ");
        str_add(code, argname);
    }
    str_flat(code, ")");
    if(fi->can_error) {
        // TODO
    }
    str_flat(code, "\n");
    // Finish
    if(has_gc_arg && (gc_rett_count == 0 || !rett)) {
        // Clear gc args
        str_flat(code, "coro.gc_stack.adr = coro.gc_stack.base\n");
    }
    if(rett_types) {
        int s_pos = 0;
        int s_pos_gc = 0;
        loop(rett_types, i) {
            char nr[16];
            itos(i, nr, 10);
            char offset[16];
            Type *type = array_get_index(rett_types, i);
            if (type_is_gc(type)) {
                itos(s_pos_gc++, offset, 10);
                str_flat(code, "@ptrv(coro.gc_stack, RETT");
                str_add(code, nr);
                str_flat(code, ", ");
                str_add(code, offset);
                str_flat(code, ") = res");
                str_add(code, nr);
                str_flat(code, "\n");
            } else {
                int size = type->size;
                itos(s_pos, offset, 10);
                s_pos += size;
                str_flat(code, "@ptrv(coro.args + ");
                str_add(code, offset);
                str_flat(code, ", RETT");
                str_add(code, nr);
                str_flat(code, ") = res");
                str_add(code, nr);
                str_flat(code, "\n");
            }
        }
        if(gc_rett_count > 0) {
            char nr[16];
            itos(gc_rett_count, nr, 10);
            str_flat(code, "coro.gc_stack.adr = coro.gc_stack.base + sizeof(ptr) * ");
            str_add(code, nr);
            str_flat(code, "\n");
        }
    }
    str_flat(code, "coro.complete()\n");
    str_flat(code, "}\n");

    char* content = str_to_chars(b->alc, code);
    // printf("------------\n%s\n-----------\n", content);
    Chunk *chunk = chunk_make(b->alc, b, NULL);
    chunk_set_content(b, chunk, content, code->length);

    // Create new parser
    parser_new_context(&p);

    *p->chunk = *chunk;
    p->scope = parent;
    parse_handle_func_args(p, func);
    stage_types_func(p, func);

    // Return parser
    parser_pop_context(&p);

    array_push(p->func->used_functions, func);

    ///////////////////////
    // Generate init coro
    ///////////////////////

    str_clear(code);

    Scope* sub = scope_sub_make(b->alc, sc_default, scope);

    idf = idf_make(alc, idf_class, get_valk_class(b, "core", "Coro"));
    scope_set_idf(sub, "CORO_CLASS", idf, NULL);
    idf = idf_make(alc, idf_value, fcall->on);
    scope_set_idf(sub, "HANDLER", idf, NULL);
    idf = idf_make(alc, idf_value, vgen_func_ptr(alc, func, NULL));
    scope_set_idf(sub, "START_FUNC", idf, NULL);

    // Coro start function code
    str_flat(code, "<{\n");
    str_flat(code, "let coro = CORO_CLASS.new(HANDLER, START_FUNC)\n");
    if(has_arg) {
        str_flat(code, "let args = coro.args\n");
    }
    if(has_gc_arg) {
        str_flat(code, "let gc_args = coro.gc_stack.adr\n");
    }
    // Load args
    Array* values = fcall->args;
    loop(values, i) {
        Value* val = array_get_index(values, i);
        Type* type = val->rett;

        char nr[32];
        char tnr[33];
        sprintf(nr, "%d", i);
        sprintf(tnr, "ARG_T_%d", i);
        idf = idf_make(b->alc, idf_type, type);
        scope_set_idf(sub, tnr, idf, NULL);

        sprintf(tnr, "ARG_V_%d", i);
        idf = idf_make(b->alc, idf_value, val);
        scope_set_idf(sub, tnr, idf, NULL);

        str_flat(code, "@ptrv(");
        str_add(code, type_is_gc(type) ? "gc_args" : "args");
        str_flat(code, ", ARG_T_");
        str_add(code, nr);
        str_flat(code, ", 0) = ARG_V_");
        str_add(code, nr);
        str_flat(code, "\n");
        str_add(code, type_is_gc(type) ? "gc_args" : "args");
        str_flat(code, " += sizeof(ARG_T_");
        str_add(code, nr);
        str_flat(code, ")\n");
    }
    // Update adr
    if(has_gc_arg) {
        str_flat(code, "coro.gc_stack.adr = gc_args\n");
    }
    str_flat(code, "CORO_CLASS.yield_current()\n");
    // Return coro, but clear the value from the stack
    // str_flat(code, "let coro_ptr = coro @as ptr\n");
    // str_flat(code, "@ptrv(@ptr_of(coro), ?ptr) = null\n");
    // str_flat(code, "return coro_ptr @as CORO_CLASS\n");
    str_flat(code, "return coro\n");
    str_flat(code, "}\n");

    content = str_to_chars(b->alc, code);
    // printf("------------\n%s\n-----------\n", content);
    chunk = chunk_make(b->alc, b, NULL);
    chunk_set_content(b, chunk, content, code->length);

    // Create new parser
    parser_new_context(&p);

    *p->chunk = *chunk;
    p->scope = sub;
    Value* v = read_value(alc, p, true, 0);

    // Return parser
    parser_pop_context(&p);

    v->rett = type_gen_promise(alc, b, fi);

    return v;
}

Value* coro_await(Allocator* alc, Parser* p, Value* on) {

    Build* b = p->b;
    Str* code = b->str_buf;
    str_clear(code);

    TypeFuncInfo* fi = on->rett->func_info;
    Type* rett = fi->rett;
    Array* rett_types = rett_types_of(alc, rett);
    Scope* sub = scope_sub_make(b->alc, sc_default, p->scope);

    Idf* idf = idf_make(alc, idf_class, get_valk_class(b, "core", "Coro"));
    scope_set_idf(sub, "CORO_CLASS", idf, NULL);
    idf = idf_make(alc, idf_value, on);
    scope_set_idf(sub, "CORO_VAL", idf, NULL);

    // Return type identifiers
    if (rett_types) {
        loop(rett_types, i) {
            char buf[16];
            sprintf(buf, "RETT%d", i);
            Type *rett = array_get_index(rett_types, i);
            idf = idf_make(b->alc, idf_type, rett);
            scope_set_idf(sub, buf, idf, NULL);
        }
    }

    Global *g_cc = get_valk_global(b, "core", "current_coro");
    idf = idf_make(alc, idf_global, g_cc);
    scope_set_idf(sub, "CURRENT_CORO", idf, NULL);

    // Coro start function code
    str_flat(code, "<{\n");
    str_flat(code, "let coro = CORO_VAL @as CORO_CLASS\n");
    str_flat(code, "while(!coro.done) { CURRENT_CORO.await_coro(coro) }\n");
    if(!rett_types || rett_types->length == 0) {
        str_flat(code, "return 0\n");
    } else {
        // Return values
        // str_flat(code, "let retv = @ptrv(coro.result, RETT)\n");
        int s_pos = 0;
        int s_pos_gc = 0;
        loop(rett_types, i) {
            Type* type = array_get_index(rett_types, i);
            char offset[16];
            char nr[16];
            itos(i, nr, 10);
            str_flat(code, "let retv");
            str_add(code, nr);
            str_flat(code, " = ");
            if(type_is_gc(type)) {
                itos(s_pos_gc++, offset, 10);
                str_flat(code, "@ptrv(coro.gc_stack, RETT");
                str_add(code, nr);
                str_flat(code, ", ");
                str_add(code, offset);
                str_flat(code, ")\n");
            } else {
                int size = type->size;
                itos(s_pos, offset, 10);
                s_pos += size;
                itos(s_pos_gc++, offset, 10);
                str_flat(code, "@ptrv(coro.args + ");
                str_add(code, offset);
                str_flat(code, ", RETT");
                str_add(code, nr);
                str_flat(code, ")\n");
            }
        }

        // Return statement
        // str_flat(code, "return retv{nr}, retv{nr}, ...\n");
        str_flat(code, "return ");
        loop(rett_types, i) {
            if(i > 0)
                str_flat(code, ", ");
            str_flat(code, "retv");
            char nr[16];
            itos(i, nr, 10);
            str_add(code, nr);
        }
        str_flat(code, "\n");
    }
    str_flat(code, "}\n");

    char* content = str_to_chars(b->alc, code);
    // printf("------------\n%s\n-----------\n", content);
    Chunk* chunk = chunk_make(b->alc, b, NULL);
    chunk_set_content(b, chunk, content, code->length);

    // Create new parser
    parser_new_context(&p);

    *p->chunk = *chunk;
    p->scope = sub;
    Value* v = read_value(alc, p, true, 0);

    // Return parser
    parser_pop_context(&p);

    return v;
}
