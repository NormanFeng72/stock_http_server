# 模拟股票交易 WebService 程序（模糊测试靶场）

本项目是一个使用 C/C++ 编写的 WebService 程序，用于模拟股票交易系统，作为模糊测试（Fuzz Testing）的靶场程序。项目基于 [libhv](https://github.com/ithewei/libhv ) 网络库和 [cJSON](https://github.com/DaveGamble/cJSON ) JSON 解析库构建，采用 CSV 静态数据模拟股票行情，支持基本的行情查询与下单功能。

该项目主要用于测试模糊测试工具对金融类 Web 接口的探测能力，可扩展性强，便于集成到 fuzzing 测试平台中。

---

## 🧩 功能概述

- **HTTP WebService**：提供 RESTful API 接口
- **股票行情查询接口**：读取本地 CSV 数据，返回实时行情模拟数据
- **股票下单接口**：接收 JSON 格式的下单请求并返回响应
- **易于 fuzzing**：无业务逻辑依赖外部服务，适合作为模糊测试目标
- **轻量级部署**：不依赖数据库或其他中间件，便于部署运行

---

## 📦 技术栈

| 技术 | 说明 |
|------|------|
| libhv | 跨平台网络库，用于构建 HTTP 服务 |
| cJSON | C语言实现的轻量级 JSON 解析/生成库 |
| CSV 文件 | 存储静态行情数据，用于模拟市场行情 |
| CMake | 构建管理工具 |

---

## 📁 项目结构
