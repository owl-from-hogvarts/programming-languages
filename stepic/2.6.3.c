#include "./linked_list.c"

// Робот и его callback'и
// callback'ов может быть неограниченное количество
struct robot {
  const char * name;
  struct list * callbacks;
};



// Добавить callback к роботу, чтобы он вызывался при движении
// В callback будет передаваться направление движения
void register_callback(struct robot* robot, move_callback new_cb) {
  list_add_back(&robot->callbacks, new_cb);
}

// Отменить все подписки на события.
// Это нужно чтобы освободить зарезервированные ресурсы
// например, выделенную в куче память
void unregister_all_callbacks(struct robot* robot) {
  list_destroy(robot->callbacks);
}

// Вызывается когда робот движется
// Эта функция должна вызвать все обработчики событий
void move(struct robot* robot, enum move_dir dir) {
  struct list * list = robot->callbacks;

  while (list != NULL) {
    list->value(dir);
    list = list->next;
  }
}

