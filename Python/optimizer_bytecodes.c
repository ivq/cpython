#include "Python.h"
#include "pycore_uops.h"
#include "pycore_uop_ids.h"
#include "internal/pycore_moduleobject.h"

#define op(name, ...) /* NAME is ignored */

typedef struct _Py_UOpsSymType _Py_UOpsSymType;
typedef struct _Py_UOpsAbstractInterpContext _Py_UOpsAbstractInterpContext;
typedef struct _Py_UOpsAbstractFrame _Py_UOpsAbstractFrame;

static int
dummy_func(void) {

    PyCodeObject *code;
    int oparg;
    _Py_UOpsSymType *flag;
    _Py_UOpsSymType *left;
    _Py_UOpsSymType *right;
    _Py_UOpsSymType *value;
    _Py_UOpsSymType *res;
    _Py_UOpsSymType *iter;
    _Py_UOpsSymType *top;
    _Py_UOpsSymType *bottom;
    _Py_UOpsAbstractFrame *frame;
    _Py_UOpsAbstractInterpContext *ctx;
    _PyUOpInstruction *this_instr;
    _PyBloomFilter *dependencies;
    int modified;

// BEGIN BYTECODES //

    op(_LOAD_FAST_CHECK, (-- value)) {
        value = GETLOCAL(oparg);
        // We guarantee this will error - just bail and don't optimize it.
        if (_Py_uop_sym_is_null(value)) {
            goto out_of_space;
        }
    }

    op(_LOAD_FAST, (-- value)) {
        value = GETLOCAL(oparg);
    }

    op(_LOAD_FAST_AND_CLEAR, (-- value)) {
        value = GETLOCAL(oparg);
        _Py_UOpsSymType *temp;
        OUT_OF_SPACE_IF_NULL(temp = _Py_uop_sym_new_null(ctx));
        GETLOCAL(oparg) = temp;
    }

    op(_STORE_FAST, (value --)) {
        GETLOCAL(oparg) = value;
    }

    op(_PUSH_NULL, (-- res)) {
        res = _Py_uop_sym_new_null(ctx);
        if (res == NULL) {
            goto out_of_space;
        };
    }

    op(_GUARD_BOTH_INT, (left, right -- left, right)) {
        if (_Py_uop_sym_matches_type(left, &PyLong_Type) &&
            _Py_uop_sym_matches_type(right, &PyLong_Type)) {
            REPLACE_OP(this_instr, _NOP, 0, 0);
        }
        _Py_uop_sym_set_type(left, &PyLong_Type);
        _Py_uop_sym_set_type(right, &PyLong_Type);
    }

    op(_GUARD_BOTH_FLOAT, (left, right -- left, right)) {
        if (_Py_uop_sym_matches_type(left, &PyFloat_Type) &&
            _Py_uop_sym_matches_type(right, &PyFloat_Type)) {
            REPLACE_OP(this_instr, _NOP, 0 ,0);
        }
        _Py_uop_sym_set_type(left, &PyFloat_Type);
        _Py_uop_sym_set_type(right, &PyFloat_Type);
    }

    op(_GUARD_BOTH_UNICODE, (left, right -- left, right)) {
        if (_Py_uop_sym_matches_type(left, &PyUnicode_Type) &&
            _Py_uop_sym_matches_type(right, &PyUnicode_Type)) {
            REPLACE_OP(this_instr, _NOP, 0 ,0);
        }
        _Py_uop_sym_set_type(left, &PyUnicode_Type);
        _Py_uop_sym_set_type(right, &PyUnicode_Type);
    }

    op(_BINARY_OP_ADD_INT, (left, right -- res)) {
        if (_Py_uop_sym_is_const(left) && _Py_uop_sym_is_const(right)) {
            assert(PyLong_CheckExact(_Py_uop_sym_get_const(left)));
            assert(PyLong_CheckExact(_Py_uop_sym_get_const(right)));
            PyObject *temp = _PyLong_Add((PyLongObject *)_Py_uop_sym_get_const(left),
                                         (PyLongObject *)_Py_uop_sym_get_const(right));
            if (temp == NULL) {
                goto error;
            }
            OUT_OF_SPACE_IF_NULL(res = _Py_uop_sym_new_const(ctx, temp));
            // TODO gh-115506:
            // replace opcode with constant propagated one and add tests!
        }
        else {
            OUT_OF_SPACE_IF_NULL(res = _Py_uop_sym_new_type(ctx, &PyLong_Type));
        }
    }

    op(_BINARY_OP_SUBTRACT_INT, (left, right -- res)) {
        if (_Py_uop_sym_is_const(left) && _Py_uop_sym_is_const(right)) {
            assert(PyLong_CheckExact(_Py_uop_sym_get_const(left)));
            assert(PyLong_CheckExact(_Py_uop_sym_get_const(right)));
            PyObject *temp = _PyLong_Subtract((PyLongObject *)_Py_uop_sym_get_const(left),
                                              (PyLongObject *)_Py_uop_sym_get_const(right));
            if (temp == NULL) {
                goto error;
            }
            OUT_OF_SPACE_IF_NULL(res = _Py_uop_sym_new_const(ctx, temp));
            // TODO gh-115506:
            // replace opcode with constant propagated one and add tests!
        }
        else {
            OUT_OF_SPACE_IF_NULL(res = _Py_uop_sym_new_type(ctx, &PyLong_Type));
        }
    }

    op(_BINARY_OP_MULTIPLY_INT, (left, right -- res)) {
        if (_Py_uop_sym_is_const(left) && _Py_uop_sym_is_const(right)) {
            assert(PyLong_CheckExact(_Py_uop_sym_get_const(left)));
            assert(PyLong_CheckExact(_Py_uop_sym_get_const(right)));
            PyObject *temp = _PyLong_Multiply((PyLongObject *)_Py_uop_sym_get_const(left),
                                              (PyLongObject *)_Py_uop_sym_get_const(right));
            if (temp == NULL) {
                goto error;
            }
            OUT_OF_SPACE_IF_NULL(res = _Py_uop_sym_new_const(ctx, temp));
            // TODO gh-115506:
            // replace opcode with constant propagated one and add tests!
        }
        else {
            OUT_OF_SPACE_IF_NULL(res = _Py_uop_sym_new_type(ctx, &PyLong_Type));
        }
    }

    op(_BINARY_OP_ADD_FLOAT, (left, right -- res)) {
        if (_Py_uop_sym_is_const(left) && _Py_uop_sym_is_const(right)) {
            assert(PyFloat_CheckExact(_Py_uop_sym_get_const(left)));
            assert(PyFloat_CheckExact(_Py_uop_sym_get_const(right)));
            PyObject *temp = PyFloat_FromDouble(
                PyFloat_AS_DOUBLE(_Py_uop_sym_get_const(left)) +
                PyFloat_AS_DOUBLE(_Py_uop_sym_get_const(right)));
            if (temp == NULL) {
                goto error;
            }
            res = _Py_uop_sym_new_const(ctx, temp);
            // TODO gh-115506:
            // replace opcode with constant propagated one and update tests!
        }
        else {
            OUT_OF_SPACE_IF_NULL(res = _Py_uop_sym_new_type(ctx, &PyFloat_Type));
        }
    }

    op(_BINARY_OP_SUBTRACT_FLOAT, (left, right -- res)) {
        if (_Py_uop_sym_is_const(left) && _Py_uop_sym_is_const(right)) {
            assert(PyFloat_CheckExact(_Py_uop_sym_get_const(left)));
            assert(PyFloat_CheckExact(_Py_uop_sym_get_const(right)));
            PyObject *temp = PyFloat_FromDouble(
                PyFloat_AS_DOUBLE(_Py_uop_sym_get_const(left)) -
                PyFloat_AS_DOUBLE(_Py_uop_sym_get_const(right)));
            if (temp == NULL) {
                goto error;
            }
            res = _Py_uop_sym_new_const(ctx, temp);
            // TODO gh-115506:
            // replace opcode with constant propagated one and update tests!
        }
        else {
            OUT_OF_SPACE_IF_NULL(res = _Py_uop_sym_new_type(ctx, &PyFloat_Type));
        }
    }

    op(_BINARY_OP_MULTIPLY_FLOAT, (left, right -- res)) {
        if (_Py_uop_sym_is_const(left) && _Py_uop_sym_is_const(right)) {
            assert(PyFloat_CheckExact(_Py_uop_sym_get_const(left)));
            assert(PyFloat_CheckExact(_Py_uop_sym_get_const(right)));
            PyObject *temp = PyFloat_FromDouble(
                PyFloat_AS_DOUBLE(_Py_uop_sym_get_const(left)) *
                PyFloat_AS_DOUBLE(_Py_uop_sym_get_const(right)));
            if (temp == NULL) {
                goto error;
            }
            res = _Py_uop_sym_new_const(ctx, temp);
            // TODO gh-115506:
            // replace opcode with constant propagated one and update tests!
        }
        else {
            OUT_OF_SPACE_IF_NULL(res = _Py_uop_sym_new_type(ctx, &PyFloat_Type));
        }
    }

    op(_LOAD_CONST, (-- value)) {
        // There should be no LOAD_CONST. It should be all
        // replaced by peephole_opt.
        Py_UNREACHABLE();
    }

    op(_LOAD_CONST_INLINE, (ptr/4 -- value)) {
        OUT_OF_SPACE_IF_NULL(value = _Py_uop_sym_new_const(ctx, ptr));
    }

    op(_LOAD_CONST_INLINE_BORROW, (ptr/4 -- value)) {
        OUT_OF_SPACE_IF_NULL(value = _Py_uop_sym_new_const(ctx, ptr));
    }

    op(_LOAD_CONST_INLINE_WITH_NULL, (ptr/4 -- value, null)) {
        OUT_OF_SPACE_IF_NULL(value = _Py_uop_sym_new_const(ctx, ptr));
        OUT_OF_SPACE_IF_NULL(null = _Py_uop_sym_new_null(ctx));
    }

    op(_LOAD_CONST_INLINE_BORROW_WITH_NULL, (ptr/4 -- value, null)) {
        OUT_OF_SPACE_IF_NULL(value = _Py_uop_sym_new_const(ctx, ptr));
        OUT_OF_SPACE_IF_NULL(null = _Py_uop_sym_new_null(ctx));
    }


    op(_COPY, (bottom, unused[oparg-1] -- bottom, unused[oparg-1], top)) {
        assert(oparg > 0);
        top = bottom;
    }

    op(_SWAP, (bottom, unused[oparg-2], top --
        top, unused[oparg-2], bottom)) {
    }

    op(_LOAD_ATTR_INSTANCE_VALUE, (index/1, owner -- attr, null if (oparg & 1))) {
        _LOAD_ATTR_NOT_NULL
        (void)index;
        (void)owner;
    }

    op(_CHECK_ATTR_MODULE, (dict_version/2, owner -- owner)) {
        (void)dict_version;
        if (_Py_uop_sym_is_const(owner)) {
            PyObject *cnst = _Py_uop_sym_get_const(owner);
            if (PyModule_CheckExact(cnst)) {
                PyModuleObject *mod = (PyModuleObject *)cnst;
                PyObject *dict = mod->md_dict;
                uint64_t watched_mutations = get_mutations(dict);
                if (watched_mutations < _Py_MAX_ALLOWED_GLOBALS_MODIFICATIONS) {
                    PyDict_Watch(GLOBALS_WATCHER_ID, dict);
                    _Py_BloomFilter_Add(dependencies, dict);
                    this_instr->opcode = _NOP;
                }
            }
        }
    }

    op(_LOAD_ATTR_MODULE, (index/1, owner -- attr, null if (oparg & 1))) {
        (void)index;
        OUT_OF_SPACE_IF_NULL(null = _Py_uop_sym_new_null(ctx));
        attr = NULL;
        if (this_instr[-1].opcode == _NOP) {
            // Preceding _CHECK_ATTR_MODULE was removed: mod is const and dict is watched.
            assert(_Py_uop_sym_is_const(owner));
            PyModuleObject *mod = (PyModuleObject *)_Py_uop_sym_get_const(owner);
            assert(PyModule_CheckExact(mod));
            PyObject *dict = mod->md_dict;
            PyObject *res = convert_global_to_const(this_instr, dict);
            if (res != NULL) {
                this_instr[-1].opcode = _POP_TOP;
                OUT_OF_SPACE_IF_NULL(attr = _Py_uop_sym_new_const(ctx, res));
            }
        }
        if (attr == NULL) {
            /* No conversion made. We don't know what `attr` is. */
            OUT_OF_SPACE_IF_NULL(attr = _Py_uop_sym_new_not_null(ctx));
        }
    }

    op(_LOAD_ATTR_WITH_HINT, (hint/1, owner -- attr, null if (oparg & 1))) {
        _LOAD_ATTR_NOT_NULL
        (void)hint;
        (void)owner;
    }

    op(_LOAD_ATTR_SLOT, (index/1, owner -- attr, null if (oparg & 1))) {
        _LOAD_ATTR_NOT_NULL
        (void)index;
        (void)owner;
    }

    op(_LOAD_ATTR_CLASS, (descr/4, owner -- attr, null if (oparg & 1))) {
        _LOAD_ATTR_NOT_NULL
        (void)descr;
        (void)owner;
    }

    op(_LOAD_ATTR_METHOD_WITH_VALUES, (descr/4, owner -- attr, self if (1))) {
        (void)descr;
        OUT_OF_SPACE_IF_NULL(attr = _Py_uop_sym_new_not_null(ctx));
        self = owner;
    }

    op(_LOAD_ATTR_METHOD_NO_DICT, (descr/4, owner -- attr, self if (1))) {
        (void)descr;
        OUT_OF_SPACE_IF_NULL(attr = _Py_uop_sym_new_not_null(ctx));
        self = owner;
    }

    op(_LOAD_ATTR_METHOD_LAZY_DICT, (descr/4, owner -- attr, self if (1))) {
        (void)descr;
        OUT_OF_SPACE_IF_NULL(attr = _Py_uop_sym_new_not_null(ctx));
        self = owner;
    }

    op(_INIT_CALL_BOUND_METHOD_EXACT_ARGS, (callable, unused, unused[oparg] -- func, self, unused[oparg])) {
        (void)callable;
        OUT_OF_SPACE_IF_NULL(func = _Py_uop_sym_new_not_null(ctx));
        OUT_OF_SPACE_IF_NULL(self = _Py_uop_sym_new_not_null(ctx));
    }


    op(_CHECK_FUNCTION_EXACT_ARGS, (func_version/2, callable, self_or_null, unused[oparg] -- callable, self_or_null, unused[oparg])) {
        _Py_uop_sym_set_type(callable, &PyFunction_Type);
        (void)self_or_null;
        (void)func_version;
    }

    op(_CHECK_CALL_BOUND_METHOD_EXACT_ARGS, (callable, null, unused[oparg] -- callable, null, unused[oparg])) {
        _Py_uop_sym_set_null(null);
        _Py_uop_sym_set_type(callable, &PyMethod_Type);
    }

    op(_INIT_CALL_PY_EXACT_ARGS, (callable, self_or_null, args[oparg] -- new_frame: _Py_UOpsAbstractFrame *)) {
        int argcount = oparg;

        (void)callable;

        PyFunctionObject *func = (PyFunctionObject *)(this_instr + 2)->operand;
        if (func == NULL) {
            goto error;
        }
        PyCodeObject *co = (PyCodeObject *)func->func_code;

        assert(self_or_null != NULL);
        assert(args != NULL);
        if (_Py_uop_sym_is_not_null(self_or_null)) {
            // Bound method fiddling, same as _INIT_CALL_PY_EXACT_ARGS in VM
            args--;
            argcount++;
        }

        _Py_UOpsSymType **localsplus_start = ctx->n_consumed;
        int n_locals_already_filled = 0;
        // Can determine statically, so we interleave the new locals
        // and make the current stack the new locals.
        // This also sets up for true call inlining.
        if (_Py_uop_sym_is_null(self_or_null) || _Py_uop_sym_is_not_null(self_or_null)) {
            localsplus_start = args;
            n_locals_already_filled = argcount;
        }
        OUT_OF_SPACE_IF_NULL(new_frame =
                             _Py_uop_ctx_frame_new(ctx, co, localsplus_start, n_locals_already_filled, 0));
    }

    op(_POP_FRAME, (retval -- res)) {
        SYNC_SP();
        ctx->frame->stack_pointer = stack_pointer;
        _Py_uop_ctx_frame_pop(ctx);
        stack_pointer = ctx->frame->stack_pointer;
        res = retval;
    }

    op(_PUSH_FRAME, (new_frame: _Py_UOpsAbstractFrame * -- unused if (0))) {
        SYNC_SP();
        ctx->frame->stack_pointer = stack_pointer;
        ctx->frame = new_frame;
        ctx->curr_frame_depth++;
        stack_pointer = new_frame->stack_pointer;
    }

    op(_UNPACK_SEQUENCE, (seq -- values[oparg])) {
        /* This has to be done manually */
        (void)seq;
        for (int i = 0; i < oparg; i++) {
            OUT_OF_SPACE_IF_NULL(values[i] = _Py_uop_sym_new_unknown(ctx));
        }
    }

    op(_UNPACK_EX, (seq -- values[oparg & 0xFF], unused, unused[oparg >> 8])) {
        /* This has to be done manually */
        (void)seq;
        int totalargs = (oparg & 0xFF) + (oparg >> 8) + 1;
        for (int i = 0; i < totalargs; i++) {
            OUT_OF_SPACE_IF_NULL(values[i] = _Py_uop_sym_new_unknown(ctx));
        }
    }

    op(_ITER_NEXT_RANGE, (iter -- iter, next)) {
       OUT_OF_SPACE_IF_NULL(next = _Py_uop_sym_new_type(ctx, &PyLong_Type));
       (void)iter;
    }




// END BYTECODES //

}
