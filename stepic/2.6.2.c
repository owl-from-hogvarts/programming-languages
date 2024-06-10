// Мы хотим, чтобы в структуре user хранились ссылки только на строчки из кучи.
typedef struct { char* addr; } string_heap ;

/*  Тип для идентификаторов пользователей
    и его спецификаторы ввода и вывода для printf */
typedef uint64_t uid;
#define PRI_uid PRIu64
#define SCN_uid SCNu64

enum city {C_SARATOV, C_MOSCOW, C_PARIS, C_LOS_ANGELES, C_OTHER};

/*  Массив, где элементам перечисления сопоставляются их текстовые представления */
const char* city_string[] = {
  [C_SARATOV] = "Saratov",
  [C_MOSCOW] = "Moscow",
  [C_PARIS] = "Paris",
  [C_LOS_ANGELES] = "Los Angeles",
  [C_OTHER] = "Other"
};


struct user {
  const uid id;
  const string_heap name;
  enum city city;
};

int32_t compare_by_uid_adapter( const void * const a, const void * const b );
/* Сортировать массив пользователей по полю uid по возрастанию */
void users_sort_uid(struct user users[], size_t sz) {
    qsort(users, sz, sizeof(struct user), compare_by_uid_adapter);
}

int32_t compare_by_uid(const struct user * const a, const struct user * const b) {
    const uid x = a->id;
    const uid y = b->id;
    if (x > y) return 1;
    if (x < y) return -1;
    return 0;
}

int32_t compare_by_uid_adapter( const void * const a, const void * const b ) {
    return compare_by_uid(a, b);
}

int32_t compare_by_name_adapter( const void * const a, const void * const b );
/* Сортировать массив пользователей по полю name */
/* Порядок строк лексикографический; можно использовать компаратор 
   строк -- стандартную функцию strcmp */
void users_sort_name(struct user users[], size_t sz) {
    qsort(users, sz, sizeof(struct user), compare_by_name_adapter);
}

int32_t compare_by_name(const struct user * const a, const struct user * const b) {
    return strcmp(a->name.addr, b->name.addr);
}

int32_t compare_by_name_adapter( const void * const a, const void * const b ) {
    return compare_by_name(a, b);
}

int32_t compare_by_city_adapter(const void * const a, const void * const b);
/* Сортировать массив по _текстовому представлению_ города */
void users_sort_city(struct user users[], size_t sz) {
    qsort(users, sz, sizeof(struct user), compare_by_city_adapter);
}

int32_t compare_by_city(const struct user * const a, const struct user * const b) {
    return strcmp(city_string[a->city], city_string[b->city]);
}

int32_t compare_by_city_adapter(const void * const a, const void * const b) {
    return compare_by_city(a, b);
}


