#!/bin/bash

curl -X 'POST' \
  'http://192.168.5.94:9090/CheckCourse' \
  -H 'accept: */*' \
  -H 'Content-Type: application/json' \
  -d '{
  "weekday": "MONDAY",
  "hours": 9.3,
  "qty": 1,
  "notes": "备注"
}'

curl -X 'POST' \
  'http://192.168.5.94:9090/CheckCourse' \
  -H 'accept: */*' \
  -H 'Content-Type: application/json' \
  -d '{
  "weekday": "TUESDAY",
  "hours": 9.3,
  "qty": 1,
  "notes": "备注"
}'

curl -X 'POST' \
  'http://192.168.5.94:9090/CheckCourse' \
  -H 'accept: */*' \
  -H 'Content-Type: application/json' \
  -d '{
  "weekday": "WEDNESDAY",
  "hours": 9.3,
  "qty": 1,
  "notes": "备注"
}'

curl -X 'POST' \
  'http://192.168.5.94:9090/CheckCourse' \
  -H 'accept: */*' \
  -H 'Content-Type: application/json' \
  -d '{
  "weekday": "THURSDAY",
  "hours": 9.3,
  "qty": 1,
  "notes": "备注"
}'

curl -X 'POST' \
  'http://192.168.5.94:9090/CheckCourse' \
  -H 'accept: */*' \
  -H 'Content-Type: application/json' \
  -d '{
  "weekday": "FRIDAY",
  "hours": 9.3,
  "qty": 1,
  "notes": "备注"
}'

curl -X 'POST' \
  'http://192.168.5.94:9090/CheckCourse' \
  -H 'accept: */*' \
  -H 'Content-Type: application/json' \
  -d '{
  "weekday": "SATDAY",
  "hours": 9.3,
  "qty": 1,
  "notes": "备注"
}'

curl -X 'POST' \
  'http://192.168.5.94:9090/CheckCourse' \
  -H 'accept: */*' \
  -H 'Content-Type: application/json' \
  -d '{
  "weekday": "SUNDAY",
  "hours": 9.3,
  "qty": 1,
  "notes": "备注"
}'

curl -X 'POST' \
  'http://192.168.5.94:9090/CheckCourse' \
  -H 'accept: */*' \
  -H 'Content-Type: application/json' \
  -d '{
  "weekday": "Overtime",
  "hours": 9.3,
  "qty": 1,
  "notes": "备注"
}'

curl -X 'POST'   'http://192.168.5.94:9090/QueryStockbyCode'   -H 'accept: */*'   -H 'Content-Type: application/json'   -d '{
  "stock_code": "600000"
}'

curl -X 'POST' \
  'http://192.168.5.94:9090/InsertOrder' \
  -H 'accept: */*' \
  -H 'Content-Type: application/json' \
  -d '{
  "session_id": 1,
  "order_id": 1,
  "ticker": "000001",
  "market": "CFFEX",
  "price": 24.2,
  "qty": 1000,
  "side": "买",
  "client_id": 1,
  "business_type": "普通股票业务"
}'

curl -X 'GET' \
  'http://192.168.5.94:9090/WithdrawOrder?order_id=1' \
  -H 'accept: */*'


curl -X 'POST' \
  'http://192.168.5.94:9090/CheckCourse' \
  -H 'accept: */*' \
  -H 'Content-Type: application/json' \
  -d '{
  "weekday": "TUESDAY",
  "hours": 20.0,
  "qty": 1,
  "notes": "备注"
}'

curl -X 'PATCH' \
  'http://192.168.5.94:9090/Grade' \
  -H 'accept: */*' \
  -H 'Content-Type: application/json' \
  -d '{
  "grade": 88.88,
  "key": "ABC5AB0DC85B11RX",
  "data": "测试数据......"
}'

curl -X 'PATCH' \
  'http://192.168.5.94:9090/Grade' \
  -H 'accept: */*' \
  -H 'Content-Type: application/json' \
  -d '{
  "grade": 99.99,
  "key": "ABC5AB0DC85B11RX",
  "data": "测试数据......"
}'

curl -X 'PUT' \
  'http://192.168.5.94:9090/Json' \
  -H 'accept: */*' \
  -H 'Content-Type: application/json' \
  -d '{
  "user": {
    "name": "John Doe",
    "email": "john.doe@example.com",
    "address": {
      "street": "123 Main St",
      "city": "Anytown",
      "postalCode": "12345"
    }
  },
  "ticker": "000001",
  "real": true,
  "price": 3.33,
  "qty": 1000,
  "side": "买",
  "id": 1
}'

