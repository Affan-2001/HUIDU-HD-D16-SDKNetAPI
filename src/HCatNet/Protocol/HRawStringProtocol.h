

#ifndef __HRAWSTRINGPROTOCOL_H__
#define __HRAWSTRINGPROTOCOL_H__



#include <Protocol/IProtocolBase.h>


namespace cat
{


/**
 * @brief The HRawStringProtocol class 发送原始数据和接收原始数据不做任何修改
 */
class HRawStringProtocol : public IProtocolBase
{
public:
    explicit HRawStringProtocol(const EventCoreWeakPtr &core);
    virtual ~HRawStringProtocol();

    virtual bool Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData = HCatAny()) override;
    virtual std::string Protocol() const override { return Protocol::HRawStringProtocol; }

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


#endif // __HRAWSTRINGPROTOCOL_H__
