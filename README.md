## Stock Trading WebService Program (Fuzz Testing Target)
This project is a lightweight, standalone WebService program written in C/C++ , designed to simulate a stock trading system for use as a fuzz testing target . Built on top of the libhv networking library and cJSON JSON parsing library, it uses static CSV files to simulate real-time stock market data and supports basic functionalities such as querying quotes and submitting orders via HTTP APIs.

The main purpose of this project is to serve as a testbed for fuzzing tools , especially for evaluating the robustness and security of financial-grade web interfaces. It has no external dependencies or complex business logic, making it ideal for integration into fuzz testing platforms like AFL, libFuzzer, or Boofuzz.

## 🧩 Features
✅ HTTP WebService : Provides RESTful API endpoints
✅ Stock Quote Query Interface : Reads local CSV files to return simulated real-time stock data
✅ Order Submission Interface : Accepts JSON-formatted order requests and returns responses
✅ Fuzz-friendly Design : No external dependencies or complex business logic
✅ Lightweight & Standalone : No reliance on databases or middleware, easy to deploy

## 📦 Tech Stack
| Technology | Description                                                     |
|------------|-----------------------------------------------------------------|
| libhv      | Cross-platform networking library used to build the HTTP server |
| cJSON      | Lightweight JSON parsing and generation library written in C    |
| CSV files  | Stores static stock data for simulating market quotes           |
| CMake      | Build system configuration tool                                 |
