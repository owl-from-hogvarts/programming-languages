#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

// Один из двух случаев:
// - valid = true и value содержит осмысленный результат
// - valid = false и value может быть любым

void print_newline(void);

struct maybe_int64 {
  bool valid;
  int64_t value;
};

// Первый случай; создаем функцию в помощь
// Не бойтесь за производительность
struct maybe_int64 some_int64( int64_t i ) {
  return (struct maybe_int64) { .value = i, .valid = true };
}

// Второй случай; можно создать не функцию, 
// а константный экземпляр структуры
// Все поля инициализированы нулями
// .value = 0, .valid = false
const struct maybe_int64 none_int64 = { 0 };

struct array_int {
    int64_t* data; 
    size_t size;
};


/* Вы можете пользоваться этими функциями из предыдущих заданий */
size_t read_size() { size_t i; scanf("%zu", &i); return i; }

void array_int_fill( int64_t array[], size_t sz );

struct array_int array_int_read();
struct maybe_int64 array_int_get( struct array_int a, size_t i );
bool array_int_set( struct array_int a, size_t i, int64_t value );
void array_int_print( struct array_int array );
struct maybe_int64 array_int_min( struct array_int array );
void array_int_free( struct array_int a );

void array_int_normalize( struct array_int array, int64_t m ) {
  for (size_t i = 0; i < array.size; i = i + 1) {
    array.data[i] = array.data[i] - m;
  }
}

/*  ---- maybe int[] ---- */

struct maybe_array_int {
  struct array_int value;
  bool valid;
};

struct maybe_array_int some_array_int(struct array_int array) {
  return (struct maybe_array_int) { array, true };
}
const struct maybe_array_int none_array_int = { {NULL, 0}, false };


/*  ---- int[][] ---- */

struct array_array_int {
  struct array_int* data;
  size_t size;
};

/*  --- строки ---  */

struct maybe_array_int array_array_int_get_row( struct array_array_int a, size_t i ) {
  if ( 0 <= i && i < a.size ) { return some_array_int( a.data[i] ); }
  else { return none_array_int; }
}

bool array_array_int_set_row( struct array_array_int a, size_t i, struct array_int value ) {
  if (0 <= i && i < a.size) {
    a.data[i] = value;
    return true;
  }
  else { return false; }
}

/*  --- get/set ---  */

struct maybe_int64 array_array_int_get( struct array_array_int a, size_t i, size_t j ) {
    const struct maybe_array_int maybe_nested_array = array_array_int_get_row(a, i);

    if (!maybe_nested_array.valid) {
        return none_int64;
    }

    const struct array_int nested_array = maybe_nested_array.value;
    return array_int_get(nested_array, j);
}

bool array_array_int_set( struct array_array_int a, size_t i, size_t j, int64_t value ) {
    const struct maybe_array_int maybe_row = array_array_int_get_row(a, i);

    if (maybe_row.valid) {
        return false;
    }

    const struct array_int row = maybe_row.value;
    return array_int_set(row, j, value);
}

/*  --- read/print ---  */

struct array_array_int array_array_int_read() {
    // read amount of rows
    const size_t rows_count = read_size();
    // allocate array of array structs
    struct array_int *maybe_free_space = malloc(sizeof(struct array_int) * rows_count);
    if (!maybe_free_space) {
        // abort execution here
        exit(1);
    }
    const struct array_array_int arrays = { .size = rows_count, .data = maybe_free_space };
    // read until rows_count is reached
    size_t i = 0;
    for (i = 0; i < rows_count; i++) {
        array_array_int_set_row(arrays, i, array_int_read());
    }

    return arrays;
}


void array_array_int_print( struct array_array_int array) {
    size_t i = 0;
    for (i = 0; i < array.size; i++) {
        // value should always be present
        const struct array_int nested_array = array_array_int_get_row(array, i).value;
        array_int_print(nested_array);
        print_newline();
    }
}


/*  --- min/normalize ---  */

// найти минимальный элемент в массиве массивов
struct maybe_int64 array_array_int_min( struct array_array_int array ) {
    struct maybe_int64 min = none_int64;
    size_t i = 0;
    for (i = 0; i < array.size; i++) {
        const struct maybe_int64 maybe_current_array_min = array_int_min(array.data[i]);
        if (!maybe_current_array_min.valid) {
            continue;
        }

        if (!min.valid || maybe_current_array_min.value < min.value) {
            min = maybe_current_array_min;
        }
    }

    return min;
}

// вычесть из всех элементов массива массивов число m
void array_array_int_normalize( struct array_array_int array, int64_t m) {
  for (size_t i = 0; i < array.size; i = i + 1) {
    const struct maybe_array_int cur_row = array_array_int_get_row( array, i );
    if (cur_row.valid) {
         array_int_normalize( cur_row.value, m );
    }
  }
}

void array_array_int_free( struct array_array_int array ) {
    size_t i = 0;
    for (i = 0; i < array.size; i++) {
        // value should always be present
        const struct array_int nested_array = array_array_int_get_row(array, i).value;
        array_int_free(nested_array);
    }

    free(array.data);
}
