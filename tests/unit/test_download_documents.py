import pytest
import requests
import time
import json
import sys

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
                payload = documents
                expected_count = len(payload)
            elif isinstance(documents, dict) and "content" in documents:
                payload = [{"content": documents["content"]}]
                expected_count = 1
            elif isinstance(documents, dict) and "documents" in documents:
                payload = [{"content": doc} for doc in documents["documents"]]
                expected_count = len(payload)
            else:
                raise ValueError(f"Неизвестный формат: {type(documents)}")
            return payload, expected_count

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
        """Тест с полной переиндексацией"""
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
        """Тест с query-параметром incremental=true"""
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
        """Тест с query-параметром incremental='1'"""
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
        """Тест с query-параметром incremental=1"""
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
        """Тест с query-параметром incremental=yes"""
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
        """Тест с обновлением документа"""
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
        """Тест с обновлением и замещением документов"""
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

    def test_invalid_json_format(self):
        """Тест с некорректным JSON форматом"""
        invalid_payload = {"invalid": "format"}

        response = requests.post(url, headers=headers, json=invalid_payload)
        assert response.status_code == 400
        response_json = response.json()
        print(response.status_code)
        assert "code" in response_json
        assert "message" in response_json
        assert response_json["code"] == "invalid_request"
        assert "массивом документов" in response_json["message"]

    def test_invalid_incremental_value(self, load_and_prepare):
        """Тест с невалидным значением incremental"""
        payload, expected_count = load_and_prepare('../data_documents/documents_1.json')

        response = requests.post(url, params={'incremental': 'invalid_value'}, headers=headers, json=payload)

        assert response.status_code == 202

        result = response.json()
        assert "received" in result
        assert result["received"] == expected_count

    def test_without_required_fields(self):
        """Тест с отсутствием обязательных полей"""
        payload = [{"wrong_field": "content"}]

        response = requests.post(url, headers=headers, json=payload)

        assert response.status_code == 400

    def test_idempotency(self, load_and_prepare):
        """ Тест идемпотентности - повторная отправка тех же данных"""
        payload, expected_count = load_and_prepare('../data_documents/documents_1.json')

        response1 = requests.post(url, headers=headers, json=payload)
        result1 = response1.json()

        response2 = requests.post(url, headers=headers, json=payload)
        result2 = response2.json()

        assert response1.status_code == response2.status_code
        assert result1["received"] == result2["received"]

        if "inserted" in result2:
            assert result2["inserted"] == 0 or result2["updated"] > 0

    def test_data_resave(self, load_and_prepare):
        """Тест сохранения данных после переиндексации"""
        payload1, count1 = load_and_prepare('../data_documents/base_documents.json')
        response1 = requests.post(url, headers=headers, json=payload1)
        assert response1.status_code == 202

        payload2, count2 = load_and_prepare('../data_documents/documents_2.json')
        response2 = requests.post(url, params={'incremental': 'true'}, headers=headers, json=payload2)
        result2 = response2.json()

        assert result2["received"] == count2
        assert count2 != count1

    def test_max_document_size_different_counts(self):
        """Тест на производительность с разным количеством документов"""
        large_content = "x" * 1000000

        document_counts = [1, 3, 5, 10]

        for count in document_counts:
            payload = []
            for i in range(1, count + 1):
                payload.append({
                    "content": {
                        "id": i,
                        "name": f"Документ {i}",
                        "content": large_content
                    }
                })

            expected_count = len(payload)

            payload_size_bytes = sys.getsizeof(json.dumps(payload))
            payload_size_mb = payload_size_bytes / (1024 * 1024)

            if payload_size_mb <= 1:
                time_limit_ms = 2000
            elif payload_size_mb <= 5:
                time_limit_ms = 3000
            elif payload_size_mb <= 10:
                time_limit_ms = 4000
            else:
                time_limit_ms = 5000

            print(f"\n{'=' * 50}")
            print(f"📊 Тест с {count} документами")
            print(f"Размер данных: {payload_size_mb:.2f} МБ")
            print(f"Ожидаемый лимит: {time_limit_ms} мс")

            start_time = time.time()
            response = requests.post(url, headers=headers, json=payload)
            end_time = time.time()

            assert response.status_code == 202
            execution_time_sec = end_time - start_time
            execution_time_ms = execution_time_sec * 1000

            assert execution_time_ms < time_limit_ms, \
                f"Для {count} документов время {execution_time_ms:.2f}мс превышает лимит {time_limit_ms}мс"

            print(f" Время: {execution_time_ms:.2f} мс")
            print(f" Пропускная способность: {payload_size_mb / execution_time_sec:.2f} МБ/сек")
            print(f" Среднее время на документ: {execution_time_ms / expected_count:.2f} мс")

    def test_max_document_size_mixed_sizes(self):
        """Тест производительности с документами разного размера"""

        payload = []

        payload.append({
            "content": {
                "id": 1,
                "name": "Маленький документ",
                "content": "x" * 100000  # 0.1MB
            }
        })

        payload.append({
            "content": {
                "id": 2,
                "name": "Средний документ",
                "content": "x" * 500000  # 0.5MB
            }
        })

        payload.append({
            "content": {
                "id": 3,
                "name": "Большой документ",
                "content": "x" * 1000000  # 1MB
            }
        })

        payload.append({
            "content": {
                "id": 4,
                "name": "Очень большой документ",
                "content": "x" * 2000000  # 2MB
            }
        })

        expected_count = len(payload)

        payload_size_bytes = sys.getsizeof(json.dumps(payload))
        payload_size_mb = payload_size_bytes / (1024 * 1024)

        if payload_size_mb <= 1:
            time_limit_ms = 2000
        elif payload_size_mb <= 5:
            time_limit_ms = 3000
        elif payload_size_mb <= 10:
            time_limit_ms = 4000
        else:
            time_limit_ms = 5000

        print(f"\n{'=' * 50}")
        print(f"📊 Тест с документами разного размера")
        print(f"Количество документов: {expected_count}")
        print(f"Общий размер данных: {payload_size_mb:.2f} МБ")
        print(f"Ожидаемый лимит: {time_limit_ms} мс")

        start_time = time.time()
        response = requests.post(url, headers=headers, json=payload)
        end_time = time.time()

        assert response.status_code == 202
        execution_time_ms = (end_time - start_time) * 1000

        assert execution_time_ms < time_limit_ms, \
            f"Время {execution_time_ms:.2f}мс превышает лимит {time_limit_ms}мс"

        print(f"✅ Время обработки: {execution_time_ms:.2f} мс")
        print(f"🚀 Пропускная способность: {payload_size_mb / (execution_time_ms / 1000):.2f} МБ/сек")
        print(f"📊 Среднее время на документ: {execution_time_ms / expected_count:.2f} мс")
