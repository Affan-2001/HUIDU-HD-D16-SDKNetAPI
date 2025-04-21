

#ifndef __HSERIALSDKPROTOCOL_H__
#define __HSERIALSDKPROTOCOL_H__


#include <Protocol/IProtocolBase.h>


namespace cat
{


/**
 * @brief The HSerialSDKProtocol class 串口SDK. 需要通过SDK2.0设置设备开启串口SDK
 */
class HSerialSDKProtocol : public IProtocolBase
{
public:
    explicit HSerialSDKProtocol(const EventCoreWeakPtr &core);
    virtual ~HSerialSDKProtocol();

    virtual bool Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData = HCatAny()) override;
    virtual std::string Protocol() const override { return Protocol::HSerialSDKProtocol; }

    static std::list<HCatBuffer> SplitSdkData(const HCatBuffer &xml);
    static std::list<HCatBuffer> SplitSdkData(const std::string &xml) { return SplitSdkData(HCatBuffer(xml)); }

private:
    void InitData();

    bool SendData(const HCatBuffer &data, const HCatAny &userData = HCatAny());
    bool SendNotSDKData(const HCatBuffer &data, const HCatAny &userData = HCatAny());
    bool SendSourceData(const HCatBuffer &data, const HCatAny &userData = HCatAny());

    bool ParseReadData(const HCatBuffer &data, const HCatAny &userData = HCatAny());
    bool ParseSendSDK(const HCatBuffer &data, const HCatAny &userData);
    bool ParseSDKCmdAnswer(HCatBuffer &data);

    ///< 处理日志
    void ParseLog(const std::string &log);

private:
    HCatBuffer readBuff_;
    HCatBuffer sdkBuff_;
    bool sendfaild_;
};



}


#endif // __HSERIALSDKPROTOCOL_H__
