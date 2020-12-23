# Assignment #7: Memory allocator
---
Лабораторная работа №7: аллокатор памяти


# Подготовка 

- Прочитайте про [автоматические переменные](https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html) в `Makefile`
- Прочитайте главу 12 (стр. 235-239), главу 13 (целиком) "Low-level programming: C, assembly and program execution". 
- Виртуальная память
  - Освежите ваши знания о виртуальной памяти и её организации (глава 4 "Low-level programming: C, assembly and program execution").
  - Вспомните, что такое файловые дескрипторы. С ними работают системные вызовы `open`, `close`, `write` и другие.
  - Прочитайте внимательно `map mmap`, в особенности значение флага `MAP_FIXED` и `MAP_FIXED_NOREPLACE`.
- Возможности языка. Удостоверьтесь, что вы знаете о том, как работают:
   - ключевые слова `inline` (стр. 280 в учебнике) и `restrict` (стр. 281 в учебнике).
   - Flexible array members (стр. 209 в учебнике).
   - Макрос `offsetof` и особенно [эта страница](https://p99.gforge.inria.fr/p99-html/group__flexible.html).
   - использование структур для создания новых псевдонимов типов (как просто `typedef`) но без неявного преобразования (см. [урок на Stepik](https://stepik.org/lesson/408350/step/15)).
 
На защите мы можем обсуждать любые вопросы по этим материалам.

# Аллокатор памяти

Мы уже неоднократно пользовались аллокатором памяти, который является частью стандартной библиотеки C. 
Работа с ним осуществляется через функции `malloc` и `free` (а также `calloc` и `realloc`).
Чтобы лучше прочувствовать то, как он устроен, мы напишем свою упрощённую версию аллокатора.

# Нулевое приближение

Аллокатор памяти позволяет запрашивать блоки произвольного размера, а затем ему можно эти блоки возвращать чтобы переиспользовать память.

Аллокатор резервирует большую область памяти с помощью `mmap`. Он размечает её на блоки с заголовками, заголовки образуют связный список. 
В заголовке указано, свободен ли блок, его размер и кто его следующий блок.

- `malloc(size_t n)` : ищем свободный блок размера не меньше `n` и делим его на два блока:
   - блок размера $`n`$
   - оставшаяся часть
   
   При этом по ходу поиска мы объединяем соседние свободные блоки в свободные блоки большего размера.
- При освобождении памяти мы помечаем блок как незанятый и объединяем со
  следующим блоком, пока возможно (пока и текущий и следующий блок свободны и
  пока следующий блок идёт в памяти сразу после данного).
- Если большая область памяти кончается, мы резервируем ещё память. Сначала
  пытаемся сделать это вплотную, сразу после последнего блока, но если не
  получается &mdash; позволяем `mmap` выбрать подходящий адрес для начала нового
  региона.
 


# Первое приближение

Представим себе достаточно большую область памяти, которую мы выделяем под кучу. Назовём её *регионом*.
Разметим регион на блоки; каждый блок начинается с заголовка, а сразу после него идут данные.

```
|___заголовок1____|____данные1___||___заголовок2____|____данные2___||___заголовок3____|____...
```

Блоки заполняют собой всё пространство региона. 

## Заголовок 

В заголовке блока содержится ссылка на следующий блок и пометка о статусе блока (занят или свободен).

```c
/* mem_internals.h */

//См. https://stepik.org/lesson/408350/step/15
typedef struct { size_t bytes; } block_capacity;

struct block_header {
  struct block_header*    next;
  block_capacity capacity;
  bool           is_free;
  uint8_t        contents[];  // flexible array member
};
```
Куча задаётся ссылкой на заголовок первого блока.

## Размер и вместимость

У каждого блока есть две характеристики: *размер* и *вместимость*. Чтобы их не путать, мы создадим для них два типа.

```c
/* mem_internals.h */
typedef struct { size_t bytes; } block_capacity;
typedef struct { size_t bytes; } block_size;
```

Размер блока всегда на `offsetof(struct block_header, contents)` больше, чем его вместимость.

```c
/* mem_internals.h */
inline block_size size_from_capacity( block_capacity cap ) { 
   return (block_size) {cap.bytes + offsetof( struct block_header, contents ) }; 
}
inline block_capacity capacity_from_size( block_size sz ) { 
   return (block_capacity) {sz.bytes - offsetof( struct block_header, contents ) }; 
}
```

- В заголовке хранится вместимость блока, а не его суммарный размер вместе с заголовком.
- Нельзя использовать `sizeof( struct block_header )`, т.к. из-за выравнивания размер структуры будет больше. На машине автора, например, размер структуры был равен 24, а `offsetof( struct block_header, contents ) == 17`, что правильно.

## Алгоритм `malloc(n)`

- Перебираем блоки пока не находим "хороший".
Хороший блок &mdash; такой, в который можно уместить `n` байт.
- Если хорошего блока не нашлось, то см. второе приближение.
- Хороший блок может быть слишком большим, скажем, нам нужно выделить 20 байт, а его размер 30 мегабайт. Тогда мы разделяем блок на две части: в первом блоке будет `20 + sizeof( struct block_header ) ` байт.
  Адрес содержимого этого блока и вернёт `malloc`.

## Алгоритм `free(void* addr)`

- Если `addr == NULL`, то не делаем ничего. 
- Нам нужно получить из `addr` (который указывает на начало поля `contents`) адрес начала заголовка (для этого вычтем из него `sizeof(struct mem)`).
  В заголовке блока установим `is_free = true`, всё.



# Второе приближение

Теперь мы опишем ещё несколько аспектов аллокации.

- что делать с большим количеством последовательно идущих свободных блоков?
- как избежать появления слишком маленьких блоков?
- что делать, если память в куче кончилась?


## Алгоритм `malloc(n)`

- Нет смысла выделять блок размером, скажем, 1 байт; даже его заголовок будет занимать больше места.
  Пусть минимальная вместимость блока будет обозначаться так:
  ```c
    #define BLOCK_MIN_CAPACITY 24
  ```

  Слишком маленькие блоки могут образовываться в двух случаях:
  - `n  < BLOCK_MIN_CAPACITY`. Тогда мы будем запрашивать блок не размера `n`, а размера `BLOCK_MIN_CAPACITY`.
  - Мы нашли хороший блок, и его размер немногим больше `n`. Если разделить блок на две части, вместимость второй части окажется меньше `BLOCK_MIN_CAPACITY`. Не будем делить такие блоки, а отдадим блок целиком.
  
- При поиске хорошего блока мы проходимся по блокам кучи. Прежде чем решать, хороший блок или нет, объединим его со всеми идущими за ним свободными блоками.
- Если в куче память кончилась, надо её расширить. Для этого будем использовать системный вызов `mmap`. Обязательно прочитайте `man` чтобы понять, с какими аргументами (`prot` и `flags`) его вызывать!
  
  - Сначала надо попытаться выделить память вплотную к концу кучи и разметить её в один большой свободный блок. Если в первом регионе кучи последний блок был свободен, надо объединить его со следующим.
  - Если выделить регион вплотную не получилось, 
 
 

# Задание 

- Реализуйте аллокатор, используя заготовку в репозитории.
- Придумайте тесты, показывающие работу аллокатора в важных случаях:
   - Обычное успешное выделение памяти.
   - Освобождение одного блока из нескольких выделенных.
   - Освобождение двух блоков из нескольких выделенных.
   - Память закончилась, новый регион памяти расширяет старый.
   - Память закончилась, старый регион памяти не расширить из-за другого выделенного диапазона адресов, новый регион выделяется в другом месте.
  Тесты должны запускаться из `main.c`, но могут быть описаны в отдельном (отдельных) файлах.
  Алгоритм не самый простой, легко ошибиться. Чтобы не тратить времени на отладку, обязательно делайте разбиение на маленькие функции!
- Разберитесь с тем, как написан `Makefile` и исправьте его так, чтобы в итоговый код включался скомпилированный `main.c`. При желании вы можете написать свой `Makefile`, но только если он будет более красив и выразителен.
  

# Для самопроверки:

- Функции должны получать все необходимые им данные через аргументы. 
- Функции, которые предназначены только для использования в одном модуле, должны быть помечены `static` (почему?)
- Никаких изменяемых глобальных переменных.
- Нельзя смешивать логику вычислений и ввод-вывод.
- Нельзя использовать `typedef` для определения структур ([объяснение](https://stepik.org/lesson/408350/step/2)), кроме [структур с одним полем](https://stepik.org/lesson/408350/step/15)).
- Ваш код должен компилироваться с флагами `-std=c18 -pedantic -Wall -Werror` или `-std=c18 -pedantic -Wall -Werror`.
- Типы:

  - Проверьте, что вы максимально возможным образом расставили `const`.
  - Проверьте, что индексы используют только тип `size_t`.
  - Проверьте, что вы используете только платформо-независимые типы, такие, как `int64_t` или `int_fast64_t`.
- Вы можете добавлять любое число вспомогательных функций для удобства, это поощряется.
- Проверьте архитектуру.


# Дополнительные материалы

- Ulrich Drepper. ["What every programmer should know about memory"](https://people.freebsd.org/~lstewart/articles/cpumemory.pdf)
- [`man mmap` online](https://man7.org/linux/man-pages/man2/mmap.2.html)
- [Статья Doug Lea о том, как работает аллокатор в `glibc`](http://gee.cs.oswego.edu/dl/html/malloc.html). Текущая версия аллокатора работает по более сложному алгоритму. 
- [Исходный код одной из последних версий аллокатора в `glibc`. Очень много хорошо написанных комментариев](docs/malloc-impl.c)
