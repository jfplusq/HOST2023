// ========================================================================================================
// ========================================================================================================
// ***************************************** device_hardware.h ********************************************
// ========================================================================================================
// ========================================================================================================
//
//--------------------------------------------------------------------------------
// Company: IC-Safety, LLC and University of New Mexico
// Engineer: Professor Jim Plusquellic
// Exclusive License: IC-Safety, LLC
// Copyright: Univ. of New Mexico
//--------------------------------------------------------------------------------

#ifndef DEVICE_HARDWARE
#define DEVICE_HARDWARE

// ==============
// OUTPUTS TO PUF
#define OUT_CP_RESET 31
#define OUT_CP_PUF_START 30

#define OUT_CP_NUM_SAM1 29
#define OUT_CP_NUM_SAM0 28

#define OUT_CP_REUSE_PNS_MODE 27

#define OUT_CP_LFSR_LOAD_SEED 26

#define OUT_CP_LM_ULM_DONE 25
#define OUT_CP_HANDSHAKE 24

// "000" DeviceAuthentication, "001" VerifierAuthentication, "010" SessionKeyGen, "011" Provisioning
#define OUT_CP_MODE1 23
#define OUT_CP_MODE0 22
#define OUT_CP_KEK 21

#define OUT_CP_DTO_DATA_READY 20
#define OUT_CP_DTO_RESTART 19
#define OUT_CP_DTO_VEC_LOADED 18

#define OUT_CP_DO_CALIBRATION 16

// ==============
// INPUTS FROM PUF
#define IN_SM_READY 31

#define IN_SM_DTO_DONE_READING 29

#define IN_SM_HANDSHAKE 28

#define IN_CAL_ERR 27

// For the TDC, I OR these two errors together.
#define IN_GPEVCAL_ERR 26
#define IN_PARAM_ERR 26

#define IN_PNDIFF_OVERFLOW_ERR 25

#define IN_MPS_3 24
#define IN_SM_LOAD_VEC_PAIR 23
#define IN_SM_DONE_ALL_VECS 22
#define IN_BG_SBS_DONE_OR_MPS_0 21

#define IN_FU_OUTPUT_END 20
#define IN_FU_OUTPUT_START 12

// GPIO 0 -- /borg_data/ version
//#define GPIO_0_BASE_ADDR 0x41200000
//#define CTRL_DIRECTION_MASK 0x00
//#define DATA_DIRECTION_MASK 0xFFFFFFFF

// GPIO 0
#define GPIO_0_BASE_ADDR 0x41220000
#define CTRL_DIRECTION_MASK 0x00
#define DATA_DIRECTION_MASK 0xFFFFFFFF

#endif
