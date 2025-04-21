

#ifndef __HHEXPROTOCOL_H__
#define __HHEXPROTOCOL_H__



#include <Protocol/IProtocolBase.h>


namespace cat
{


/**
 * @brief The HHexProtocol class 16进制数据转换
 * 发送的数据会从16进制转换为原始数据
 * 读取的数据会从原始数据转换成16进制数据
 */
class HHexProtocol : public IProtocolBase
{
public:
    explicit HHexProtocol(const EventCoreWeakPtr &core);
    virtual ~HHexProtocol();

    virtual bool Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData = HCatAny()) override;
    virtual std::string Protocol() const override { return Protocol::HHexRawProtocol; }

private:
    bool SendData(const HCatBuffer &data, const HCatAny &userData = HCatAny());

    ///< 处理日志
    void ParseLog(const std::string &log);

    bool DelaySend();

private:
    std::list<delaySend> delaySend_;
    bool sendfaild_;
    bool negotiate_;            ///< 协商状态
};


}


#endif // __HHEXPROTOCOL_H__
