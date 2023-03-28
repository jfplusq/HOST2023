// ========================================================================================================
// ========================================================================================================
// ******************************************* verifier_common.h ******************************************
// ========================================================================================================
// ========================================================================================================
//
//--------------------------------------------------------------------------------
// Company: IC-Safety, LLC and University of New Mexico
// Engineer: Professor Jim Plusquellic
// Exclusive License: IC-Safety, LLC
// Copyright: Univ. of New Mexico
//--------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <sqlite3.h>
#include "commonDB.h"
#include "common.h"

#ifndef SRFAlgoStruct 

typedef struct
   {
   int SBS_num_bits;
   int SBS_num_bytes;
   int SHD_num_bytes;
   unsigned char *SBS;
   unsigned char *SHD;
   } HelpBitstringStruct;

typedef struct
   {
   char *DB_name_NAT;
   char *DB_name_AT;
   sqlite3 *database_NAT;
   sqlite3 *database_AT;
   sqlite3 *database_RT;

   pthread_mutex_t *RT_DB_mutex_ptr;
   pthread_mutex_t *FileStat_mutex_ptr;
   pthread_mutex_t *GenChallenge_mutex_ptr; 
   pthread_mutex_t *Authentication_mutex_ptr; 

   pthread_mutex_t *PUFCash_WRec_DB_mutex_ptr;
   pthread_mutex_t *PUFCash_POP_DB_mutex_ptr;

   char *Netlist_name;
   char *Synthesis_name;
   int design_index;
   int num_PIs;
   int num_POs;

   char *ChallengeSetName_NAT;
   char *ChallengeSetName_AT;
   int gen_random_challenge;

   int use_database_chlngs;
   unsigned int DB_ChallengeGen_seed;

   int fix_params;

   float **PNR; 
   float **PNF;
   float *fPND; 
   float *fPNDc; 
   float *fPNDco; 

   float *MedianPNR;
   float *MedianPNF;
   int num_qualified_rise_PNs;
   int num_qualified_fall_PNs;

   int num_required_PNDiffs;
   float *fSpreadFactors;

   int num_SF_bytes;
   int num_SF_words;
   int iSpreadFactorScaler;
   signed char *iSpreadFactors;

   unsigned int SpreadFactor_random_seed;

   unsigned char *verifier_SHD;
   unsigned char *verifier_SBS;
   unsigned char *device_SHD;
   unsigned char *device_SBS;

   int verifier_SHD_num_bytes; 
   int verifier_SBS_num_bytes; 
   int device_SHD_num_bytes; 
   int device_SBS_num_bytes; 

   int do_save_bitstrings_to_RT_DB;
   int do_save_PARCE_COBRA_file_stats;
   int do_save_COBRA_SHD;
   int do_save_SKE_SHD;
   unsigned char *DHD_HD; 
   unsigned char *verifier_DHD_SBS; 
   unsigned char *device_DHD_SBS; 
   int DHD_SBS_num_bits; 

   unsigned char *verifier_n2;
   unsigned char *XOR_nonce;

   int nonce_base_address;
   int num_required_nonce_bytes; 

   int dist_range;
   int range_low_limit;
   int range_high_limit;

   int vec_chunk_size;
   int XMR_val;

   unsigned char AES_IV[AES_IV_NUM_BYTES];

   unsigned int SE_target_num_key_bits;
   unsigned char *SE_final_key;
   int authen_min_bitstring_size;

// We do NOT have the KEK key on the verifier, ONLY ON THE DEVICE. But we can use this field to get statistics on the KEK key by having
// the device transmit it to the verifier and then store it in the statistics database.
   unsigned int KEK_target_num_key_bits; 
   unsigned char *KEK_final_enroll_key;

   unsigned char *KEK_authentication_nonce; 
   int num_KEK_authen_nonce_bits;
   unsigned char *KEK_authen_XMR_SHD_chunk; 
   unsigned char *DA_cobra_key;
   unsigned char *DA_nonce_reproduced;

   int num_chips; 

   int num_vecs;
   int num_rise_vecs;
   int has_masks;
   unsigned char **first_vecs_b; 
   unsigned char **second_vecs_b; 
   unsigned char **masks_b; 

   int chip_num; 
   int anon_chip_num; 

   unsigned int param_LFSR_seed_low;
   unsigned int param_LFSR_seed_high;
   unsigned int param_RangeConstant;
   unsigned short param_SpreadConstant;
   unsigned short param_Threshold;
   unsigned short param_TrimCodeConstant;
   int param_PCR_or_PBD_or_PO; 

   float *ChipScalingConstantArr;
   int *ChipScalingConstantNotifiedArr;

   int do_PO_dist_flip; 

   int use_TVC_cache;
   TimingValCacheStruct *TVC_arr_NAT;
   int num_TVC_arr_NAT;
   TimingValCacheStruct *TVC_arr_AT;
   int num_TVC_arr_AT;

   HelpBitstringStruct *HBS_arr;

   int first_chip_num;
   int first_num_match_bits;
   int first_num_mismatch_bits;
   int second_chip_num;
   int second_num_match_bits;
   int second_num_mismatch_bits;

   int do_COBRA; 
   int COBRA_AND_correlation_results;
   int COBRA_XNOR_correlation_results;

   int my_chip_num;
   float my_scaling_constant;
   char *my_IP;
   int my_bitstream;

// 11_1_2021: MAKE protocol
   int MAT_LLK_num_bytes;

// PeerTrust protocol
   char *DB_name_MAKE_PT_AT;
   sqlite3 *DB_MAKE_PT_AT;
   int PHK_A_num_bytes;

// PUF-Cash V3.0
   char *DB_name_PUFCash_V3; 
   sqlite3 *DB_PUFCash_V3; 
   int eCt_num_bytes;

// Used for GenLLK
   int KEK_LLK_num_bytes; 

// 7_4_2022: Used for POP
   int POP_LLK_num_bytes; 

   int DUMP_BITSTRINGS; 
   int DEBUG_FLAG; 
   } SRFAlgoParamsStruct;

typedef struct
   {
   int num_Accts; 
   int *Accts_arr; 
   int *PI_indexes; 
   int *Amount_arr; 

// 11_14_2021: PUF-Cash 3.0
   int *Withdraw_ID; 
   } AccountStruct;

#define SRFAlgoStruct 
#endif

// **************************** LABVIEW **********************************
// CoreSM0 is the Agilent B2910A 
#define InstrCoreSM0 "CoreSrcMeter0"

// CoreSM1 is the Keithley 2400 
#define InstrCoreSM1 "CoreSrcMeter1"

#define InstrTC "TempChamber"
#define CommandPwrUp "PUP"
#define CommandPwrDwn "PWN"
#define CommandReadCurrentAmplitude "ReadCurrConfigAmpl"
#define CommandSetCurrentAmplitude "SetCurrConfigAmpl" 
#define CommandReadTemperature "ReadTemperature" 
#define CommandSetTemperature "SetTemperature" 
#define CommandSetTemperatureNB "SetTemperatureNB" 

