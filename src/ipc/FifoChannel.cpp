/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** FifoChannel
*/

#include "plazza/ipc/FifoChannel.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <system_error>

namespace {

    void writeAll(int fd, const void* buf, std::size_t n)
    {
        const uint8_t* p = static_cast<const uint8_t*>(buf);
        while (n > 0) {
            ssize_t w = write(fd, p, n);
            if (w < 0) throw std::system_error(errno, std::system_category(), "write");
            p += w;
            n -= static_cast<std::size_t>(w);
        }
    }

    bool readAll(int fd, void* buf, std::size_t n)
    {
        uint8_t* p = static_cast<uint8_t*>(buf);
        while (n > 0) {
            ssize_t r = read(fd, p, n);
            if (r == 0) return false;
            if (r < 0) throw std::system_error(errno, std::system_category(), "read");
            p += r;
            n -= static_cast<std::size_t>(r);
        }
        return true;
    }

}  // namespace

void plazza::ipc::FifoChannel::create(const std::string& basePath)
{
    std::string fwd = basePath + ".fwd";
    std::string bwd = basePath + ".bwd";

    unlink(fwd.c_str());
    unlink(bwd.c_str());

    if (mkfifo(fwd.c_str(), 0600) < 0)
        throw std::system_error(errno, std::system_category(), "mkfifo " + fwd);
    if (mkfifo(bwd.c_str(), 0600) < 0) {
        unlink(fwd.c_str());
        throw std::system_error(errno, std::system_category(), "mkfifo " + bwd);
    }
}

plazza::ipc::FifoChannel::FifoChannel(std::string basePath, Role role)
    : _pathFwd(basePath + ".fwd"), _pathBwd(basePath + ".bwd"), _role(role)
{
    if (role == Role::Owner) {
        _writeFd = open(_pathFwd.c_str(), O_WRONLY);
        if (_writeFd < 0)
            throw std::system_error(errno, std::system_category(), "open " + _pathFwd);
        _readFd = open(_pathBwd.c_str(), O_RDONLY);
        if (_readFd < 0) {
            close(_writeFd);
            _writeFd = -1;
            throw std::system_error(errno, std::system_category(), "open " + _pathBwd);
        }
    } else {
        _readFd = open(_pathFwd.c_str(), O_RDONLY);
        if (_readFd < 0) throw std::system_error(errno, std::system_category(), "open " + _pathFwd);
        _writeFd = open(_pathBwd.c_str(), O_WRONLY);
        if (_writeFd < 0) {
            close(_readFd);
            _readFd = -1;
            throw std::system_error(errno, std::system_category(), "open " + _pathBwd);
        }
    }
}

plazza::ipc::FifoChannel::~FifoChannel()
{
    if (_writeFd >= 0) close(_writeFd);
    if (_readFd >= 0) close(_readFd);
    if (_role == Role::Owner) {
        unlink(_pathFwd.c_str());
        unlink(_pathBwd.c_str());
    }
}

plazza::ipc::FifoChannel::FifoChannel(FifoChannel&& other) noexcept
    : _pathFwd(std::move(other._pathFwd)),
      _pathBwd(std::move(other._pathBwd)),
      _writeFd(other._writeFd),
      _readFd(other._readFd),
      _role(other._role)
{
    other._writeFd = -1;
    other._readFd = -1;
}

plazza::ipc::FifoChannel& plazza::ipc::FifoChannel::operator=(FifoChannel&& other) noexcept
{
    if (this != &other) {
        if (_writeFd >= 0) close(_writeFd);
        if (_readFd >= 0) close(_readFd);
        _pathFwd = std::move(other._pathFwd);
        _pathBwd = std::move(other._pathBwd);
        _writeFd = other._writeFd;
        _readFd = other._readFd;
        _role = other._role;
        other._writeFd = -1;
        other._readFd = -1;
    }
    return *this;
}

void plazza::ipc::FifoChannel::send(const std::vector<uint8_t>& payload)
{
    uint32_t len = static_cast<uint32_t>(payload.size());
    writeAll(_writeFd, &len, sizeof(len));
    if (len > 0) writeAll(_writeFd, payload.data(), len);
}

std::optional<std::vector<uint8_t>> plazza::ipc::FifoChannel::recv()
{
    uint32_t len = 0;
    if (!readAll(_readFd, &len, sizeof(len))) return std::nullopt;
    std::vector<uint8_t> payload(len);
    if (len > 0 && !readAll(_readFd, payload.data(), len)) return std::nullopt;
    return payload;
}

void plazza::ipc::FifoChannel::closeWrite()
{
    if (_writeFd >= 0) {
        close(_writeFd);
        _writeFd = -1;
    }
}
