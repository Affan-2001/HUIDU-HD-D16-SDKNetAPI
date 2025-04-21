

#ifndef __HUDPFINDDEVICEPROTOCOL_H__
#define __HUDPFINDDEVICEPROTOCOL_H__



#include <Protocol/IProtocolBase.h>


namespace cat
{


/**
 * @brief The HUdpFindDeviceProtocol class udp查找设备协议
 * 兼容9527和10001的接收处理, 默认9527端口协议数据, 发送10001协议数据需要在userData传入整数10001
 * 发送任何数据都只是触发探测发送的指令, 数据本身无任何作用
 */
class HUdpFindDeviceProtocol : public IProtocolBase
{
public:
    explicit HUdpFindDeviceProtocol(const EventCoreWeakPtr &core);
    virtual ~HUdpFindDeviceProtocol();

    virtual bool Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData = HCatAny()) override;
    virtual std::string Protocol() const override { return Protocol::HUdpFindDeviceProtocol; }

    static std::string ToIp(huint32 value);

private:
    ///< 处理日志
    void ParseLog(const std::string &log);
    bool ParseReadData(const HCatBuffer &data, const HCatAny &userData);
};


}

#endif // __HUDPFINDDEVICEPROTOCOL_H__
