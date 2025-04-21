

#include <Protocol/HStruct.h>
#include <HCatTool.h>
#include <string>



#define CMDSTR(value) \
    case value: { result = #value; } break


using namespace cat;


std::string HCmdType::HCmdTypeStr(hint32 cmd)
{
    std::string result;

    switch (cmd) {
    CMDSTR(kTcpHeartbeatAsk);
    CMDSTR(kTcpHeartbeatAnswer);
    CMDSTR(kSearchDeviceAsk);
    CMDSTR(kSearchDeviceAnswer);
    CMDSTR(kErrorAnswer);

    CMDSTR(kLCDServiceAsk);
    CMDSTR(kLCDServiceAnswer);
    CMDSTR(kLCDMsgAsk);
    CMDSTR(kLCDMsgAnswer);

    CMDSTR(kSDKServiceAsk);
    CMDSTR(kSDKServiceAnswer);
    CMDSTR(kSDKCmdAsk);
    CMDSTR(kSDKCmdAnswer);
    CMDSTR(kGPSInfoAnswer);
    CMDSTR(kFileStartAsk);
    CMDSTR(kFileStartAnswer);
    CMDSTR(kFileContentAsk);
    CMDSTR(kFileContentAnswer);
    CMDSTR(kFileEndAsk);
    CMDSTR(kFileEndAnswer);
    CMDSTR(kReadFileAsk);
    CMDSTR(kReadFileAnswer);

    CMDSTR(kFileListStartAsk);
    CMDSTR(kFileListStartAnswer);
    CMDSTR(kFileListEndAsk);
    CMDSTR(kFileListEndAnswer);
    default:
        result = cat::HCatTool::ToType<std::string>(cmd);
        break;
    }

    return result;
}



std::string HOldCmdType::HOldCmdTypeStr(hint32 cmd)
{
    std::string result;

    switch (cmd) {
    CMDSTR(kSearchAsk);
    CMDSTR(kSearchAnswer);
    CMDSTR(kDeviceInfoAsk);
    CMDSTR(kDeviceInfoAnswer);
    CMDSTR(kUpdateDeviceInfoAsk);
    CMDSTR(kUpdateDeviceInfoAnswer);
    CMDSTR(kSetNetAddrAsk);
    CMDSTR(kSetNetAddrAnswer);
    CMDSTR(kGetNetAddrAsk);
    CMDSTR(kGetNetAddrAnswer);
    CMDSTR(kVersionAsk);
    CMDSTR(kVersionAnswer);
    CMDSTR(kUpdateProjectAsk);
    CMDSTR(kUpdateProjectAnswer);
    CMDSTR(kFreeSpaceSizeAsk);
    CMDSTR(kFreeSpaceSizeAnswer);
    CMDSTR(kFileListAsk);
    CMDSTR(kFileListAnswer);
    CMDSTR(kImcompleteFileAsk);
    CMDSTR(kImcompleteFileAnswer);
    CMDSTR(kRemoveFileListAsk);
    CMDSTR(kRemoveFileListAnswer);
    CMDSTR(kOpenFileAsk);
    CMDSTR(kOpenFileAnswer);
    CMDSTR(kFileContentAsk);
    CMDSTR(kFileContentAnswer);
    CMDSTR(kCloseFileAsk);
    CMDSTR(kCloseFileAnswer);
    CMDSTR(kTransEndAsk);
    CMDSTR(kRecvEndAnswer);
    CMDSTR(kUpdateProjectQuit);
    CMDSTR(kProjectQuitAnswer);
    CMDSTR(kFPGASettingInAsk);
    CMDSTR(kFPGASettingInAnswer);
    CMDSTR(kFPGASettingOutAsk);
    CMDSTR(kFPGASettingOutAnswer);
    CMDSTR(kFPGAParamSetAsk);
    CMDSTR(kFPGAParamSetAnswer);
    CMDSTR(kFPGASetCMDAsk);
    CMDSTR(kFPGASetCMDAnswer);
    CMDSTR(kBootScreenInAsk);
    CMDSTR(kBootScreenInAnswer);
    CMDSTR(kBootScreenOutAsk);
    CMDSTR(kBootScreenOutAnswer);
    CMDSTR(kRemoveBootScreenAsk);
    CMDSTR(kRemoveBootScreenAnswer);
    CMDSTR(kLightSetInAsk);
    CMDSTR(kLightSetInAnswer);
    CMDSTR(kLightSetOutAsk);
    CMDSTR(kLightSetOutAnswer);
    CMDSTR(kLightFileAsk);
    CMDSTR(kLightFileAnswer);
    CMDSTR(kTimeSetInAsk);
    CMDSTR(kTimeSetInAnswer);
    CMDSTR(kTimeSetOutAsk);
    CMDSTR(kTimeSetOutAnswer);
    CMDSTR(kSetTimeAsk);
    CMDSTR(kSetTimeAnswer);
    CMDSTR(kGetTimeAsk);
    CMDSTR(kGetTimeAnswer);
    CMDSTR(kScreenTestInAsk);
    CMDSTR(kScreenTestInAnswer);
    CMDSTR(kScreenTestOutAsk);
    CMDSTR(kScreenTestOutAnswer);
    CMDSTR(kScreenTestCMDAsk);
    CMDSTR(kScreenTestCMDAnswer);
    CMDSTR(kBoxPlayerInAsk);
    CMDSTR(kBoxPlayerInAnswer);
    CMDSTR(kBoxPlayerOutAsk);
    CMDSTR(kBoxPlayerOutAnswer);
    CMDSTR(kBoxPlayerPlayAsk);
    CMDSTR(kBoxPlayerPlayAnswer);
    CMDSTR(kBoxScreenTestDataAsk);
    CMDSTR(kBoxScreenTestDataAnswer);
    CMDSTR(kBoxNetworkErrorAsk);
    CMDSTR(kBoxNetworkErrorAnswer);
    CMDSTR(kBoxPlayerStopAsk);
    CMDSTR(kBoxPlayerStopAnswer);
    CMDSTR(kBoxPlayerPlayImageAsk);
    CMDSTR(kBoxPlayerPlayImageAnswer);
    CMDSTR(kBoxPlayerStopImageAsk);
    CMDSTR(kBoxPlayerStopImageAnswer);
    CMDSTR(kUpgradeInAsk);
    CMDSTR(kUpgradeInAnswer);
    CMDSTR(kUpgradeCMDAsk);
    CMDSTR(kUpgradeCMDAnswer);
    CMDSTR(kUpgradeOutAsk);
    CMDSTR(kUpgradeOutAnswer);
    CMDSTR(kScreenWidthHeightAsk);
    CMDSTR(kScreenWidthHeightAsnwer);
    CMDSTR(kUpdateIDAsk);
    CMDSTR(kUpdateIDAnswer);
    CMDSTR(kUpdateMACAsk);
    CMDSTR(kUpdateMACAnswer);
    CMDSTR(kTcpHeartbeatPacketAsk);
    CMDSTR(kTcpHeartbeatPacketAnswer);
    CMDSTR(kUpdateDevNameAsk);
    CMDSTR(kUpdateDevNameAnswer);
    CMDSTR(kSensorCMD);
    CMDSTR(kSetServerAddrAsk);
    CMDSTR(kSetServerAddrAnswer);
    CMDSTR(kGetServerAddrAsk);
    CMDSTR(kGetServerAddrAnswer);
    CMDSTR(kDeviceRebootInAsk);
    CMDSTR(kDeviceRebootInAnswer);
    CMDSTR(kRebootBackAsk);
    CMDSTR(kRebootBackAnswer);
    CMDSTR(kRebootSetAsk);
    CMDSTR(kRebootSetAnswer);
    CMDSTR(kDeviceRebootOutAsk);
    CMDSTR(kDeviceRebootOutAnswer);
    CMDSTR(kRebootNowAsk);
    CMDSTR(kRebootNowAnswer);
    CMDSTR(kKeyDefinitionsInAsk);
    CMDSTR(kKeyDefinitionsInAnswer);
    CMDSTR(kKeyDefinitionsSetInfoAsk);
    CMDSTR(kKeyDefinitionsSetInfoAnswer);
    CMDSTR(kKeyDefinitionsGetInfoAnswer);
    CMDSTR(kKeyDefinitionsGetInfoAsk);
    CMDSTR(kKeyDefinitionsOutAsk);
    CMDSTR(kKeyDefinitionsOutAnswer);
    CMDSTR(kReloadKeyDefinitionsAsk);
    CMDSTR(kBoxPlayerSwitchingProgram);
    CMDSTR(kSwitchProgramIndexAsk);
    CMDSTR(kSwitchProgramIndexAnswer);
    CMDSTR(kSwitchScreenInAsk);
    CMDSTR(kSwitchScreenInAnswer);
    CMDSTR(kSwitchScreenOutAsk);
    CMDSTR(kSwitchScreenOutAnswer);
    CMDSTR(kSwitchScreenFileAsk);
    CMDSTR(kSwitchScreenFileAnswer);
    CMDSTR(kBoxIOClientInAsk);
    CMDSTR(kBoxIOClientInAnswer);
    CMDSTR(kBoxIOClientOutAsk);
    CMDSTR(kBoxIOClientOutAnswer);
    CMDSTR(kBoxPlayerConnectChangeAsk);
    CMDSTR(kBoxPlayerConnectChangeAnswer);
    CMDSTR(kLogInAsk);
    CMDSTR(kLogInAnswer);
    CMDSTR(kLogOutAsk);
    CMDSTR(kLogOutAnswer);
    CMDSTR(kUpdateTypeAsk);
    CMDSTR(kUpdateTypeAnswer);
    CMDSTR(kProjectCompleteAsk);
    CMDSTR(kProjectCompleteAnswer);
    CMDSTR(kRemoveItemListAsk);
    CMDSTR(kRemoveItemListAnswer);
    CMDSTR(kRemoveFinishAsk);
    CMDSTR(kRemoveFinishAnswer);
    CMDSTR(kRemoveAllAsk);
    CMDSTR(kRemoveAllAnswer);
    CMDSTR(kItemStatusInAsk);
    CMDSTR(kItemStatusInAnswer);
    CMDSTR(kItemStatusOutAsk);
    CMDSTR(kItemStatusOutAnswer);
    CMDSTR(kUploadDeviceInfoAsk);
    CMDSTR(kUploadDeviceInfoAnswer);
    CMDSTR(kItemNoListAsk);
    CMDSTR(kItemNoListAnswer);
    CMDSTR(kFileMD5ListAsk);
    CMDSTR(kFileMD5ListAnswer);
    CMDSTR(kGetNoSendMD5ListAsk);
    CMDSTR(kGetNoSendMD5ListAnswer);
    CMDSTR(kFunNodePositionAsk);
    CMDSTR(kFunNodePositionAnswer);
    CMDSTR(kMemoryDataAsk);
    CMDSTR(kMemoryDataAnswer);
    CMDSTR(kItemStatusSelectFileAsk);
    CMDSTR(kItemStatusSelectFileAnswer);
    CMDSTR(kItemStatusResultFileAsk);
    CMDSTR(kItemStatusResultFileAnswer);
    CMDSTR(kSetItemToNullAsk);
    CMDSTR(kSetItemToNullAnswer);
    CMDSTR(kFunNodeUpdateAsk);
    CMDSTR(kFunNodeUpdateAnswer);
    CMDSTR(kGetItemStatusAsk);
    CMDSTR(kGetItemStatusAnswer);
    CMDSTR(kGetParseResultAsk);
    CMDSTR(kGetParseResultAnswer);
    CMDSTR(kDynamicEditInAsk);
    CMDSTR(kDynamicEditInAnswer);
    CMDSTR(kParseCmdFileAsk);
    CMDSTR(kParseCmdFileAnswer);
    CMDSTR(kDynamicEditOutAsk);
    CMDSTR(kDynamicEditOutAnswer);
    CMDSTR(kProgramFileReadyAsk);
    CMDSTR(kDownloadProgramFileAnswer);
    CMDSTR(kPppoeSetInAsk);
    CMDSTR(kPppoeSetInAnswer);
    CMDSTR(kPppoeSetOutAsk);
    CMDSTR(kPppoeSetOutAnswer);
    CMDSTR(kPppoeInfoAsk);
    CMDSTR(kPppoeInfoAnswer);
    CMDSTR(kGetPppoeInfoAsk);
    CMDSTR(kGetPppoeInfoAnswer);
    CMDSTR(kPppoeOverAsk);
    CMDSTR(kPppoeOverAnswer);
    CMDSTR(kWirelessSetInAsk);
    CMDSTR(kWirelessSetInAnswer);
    CMDSTR(kWirelessInfoAsk);
    CMDSTR(kWirelessInfoAnswer);
    CMDSTR(kGetWirelessInfoAnswer);
    CMDSTR(kGetWirelessInfoAsk);
    CMDSTR(kSetWirelessAsk);
    CMDSTR(kSetWirelessAnswer);
    CMDSTR(kWirelessOverAsk);
    CMDSTR(kWirelessOverAnswer);
    CMDSTR(kNetworkStatusAnswer);
    CMDSTR(kNetworkConnected);
    CMDSTR(kNetworkUnconnected);
    CMDSTR(kContentStartAsk);
    CMDSTR(kContentStartAnswer);
    CMDSTR(kContentDataAsk);
    CMDSTR(kContentDataAnswer);
    CMDSTR(kContentEndAsk);
    CMDSTR(kContentEndAnswer);
    CMDSTR(kAppExternInAsk);
    CMDSTR(kAppExternInAnswer);
    CMDSTR(kAppExternOutAsk);
    CMDSTR(kAppExternOutAnswer);
    CMDSTR(kTcpTranInAsk);
    CMDSTR(kTcpTranInAnswer);
    CMDSTR(kTcpTranOutAsk);
    CMDSTR(kTcpTranOutAnswer);
    CMDSTR(kUdpTranAsk);
    CMDSTR(kUdpTranAnswer);
    CMDSTR(kUpdateDeviceInfoExt1Ask);
    CMDSTR(kUpdateDeviceInfoExt1Answer);
    CMDSTR(kProgramIndexChangedAsk);
    CMDSTR(kProgramIndexChangedAnswer);
    CMDSTR(kBoxPlayerTimeZoneAsk);
    CMDSTR(kBoxPlayerTimeZoneAnswer);
    CMDSTR(kReloadAllFontsAsk);
    CMDSTR(kReloadAllFontsAnswer);
    CMDSTR(kGUIDSwitchProgramAsk);
    CMDSTR(kGUIDSwitchProgramAnswer);
    CMDSTR(kChangeProgramAsk);
    CMDSTR(kChangeProgramAnswer);
    CMDSTR(kTestDeviceLockerAsk);
    CMDSTR(kTestDeviceLockerAnswer);
    CMDSTR(kCheckDeviceLockerAsk);
    CMDSTR(kCheckDeviceLockerAnswer);
    CMDSTR(kHDMIInAsk);
    CMDSTR(kHDMIInAnswer);
    CMDSTR(kClientInfoAsk);
    CMDSTR(kClientInfoAnswer);
    CMDSTR(kReloadFPGAParamAsk);
    CMDSTR(kReloadFPGAParamAnswer);
    CMDSTR(kUpgradeFinishAsk);
    CMDSTR(kUpgradeFinishAnswer);
    CMDSTR(kConvertDataToOldAsk);
    CMDSTR(kConvertDataToOldAnswer);
    default:
        result = cat::HCatTool::ToType<std::string>(cmd);
        break;
    }

    return result;
}

std::string HErrorCode::HErrorCodeStr(hint32 cmd)
{
    std::string result;

    switch (cmd) {
    CMDSTR(kSuccess);
    CMDSTR(kWriteFinish);
    CMDSTR(kProcessError);
    CMDSTR(kVersionTooLow);
    CMDSTR(kDeviceOccupa);
    CMDSTR(kFileOccupa);
    CMDSTR(kReadFileExcessive);
    CMDSTR(kInvalidPacketLen);
    CMDSTR(kInvalidParam);
    CMDSTR(kNotSpaceToSave);
    CMDSTR(kCreateFileFailed);
    CMDSTR(kWriteFileFailed);
    CMDSTR(kReadFileFailed);
    CMDSTR(kInvalidFileData);
    CMDSTR(kFileContentError);
    CMDSTR(kOpenFileFailed);
    CMDSTR(kSeekFileFailed);
    CMDSTR(kRenameFailed);
    CMDSTR(kFileNotFound);
    CMDSTR(kFileNotFinish);
    CMDSTR(kXmlCmdTooLong);
    CMDSTR(kInvalidXmlIndex);
    CMDSTR(kParseXmlFailed);
    CMDSTR(kInvalidMethod);
    CMDSTR(kMemoryFailed);
    CMDSTR(kSystemError);
    CMDSTR(kUnsupportVideo);
    CMDSTR(kNotMediaFile);
    CMDSTR(kParseVideoFailed);
    CMDSTR(kUnsupportFrameRate);
    CMDSTR(kUnsupportResolution);
    CMDSTR(kUnsupportFormat);
    CMDSTR(kUnsupportDuration);
    CMDSTR(kDownloadFileFailed);
    CMDSTR(kDownloadingFile);
    CMDSTR(kProcessing);
    CMDSTR(kScreenNodeIsNull);
    CMDSTR(kNodeExist);
    CMDSTR(kNodeNotExist);
    CMDSTR(kPluginNotExist);
    CMDSTR(kCheckLicenseFailed);
    CMDSTR(kNotFoundWifiModule);
    CMDSTR(kTestWifiUnsuccessful);
    CMDSTR(kRunningError);
    CMDSTR(kUnsupportMethod);
    CMDSTR(kInvalidGUID);
    CMDSTR(kFirmwareFormatError);
    CMDSTR(kTagNotFound);
    CMDSTR(kAttrNotFound);
    CMDSTR(kCreateTagFailed);
    CMDSTR(kUnsupportDeviceType);
    CMDSTR(kPermissionDenied);
    CMDSTR(kPasswdTooSimple);
    CMDSTR(kUsbNotInsert);
    CMDSTR(kDelayRespond);
    CMDSTR(kShortlyReturn);
    default:
        result = cat::HCatTool::ToType<std::string>(cmd);
        break;
    }

    return result;
}
