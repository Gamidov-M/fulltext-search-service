import pytest
import requests
import time
import json

collection = "dissertations"
headers = {"Content-Type": "application/json"}
url = f"http://127.0.0.1:8001/indexes/{collection}/documents"


class TestDownload:

    @pytest.fixture
    def load_and_prepare(self):
        def _prepare(file_path):
            with open(file_path, 'r', encoding='utf-8') as f:
                documents = json.load(f)

            if isinstance(documents, list):
                payload = [{"content": doc} for doc in documents]
            elif isinstance(documents, dict) and "documents" in documents:
                payload = [{"content": doc} for doc in documents["documents"]]
            else:
                raise ValueError(f"Неизвестный формат: {type(documents)}")

            return payload, len(payload)
        return _prepare

    def test_full_reindex_main(self, load_and_prepare):
        payload, expected_count = load_and_prepare('../data_documents/base_documents.json')

        start_time = time.time()
        response = requests.post(url, headers=headers, json=payload)
        end_time = time.time()

        assert response.status_code == 202
        result = response.json()
        assert result["received"] == expected_count
        assert "incremental" not in result

        print(f"\n Полная переиндексация")
        print(f" Документов: {expected_count}")
        print(f" Статус: {response.status_code}")
        print(f" Ответ: {result}")
        print(f" Время выполнения запроса: {(end_time - start_time) * 1000:.2f} миллисекунд")

    def test_full_reindex(self, load_and_prepare):
        payload, expected_count = load_and_prepare('../data_documents/documents_1.json')

        start_time = time.time()
        response = requests.post(url, headers=headers, json=payload)
        end_time = time.time()

        assert response.status_code == 202
        result = response.json()
        assert result["received"] == expected_count
        assert "incremental" not in result

        print(f"\n Полная переиндексация")
        print(f" Документов: {expected_count}")
        print(f" Статус: {response.status_code}")
        print(f" Ответ: {result}")
        print(f" Время выполнения запроса: {(end_time - start_time) * 1000:.2f} миллисекунд")

    def test_incremental_update_with_true(self, load_and_prepare):
        payload, expected_count = load_and_prepare('../data_documents/documents_2.json')

        start_time = time.time()
        response = requests.post(url, params={'incremental': 'true'}, headers=headers, json=payload)
        end_time = time.time()

        assert response.status_code == 202
        result = response.json()
        assert result["received"] == expected_count
        assert result["incremental"] is True

        print(f"\n Инкрементальное обновление c значением true")
        print(f" Документов: {expected_count}")
        print(f" Вставлено: {result.get('inserted', 0)}")
        print(f" Обновлено: {result.get('updated', 0)}")
        print(f" Статус: {response.status_code}")
        print(f" Время выполнения запроса: {(end_time - start_time) * 1000:.2f} миллисекунд")

    def test_incremental_update_with_str_1(self, load_and_prepare):
        payload, expected_count = load_and_prepare('../data_documents/documents_2.json')

        start_time = time.time()
        response = requests.post(url, params={'incremental': "1"}, headers=headers, json=payload)
        end_time = time.time()

        assert response.status_code == 202
        result = response.json()
        assert result["received"] == expected_count
        assert result["incremental"] is True

        print(f"\n Инкрементальное обновление c значением 1")
        print(f" Документов: {expected_count}")
        print(f" Вставлено: {result.get('inserted', 0)}")
        print(f" Обновлено: {result.get('updated', 0)}")
        print(f" Статус: {response.status_code}")
        print(f" Время выполнения запроса: {(end_time - start_time) * 1000:.2f} миллисекунд")

    def test_incremental_update_with_int_1(self, load_and_prepare):
        payload, expected_count = load_and_prepare('../data_documents/documents_2.json')

        start_time = time.time()
        response = requests.post(url, params={'incremental': 1}, headers=headers, json=payload)
        end_time = time.time()

        assert response.status_code == 202
        result = response.json()
        assert result["received"] == expected_count
        assert result["incremental"] is True

        print(f"\n Инкрементальное обновление c значением 1")
        print(f" Документов: {expected_count}")
        print(f" Вставлено: {result.get('inserted', 0)}")
        print(f" Обновлено: {result.get('updated', 0)}")
        print(f" Статус: {response.status_code}")
        print(f" Время выполнения запроса: {(end_time - start_time) * 1000:.2f} миллисекунд")

    def test_incremental_update_with_yes(self, load_and_prepare):
        payload, expected_count = load_and_prepare('../data_documents/documents_2.json')

        start_time = time.time()
        response = requests.post(url, params={'incremental': "yes"}, headers=headers, json=payload)
        end_time = time.time()

        assert response.status_code == 202
        result = response.json()
        assert result["received"] == expected_count
        assert result["incremental"] is True

        print(f"\n Инкрементальное обновление c значением yes")
        print(f" Документов: {expected_count}")
        print(f" Вставлено: {result.get('inserted', 0)}")
        print(f" Обновлено: {result.get('updated', 0)}")
        print(f" Статус: {response.status_code}")
        print(f" Время выполнения запроса: {(end_time - start_time) * 1000:.2f} миллисекунд")

    def test_update(self, load_and_prepare):
        payload, expected_count = load_and_prepare('../data_documents/documents_3.json')

        start_time = time.time()
        response = requests.post(url, params={'incremental': "true"}, headers=headers, json=payload)
        end_time = time.time()

        assert response.status_code == 202
        result = response.json()
        assert result["received"] == expected_count
        assert result["incremental"] is True

        print(f"\n Обновление объекта")
        print(f" Документов: {expected_count}")
        print(f" Вставлено: {result.get('inserted', 0)}")
        print(f" Обновлено: {result.get('updated', 0)}")
        print(f" Статус: {response.status_code}")
        print(f" Время выполнения запроса: {(end_time - start_time) * 1000:.2f} миллисекунд")

    def test_mixed_operations(self, load_and_prepare):
        payload, expected_count = load_and_prepare('../data_documents/documents_4.json')

        start_time = time.time()
        response = requests.post(url, params={'incremental': 'true'}, headers=headers, json=payload)
        end_time = time.time()

        assert response.status_code == 202
        result = response.json()
        assert result["received"] == expected_count
        assert result["incremental"] is True
        assert result["inserted"] + result["updated"] == result["received"]

        print(f"\n Смешанные операции")
        print(f" Документов: {expected_count}")
        print(f" Вставлено: {result.get('inserted', 0)}")
        print(f" Обновлено: {result.get('updated', 0)}")
        print(f" Статус: {response.status_code}")
        print(f" Время выполнения запроса: {(end_time - start_time) * 1000:.2f} миллисекунд")
