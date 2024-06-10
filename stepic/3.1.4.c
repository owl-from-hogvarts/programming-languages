#include "2.3.4.c"
#include <stdint.h>
#include <stdio.h>

enum opcode { BC_PUSH, BC_IPRINT, BC_IREAD, BC_IADD, BC_STOP };

struct maybe_int64 maybe_read_int64();

struct bc_noarg {
  enum opcode opcode;
};
struct bc_arg64 {
  enum opcode opcode;
  int64_t arg;
};
union ins {
  enum opcode opcode;
  struct bc_arg64 as_arg64;
  struct bc_noarg as_noarg;
};

enum ins_arg_type { IAT_NOARG, IAT_I64 };

struct ins_descr {
  const char*       mnemonic;     // мнемоника
  enum ins_arg_type argtype;      // тип аргументов: 0, 1 численных аргументов
  bool              affects_ip;   // изменяет ли инструкция IP?

// Новые поля:
  size_t            stack_min;    // минимальное количество элементов для инструкции
  int64_t           stack_delta;  // сколько требуется аргументов в стеке
};

extern const struct ins_descr *  instructions;

struct vm_state {
  /* 
   * NULL means halt
   */
  const union ins *ip;
  struct stack data_stack;
};
/*--------------------*/

/* Вам доступны:


struct maybe_int64 {
    int64_t value; 
    bool valid; 
};

struct maybe_int64 some_int64(int64_t i);

extern const struct maybe_int64 none_int64;

void maybe_int64_print( struct maybe_int64 i );
struct maybe_int64 maybe_read_int64();
void print_int64(int64_t i)
*/
struct stack * get_datastack(struct vm_state * state) {
  return &state->data_stack;
}

void interpret_push(struct vm_state* state) {
  stack_push(get_datastack(state), state->ip->as_arg64.arg);
}

void interpret_iread(struct vm_state* state ) {
      const struct maybe_int64 maybe_int = maybe_read_int64();
      if (!maybe_int.valid) {
        // if could not read number it is fatal error
        return;
      }

      stack_push(get_datastack(state), maybe_int.value);
}
void interpret_iadd(struct vm_state* state ) {
      struct stack *const stack_pointer = get_datastack(state);
      const struct maybe_int64 b = stack_pop(stack_pointer);
      const struct maybe_int64 a = stack_pop(stack_pointer);
      int64_t result = 0;

      if (a.valid) {
        result += a.value;
      }

      if (b.valid) {
        result += b.value;
      }

      stack_push(stack_pointer, result);
}
void interpret_iprint(struct vm_state* state ) {
      const struct maybe_int64 what_to_print = stack_pop(get_datastack(state));
      if (!what_to_print.valid) {
        // nothing to print
        // just skip
        return;
      }

      print_int64(what_to_print.value);
}

/*  Интерпретаторы команд */
void interpret_swap  ( struct vm_state* state )  {
  struct stack * const stack = get_datastack(state);
  struct maybe_int64 a = stack_pop(stack);
  if (!a.valid) {
    return;
  }

  struct maybe_int64 b = stack_pop(stack);
  if (!b.valid) {
    return;
  }

  stack_push(stack, a.value);
  stack_push(stack, b.value);

}
void interpret_pop   ( struct vm_state* state )  {
  stack_pop(get_datastack(state));
}

void interpret_dup   ( struct vm_state* state )  {
  struct stack * const stack = get_datastack(state);
  struct maybe_int64 poped_value = stack_pop(stack);
  if (!poped_value.valid) {
    return;
  }
  stack_push(stack, poped_value.value);
  stack_push(stack, poped_value.value);
}

// Эти функции поднимают операции над числами на уровень стековых операций
// lift_unop применяет операцию к вершине стека;
void lift_unop( struct stack* s, int64_t (f)(int64_t)) {
  struct maybe_int64 poped_value = stack_pop(s);
  if (!poped_value.valid) {
    return;
  }
  const int64_t computed_value = f(poped_value.value);
  stack_push(s, computed_value);
}

// lift_binop забирает из стека два аргумента,
// применяет к ним функцию от двух переменных и возвращает в стек результат
void lift_binop( struct stack* s, int64_t (f)(int64_t, int64_t)) {
  struct maybe_int64 a = stack_pop(s);
  if (!a.valid) {
    return;
  }

  struct maybe_int64 b = stack_pop(s);
  if (!b.valid) {
    return;
  }

  const int64_t computed_value = f(b.value, a.value);
  stack_push(s, computed_value);
}


/* Подсказка: можно выполнять программу пока ip != NULL,
    тогда чтобы её остановить достаточно обнулить ip */
void interpret_stop(struct vm_state* state ) {
  state->ip = NULL;
}

typedef void (*interpter)(struct vm_state* state);

const interpter interpreters[] = {
  [BC_PUSH] = interpret_push,
  [BC_IPRINT] = interpret_iprint,
  [BC_IREAD] = interpret_iread,
  [BC_IADD] = interpret_iadd,
  [BC_STOP] = interpret_stop,
};

void interpret(struct vm_state* state) {
  if (state == NULL) {
    return;
  }

  while (state->ip) {
    const union ins* ins = state->ip;
    struct stack * const stack = get_datastack(state);

    const struct ins_descr * ins_descr = instructions + ins->opcode;

    const int64_t elements_in_stack = stack_count(stack);
    if (elements_in_stack < ins_descr->stack_min) {
      puts("Stack underflow");
      return;
    }
    if (elements_in_stack + ins_descr->stack_delta > stack->data.size) {
      puts("Stack overflow");
      return;
    }

    interpreters[ins->opcode](state);

    if (!ins_descr->affects_ip) { state->ip = state->ip + 1; }
  }
}
