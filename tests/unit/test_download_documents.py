import requests
import time
import json

collection = "dissertations"
url = f"http://127.0.0.1:8001/indexes/{collection}/documents"
headers = {"Content-Type": "application/json"}

with open('data_documents.json', 'r', encoding='utf-8') as f:
    documents = json.load(f)

payload = [{"content": doc} for doc in documents]

start_time = time.time()
response = requests.post(url, headers=headers, json=payload)
end_time = time.time()

print(f"Status Code: {response.status_code}")
print(f"Response: {response.json()}")
print(f"⏱️ Время выполнения запроса: {(end_time - start_time) * 1000:.2f} миллисекунд")
