import json

with open('data_documents.json', 'r', encoding='utf-8') as f:
    data = json.load(f)

for doc in data:
    content = doc['content']
    doc_id = doc.get('id', 'Неизвестный ID')
    doc_name = doc.get('name', 'Без названия')
    print(f"Документ {doc_id} («{doc_name}»):")
    print(f"  Символов: {len(content)}")
    print(f"  Слов: {len(content.split())}")
