#include "2.3.4.c"
#include <stdint.h>

#define STACK_CAPACITY 50

/* Описание инструкций (см. предыдущий шаг) */
enum opcode { BC_PUSH, BC_IPRINT, BC_IREAD, BC_IADD, BC_STOP };

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

/* ------------------------ */

struct vm_state {
  const union ins *ip;
  struct stack data_stack;
};


/* Начальная вместимость стека задаётся определением STACK_CAPACITY */
struct vm_state state_create(const union ins *ip) {
  return (struct vm_state){.ip = ip,
                           .data_stack = stack_create(STACK_CAPACITY)};
}

// Как правильно деинициализировать состояние, освободить все ресурсы?
void state_destroy(struct vm_state *state) {
  if (state != NULL) {
    free(state);
  }
}

/* Вы можете использовать эти функции: */
void print_int64(int64_t);
struct maybe_int64 maybe_read_int64();

struct stack stack_create(size_t size);
void stack_destroy(struct stack *s);
bool stack_push(struct stack *s, int64_t value);
struct maybe_int64 stack_pop(struct stack *s);

// enum opcode { BC_PUSH, BC_IPRINT, BC_IREAD, BC_IADD, BC_STOP };

/* Опишите цикл интерпретации с выборкой и выполнением команд (пока не выполним
 * STOP) */
void interpret(struct vm_state *state) {
  // treat ip as static ref to array of instructions
  // make separate varibale for instruction count
  size_t current_instruction_index = 0;
  struct stack *const stack_pointer = &state->data_stack;

  while (state->ip[current_instruction_index].opcode != BC_STOP) {
    const union ins current_instruction = state->ip[current_instruction_index];
    // should be done strictly after fecthing the current instruction
    current_instruction_index += 1;

    switch (current_instruction.opcode) {
    case BC_IADD: {
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
      break;
    };
    case BC_IPRINT: {
      const struct maybe_int64 what_to_print = stack_pop(stack_pointer);
      if (!what_to_print.valid) {
        // nothing to print
        // just skip
        continue;
      }

      print_int64(what_to_print.value);
      break;
    };
    case BC_IREAD: {
      const struct maybe_int64 maybe_int = maybe_read_int64();
      if (!maybe_int.valid) {
        // if could not read number it is fatal error
        return;
      }

      stack_push(stack_pointer, maybe_int.value);
      break;
    };
    case BC_PUSH: {
      const int64_t arg = current_instruction.as_arg64.arg;
      stack_push(stack_pointer, arg);
      break;
    };
    case BC_STOP: ;
    default:
      return;
    }
  }
}

void interpret_program(const union ins *program) {
  struct vm_state state = state_create(program);
  interpret(&state);
  state_destroy(&state);
}