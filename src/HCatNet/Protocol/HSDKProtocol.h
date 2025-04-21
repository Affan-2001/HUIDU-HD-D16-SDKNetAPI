

#ifndef __HSDKPROTOCOL_H__
#define __HSDKPROTOCOL_H__


#include <Protocol/IProtocolBase.h>
#include <HCatNetInfo.h>
#include <fstream>


namespace cat
{


/**
 * @brief The HSDKProtocol class SDK2.0, 当前SDK主要使用的协议
 */
class HSDKProtocol : public IProtocolBase
{
public:
    explicit HSDKProtocol(const EventCoreWeakPtr &core);
    ~HSDKProtocol();

    virtual bool Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData = HCatAny()) override;

    virtual std::string Protocol() const override { return Protocol::HSDKProtocol; }

    static std::list<HCatBuffer> SplitSdkData(const HCatBuffer &xml);
    static std::list<HCatBuffer> SplitFileData(const HCatBuffer &fileData);

private:
    void InitData();

    ///< 相关发送功能
    bool SendData(const HCatBuffer &data, const HCatAny &userData = HCatAny());
    bool SendNotSDKData(const HCatBuffer &data, const HCatAny &userData = HCatAny());
    bool SendSourceData(const HCatBuffer &data, const HCatAny &userData = HCatAny());

    ///< 通知信号
    void NotifyReadyData(const HCatBuffer &data, const HCatAny &userData = HCatAny());
    void NotifySendFileStatus(const HCatBuffer &data, const HCatAny &userData = HCatAny());
    void NotifyReset();

    ///< 处理协议
    bool ParseReadData(const HCatBuffer &data, const HCatAny &userData = HCatAny());
    bool ParseNegotiate(int num);
    bool ParseSDKServiceAnswer(const HCatBuffer &data);
    bool ParseSDKCmdAnswer(HCatBuffer &data);
    bool ParseHeartBeat();
    bool ParseSendSDK(hint32 cmd, const HCatBuffer &data, const HCatAny &userData);

    ///< 处理发送文件
    bool SendFileStart(const HSendFile &sendFile, const HCatAny &userData);
    bool ParseFileStartAnswer(const HCatBuffer &data);
    bool ThreadSendFile(const HCatAny &userData);
    bool SendFileEnd();

    ///< 发送延迟未发送数据
    bool DelaySend();

    ///< 获取文件大小
    hint64 GetFileSize(const std::string &filePath);
    std::string IconvFilePath(const std::string &filePath) const;

    ///< 处理日志
    void ParseLog(const std::string &log);

private:
    struct FileCallBack
    {
        std::string filePath;
        hint64 fileSize;
        hint64 sendSize;
        hint64 readSize;
        std::fstream sendFile;
        HCatBuffer sendData;
        bool GetSendData(hint64 offset = -1);
        ~FileCallBack(){
            if (sendFile.is_open()) {
                sendFile.close();
            }
        }
    };

    using threadSendFilePtr = std::shared_ptr<FileCallBack>;

private:
    std::list<delaySend> delaySend_;
    HSendProtocolFileInfo sendFileList_;
    std::string guid_;
    HCatBuffer sdkBuff_;
    HCatBuffer readBuff_;
    bool sendfaild_;            ///< 重置状态
    bool negotiate_;            ///< 协商状态
    bool processing_;           ///< 发送文件中独占
    bool threadRun_;
};


}



#endif // __HSDKPROTOCOL_H__
