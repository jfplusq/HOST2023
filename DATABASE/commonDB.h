// ========================================================================================================
// ========================================================================================================
// ******************************************** commonDB.h ************************************************
// ========================================================================================================
// ========================================================================================================
//
//--------------------------------------------------------------------------------
// Company: IC-Safety, LLC and University of New Mexico
// Engineer: Professor Jim Plusquellic
// Exclusive License: IC-Safety, LLC
// Copyright: Univ. of New Mexico
// 
// Create Date: 8/15/2015
// Functions covered by License and Copyright: All 
//--------------------------------------------------------------------------------

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>  
#include <sys/mman.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netdb.h> 

#include <time.h>
#include <sys/time.h>

#include <math.h>

#include <pthread.h>

#include <sqlite3.h>

#include "utility.h"

// For reporting outliers in enrollment data.
#define MAX_TSIG 10.0

#define NUM_REQUIRED_PNS (2 * NUM_REQUIRED_PNDIFFS)
#define NUM_RISE_REQUIRED_PNS (NUM_REQUIRED_PNDIFFS)
#define NUM_FALL_REQUIRED_PNS (NUM_REQUIRED_PNDIFFS)

extern const char *SQL_PUFDesign_get_index_cmd;
extern const char *SQL_PUFDesign_insert_into_cmd;

extern const char *SQL_PUFInstance_get_index_cmd;
extern const char *SQL_PUFInstance_insert_into_cmd;
extern const char *SQL_PUFInstance_delete_cmd;

extern const char *SQL_Vectors_insert_into_cmd;
extern const char *SQL_Vectors_read_vector_cmd;
extern const char *SQL_Vectors_get_index_cmd;

extern const char *SQL_VecPairs_insert_into_cmd;
extern const char *SQL_VecPairs_get_index_cmd;

extern const char *SQL_TimingVals_insert_into_cmd;

extern const char *SQL_PathSelectMasks_insert_into_cmd;
extern const char *SQL_PathSelectMasks_get_index_cmd;

extern const char *SQL_Challenges_insert_into_cmd;
extern const char *SQL_Challenges_get_index_cmd;

extern const char *SQL_ChallengeVecPairs_insert_into_cmd;
extern const char *SQL_ChallengeVecPairs_get_index_cmd;

#ifndef DATABASE_STRUCTS
typedef struct
   {
   int *int_arr;
   int num_ints;
   } SQLIntStruct;

typedef struct
   {
   float *float_arr;
   int num_floats;
   } SQLFloatStruct;

typedef struct
   {
   char **ColNames;
   char **ColStringVals;
   int num_cols;
   } SQLRowStringsStruct;

typedef struct
   {
   int path_num;                  // Sequential path number
   int PO_num;                    // Primary output (PO) that is sensitized
   char path_selected;            // Flag to select this path if it has low TVN/high WID
   int vecpair_num;                // Vector that generated this PN
   char path_qualifies;           // Field used to select this path as low TVN, high WID
   char rise_or_fall;
   int random_order_number;
   } PathInfoStruct;

typedef struct
   {
   int vecpair_id;
   int PO_num;
   } VecPairPOStruct; 
#define DATABASE_STRUCTS
#endif

int LoadOrSaveDb(sqlite3 *pInMemory, const char *zFilename, int isSave);

void Get_IDs(int max_string_len, sqlite3 *db, char *table_name, SQLIntStruct *index_struct_ptr);
void Delete_ForID(int max_string_len, sqlite3 *db, char *table_name, int index);

int SQL_callback(void *UserSpecifiedData, int argc, char **argv, char **azColName);
void GetAllocateListOfInts(int max_string_len, sqlite3 *db, char *sql_command_str, SQLIntStruct *int_struct);
void GetStringsDataForRow(int max_string_len, sqlite3 *db, char *sql_command_str, 
   SQLRowStringsStruct *row_strings_struct_ptr);
void GetRowResultInt(SQLRowStringsStruct *row_strings_struct_ptr, char *calling_routine_str, int num_cols, 
   int col_index, char *field_name_str, int *field_val_int_ptr);
void GetRowResultString(SQLRowStringsStruct *row_strings_struct_ptr, char *calling_routine_str, int num_cols, 
   int col_index, char *field_name_str, int required_string_len, char *field_val_str);
void FreeStringsDataForRow(SQLRowStringsStruct *row_strings_struct_ptr);

void UpdateVecPairsNumPNsField(int max_string_len, sqlite3 *db, int num_vals_per_vecpair, int vecpair_index);
int GetVecPairsRiseFallStrField(int max_string_len, sqlite3 *db, int vecpair_index);
int GetVecPairsNumPNsField(int max_string_len, sqlite3 *db, int vecpair_index);

void UpdateChallengesNumPNFields(int max_string_len, sqlite3 *db, int challenge_index, int tot_PNs, int tot_rise_PNs);
void UpdateChallengesNumVecFields(int max_string_len, sqlite3 *db, int challenge_index, int tot_vecs, int tot_rise_vecs);
void GetChallengeNumVecsNumPNs(int max_string_len, sqlite3 *db, int *num_vecpairs_ptr, int *num_rising_vecpairs_ptr,
   int *num_PNs_ptr, int *num_rising_PNs_ptr, int challenge_index);
void GetChallengeParams(int max_string_len, sqlite3 *db, char *ChallengeSetName, int *challenge_index_ptr, 
   int *num_vecpairs_ptr, int *num_rising_vecpairs_ptr, int *num_qualified_PNs_ptr, int *num_rise_qualified_PNs_ptr);

void GetPUFInstanceIDsForInstanceName(int max_string_len, sqlite3 *db, SQLIntStruct *PUF_instance_index_struct_ptr, 
   char *PUF_instance_name);
void GetPUFInstanceInfoForID(int max_string_len, sqlite3 *db, int PUFInstance_id, char *Instance_name, char *Dev, 
   char *Placement);

void GetPUFDesignNumPIPOFields(int max_string_len, sqlite3 *db, int *num_PIs_ptr, int *num_POs_ptr, int design_index);
int GetPUFDesignParams(int max_string_len, sqlite3 *db, char *Netlist_name, char *Synthesis_name, int *design_index_ptr, 
   int *num_PIs_ptr, int *num_POs_ptr);

int ReadBinaryBlob(sqlite3 *db, const char *sql_command_str, int id, unsigned char *binary_vec, int expected_size, 
   int allocate_storage, unsigned char **binary_vec_ptr);

int GetIndexFromTable(int max_string_len, sqlite3 *db, char *Table, const char *SQL_cmd, unsigned char *blob, 
   int blob_size_bytes, char *text1, char *text2, char *text3, char *text4, int int1, int int2, int int3);

int InsertIntoTable(int max_string_len, sqlite3 *db, char *Table, const char *SQL_cmd, unsigned char *vector,
   int vector_size_bytes, char *text1, char *text2, char *text3, char *text4, char *text5, int int1, int int2, 
   int int3, int int4, int int5, float float1, float float2);

void DetermineNumSelectedPOsInMask(char *mask, int num_POs, int *num_no_trans_ptr, int *num_hard_selected_ptr,
   int *num_unqual_selected_ptr, int *num_qual_selected_ptr);

int DeletePUFInstance(int max_string_len, sqlite3 *db, const char *SQL_cmd, int instance_index);

int ReadChipEnrollPNs(int max_string_len, char *ChipEnrollDatafile, float **PNX_ptr, 
   float **PNX_Tsig_ptr, int **rise_fall_ptr, int **vec_pairs_ptr, int **POs_ptr, int *num_PNX_ptr, 
   int num_POs, int has_masks, int num_vec_pairs, char **master_masks, int debug_flag);

void GetCreatePUFDesignAndInstance(int max_string_len, sqlite3 *db, char *Netlist_name, char *Synthesis_name, 
   char *Chip_name, char *Device_name, char *Placement_name, int *design_index_ptr, int *instance_index_ptr, 
   int num_PIs, int num_POs);

void ProcessMasterVecsAndTimingData(int max_string_len, sqlite3 *db, int design_index, int instance_index,
   int master_num_vec_pairs, int master_num_rise_vec_pairs, unsigned char **master_first_vecs_b, 
   unsigned char **master_second_vecs_b, float *PNX, float *PNX_Tsig, int *rise_fall, int *vec_pairs, 
   int *POs, int num_PNR, int num_PNX, int num_PIs, int num_POs);

void CheckMaskIsConsistentWithVecPairTimingVals(int max_string_len, sqlite3 *db, int vecpair_index, 
   char *challenge_mask, int num_POs);

int ReadVectorAndASCIIMaskFiles(int str_length, char *vec_pair_path, int *num_rise_vecs_ptr, unsigned char ***first_vecs_b_ptr, 
   unsigned char ***second_vecs_b_ptr, int has_masks, char *mask_file_path, char ***masks_ptr, int num_PIs, int num_POs,
   int rise_fall_bit_pos);

void FindQualifyingPaths(int max_string_len, sqlite3 *db, PathInfoStruct **tested_path_info_ptr, int num_rising_vectors, 
   int num_falling_vectors, int *num_rise_tested_PNs_ptr, int *num_fall_tested_PNs_ptr, int num_POs, 
   int num_rise_required_PNs, int num_fall_required_PNs, int num_rise_qualified_PNs_expected, 
   int num_fall_qualified_PNs_expected, PathInfoStruct **qualified_path_info_ptr, int challenge_index, 
   int **vecpair_ids_ptr);

void CreateVecsMasks(int max_string_len, sqlite3 *db, char *outfile_vecs, char *outfile_masks, int num_PNs_tested, 
   int num_POs, PathInfoStruct *tested_path_info, int num_rising_vectors, int num_falling_vectors, int num_required_PNs, 
   int *vecpair_ids, char ***masks_ptr);

void GetPUFInstanceTimingInfoUsingVecPairPOStruct(int max_string_len, sqlite3 *db, int PUF_instance_index, int timing_or_tsig,
   VecPairPOStruct *vecpair_id_PO, int num_VPPO_eles, int allocate_float_arrs, float **PNR_TSig_ptr, float **PNF_TSig_ptr,
   TimingValCacheStruct *TVC_arr, int num_TVC_arr, int use_TVC_cache, int TVC_chip_num);

void GetAllPUFInstanceTimingValsForChallenge(int max_string_len, sqlite3 *db, VecPairPOStruct *challenge_vecpair_id_PO_arr, 
   int num_challenge_vecpair_id_PO, char *PUF_instance_name_to_match, float ***PNR_ptr, float ***PNF_ptr, int *num_chips_ptr,
   TimingValCacheStruct *TVC_arr, int num_TVC_arr, int use_TVC_cache);

int GenChallengeDB(int max_string_len, sqlite3 *db, int design_index, char *ChallengeSetName, unsigned int Seed, int save_vecs_masks, 
   char *outfile_vecs, char *outfile_masks, unsigned char ***vecs1_bin_ptr, unsigned char ***vecs2_bin_ptr, 
   unsigned char ***masks_bin_ptr, int *num_vecs_masks_ptr, int *num_rise_vecs_masks_ptr, pthread_mutex_t *GenChallenge_mutex_ptr,
   int *num_challenge_vecpair_id_PO_ptr, VecPairPOStruct **challenge_vecpair_id_PO_ptr);

void GetVectorAndVecPairIndexesForBinaryVectors(int max_string_len, sqlite3 *db, int design_index, int vec_len_bytes, 
   unsigned char *first_vecs_b, unsigned char *second_vecs_b, int *first_vec_index_ptr, int *second_vec_index_ptr, 
   int *vecpair_index_ptr, int vecpair_num);

void GetVecPairPOStructForBinaryVecsMasks(int max_string_len, sqlite3 *db, int num_PIs, int num_POs, int design_index, 
   unsigned char **vecs1_bin, unsigned char **vecs2_bin, unsigned char **masks_bin, int num_vecs_masks, 
   int num_rise_vecs_masks, int *num_challenge_vecpair_id_PO_ptr, VecPairPOStruct **challenge_vecpair_id_PO_ptr);

int CreateTimingValsCacheFromChallengeSet(int max_string_len, sqlite3 *db, int design_index, char *ChallengeSetName, 
   char *PUF_instance_name_to_match, TimingValCacheStruct **TVC_ptr, int *num_TVC_ptr);
