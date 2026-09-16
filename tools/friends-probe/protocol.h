// FRIENDS-002, OpenAI Codex, 2026-09-16. GPL-2.0-or-later.
#pragma once
#include <cstdint>
#include <deque>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

namespace friends {
constexpr size_t MaxSignal = 16384;
constexpr size_t MaxDatagram = 1400;
// FTE netinc.h protocol constants; implementation here is new adapter code.
enum Command : uint8_t { PeerLost=0, Greeting=1, NewPeer=2, Offer=3,
    Candidate=4, Accept=5, ServerInfo=6, ServerUpdate=7, NameInUse=8 };
struct Message { uint8_t command; uint16_t peer; std::string body; };
struct Invite { std::string room, key, fingerprint; };
void require(bool condition, const char* error);
bool hex(const std::string& s, size_t size);
std::string encode(const Invite& invite);
Invite decode(const std::string& text);
std::vector<char> pack(uint8_t command, uint16_t peer, const std::string& body);
Message unpack(const std::vector<char>& data);
std::string fingerprintFromSDP(const std::string& text);
int selfTest();

// All callbacks enqueue bounded data. Crypto and admission run on the main thread.
template<typename T> class Queue {
    std::mutex mutex;
    std::deque<T> items;
    bool overflow = false;
public:
    bool push(T value) {
        std::lock_guard<std::mutex> guard(mutex);
        if (items.size() >= 128) { overflow=true; return false; }
        items.push_back(std::move(value)); return true;
    }
    bool pop(T& value) {
        std::lock_guard<std::mutex> guard(mutex);
        require(!overflow, "Bounded network queue overflow");
        if (items.empty()) return false;
        value=std::move(items.front()); items.pop_front(); return true;
    }
};
}
