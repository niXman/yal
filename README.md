yal
===

Yet Another Logger

### YAL задуман быть простым. Очень простым.

Возможности:

1. Сессии: вы можете создать(`yal::logger::create()`) нужное кол-во сессий, и каждой из них задать свои параметры(путь/имя хранения логов, размер тома, лимит томов).
2. Ротация: при достижении томом указанного размера(второй аргумент для `yal::logger::create(..., 1024, ...)`) - будет создан новый том. Если окажется так, что при записи какого-то сообщения будет превышен размер тома, то это сообщение будет записано в текущий том, и только после этого будет создан новый том.
3. Именование томов: к примеру, при создании сессии вы указали следующие значения: `yal::logger::create("session1")`, тогда, префиксом томов будет `session1`. Имена томов будут следующего формата: `session1-0000-21.10.2013-01.13.26`,`session1-0001-21.10.2013-01.13.26`,`session1-0002-21.10.2013-01.13.26`. Тут, следующее значение после имени тома - номер тома. Почему четыри цифры, спросите вы? - для корректной сортировки томов при использовании любого файлового менеджера. Почему именно четыре цифры? - при создании сессии используя `yal::logger::create()`, третим аргументом передается лимит кол-ва томов, который, по умолчанию, установлен в значение `9999`(вот откуда четыре цифры). Таким образом, так как именование томов начинается с нуля, мы получим `10000` томов. В случае же, если в ходе работы программы кол-во томов превысит лимит(ну, не переставать же логировать, в самом деле!) - будет установлен новый лимит, который будет в десять раз больше предыдущего, и так далее.

### Использование

Основа YAL - класс `yal::logger`, который является потокобезопасным синглтоном. Таким образом в вашем приложении может существовать только одна копия YAL. Но это не проблема, у нас же в распоряжении есть возможность создать неограниченное кол-во сессий!

Первым делом, если вы не желаете хранить логи в текущей директории, вы должны вызвать `yal::logger::root_path()` и аргументом указать путь, который будет корневым каталогом логов. Использовать эту возможность можно только еднажды, и прежде создания сессий.

Как уже было сказанно выше, сессии создаются использованием `yal::logger::create()`, где, первым аргументом указывается имя сессии. В имени сессии так же можно указать относительный путь к каталогу в котором будут сохраняться тома. К примеру, вызвав `yal::logger::create("session1/session1")`, в каталоге который был указан при помощи `yal::logger::root_path()` будет создан подкаталог 'session1', и в нем будут храниться логи.

Так же, в распоряжении пользователя доступен метод `yal::logger::flush()`, который, принудительно сбросит буфера всех существующих сессий на диск.

Пример:
```cpp
#include <yal/yal.hpp>

int main() {
        yal::logger::root_path("/var/log/myapplication");
        auto session1 = yal::logger::create("session1");
        auto session2 = yal::logger::create("session2");
        
        ...
        
        yal::logger::flush();
}
```
