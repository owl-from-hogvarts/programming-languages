#include "2.3.4.c"
#include <stdint.h>

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

void interpret(struct vm_state* total_state) {
  if (total_state == NULL) {
    return;
  }
  // treat ip as static ref to array of instructions
  // make separate varibale for instruction count
  size_t current_instruction_index = 0;
  struct vm_state current_state = *total_state;

  // always use `current_state` in the body of the loop
  // current_instruction_index is special variable: modifing it may result in UB
  // { BC_PRINT, BC_STOP }
  while (current_state.ip != NULL) {
    current_state.ip = &total_state->ip[current_instruction_index];
    current_instruction_index += 1;

    const interpter executor = interpreters[current_state.ip->opcode];
    executor(&current_state);
  }
}
