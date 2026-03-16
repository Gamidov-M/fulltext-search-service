import requests
import time

url = "http://127.0.0.1:8001/indexes/collections"

headers = {"Content-Type": "application/json"}

payload = {"name": "dissertations", "fields": [{"name": "id", "type": "int"}, {"name": "name", "type": "string"}, {"name": "content", "type": "string"}]}

start_time = time.time()

response = requests.post(url, headers=headers, json=payload)

end_time = time.time()

request_time = end_time - start_time

print(f"Status Code: {response.status_code}")
print(f"Response: {response.json()}")
print(f"⏱️ Время выполнения запроса: {request_time * 1000:.2f} миллисекунд")

