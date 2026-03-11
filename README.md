# FullTextSearchService

Сервис полнотекстового поиска с HTTP API

### Реализовано

- [x] Загрузка документов, пересборка индекса
- [x] Список документов с пагинацией
- [x] Полнотекстовый поиск с ранжированием
- [x] Персистентность индекса
- [x] Конфигурация через конфиг-файл и systemd конфиг
- [x] Логирование (флаг --dev) и Dockerfile
- [x] Ранжирование BM25
- [x] Rate limiting и лимиты размера запросов
- [x] Схема документов
- [x] Документы как объект по схеме
- [x] Индексация для поиска только по строковым полям схемы

### Планируется

- [ ] Стоп-слова при индексации и в запросе
- [ ] Стемминг или лемматизация (нормализация словоформ)
- [ ] Подсветка совпадений (snippets/highlight) в результатах поиска
- [ ] Фразовый поиск (поиск точной фразы)
- [ ] Нечёткий поиск (fuzzy, поиск с опечатками)
- [ ] Инкрементальное обновление индекса
- [ ] Метаданные документов

## Сборка на хосте

#### Зависимости

- C++ 23
- GCC 14
- CMake 3.22
- Git (для загрузки сторонних библиотек через FetchContent)
- make (по умолчанию, входит в build-essential) или ninja (опционально: cmake -G Ninja -B build)

При первой конфигурации CMake скачивает исходники зависимостей (nlohmann/json, cpp-httplib, yaml-cpp) в каталог
third_party

#### Сборка

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

## Запуск

```bash
./build/fulltext-search-service
# или
./build/fulltext-search-service --config=config.yaml
```

Без опции --config конфиг обязательно читается из /etc/fulltext-search-service/config.yaml
при отсутствии или ошибке файла сервис завершается с ошибкой

```bash
./build/fulltext-search-service --config=config.yaml
```

```bash
sudo mkdir -p /var/lib/fulltext-search-service /etc/fulltext-search-service
sudo cp config.yaml /etc/fulltext-search-service/config.yaml

sudo cp build/fulltext-search-service /usr/local/bin/

sudo cp systemd/fulltext-search-service.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now fulltext-search-service
```

### Docker

Получить бинарник

```bash
docker build --target binary -t fulltext-search-service-binary .
mkdir -p build
docker run --rm -v $(pwd)/build:/output fulltext-search-service-binary
```

Собрать образ и запустить сервис

```bash
docker build -t fulltext-search-service .
docker run --rm -p 8000:8000 fulltext-search-service
```

Запуск с сохранением индекса на хосте

```bash
docker run --rm -p 8000:8000 -v $(pwd)/data:/var/lib/fulltext-search-service fulltext-search-service
```

## API

### Схема

Перед загрузкой документов нужно создать схему
Поддерживаются типы полей int, string
Строковые поля индексируются для поиска

#### Создать схему

```bash
curl -X POST 'http://127.0.0.1:8000/indexes/schemes' \
  -H 'Content-Type: application/json' \
  -d '{"fields": [{ "name": "id", "type": "int" }, { "name": "name", "type": "string" }]}'
```

Пример ответа

```json
{
  "fields": [
    {
      "name": "id",
      "type": "int"
    },
    {
      "name": "name",
      "type": "string"
    }
  ]
}
```

#### Получить схему

```bash
curl 'http://127.0.0.1:8000/indexes/schemes'
```

#### Удалить схему

```bash
curl -X DELETE 'http://127.0.0.1:8000/indexes/schemes'
```

### Загрузка документов

```bash
curl -X POST 'http://127.0.0.1:8000/indexes/documents' \
  -H 'Content-Type: application/json' \
  -d '[{"content": {"id": 1, "name": "Первый документ"}}, {"content": {"id": 2, "name": "Второй документ"}}]'
```

| Поле      | Тип    | Описание                        |
|-----------|--------|---------------------------------|
| `content` | object | Объект по схеме (поля из схемы) |

Пример ответа

```json
{
  "received": 2
}
```

### Список документов

```bash
curl 'http://127.0.0.1:8000/indexes/documents?offset=0&limit=10'
```

| Параметр | По умолчанию | Описание |
|----------|--------------|----------|
| `limit`  | int          | до 100   |
| `offset` | int          | 0        |

Пример ответа

```json
{
  "results": [
    {
      "id": 0,
      "content": {
        "id": 1,
        "name": "Первый документ"
      }
    },
    {
      "id": 1,
      "content": {
        "id": 2,
        "name": "Второй документ"
      }
    }
  ],
  "limit": 10,
  "offset": 0,
  "total": 2
}
```

### Поиск

```bash
curl -X POST 'http://127.0.0.1:8000/indexes/search' \
  -H 'Content-Type: application/json' \
  -d '{"q": "Второй", "limit": 5, "offset": 0}'
```

| Параметр | Тип    | По умолчанию | Описание |
|----------|--------|--------------|----------|
| `q`      | string | `""`         |          |
| `limit`  | int    | 20           | до 100   |
| `offset` | int    | 0            |          |

Пример ответа

```json
{
  "results": [
    {
      "id": 0,
      "content": {
        "id": 2,
        "name": "Второй документ"
      },
      "_rankingScore": 0.95
    }
  ],
  "limit": 5,
  "offset": 0,
  "total": 1,
  "processingTimeMs": 2,
  "query": "Второй"
}
```

_rankingScore - по формуле BM25 учитываются частоты слов в документе (tf), редкость слов в коллекции (idf), длина документа
processingTimeMs - время обработки запроса на сервере в миллисекундах