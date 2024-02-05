
#include "../all.h"

char *ir_type(IR *ir, Type *type) {
    //
    if (type->is_pointer)
        return "ptr";

    Class *class = type->class;

    if (type_is_void(type)) {
		return "void";
    } else if (type->type == type_bool) {
		return "i8";
    } else if (type->type == type_struct) {

        ir_define_struct(ir, class);

        char name[256];
        sprintf(name, "%%struct.%s", class->ir_name);
		return dups(ir->alc, name);

    } else if (type->type == type_int) {
        int bytes = type->size;
        return ir_type_int(ir, bytes);
    }

    printf("Type: %d\n", type->type);
    die("Unknown IR type (compiler bug)");
    return NULL;
}

char *ir_type_real(IR *ir, Type *type) {
    Class *class = type->class;
    if (class && type->type == type_struct) {
        Str *result = str_make(ir->alc, 256);
        int depth = 1;

        ir_define_struct(ir, class);

        char name[256];
        sprintf(name, "%%struct.%s", class->ir_name);
        str_append_chars(result, name);
        //
        while (depth > 0) {
            str_append_chars(result, "*");
            depth--;
        }
        //
        return str_to_chars(ir->alc, result);
    }

    return ir_type(ir, type);
}

char *ir_type_int(IR *ir, int bytes) {
    if (bytes == 1) {
        return "i8";
    } else if (bytes == 2) {
        return "i16";
    } else if (bytes == 4) {
        return "i32";
    } else if (bytes == 8) {
        return "i64";
    }

    die("Unsupported integer size (IR Generator)");
	return "";
}