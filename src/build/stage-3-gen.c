
#include "../all.h"

void stage_generate_main(Build *b);

void stage_3_gen(Build* b) {

    if (b->verbose > 2)
        printf("Stage 2 | Update class sizes\n");

    usize start = microtime();

    stage_generate_main(b);

    b->time_parse += microtime() - start;

    stage_add_item(b->stage_4_ast, b);
}

void stage_generate_main(Build *b) {
    //
    Unit* u = array_get_index(b->units, 0);

    bool main_has_return = false;
    bool main_has_arg = false;
    if(b->func_main) {
        main_has_return = !type_is_void(b->func_main->rett);
        main_has_arg = b->func_main->arg_types->length > 0;
    }

    // Generate main function
    Scope* scope = b->func_main ? b->func_main->scope->parent : scope_make(b->alc, sc_default, NULL);
    Func* func = func_make(b->alc, u, scope, "main", "main");
    b->func_main_gen = func;

    Idf *idf = idf_make(b->alc, idf_class, get_valk_class(b, "core", "Coro"));
    scope_set_idf(func->scope, "CORO_CLASS", idf, NULL);

    // Generate main AST
    Str* code = b->str_buf;
    str_clear(code);

    str_flat(code, "(argc: i32, argv: ptr) i32 {\n");
    // CLI args
    str_flat(code, "let arr = Array[String].new(10);\n");
    str_flat(code, "let i = 0\n");
    str_flat(code, "while i < argc {\n");
    str_flat(code, "let cstr = @ptrv(argv, cstring, i)\n");
    str_flat(code, "arr.push(cstr)\n");
    str_flat(code, "i++\n");
    str_flat(code, "}\n");

    Nsc* nsc_fs = map_get(b->pkc_valk->namespaces, "fs");
    if(b->target_os == os_macos && nsc_fs) {
        Idf *idf = idf_make(b->alc, idf_scope, nsc_fs->scope);
        scope_set_idf(func->scope, "FS_NSC", idf, NULL);
        str_flat(code, "FS_NSC:FIRST_ARG = arr.get(0) ? null @as String\n");
    }

    if (b->is_test) {
        int count = 0;
        char* buf = b->char_buf;

        Idf *idf = idf_make(b->alc, idf_func, get_valk_func(b, "core", "test_init"));
        scope_set_idf(func->scope, "VALK_TEST_INIT", idf, NULL);
        idf = idf_make(b->alc, idf_func, get_valk_func(b, "core", "test_result"));
        scope_set_idf(func->scope, "VALK_TEST_RESULT", idf, NULL);
        idf = idf_make(b->alc, idf_func, get_valk_func(b, "core", "test_final_result"));
        scope_set_idf(func->scope, "VALK_TEST_FINAL_RESULT", idf, NULL);

        // Init test result
        str_flat(code, "let result = VALK_TEST_INIT()\n");
        // Call tests
        Array* units = b->units;
        for(int i = 0; i < units->length; i++) {
            Unit* u = array_get_index(units, i);
            Array* tests = u->tests;
            for(int o = 0; o < tests->length; o++) {
                Test* t = array_get_index(tests, o);
                Func* tf = t->func;
                // Set name
                sprintf(buf, "VALK_TEST_NAME_%d", count);
                char* name_idf = dups(b->alc, buf);
                Idf *idf = idf_make(b->alc, idf_value, vgen_string(b->alc, u, t->name));
                scope_set_idf(func->scope, name_idf, idf, NULL);
                str_flat(code, "result.reset(");
                str_add(code, name_idf);
                str_flat(code, ")\n");
                // Call test
                sprintf(buf, "VALK_TEST_FUNC_%d", count);
                char* func_idf_name = dups(b->alc, buf);
                idf = idf_make(b->alc, idf_func, tf);
                scope_set_idf(func->scope, func_idf_name, idf, NULL);
                str_add(code, func_idf_name);
                str_flat(code, "(result)\n");
                // Result
                str_flat(code, "VALK_TEST_RESULT(result)\n");
                //
                count++;
            }
        }
        // Result
        str_flat(code, "VALK_TEST_FINAL_RESULT(result)\n");
        str_flat(code, "return 0;\n");
    } else {
        if (b->func_main) {
            if (main_has_return || b->func_main->is_async)
                str_flat(code, "let main_res = ");
            str_flat(code, "main(");
            if (main_has_arg) {
                str_flat(code, "arr");
            }
            str_flat(code, ");\n");
        }

        str_flat(code, "CORO_CLASS.loop()\n");

        if (main_has_return)
            str_flat(code, "return main_res\n");
        else
            str_flat(code, "return 0\n");
    }

    str_flat(code, "}\n");

    char* content = str_to_chars(b->alc, code);
    Chunk *chunk = chunk_make(b->alc, b, NULL);
    chunk_set_content(b, chunk, content, code->length);

    Parser *p = u->parser;
    *p->chunk = *chunk;
    p->scope = scope;
    parse_handle_func_args(p, func);
    stage_types_func(p, func);
}
