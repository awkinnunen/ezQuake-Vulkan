// FRIENDS-002, OpenAI Codex, 2026-09-16. GPL-2.0-or-later.
#pragma once
#include "protocol.h"
#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#endif
#include <memory>
#include <atomic>
#include <thread>

namespace friends {
class Broker {
#ifdef _WIN32
    HINTERNET session=nullptr,connection=nullptr,socket=nullptr;
    std::thread receiver;
    std::atomic<bool> stopping{false};
    std::atomic<DWORD> failure{0};
    Queue<std::vector<char>> messages;
    void receive();
#else
    struct Native;
    std::unique_ptr<Native> native;
#endif
public:
#ifndef _WIN32
    Broker();
#endif
    ~Broker();
    void open(bool host,const std::string& room);
    void send(uint8_t command,uint16_t peer,const std::string& body);
    bool poll(Message& msg);
    void close();
};
}
