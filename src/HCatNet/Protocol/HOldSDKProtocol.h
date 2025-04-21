

#ifndef __HOLDSDKPROTOCOL_H__
#define __HOLDSDKPROTOCOL_H__


#include <Protocol/IProtocolBase.h>


namespace cat
{


/**
 * @brief The HOldSDKProtocol class SDK1.0, 发送xml即可, 协商都会在协议内部自动完成
 */
class HOldSDKProtocol : public IProtocolBase
{
public:
    explicit HOldSDKProtocol(const EventCoreWeakPtr &core);

    virtual ~HOldSDKProtocol();
    virtual bool Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData = HCatAny()) override;
    virtual std::string Protocol() const override { return Protocol::HOldSDKProtocol; }

private:
    struct SDKData
    {
        std::list<HCatBuffer> sendData;
        huint16 cmd;
    };
    static std::list<SDKData> SplitSdkData(const HCatBuffer &xml);

private:
    bool SendData(const HCatBuffer &data, const HCatAny &userData = HCatAny());
    bool SendNotSDKData(const HCatBuffer &data, const HCatAny &userData = HCatAny());
    bool SendSourceData(const HCatBuffer &data, const HCatAny &userData = HCatAny());


    void InitData();

    bool ParseNegotiate(int num);
    bool ParseHeartBeat();
    bool ParseSendSDK(hint32 cmd, const HCatBuffer &data, const HCatAny &userData);
    bool ParseReadData(const HCatBuffer &data, const HCatAny &userData = HCatAny());

    ///< 处理日志
    void ParseLog(const std::string &log);

private:
    std::list<delaySend> delaySend_;
    std::list<SDKData> sendQueue_;
    HCatBuffer buff_;
    HCatBuffer readBuff_;
    bool negotiate_;    ///< 协商状态
    bool sendfaild_;
};



}



#endif // __HOLDSDKPROTOCOL_H__
