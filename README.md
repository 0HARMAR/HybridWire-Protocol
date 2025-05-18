# HybridWire Protocol (HWP)

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://github.com/yourname/hybridwire/actions/workflows/cmake.yml/badge.svg)](https://github.com/yourname/hybridwire/actions)

HybridWire 是一个基于TCP的双模网络协议，**同时支持HTTP短连接和自定义长连接会话**，适用于需要灵活通信场景的应用程序。

## 🚀 功能特性

- **双模式兼容**
    - `HTTP Mode`: 完全兼容标准HTTP/1.1请求
    - `Wire Mode`: 自定义二进制协议，支持持久会话
- **高性能**：基于Boost.Asio的异步IO模型
- **会话管理**：内置LRU缓存会话池（TTL可配置）
- **安全传输**：可选TLS加密（OpenSSL集成）
- **跨平台**：支持Linux/macOS/Windows

## 📦 安装依赖

### 必需组件
- C++20 编译器（GCC/Clang/MSVC）
- [Boost.Asio](https://www.boost.org/) (1.70+)

### 可选组件
- [msgpack-c](https://msgpack.org/)（用于二进制序列化）
- OpenSSL（TLS支持）

**Ubuntu/Debian:**
```bash
sudo apt install libboost-dev libmsgpack-dev openssl