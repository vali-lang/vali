
#include "../all.h"

void ir_gen_func(IR *ir, IRFunc *func);

void ir_gen_functions(IR* ir) {
    //
    Array* funcs = ir->fc->funcs;

    for (int i = 0; i < funcs->length; i++) {
        Func* vfunc = array_get_index(funcs, i);
        IRFunc* func = al(ir->alc, sizeof(IRFunc));
        func->ir = ir;
        func->func = vfunc;
        //
        func->blocks = array_make(ir->alc, 20);
        //
        func->stack_save_vn = NULL;
        func->di_scope = NULL;
        //
        func->var_count = 0;

        IRBlock* start = ir_block_make(ir, func);
        IRBlock* code = ir_block_make(ir, func);

        func->block_start = start;
        func->block_code = code;

        array_push(ir->funcs, func);

        ir_gen_func(ir, func);
    }
}

void ir_gen_func(IR *ir, IRFunc *func) {
    // Arg vars
    Func* vfunc = func->func;
    Array *args = vfunc->args->values;
    for (int i = 0; i < args->length; i++) {
        FuncArg *arg = array_get_index(args, i);
        char *v = ir_var(func);
        arg->decl->ir_var = v;
    }

    // Decls
    Scope* scope = vfunc->scope;
    Array* decls = scope->decls;
    for (int i = 0; i < decls->length; i++) {
        Decl* decl = array_get_index(decls, i);
        if(decl->is_mut) {
            decl->ir_var = ir_alloca(ir, func, decl->type);
        }
    }

    // AST
    ir->func = func;
    ir->block = func->block_code;

    ir_write_ast(ir, vfunc->scope);
    //
    if (!vfunc->scope->did_return) {
        str_flat(ir->block->code, "  ret void\n");
    }

    // Jump from start to code block
    ir->block = func->block_start;
    ir_jump(ir, func->block_code);

    ir->func = NULL;
    ir->block = NULL;
}

void ir_func_definition(Str* code, IR* ir, Func *vfunc, bool is_extern) {

    Array *args = vfunc->args->values;
    int argc = args->length;

    if (is_extern) {
        str_flat(code, "declare ");
    } else {
        str_flat(code, "define dso_local ");
    }
    str_add(code, ir_type(ir, vfunc->rett));
    str_flat(code, " @");
    str_add(code, vfunc->export_name);
    str_flat(code, "(");

    // Args
    for (int i = 0; i < argc; i++) {
        FuncArg *arg = array_get_index(args, i);
        if (i > 0)
            str_flat(code, ", ");
        str_add(code, ir_type(ir, arg->type));
        str_flat(code, " noundef");

        Type *type = arg->type;
        bool notnull = type->type == type_struct && type->is_pointer && type->nullable == false;
        if (notnull)
            str_flat(code, " nonnull");

        if(!is_extern) {
            str_flat(code, " ");
            str_add(code, arg->decl->ir_var);
        }
    }
    if (is_extern) {
        str_flat(code, ")\n");
    }else {
        str_flat(code, ")");
        if (vfunc->is_inline)
            str_flat(code, " alwaysinline");

        str_flat(code, " {\n");
    }
}

void ir_define_ext_func(IR* ir, Func* func) {
    if(!array_contains(ir->declared_funcs, func, arr_find_adr)) {
        Str *code = ir->code_extern;
        ir_func_definition(code, ir, func, true);
        array_push(ir->declared_funcs, func);
    }
}

char *ir_alloca(IR *ir, IRFunc* func, Type *type) {
    IRBlock *block = func->block_start;
    Str *code = block->code;

    char bytes[20];

    char *var = ir_var(func);
    str_flat(code, "  ");
    str_add(code, var);
    str_flat(code, " = alloca ");
    str_add(code, ir_type(ir, type));
    str_flat(code, ", align ");
    str_add(code, ir_type_align(ir, type, bytes));
    str_flat(code, "\n");

    return var;
}

char *ir_alloca_by_size(IR *ir, IRFunc* func, char* size) {
    IRBlock *block = func->block_start;
    Str *code = block->code;

    char *var = ir_var(func);
    str_flat(code, "  ");
    str_add(code, var);
    str_flat(code, " = alloca i8, i32 ");
    str_add(code, size);
    str_flat(code, ", align 8\n");

    return var;
}