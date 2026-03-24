import json

with open('../data_documents/base_documents.json', 'r', encoding='utf-8') as f:
    data = json.load(f)
total_symbols = 0
total_words = 0

for doc in data:
    content = doc['content']
    doc_id = doc.get('id', 'Неизвестный ID')
    doc_name = doc.get('name', 'Без названия')
    symbols = len(content)
    words = len(content.split())

    print(f"Документ {doc_id} («{doc_name}»):")
    print(f"  Символов: {symbols}")
    print(f"  Слов: {words}")

    total_symbols += symbols
    total_words += words

    print("\n" + "=" * 40)
    print(f"ОБЩЕЕ:")
    print(f"  Всего символов: {total_symbols}")
    print(f"  Всего слов: {total_words}")
    print(f"  Количество документов: {len(data)}")
