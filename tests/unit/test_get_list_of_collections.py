import requests
import time

url = "http://127.0.0.1:8001/indexes/collections"

headers = {"Content-Type": "application/json"}

start_time = time.time()

response = requests.get(url, headers=headers)

end_time = time.time()

request_time = end_time - start_time

print(f"Status Code: {response.status_code}")
print(f"Response: {response.json()}")
print(f"⏱️ Время выполнения запроса: {request_time * 1000:.2f} миллисекунд")
