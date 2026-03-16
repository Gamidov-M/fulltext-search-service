import requests
import time

url = "http://127.0.0.1:8001/indexes/collections"

collection = "/4t4t4t"

start_time = time.time()

response = requests.delete(f"{url}{collection}")

end_time = time.time()

request_time = end_time - start_time

print(f"Status Code: {response.status_code}")
print(f"Response: {response.json()}")
print(f"⏱️ Время выполнения запроса: {request_time * 1000:.2f} миллисекунд")
