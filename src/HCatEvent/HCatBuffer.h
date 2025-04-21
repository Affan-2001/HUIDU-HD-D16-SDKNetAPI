

#ifndef __HCATBUFFER_H__
#define __HCATBUFFER_H__


#include <memory>
#include <string>


namespace cat
{


class HCatBuffer
{
public:
    HCatBuffer() : buff_(new std::string()) {}
    explicit HCatBuffer(std::size_t len) : buff_(new std::string(len, '\0')) {}
    HCatBuffer(std::size_t len, char c) : buff_(new std::string(len, c)) {}
    HCatBuffer(const char *buff, std::size_t len) : buff_(new std::string(buff, len)) {}
    HCatBuffer(const std::string &buff) : buff_(new std::string(buff)) {}
    HCatBuffer(std::string &&buff) : buff_(new std::string(std::move(buff))) {}
    HCatBuffer(const HCatBuffer &buff) : buff_(buff.buff_) {}
    HCatBuffer(HCatBuffer &&buff) : buff_(std::move(buff.buff_)) {}

    std::string GetString() const { return *buff_; }
    std::string GetString(std::size_t len) const { return buff_->substr(0, len); }
    const std::string &GetConstString() const { return *buff_; }

    std::string::pointer Data() { Detach(); return &buff_->front(); }
    std::string::const_pointer Data() const { return &buff_->front(); }
    std::string::const_pointer ConstData() const { return buff_->data(); }
    std::size_t Size() const { return buff_->size(); }
    HCatBuffer &Swap(HCatBuffer &buff) { Detach(); buff_.swap(buff.buff_); return *this; }
    std::string::reference At(std::size_t index) {  Detach(); return buff_->at(index); }
    std::string::const_reference At(std::size_t index) const { return buff_->at(index); }
    bool Empty() const { return buff_->empty(); }
    HCatBuffer &Clear() { Detach(); buff_->clear(); return *this; }

    std::string::reference operator [](std::size_t index) { Detach(); return buff_->at(index); }
    std::string::const_reference operator [](std::size_t index) const { return buff_->at(index); }

    HCatBuffer &Insert(const char *buff, std::size_t index) { Detach(); buff_->insert(index, buff); return *this; }
    HCatBuffer &Insert(const char *buff, std::size_t index, std::size_t len) { Detach(); buff_->insert(index, buff, len); return *this; }
    HCatBuffer &Insert(const std::string &buff, std::size_t index) { Detach(); buff_->insert(index, buff); return *this; }
    HCatBuffer &Insert(const HCatBuffer &buff, std::size_t index) { Detach(); buff_->insert(index, *buff.buff_); return *this; }

    HCatBuffer &Remove(std::size_t index, std::size_t num = std::string::npos) { Detach(); buff_->erase(index, num); return *this; }

    HCatBuffer Mid(std::size_t index, std::size_t len = std::string::npos) const { return HCatBuffer(buff_->substr(index, len)); }

    std::string::size_type Find(const std::string &value, std::size_t pos = 0) const { return buff_->find(value, pos); }

    HCatBuffer &Append(const std::string &buff) { Detach(); buff_->append(buff); return *this; }
    HCatBuffer &Append(std::string &&buff) { Detach(); buff_->append(std::move(buff)); return *this; }
    HCatBuffer &Append(const HCatBuffer &buff) { Detach(); buff_->append(*buff.buff_); return *this; }
    HCatBuffer &Append(HCatBuffer &&buff) { Detach(); buff_->append(std::move(*buff.buff_)); return *this; }
    HCatBuffer &Append(const char *buff) { Detach(); buff_->append(buff); return *this; }

    HCatBuffer &Replace(const std::string &src, const std::string &dest) {
        std::string::size_type pos = buff_->find(src);
        if (pos == std::string::npos) {
            return *this;
        }

        Detach();
        buff_->replace(pos, src.size(), dest);
        return *this;
    }

    HCatBuffer &operator +=(const HCatBuffer &buff) { return Append(buff); }
    HCatBuffer &operator +=(const std::string &buff) { return Append(buff); }
    HCatBuffer &operator +=(const char *buff) { return Append(buff); }

    HCatBuffer &operator =(HCatBuffer &&buff) { Detach(); buff_->assign(std::move(*buff.buff_)); return *this; }
    HCatBuffer &operator =(const HCatBuffer &buff) { Detach(); buff_->assign(*buff.buff_); return *this; }
    HCatBuffer &operator =(const std::string &buff) { Detach(); buff_->assign(buff); return *this; }
    HCatBuffer &operator =(std::string &&buff) { Detach(); buff_->assign(std::move(buff)); return *this; }
    HCatBuffer &operator =(const char *buff) { Detach(); buff_->assign(buff); return *this; }

private:
    void Detach() {
        if (buff_.use_count() <= 1) {
            return ;
        }

        buff_.reset(new std::string(*buff_));
    }

private:
    std::shared_ptr<std::string> buff_;
};


}


#endif // __HCATBUFFER_H__
