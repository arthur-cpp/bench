# Roadmap

## Задачи
- ~~сделать Release для класса и убрать BtDestroy~~
- ~~добавить возможность завести контекст - проще тесты будут~~
- ~~добавить расчет медианы в статистике~~
- ~~вынести запуски в рамках теста в отдельный класс~~
- добавить сохранение на диск всех данных для подробного пост-анализа (таймстемпы и тп)
- ~~сделать остановку по Ctrl+C если ввести режим кол-ва семплов `auto` - вероятно имеет смысл в рамках либо одного теста, либо в режиме асинхронного (единовременного) запуска всех тестов - это бенчмарк, нет смысла~~
- визуализатор (?) или предоставить ipynb

Тесты:
- ~~примитивы синхронизации~~
- аллокации
- события
- лок-фри очереди
	- https://www.reddit.com/r/cpp/comments/16nios9/colud_you_recommend_me_a_fast_lock_free_queue/
	- https://github.com/max0x7ba/atomic_queue
- maps
	- https://www.geeksforgeeks.org/how-to-use-unordered_map-efficiently-in-c/
	- https://habr.com/ru/companies/badoo/articles/328472/
- логгирование
- OpenSSL vs alternatives

## Итог
**UPD: в итоге работаю под Linux и научился в Google Benchmark.**