//
//  kern_rad.hpp
//  WhateverRed
//
//  Copyright © 2017 vit9696. All rights reserved.
//  Copyright © 2022 ChefKiss Inc. All rights reserved.
//

#ifndef kern_rad_hpp
#define kern_rad_hpp

#include <IOKit/IOService.h>
#include <IOKit/graphics/IOFramebuffer.h>
#include <IOKit/pci/IOPCIDevice.h>

#include <Headers/kern_devinfo.hpp>
#include <Headers/kern_patcher.hpp>

#include "kern_agdc.hpp"
#include "kern_atom.hpp"
#include "kern_con.hpp"

class RAD {
   public:
    void init();
    void deinit();

    void processKernel(KernelPatcher &patcher);
    bool processKext(KernelPatcher &patcher, size_t index,
                     mach_vm_address_t address, size_t size);

   private:
    static constexpr size_t MaxGetFrameBufferProcs = 3;

    using t_getAtomObjectTableForType = void *(*)(void *that,
                                                  AtomObjectTableType type,
                                                  uint8_t *sz);
    using t_getHWInfo = IOReturn (*)(IOService *accelVideoCtx, void *hwInfo);
    using t_createFirmware = void *(*)(const void *data, uint32_t size,
                                       uint32_t param3, const char *filename);
    using t_putFirmware = bool (*)(void *that, uint32_t deviceType, void *fw);
    using t_Vega10PowerTuneConstructor = void (*)(void *that, void *param1,
                                                  void *param2);
    using t_HWEngineConstructor = void (*)(void *that);
    using t_HWEngineNew = void *(*)(size_t size);

    static RAD *callbackRAD;
    ThreadLocal<IOService *, 8> currentPropProvider;

    mach_vm_address_t orgSetProperty{}, orgGetProperty{},
        orgGetConnectorsInfoV2{};
    mach_vm_address_t orgGetConnectorsInfoV1{},
        orgTranslateAtomConnectorInfoV1{};
    mach_vm_address_t orgTranslateAtomConnectorInfoV2{},
        orgATIControllerStart{};
    mach_vm_address_t orgNotifyLinkChange{}, orgPopulateAccelConfig[1]{},
        orgGetHWInfo[1]{};
    uint32_t *orgDeviceTypeTable = nullptr;
    mach_vm_address_t orgAmdTtlServicesConstructor{};
    mach_vm_address_t orgGetState{}, orgInitializeTtl{};
    mach_vm_address_t orgCreateAtomBiosProxy{};
    mach_vm_address_t orgPopulateDeviceMemory{}, orgQueryComputeQueueIsIdle{};
    mach_vm_address_t orgAMDHWChannelWaitForIdle{};

    /**
     * AMD10000Controller
     */
    mach_vm_address_t orgPopulateDeviceInfo{};

    /**
     * X5000HWLibs
     */
    mach_vm_address_t orgIpiSmuSwInit{}, orgSmuSwInit{};
    mach_vm_address_t orgSmuInternalSwInit{}, orgSmuGetHwVersion{};
    mach_vm_address_t orgPspSwInit{}, orgGcGetHwVersion{};
    mach_vm_address_t orgInternalCosReadFw{}, orgPopulateFirmwareDirectory{};
    t_createFirmware orgCreateFirmware = nullptr;
    t_putFirmware orgPutFirmware = nullptr;
    mach_vm_address_t orgMCILUpdateGfxCGPG{}, orgQueryEngineRunningState{};
    mach_vm_address_t orgCAILQueryEngineRunningState{};
    mach_vm_address_t orgCailMonitorEngineInternalState{};
    mach_vm_address_t orgCailMonitorPerformanceCounter{};
    mach_vm_address_t orgSMUMInitialize{};
    t_Vega10PowerTuneConstructor orgVega10PowerTuneConstructor = nullptr;
    mach_vm_address_t orgCosDebugPrint{}, orgMCILDebugPrint{};
    mach_vm_address_t orgCosDebugPrintVaList{};
    mach_vm_address_t orgCosReleasePrintVaList{};
    CailAsicCapEntry *orgAsicCapsTable = nullptr;
    CailInitAsicCapEntry *orgAsicInitCapsTable = nullptr;

    /**
     * X6000
     */
    t_HWEngineNew orgGFX10VCN2EngineNew = nullptr;
    t_HWEngineConstructor orgGFX10VCN2EngineConstructor = nullptr;

    /**
     * X5000
     */
    t_HWEngineNew orgGFX9PM4EngineNew = nullptr;
    t_HWEngineConstructor orgGFX9PM4EngineConstructor = nullptr;
    t_HWEngineNew orgGFX9SDMAEngineNew = nullptr;
    t_HWEngineConstructor orgGFX9SDMAEngineConstructor = nullptr;
    mach_vm_address_t orgGetHWEngine{};
    mach_vm_address_t orgCreateAccelChannels{};
    mach_vm_address_t orgGetHWCapabilities{};

    bool force24BppMode = false;
    bool dviSingleLink = false;

    void mergeProperty(OSDictionary *props, const char *name, OSObject *value);
    void mergeProperties(OSDictionary *props, const char *prefix,
                         IOService *provider);
    void applyPropertyFixes(IOService *service, uint32_t connectorNum = 0);
    void updateConnectorsInfo(void *atomutils,
                              t_getAtomObjectTableForType gettable,
                              IOService *ctrl,
                              RADConnectors::Connector *connectors,
                              uint8_t *sz);
    void autocorrectConnectors(uint8_t *baseAddr,
                               AtomDisplayObjectPath *displayPaths,
                               uint8_t displayPathNum,
                               AtomConnectorObject *connectorObjects,
                               uint8_t connectorObjectNum,
                               RADConnectors::Connector *connectors,
                               uint8_t sz);
    void autocorrectConnector(uint8_t connector, uint8_t sense, uint8_t txmit,
                              uint8_t enc, RADConnectors::Connector *connectors,
                              uint8_t sz);
    void reprioritiseConnectors(const uint8_t *senseList, uint8_t senseNum,
                                RADConnectors::Connector *connectors,
                                uint8_t sz);

    void process24BitOutput(KernelPatcher &patcher,
                            KernelPatcher::KextInfo &info,
                            mach_vm_address_t address, size_t size);
    void processConnectorOverrides(KernelPatcher &patcher,
                                   mach_vm_address_t address, size_t size);
    static uint64_t wrapGetState(void *that);
    static bool wrapInitializeTtl(void *that, void *param1);
    static void *wrapCreateAtomBiosProxy(void *param1);
    static IOReturn wrapPopulateDeviceMemory(void *that, uint32_t reg);
    static IOReturn wrapQueryComputeQueueIsIdle(void *that, uint64_t param1);
    static bool wrapAMDHWChannelWaitForIdle(void *that, uint64_t param1);

    /**
     * AMD10000Controller
     */
    static uint16_t wrapGetFamilyId();
    static IOReturn wrapPopulateDeviceInfo(uint64_t that);

    /**
     * X5000HWLibs
     */
    static void wrapAmdTtlServicesConstructor(IOService *that,
                                              IOPCIDevice *provider);
    static uint64_t wrapIpiSmuSwInit(void *tlsInstance);
    static uint64_t wrapSmuSwInit(void *input, uint64_t *output);
    static uint32_t wrapSmuInternalSwInit(uint64_t param1, uint64_t param2,
                                          void *param3);
    static uint64_t wrapSmuGetHwVersion(uint64_t param1, uint32_t param2);
    static uint64_t wrapPspSwInit(uint32_t *param1, uint32_t *param2);
    static uint32_t wrapGcGetHwVersion(uint32_t *param1);
    static uint32_t wrapInternalCosReadFw(uint64_t param1, uint64_t *param2);
    static void wrapPopulateFirmwareDirectory(uint64_t that);
    static uint64_t wrapMCILUpdateGfxCGPG(void *param1);
    static IOReturn wrapQueryEngineRunningState(void *that, void *param1,
                                                void *param2);
    static uint64_t wrapCAILQueryEngineRunningState(void *param1,
                                                    uint32_t *param2,
                                                    uint64_t param3);
    static uint64_t wrapCailMonitorEngineInternalState(void *that,
                                                       uint32_t param1,
                                                       uint32_t *param2);
    static uint64_t wrapCailMonitorPerformanceCounter(void *that,
                                                      uint32_t *param1);
    static uint64_t wrapSMUMInitialize(uint64_t param1, uint32_t *param2,
                                       uint64_t param3);
    static void *wrapCreatePowerTuneServices(void *param1, void *param2);
    static uint64_t wrapSmuGetFwConstants();
    static bool wrapTtlDevIsVega10Device();
    static uint64_t wrapSmuInternalHwInit();
    static void wrapCosDebugPrint(char *fmt, ...);
    static void wrapMCILDebugPrint(uint32_t level_max, char *fmt,
                                   uint64_t param3, uint64_t param4,
                                   uint64_t param5, uint level);
    static void wrapCosDebugPrintVaList(void *ttl, char *header, char *fmt,
                                        va_list args);
    static void wrapCosReleasePrintVaList(void *ttl, char *header, char *fmt,
                                          va_list args);

    /**
     * X6000
     */
    static bool wrapGFX10AcceleratorStart();

    /**
     * X5000
     */
    static bool wrapAllocateHWEngines(uint64_t that);
    static void *wrapGetHWEngine(void *that, uint32_t engineType);
    static uint32_t wrapCreateAccelChannels(void *that, uint32_t param1);
    static void *wrapGetHWCapabilities(void *that);

    void processHardwareKext(KernelPatcher &patcher, size_t hwIndex,
                             mach_vm_address_t address, size_t size);

    static bool wrapSetProperty(IORegistryEntry *that, const char *aKey,
                                void *bytes, unsigned length);
    static OSObject *wrapGetProperty(IORegistryEntry *that, const char *aKey);
    static uint32_t wrapGetConnectorsInfoV1(
        void *that, RADConnectors::Connector *connectors, uint8_t *sz);
    static uint32_t wrapGetConnectorsInfoV2(
        void *that, RADConnectors::Connector *connectors, uint8_t *sz);
    static uint32_t wrapTranslateAtomConnectorInfoV1(
        void *that, RADConnectors::AtomConnectorInfo *info,
        RADConnectors::Connector *connector);
    static uint32_t wrapTranslateAtomConnectorInfoV2(
        void *that, RADConnectors::AtomConnectorInfo *info,
        RADConnectors::Connector *connector);
    static bool wrapATIControllerStart(IOService *ctrl, IOService *provider);
    static bool wrapNotifyLinkChange(void *atiDeviceControl,
                                     kAGDCRegisterLinkControlEvent_t event,
                                     void *eventData, uint32_t eventFlags);
    static bool doNotTestVram(IOService *ctrl, uint32_t reg, bool retryOnFail);
    static void updateGetHWInfo(IOService *accelVideoCtx, void *hwInfo);

    mach_vm_address_t orgCallPlatformFunctionFromDrvr{};
    static uint32_t wrapCallPlatformFunctionFromDrvr(void *that,
                                                     uint32_t param1,
                                                     void *param2, void *param3,
                                                     void *param4);
};

#endif /* kern_rad_hpp */
