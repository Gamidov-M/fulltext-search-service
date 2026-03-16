import requests
import time

collection = "dissertations"

url = f"http://127.0.0.1:8001/indexes/{collection}/documents?offset=0&limit=15"

headers = {"Content-Type": "application/json"}

payload = {"limit": 15, "offset": 0}

start_time = time.time()

response = requests.get(url, headers=headers, json=payload)

end_time = time.time()

request_time = end_time - start_time

print(f"Status Code: {response.status_code}")
print(f"Response: {response.json()}")
print(f"⏱️ Время выполнения запроса: {request_time * 1000:.2f} миллисекунд")
