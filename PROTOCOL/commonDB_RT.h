// ========================================================================================================
// ========================================================================================================
// ****************************************** commonDB_RT.h ***********************************************
// ========================================================================================================
// ========================================================================================================
//
//--------------------------------------------------------------------------------
// Company: IC-Safety, LLC and University of New Mexico
// Engineer: Professor Jim Plusquellic
// Exclusive License: IC-Safety, LLC
// Copyright: Univ. of New Mexico
//--------------------------------------------------------------------------------

#include "verifier_common.h"
#include "commonDB.h"

#ifndef RT_DATABASE_STRUCTS
typedef struct
   {
   char **instance_names_arr;
   char **placement_names_arr;
   int *PUFInstance_IDs_arr;
   int num_eles;
   } SQLInstancePlacementNamesStruct;

typedef struct
   {
   int PUFInstance_ID;
   char *instance_name;
   char *placement_name;
   int num_bstrings;
   char **Bitstrings;
   char **CreationDate; 
   unsigned long *ElapsedSeconds_arr;
   int *bitstring_stop_indexes_arr;
   } ChipData;
#define RT_DATABASE_STRUCTS
#endif

void GetAllUniqueChipsForDesignIndexAndFunctionName(int max_string_len, sqlite3 *db,
   SQLInstancePlacementNamesStruct *Chip_IPN_struct_ptr, int design_index, char *function_name);

int GetBitstringsCreationDateForChipNum(int max_string_len, sqlite3 *db, int design_index, char *chip_name, char *placement_name, 
   char *function_name, char ***Bitstrings_ptr, char ***CreationDate_ptr);

void SaveDBBitstringInfo(int max_string_len, SRFAlgoParamsStruct *SAP_ptr, unsigned char *XMR_SHD, 
   int num_XMR_SHD_bytes, int current_function);

