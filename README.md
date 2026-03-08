# FullTextSearchService


### Реализовано

- [x] Загрузка документов, пересборка индекса (многопоточная индексация)
- [x] Список документов с пагинацией
- [x] Полнотекстовый поиск с ранжированием (TF, нормализация, сортировка)
- [x] Персистентность индекса (docs.dat, dict.dat, сохранение/загрузка)

### Зависимости

- C++ 23
- GCC 14
- CMake 3.22

## Сборка и Запуск

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

./build/fulltext-search-service
```

## API

### Загрузка документов

```bash
curl -X POST 'http://localhost:8000/indexes/documents' \
  -H 'Content-Type: application/json' \
  -d '[{"content": "первый документ"}, {"content": "второй документ"}]'
```

Индекс пересобирается по загруженному набору

| Поле      | Тип    | Описание        |
|-----------|--------|-----------------|
| `content` | string | Текст документа |

Пример ответа

```json
{
    "received": 2
}
```

### Список документов

```bash
curl 'http://localhost:8000/indexes/documents?offset=0&limit=10'
```

| Параметр | По умолчанию | Описание |
|----------|--------------|----------|
| `limit`  | int          | 20       | до 100   |
| `offset` | int          | 0        |          |

Пример ответа

```json
{
    "results": [
        {
            "id": 0,
            "content": "текст первого документа"
        },
        {
            "id": 1,
            "content": "текст второго документа"
        }
    ],
    "limit": 10,
    "offset": 0,
    "total": 2
}
```

### Поиск

```bash
curl -X POST 'http://localhost:8000/indexes/search' \
  -H 'Content-Type: application/json' \
  -d '{"q": "второго", "limit": 5}'
```

| Параметр | Тип    | По умолчанию | Описание |
|----------|--------|--------------|----------|
| `q`      | string | `""`         |          |
| `limit`  | int    | 20           | до 100   |
| `offset` | int    | 0            |          |

Пример ответа

```json
{
    "hits": [
        {
            "id": 0,
            "content": "текст второго документа",
            "_rankingScore": 1.0
        }
    ],
    "limit": 5,
    "offset": 0,
    "estimatedTotalHits": 1,
    "processingTimeMs": 0,
    "query": "второго"
}
```
