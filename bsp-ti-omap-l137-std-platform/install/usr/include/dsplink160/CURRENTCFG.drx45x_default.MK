#   =========================================================
#   DSP/BIOS LINK Configuration file.
#
#   CAUTION! This is a generated file.
#            All changes will be lost.
#
#   This file was generated on APR 24, 2009  13:10:28
#   =========================================================


#   =========================================================
#   When this file was created.
#   =========================================================
export  TI_DSPLINK_CFGDATE            := APR 24, 2009  13:10:28


#   =========================================================
#   DSP/BIOS LINK GPP side root directory.
#   =========================================================
export TI_DSPLINK_GPPROOT             := ${DSPLINK}\gpp
export TI_DSPLINK_DSPROOT             := ${DSPLINK}\dsp


#   =========================================================
#   GPP OS for which DSP/BIOS LINK is being built.
#   =========================================================
export TI_DSPLINK_GPPOS               := QNX
export TI_DSPLINK_GPPOSPREFIX         := 
export TI_DSPLINK_GPPDEVICE           := ARM
export TI_DSPLINK_GPPOSVERSION        := 6.4.x
export TI_DSPLINK_GPPDISTRIBUTION     := drx45x


#   =========================================================
#   Target platform for DSP/BIOS LINK.
#   =========================================================
export TI_DSPLINK_PLATFORM            := DRX45X


#   =========================================================
#   DSPs for which DSP/BIOS LINK is being built.
#   =========================================================
export TI_DSPLINK_DSPDEVICES          := DRA44XGEM 


#   =========================================================
#   External DSPS name for which DSP/BIOS LINK is being built.
#   =========================================================
export TI_DSPLINK_DSPDEVICES_EXTERNAL := DRX45XGEM 


#   =========================================================
#   DSP Config files.
#   =========================================================
export TI_DSPLINK_DSPCFGFILES        := CFG_DRX45XGEM_SHMEM.c 


#   =========================================================
#   Processor Counts and IDs.
#   =========================================================
export TI_DSPLINK_MAX_PROCESSORS     := 2
export TI_DSPLINK_MAX_DSPS           := 1
export TI_DSPLINK_ID_GPP             := 1


#   =========================================================
#   Default DSP Device and its physical interfaces for DSP/BIOS LINK.
#   =========================================================
ifndef TI_DSPLINK_BUILD_DEFAULTDSP
export  TI_DSPLINK_DSPDEVICE         := DRA44XGEM
export  TI_DSPLINK_DSPDEVICE_EXTERNAL:= DRX45XGEM
export  TI_DSPLINK_DSPDISTRIBUTION   := c64xxp_5.xx_windows
export  TI_DSPLINK_DSPOS             := DspBios
export  TI_DSPLINK_DSPOSVERSION      := 5.XX
export  TI_DSPLINK_PROCID            := 0
export  TI_DSPLINK_DSPDEVICECLASS    := C64XX
export  TI_DSPLINK_DSPTEMPPATH       := 
endif


#   =========================================================
#   Target DSP Devices and their physical interfaces for DSP/BIOS LINK.
#   =========================================================
export TI_DSPLINK_DRA44XGEM_OS                  := DspBios
export TI_DSPLINK_DRA44XGEM_DISTRIBUTION        := c64xxp_5.xx_windows
export TI_DSPLINK_DRA44XGEM_PHYINTERFACE        := SHMEM
export TI_DSPLINK_DRA44XGEM_CLASS               := C64XX
export TI_DSPLINK_DRA44XGEM_PROCID              := 0
export TI_DSPLINK_DRA44XGEM_MODE                := DSP_SWI_MODE


#   =========================================================
#   Compile time scalability options for DSP/BIOS LINK.
#   =========================================================
export TI_DSPLINK_USE_PROC            := 1
export TI_DSPLINK_USE_POOL            := 1
export TI_DSPLINK_USE_NOTIFY          := 1
export TI_DSPLINK_USE_MPCS            := 1
export TI_DSPLINK_USE_RINGIO          := 1
export TI_DSPLINK_USE_MPLIST          := 1
export TI_DSPLINK_USE_MSGQ            := 1
export TI_DSPLINK_USE_CHNL            := 0


#   =========================================================
#   Compile time physical link scalability options for device.
#   =========================================================
export TI_DSPLINK_USE_MSGQ_ZCPY_LINK  := 1
export TI_DSPLINK_USE_CHNL_ZCPY_LINK  := 0


#   =========================================================
#   Enable/ Disable Trace.
#   =========================================================
export TI_DSPLINK_TRACE  := 0


#   =========================================================
#   Enable/Diable Legacy support.
#   =========================================================
export TI_DSPLINK_LEGACY_SUPPORT  := 0


#   =========================================================
#   Filesystem used.
#   =========================================================
export TI_DSPLINK_FILESYSTEM  := PSEUDOFS


#   =========================================================
#   GPP INC PATH
#   =========================================================
export  TI_DSPLINK_GPP_INC_PATH := ${DSPLINK}\gpp\inc ${DSPLINK}\gpp\inc\usr ${DSPLINK}\gpp\inc\sys ${DSPLINK}\gpp\inc\sys\arch ${DSPLINK}\gpp\inc\sys\arch\DRA44XGEM ${DSPLINK}\gpp\inc\sys\arch\DRA44XGEM\QNX ${DSPLINK}\gpp\inc\sys\QNX ${DSPLINK}\gpp\inc\sys\QNX\6.4.x ${DSPLINK}\gpp\src\gen ${DSPLINK}\gpp\src\gen\QNX ${DSPLINK}\gpp\src\gen\QNX\6.4.x ${DSPLINK}\gpp\src\osal ${DSPLINK}\gpp\src\osal\QNX ${DSPLINK}\gpp\src\osal\QNX\6.4.x ${DSPLINK}\gpp\src\pmgr ${DSPLINK}\gpp\src\pmgr\QNX ${DSPLINK}\gpp\src\pmgr\QNX\6.4.x ${DSPLINK}\gpp\src\api ${DSPLINK}\gpp\src\api\QNX ${DSPLINK}\gpp\src\api\QNX\6.4.x ${DSPLINK}\gpp\src\ldrv ${DSPLINK}\gpp\src\ldrv\QNX ${DSPLINK}\gpp\src\ldrv\QNX\6.4.x ${DSPLINK}\gpp\src\ldrv\DRV ${DSPLINK}\gpp\src\ldrv\DATA ${DSPLINK}\gpp\src\ldrv\MPCS ${DSPLINK}\gpp\src\ldrv\MPLIST ${DSPLINK}\gpp\src\ldrv\MQT ${DSPLINK}\gpp\src\ldrv\IPS ${DSPLINK}\gpp\src\ldrv\SMM ${DSPLINK}\gpp\src\ldrv\RINGIO ${DSPLINK}\gpp\src\ldrv\POOLS

#   =========================================================
#   GPP DEFINES
#   =========================================================
export  TI_DSPLINK_GPP_DEFINES :=  MAX_DSPS=1 MAX_PROCESSORS=2 ID_GPP=1 DRX45X DSPLINK_LEGACY_SUPPORT PROC_COMPONENT POOL_COMPONENT NOTIFY_COMPONENT MPCS_COMPONENT RINGIO_COMPONENT MPLIST_COMPONENT MSGQ_COMPONENT MSGQ_ZCPY_LINK ZCPY_LINK KFILE_PSEUDO  DRA44XGEM DRA44XGEM_PHYINTERFACE=SHMEM_INTERFACE

#   =========================================================
#   DSP INC PATH
#   =========================================================
export  TI_DSPLINK_DSP0_INC_PATH := ${DSPLINK}\dsp\inc ${DSPLINK}\dsp\inc ${DSPLINK}\dsp\inc ${DSPLINK}\dsp\inc\C64XX ${DSPLINK}\dsp\inc\DspBios ${DSPLINK}\dsp\inc\DspBios\DRA44XGEM ${DSPLINK}\dsp\inc\DspBios\5.XX ${DSPLINK}\dsp\inc\DspBios\5.XX\DRA44XGEM ${DSPLINK}\dsp\src ${DSPLINK}\dsp\src\base ${DSPLINK}\dsp\src\base\hal ${DSPLINK}\dsp\src\base\hal\DspBios ${DSPLINK}\dsp\src\base\hal\DspBios\5.XX ${DSPLINK}\dsp\src\base\hal\DspBios\DRA44XGEM ${DSPLINK}\dsp\src\base\hal\DspBios\5.XX\DRA44XGEM ${DSPLINK}\dsp\src\base\drv ${DSPLINK}\dsp\src\base\drv\DspBios ${DSPLINK}\dsp\src\base\drv\DspBios\5.XX ${DSPLINK}\dsp\src\base\ips ${DSPLINK}\dsp\src\base\ips\DspBios ${DSPLINK}\dsp\src\base\ips\DspBios\5.XX ${DSPLINK}\dsp\src\base\gen ${DSPLINK}\dsp\src\base\gen\DspBios ${DSPLINK}\dsp\src\base\gen\DspBios\5.XX ${DSPLINK}\dsp\src\data ${DSPLINK}\dsp\src\data\DspBios ${DSPLINK}\dsp\src\data\DspBios\5.XX ${DSPLINK}\dsp\src\mpcs ${DSPLINK}\dsp\src\mpcs\DspBios ${DSPLINK}\dsp\src\mpcs\DspBios\5.XX ${DSPLINK}\dsp\src\mplist ${DSPLINK}\dsp\src\mplist\DspBios ${DSPLINK}\dsp\src\mplist\DspBios\5.XX ${DSPLINK}\dsp\src\msg ${DSPLINK}\dsp\src\msg\DspBios ${DSPLINK}\dsp\src\msg\DspBios\5.XX ${DSPLINK}\dsp\src\notify ${DSPLINK}\dsp\src\notify\DspBios ${DSPLINK}\dsp\src\notify\DspBios\5.XX ${DSPLINK}\dsp\src\pools ${DSPLINK}\dsp\src\pools\DspBios ${DSPLINK}\dsp\src\pools\DspBios\5.XX ${DSPLINK}\dsp\src\ringio ${DSPLINK}\dsp\src\ringio\DspBios ${DSPLINK}\dsp\src\ringio\DspBios\5.XX



#   =========================================================
#   DSP SPECIFIC DEFINES
#   =========================================================
export  TI_DSPLINK_DSP0_DEFINES :=  PROCID=0 DRA44XGEM DRA44XGEM_INTERFACE=SHMEM_INTERFACE PHYINTERFACE=SHMEM_INTERFACE DSP_SWI_MODE

#   =========================================================
#   DSP COMMON DEFINES
#   =========================================================
export  TI_DSPLINK_DSP_COMMON_DEFINES :=  MAX_DSPS=1 MAX_PROCESSORS=2 ID_GPP=1 DRX45X DSPLINK_LEGACY_SUPPORT PROC_COMPONENT POOL_COMPONENT NOTIFY_COMPONENT MPCS_COMPONENT RINGIO_COMPONENT MPLIST_COMPONENT MSGQ_COMPONENT MSGQ_ZCPY_LINK ZCPY_LINK

