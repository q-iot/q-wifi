#ifndef NAME_DEBUG_H
#define NAME_DEBUG_H


extern u8 * const gNameGlobaPktTypes[];
extern u8 * const gNameGlobaPktRes[];
extern u8 * const gNameSysParamType[];
extern u8 * const gNameMsgType[];
extern u8 * const gNameInDevType[];
extern u8 * const gNameSignalType[];
extern u8 * const gNameBulkPktType[];
extern u8 * const gNameInfoPktType[];
extern u8 * const gNameSigPktType[];
extern u8 * const gNameWnetCtrlType[];
extern u8 * const gNameInfoName[];
extern u8 * const gNameSceneItemAct[];
extern u8 * const gNameStatusPktType[];
extern u8 * const gNameVariablePktType[];
extern u8 * const gNameSrvConnStatus[];
extern u8 * const gNameSrvValueType[];
extern u8 * const gNameVarType[];
extern u8 * const gNameVarFat[];
extern u8 * const gNameVarOpt[];
extern u8 * const gNameVarState[];
extern u8 * const gNameVarIdxType[];

extern u8 * const gNameCalcMethod[];
extern u8 * const gNameTriggerType[];
extern u8 * const gNameAppClientType[];
extern u8 * const gNameAppClientStatus[];

extern u8 * const gNameWCmdPktRes[];
extern u8 * const gNameWRecvState[];
extern u8 * const gNameWRecvEvent[];
extern u8 * const gNameWSendState[];
extern u8 * const gNameWSendEvent[];
extern u8 * const gNameWaverIoMode[];
extern u8 * const gNameWaverIoAttribMode[];
extern u8 * const gNameWaverAttribUpdMode[];
extern u8 * const gNameWNetAttribBlockName[];




//for esp wifi
extern const u8 * gNameStationStatus[];
extern const u8 * gNameWifiMode[];
extern const u8 * gNameAuthMode[];
extern const u8 * gNameWifiEvent[];
extern const u8 *gNameIoMode[];

const u8 *GlobaPktTypes(u8 Type);
//const u8 *CalcTypeToStr(COMP_METHOD Method);
//const u8 *TrigTypeToStr(u8 Type);
const u8 *VarStateToStr(u8 State);





#endif
