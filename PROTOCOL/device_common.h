// ========================================================================================================
// ========================================================================================================
// ******************************************* device_common.h ********************************************
// ========================================================================================================
// ========================================================================================================
//
//--------------------------------------------------------------------------------
// Company: IC-Safety, LLC and University of New Mexico
// Engineer: Professor Jim Plusquellic
// Exclusive License: IC-Safety, LLC
// Copyright: Univ. of New Mexico
//--------------------------------------------------------------------------------

#ifndef DEVICE_COMMON
#define DEVICE_COMMON

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <sqlite3.h>
#include "commonDB.h"
#include "common.h"

static volatile int keepRunning = 1;

// TTPs and Teds
#define MAX_CONNECT_ATTEMPTS 10

typedef struct
   {
   int index;
   int chip_num;
   int on_line;
   int self;
   int is_TTP;
   char *IP;
   unsigned char *session_key;
   float comp_power;
   float distance;
   float congestion;
   int chlng_num;
   int Ted_selected;
   int customer_authen_status;
   unsigned char *Ted_key_shard;
   unsigned char **AliceBob_key_shards;
   int AliceBob_num_key_shards;
   unsigned char *AliceBob_shared_key;
   int num_ATs;
   float trust_score;
   float belief;
   } ClientInfoStruct;

typedef struct
   {
   unsigned char *Bob_alice_SHD;
   unsigned char *Bob_RpXORn2s;
   int Bob_num_RpXORn2s;
   char *Bob_RpXORn2_num_bits_str;
   unsigned char *Bob_xor_nonce;
   } BobReceiveInfoStruct;

typedef struct
   {
   volatile unsigned int *CtrlRegA;
   volatile unsigned int *DataRegA;
   unsigned int ctrl_mask;

   char *My_IP;

// 11_1_2021: Adding this for MAKE protocol. Filled in by GenLLK(). After device authenticates successfully, 
// verifier sends its ID from the NON-ANONYMOUS Timing database to the device. The device will use this as 
// it's ID. If a KEK challenge already exists, it is instead regenerated and the ID is fetched from the
// MAKEAuthenToken DB.
   int chip_num;

// 7_2_2022: Adding this for the PUF-Cash V3.0 AliceWithdrawal process. This is also filled in by GenLLK(). 
// THIS CAN BE DONE during device provisioning where the challenge are drawn from the ANONYMOUS DB, or by 
// doing an anonymous authentication at any time with the server.
   int anon_chip_num;

// ====================== DATABASE STUFF =========================
// For TTP.db or Customer.db
   sqlite3 *DB_Challenges;
   char *DB_name_Challenges;

   int use_database_chlngs;
   int DB_design_index;
   char *DB_ChallengeSetName;
   unsigned int DB_ChallengeGen_seed;

// MAKE protocol
   sqlite3 *DB_MAKE_PT_AT;
   char *DB_name_MAKE_PT_AT;
   int MAT_LLK_num_bytes;

// PeerTrust protocol
   int PHK_A_num_bytes;

// 11_12_2021: PUF-Cash V3.0
   sqlite3 *DB_PUFCash_V3;
   char *DB_name_PUFCash_V3;
   int eCt_num_bytes;

// For GenLLK
   int KEK_LLK_num_bytes; 

// For POP
   int POP_LLK_num_bytes; 

   unsigned char *Alice_EWA;
   unsigned char *Alice_K_AT;


   int num_PIs;
   int num_POs;

   int fix_params;

   int num_required_PNDiffs;

   int num_SF_bytes;
   int num_SF_words; 
   int iSpreadFactorScaler;
   signed char *iSpreadFactors;

   unsigned char *verifier_SHD;
   int verifier_SHD_num_bytes; 
   unsigned char *verifier_SBS;
   int verifier_SBS_num_bytes; 
   unsigned char *device_SHD;
   int device_SHD_num_bytes; 
   unsigned char *device_SBS;
   int device_SBS_num_bits; 

   unsigned char *device_n1;
   int num_device_n1_nonces;
   unsigned char *verifier_n2;
   unsigned char *XOR_nonce;

   int nonce_base_address;
   int num_required_nonce_bytes; 
   int max_generated_nonce_bytes; 

   int vec_chunk_size;
   int XMR_val;

   unsigned char AES_IV[AES_IV_NUM_BYTES];

   unsigned int SE_target_num_key_bits;
   unsigned char *SE_final_key;
   int authen_min_bitstring_size;

   unsigned int KEK_target_num_key_bits;
   unsigned char *KEK_final_enroll_key; 
   unsigned char *KEK_final_regen_key; 
   unsigned char *KEK_final_XMR_SHD; 

   unsigned char **KEK_BS_regen_arr;

   signed char *KEK_final_SpreadFactors_enroll; 

   int KEK_num_vecs;
   int KEK_num_rise_vecs;
   int KEK_has_masks;
   unsigned char **KEK_first_vecs_b; 
   unsigned char **KEK_second_vecs_b; 
   unsigned char **KEK_masks_b; 
   unsigned char *KEK_XOR_nonce;
   int num_direction_chlng_bits; 

   int KEK_num_iterations;

   unsigned char *KEK_authentication_nonce;
   int num_KEK_authen_nonce_bits; 
   int num_KEK_authen_nonce_bits_remaining; 
   unsigned char *KEK_authen_XMR_SHD_chunk; 
   unsigned char *DA_cobra_key;

   int num_vecs;
   int num_rise_vecs;
   int has_masks;
   unsigned char **first_vecs_b; 
   unsigned char **second_vecs_b; 
   unsigned char **masks_b; 

   unsigned char *PeerTrust_LLK; 

   unsigned int param_LFSR_seed_low;
   unsigned int param_LFSR_seed_high;
   unsigned int param_RangeConstant;
   unsigned short param_SpreadConstant;
   unsigned short param_Threshold;
   unsigned short param_TrimCodeConstant;
   int param_PCR_or_PBD_or_PO;

   int do_scaling;
   unsigned int MyScalingConstant;

   int load_SF; 
   int compute_PCR_PBD; 
   int modify_PO; 
   int dump_updated_SF; 

   unsigned char TRNG_LFSR_seed;

// For frequency statistics of the TRNG. Need to declare these here for the TTP -- can NOT make them static in multi-threaded apps.
   int num_ones; 
   int total_bits; 
   int iteration; 

   pthread_mutex_t *GenChallenge_mutex_ptr;

   int do_COBRA;

   int DUMP_BITSTRINGS; 
   int DEBUG_FLAG; 
   } SRFHardwareParamsStruct;

// MAX that the SRF Engine can generate before overflow (where further nonce bytes are ignored). 
#define MAX_GENERATED_NONCE_BYTES 1000

int ReceiveVectors(int str_length, int verifier_socket_desc, unsigned char ***first_vecs_b_ptr, 
   unsigned char ***second_vecs_b_ptr, int num_PIs, int *num_rise_vecs_ptr, int *has_masks_ptr, int num_POs, 
   unsigned char ***masks_b_ptr);

int ReceiveChlngsAndMasks(int max_string_len, int verifier_socket_desc, unsigned char ***challenges_b_ptr, 
   int num_chlng_bits, int *num_rise_chlngs_ptr, int *has_masks_ptr, int num_POs, unsigned char ***masks_b_ptr);

void LoadChlngAndMask(int max_string_len, volatile unsigned int *CtrlRegA, volatile unsigned int *DataRegA, int chlng_num, 
   unsigned char **challenges_b, int ctrl_mask, int num_chlng_bits, int chlng_chunk_size, int has_masks, int num_POs, 
   unsigned char **masks_b);

void SaveASCIIVectors(int max_string_len, int num_vecs, unsigned char **first_vecs_b, unsigned char **second_vecs_b, 
   int num_PIs, int has_masks, int num_POs, unsigned char **masks_b);

int GoGetVectors(int max_string_len, int num_POs, int num_PIs, int verifier_socket_desc, int *num_rise_vecs_ptr, 
   int *has_masks_ptr, unsigned char ***first_vecs_b_ptr, unsigned char ***second_vecs_b_ptr, 
   unsigned char ***masks_b_ptr, int send_GO, int use_database_chlngs, sqlite3 *DB, int DB_design_index,
   char *DB_ChallengeSetName, int gen_or_use_challenge_seed, unsigned int *DB_ChallengeGen_seed_ptr, 
   pthread_mutex_t *GenChallenge_mutex_ptr, int debug_flag);


int ReadFileHexASCIIToUnsignedChar(int max_string_len, char *file_name, unsigned char **bin_arr_ptr);

void WriteFileHexASCIIToUnsignedChar(int max_string_len, char *file_name, int num_bytes, unsigned char *bin_arr, 
   int overwrite_or_append);

int ReadFileHexASCIIToUnsignedCharSpecial(int max_string_len, char *file_name, int num_bytes, int alloc_arr, unsigned char **bin_arr_ptr,
   FILE *INFILE);

#endif
