// ========================================================================================================
// ========================================================================================================
// ****************************************** commonDB_RT.c ***********************************************
// ========================================================================================================
// ========================================================================================================
//
//--------------------------------------------------------------------------------
// Company: IC-Safety, LLC and University of New Mexico
// Engineer: Professor Jim Plusquellic
// Exclusive License: IC-Safety, LLC
// Copyright: Univ. of New Mexico
//--------------------------------------------------------------------------------

#include "common.h"
#include "commonDB_RT.h"


// ========================================================================================================
// ========================================================================================================
// This callback is called once for each matching row in a table referred to by the SQL query. It adds 1 
// element to the array in UserSpecifiedData on each call dynamically. 

int SQL_GetAllocateListOfInstancesPlacements_callback(void *UserSpecifiedData, int argc, char **argv, char **azColName)
   {
   SQLInstancePlacementNamesStruct *SQL_IPN_struct_ptr = (SQLInstancePlacementNamesStruct *)UserSpecifiedData;

// Sanity check. 
   if ( argc != 3 )
      { printf("ERROR: SQL_GetAllocateListOfInstancesPlacements_callback(): Expected argc to be 3!\n"); exit(EXIT_FAILURE); }

// Sanity check. Check to make sure the SQLIntStruct is NOT NULL;
   if ( SQL_IPN_struct_ptr == NULL )
      { printf("ERROR: SQL_GetAllocateListOfInstancesPlacements_callback(): UserSpecified data MUST NOT BE NULL!\n"); exit(EXIT_FAILURE); }

// Allocate or re-allocate another element to the array. THIS ROUTINE GETS CALLED MULTIPLE TIMES.
   if ( (SQL_IPN_struct_ptr->instance_names_arr = (char **)realloc(SQL_IPN_struct_ptr->instance_names_arr, sizeof(char *) * (SQL_IPN_struct_ptr->num_eles + 1))) == NULL )
      { printf("ERROR: SQL_GetAllocateListOfInstancesPlacements_callback(): Failed to reallocate storage for SQL_IPN_struct_ptr->instance_names_arr!\n"); exit(EXIT_FAILURE); }
   if ( (SQL_IPN_struct_ptr->placement_names_arr = (char **)realloc(SQL_IPN_struct_ptr->placement_names_arr, sizeof(char *) * (SQL_IPN_struct_ptr->num_eles + 1))) == NULL )
      { printf("ERROR: SQL_GetAllocateListOfInstancesPlacements_callback(): Failed to reallocate storage for SQL_IPN_struct_ptr->placement_names_arr!\n"); exit(EXIT_FAILURE); }
   if ( (SQL_IPN_struct_ptr->PUFInstance_IDs_arr = (int *)realloc(SQL_IPN_struct_ptr->PUFInstance_IDs_arr, sizeof(int) * (SQL_IPN_struct_ptr->num_eles + 1))) == NULL )
      { printf("ERROR: SQL_GetAllocateListOfInstancesPlacements_callback(): Failed to allocate storage for SQL_IPN_struct_ptr->PUFInstance_IDs_arr!\n"); exit(EXIT_FAILURE); }

   int name_len;
   name_len = strlen(argv[0]);
   if ( name_len == 0 )
      { printf("ERROR: SQL_GetAllocateListOfInstancesPlacements_callback(): Returned string length is 0!\n"); exit(EXIT_FAILURE); }
   if ( (SQL_IPN_struct_ptr->instance_names_arr[SQL_IPN_struct_ptr->num_eles] = (char *)malloc(sizeof(char) * name_len)) == NULL )
         { printf("ERROR: SQL_GetAllocateListOfInstancesPlacements_callback(): Failed to allocate storage for SQL_IPN_struct_ptr->instance_names_arr ELEMENT!\n"); exit(EXIT_FAILURE); }
   strcpy(SQL_IPN_struct_ptr->instance_names_arr[SQL_IPN_struct_ptr->num_eles], argv[0]);

   name_len = strlen(argv[1]);
   if ( name_len == 0 )
      { printf("ERROR: SQL_GetAllocateListOfInstancesPlacements_callback(): Returned string length is 0!\n"); exit(EXIT_FAILURE); }
   if ( (SQL_IPN_struct_ptr->placement_names_arr[SQL_IPN_struct_ptr->num_eles] = (char *)malloc(sizeof(char) * name_len)) == NULL )
         { printf("ERROR: SQL_GetAllocateListOfInstancesPlacements_callback(): Failed to allocate storage for SQL_IPN_struct_ptr->placement_names_arr ELEMENT!\n"); exit(EXIT_FAILURE); }
   strcpy(SQL_IPN_struct_ptr->placement_names_arr[SQL_IPN_struct_ptr->num_eles], argv[1]);

   sscanf(argv[2], "%d", &(SQL_IPN_struct_ptr->PUFInstance_IDs_arr[SQL_IPN_struct_ptr->num_eles]));

   (SQL_IPN_struct_ptr->num_eles)++;

#ifdef DEBUG
printf("SQL_GetAllocateListOfInstancesPlacements_callback(): New number of elements in integer array %d\n", SQL_IPN_struct_ptr->num_eles); fflush(stdout);
#endif

#ifdef DEBUG
int i;
for( i = 0; i < argc; i++ )
   { printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL"); }
printf("\n");
#endif

   return 0;
   }


// ========================================================================================================
// ========================================================================================================
// Get all unique chip names (instance_name and placement) associated with a design_index and function from 
// the Bitstrings table in the RunTime DB. 

void GetAllUniqueChipsForDesignIndexAndFunctionName(int max_string_len, sqlite3 *db,
   SQLInstancePlacementNamesStruct *Chip_IPN_struct_ptr, int design_index, char *function_name)
   {
   char sql_command_str[max_string_len];
   char *zErrMsg = 0;
   int fc;

#ifdef DEBUG
printf("GetAllUniqueChipsForDesignIndexAndFunctionName(): Called!\n"); fflush(stdout);
#endif

   sprintf(sql_command_str, "SELECT DISTINCT InstanceName, Placement, PUFInstanceID FROM Bitstrings WHERE DesignIndex = %d AND SecurityFunction = '%s';", 
      design_index, function_name);

// Initialize the fields before the callback is called.
   Chip_IPN_struct_ptr->instance_names_arr = NULL;
   Chip_IPN_struct_ptr->placement_names_arr = NULL;
   Chip_IPN_struct_ptr->PUFInstance_IDs_arr = NULL;
   Chip_IPN_struct_ptr->num_eles = 0;

// The callback will be called multiple times, once for each matching database element to sql_command_str. 
   fc = sqlite3_exec(db, sql_command_str, SQL_GetAllocateListOfInstancesPlacements_callback, Chip_IPN_struct_ptr, &zErrMsg);

   if ( fc != SQLITE_OK )
      { printf("SQL ERROR: %s\n", zErrMsg); sqlite3_free(zErrMsg); exit(EXIT_FAILURE); }


   return;
   }


// ========================================================================================================
// ========================================================================================================
// Given a design_index, instance_name ("C1"), placement_name ("V1") and function ("SE"), retrieve all the
// matching table ids (if any).

void GetBitstringsIDsForDesignInstancePlacementFunctionName(int max_string_len, sqlite3 *db, 
   SQLIntStruct *Bitstrings_ID_index_struct_ptr, int design_index, char *instance_name, char *placement_name, 
   char *function_name)
   {
   char sql_command_str[max_string_len];

   sprintf(sql_command_str, "SELECT ID FROM Bitstrings WHERE DesignIndex = %d AND InstanceName = '%s' AND Placement = '%s' AND SecurityFunction = '%s' ORDER BY CreationDate ASC;", 
      design_index, instance_name, placement_name, function_name);
   GetAllocateListOfInts(max_string_len, db, sql_command_str, Bitstrings_ID_index_struct_ptr);

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Get all the bitstrings associated with Bitstrings table in the RunTime DB records that match the design_index, 
// chip name fields and function, e.g., DA.

#define MAX_BITSTRING_SIZE 2048

int GetBitstringsCreationDateForChipNum(int max_string_len, sqlite3 *db, int design_index, char *chip_name, char *placement_name, 
   char *function_name, char ***Bitstrings_ptr, char ***CreationDate_ptr)
   {
   char bitstring_creation_date[MAX_BITSTRING_SIZE];
   SQLIntStruct Bitstrings_ID_index_struct;
   SQLRowStringsStruct row_strings_struct;
   char sql_command_str[max_string_len];
   char *col1_name = "Bitstring";
   char *col2_name = "CreationDate";
   int bs_num;

// Get all Bitstring table IDs for matches to the parameters. This routine returns an array of IDs. Note that this search can also be done
// using PUFInstance_ID, which is also unique and one-to-one with "chip_name" and "placement_name".
   GetBitstringsIDsForDesignInstancePlacementFunctionName(max_string_len, db, &Bitstrings_ID_index_struct, design_index, 
      chip_name, placement_name, function_name);

// Sanity check.
   if ( Bitstrings_ID_index_struct.num_ints == 0 )
      { 
      printf("ERROR: GetBitstringsCreationDateForChipNum(): No Bitstrings found that match search criteria (DI %d Chip %s Placement %s Function %s)!\n", 
         design_index, chip_name, placement_name, function_name); 
      exit(EXIT_FAILURE); 
      }

printf("Number of Bitstrings found for search criteria (DesignIndex %d Chip %s Placement %s Function %s) is %d!\n", design_index, 
   chip_name, placement_name, function_name, Bitstrings_ID_index_struct.num_ints); fflush(stdout);
#ifdef DEBUG
#endif

   if ( (*Bitstrings_ptr = (char **)malloc(sizeof(char *) * Bitstrings_ID_index_struct.num_ints)) == NULL )
      { printf("ERROR: GetBitstringsCreationDateForChipNum(): Failed to allocated storage for Bitstrings array!\n"); exit(EXIT_FAILURE); }
   if ( (*CreationDate_ptr = (char **)malloc(sizeof(char *) * Bitstrings_ID_index_struct.num_ints)) == NULL )
      { printf("ERROR: GetBitstringsCreationDateForChipNum(): Failed to allocated storage for CreationDate array!\n"); exit(EXIT_FAILURE); }

// Using the Bitstrings IDs returned above, go back to these records and get the bitstrings
   for ( bs_num = 0; bs_num < Bitstrings_ID_index_struct.num_ints; bs_num++ )
      {

// Sanity check. GetBitstringsIDsForDesignInstancePlacementFunctionName() orders IDs by CreationDate. The ID should be assending in the 
// order of creation date (the order in which they are added to the database). Warning if not??? No harm if this occurs but why would
// it happen?
      if ( bs_num > 0 && Bitstrings_ID_index_struct.int_arr[bs_num-1] >= Bitstrings_ID_index_struct.int_arr[bs_num] )
         printf("WARNING: GetBitstringsKeyFields(): Bitstring IDs NOT in CreationDate order!\n"); 

      sprintf(sql_command_str, "SELECT %s FROM Bitstrings WHERE id = %d;", col1_name, Bitstrings_ID_index_struct.int_arr[bs_num]);
      GetStringsDataForRow(max_string_len, db, sql_command_str, &row_strings_struct);

// NOTE: We ASSUME the bitstrings stored in the RunTime DB are no larger than MAX_BITSTRING_SIZE, which is currently 2048 as defined
// above. Otherwise this crashes (-1 disables the size test since bitstrings for DA are variable in length).
      GetRowResultString(&row_strings_struct, "GetPUFInstanceInfoForID", 1, 0, col1_name, -1, bitstring_creation_date);
      FreeStringsDataForRow(&row_strings_struct);

// Allocate and copy the bitstring (exact size) into the allocated storage.
      if ( ((*Bitstrings_ptr)[bs_num] = (char *)malloc(sizeof(char) * (strlen(bitstring_creation_date) + 1))) == NULL )
         { printf("ERROR: GetBitstringsCreationDateForChipNum(): Failed to allocated storage for Bitstrings string!\n"); exit(EXIT_FAILURE); }
      strcpy((*Bitstrings_ptr)[bs_num], bitstring_creation_date);


// Do the same for the creation date.
      sprintf(sql_command_str, "SELECT %s FROM Bitstrings WHERE id = %d;", col2_name, Bitstrings_ID_index_struct.int_arr[bs_num]);
      GetStringsDataForRow(max_string_len, db, sql_command_str, &row_strings_struct);

// NOTE: We ASSUME the creation dates stored in the RunTime DB are no larger than MAX_BITSTRING_SIZE, which is currently 2048 as defined
// above. Otherwise this crashes. 
      GetRowResultString(&row_strings_struct, "GetPUFInstanceInfoForID", 1, 0, col2_name, -1, bitstring_creation_date);
      FreeStringsDataForRow(&row_strings_struct);

// Allocate and copy the creation date (exact size) into the allocated storage.
      if ( ((*CreationDate_ptr)[bs_num] = (char *)malloc(sizeof(char) * (strlen(bitstring_creation_date) + 1))) == NULL )
         { printf("ERROR: GetBitstringsCreationDateForChipNum(): Failed to allocated storage for CreationDate string!\n"); exit(EXIT_FAILURE); }
      strcpy((*CreationDate_ptr)[bs_num], bitstring_creation_date);


#ifdef DEBUG
printf("CreationData %s for Bitstring %d is %s\n", (*CreationDate_ptr)[bs_num], bs_num, (*Bitstrings_ptr)[bs_num]); fflush(stdout);
#endif
      }

   free(Bitstrings_ID_index_struct.int_arr);

   return bs_num;
   }


// ========================================================================================================
// ========================================================================================================
// Save the bitstrings from device/verifier authentication and session encryption to a database as they are
// generated so we can later compute stats.

#define MAX_BSTRING_CMD_LEN 30000
char SQL_BStrings_insert_into_cmd[MAX_BSTRING_CMD_LEN];

void SaveDBBitstringInfo(int max_string_len, SRFAlgoParamsStruct *SAP_ptr, unsigned char *XMR_SHD, 
   int num_XMR_SHD_bytes, int current_function)
   {
   SQLIntStruct PUF_instance_index_struct;
   char InstanceName[500];
   char Dev[500];
   char Placement[500];

   char date_str[500];
   struct tm *tmp;
   time_t t;

   char cur_func_str[20];
   char fix_params_str[20];

   char *BitstringASCII;

   char *zErrMsg = 0;
   int fc;

#ifdef DEBUG
printf("SaveDBBitstringInfo(): CALLED!\n"); fflush(stdout); 
#endif

// Sanity check. Negative is usually -1, and indicates DA or VA failure.
   if ( SAP_ptr->chip_num < 0 )
      { 
      printf("WARNING: SaveDBBitstringInfo(): chip_num stored in SAP_ptr is negative %d!\n", SAP_ptr->chip_num); fflush(stdout); 
      return;
      }

// Get the current date/time. Only 500 characters allocated for this string above.
   t = time(NULL);
   tmp = localtime(&t);
   if (tmp == NULL) 
      { printf("localtime FAILED!\n"); exit(EXIT_FAILURE); }
   if ( strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M", tmp) == 0 ) 
      { printf("strftime FAILED!\n"); exit(EXIT_FAILURE); }

// Get the PUFInstance name using the chip_num stored in the SAP_ptr (which is the chip_num associated with the bitstring). First get 
// a list of all PUFInstance ids. Use '%' for * and '_' for ?
   GetPUFInstanceIDsForInstanceName(max_string_len, SAP_ptr->database_NAT, &PUF_instance_index_struct, "%");

// Sanity check
   if ( PUF_instance_index_struct.num_ints == 0 )
      { printf("ERROR: SaveDBBitstringInfo(): No PUFInstances found!\n"); exit(EXIT_FAILURE); }

// The chip name is stored in the PUFInstance database under the following id. Get the string information from the PUFInstance table.
// ONLY 500 characters allocated for these strings above.
   GetPUFInstanceInfoForID(max_string_len, SAP_ptr->database_NAT, PUF_instance_index_struct.int_arr[SAP_ptr->chip_num], InstanceName, 
      Dev, Placement);

// Create string from current function. 
   if ( current_function == FUNC_RB )
      strcpy(cur_func_str, "RB");
   else if ( current_function == FUNC_DA )
      strcpy(cur_func_str, "DA");
   else if ( current_function == FUNC_VA )
      strcpy(cur_func_str, "VA");
   else if ( current_function == FUNC_SE )
      strcpy(cur_func_str, "SE");
   else if ( current_function == FUNC_LL_ENROLL )
      strcpy(cur_func_str, "LLE");

// This never occurs because regeneration is done only on the device, in stand-alone mode
   else if ( current_function == FUNC_LL_REGEN )
      strcpy(cur_func_str, "LLR");
   else
      { printf("ERROR: SaveDBBitstringInfo(): Unknown 'current function' %d!\n", current_function); exit(EXIT_FAILURE); }

// Only 2 characters allocated for this string above.
   if ( SAP_ptr->fix_params == 0 )
      strcpy(fix_params_str, "N");
   else
      strcpy(fix_params_str, "Y");

// Size of the ASCII bitstring for Raw is 2048 ASCII characters currently.
   BitstringASCII = NULL;
   if ( current_function == FUNC_RB )
      {
      if ( (BitstringASCII = (char *)malloc(SAP_ptr->num_required_PNDiffs * sizeof(char))) == NULL )
         { printf("ERROR: SaveDBBitstringInfo(): Failed to allocated storage for BitstringASCII!\n"); exit(EXIT_FAILURE); }

// Convert the unsigned char array into an ASCII string of 0's and 1's.
      ConvertBinVecMaskToASCII(SAP_ptr->DHD_SBS_num_bits, SAP_ptr->verifier_DHD_SBS, BitstringASCII);
      }

// Size of the ASCII bitstring for device and verifier authentication is num_XMR_SHD_bytes*8 ASCII characters 
   else if ( current_function == FUNC_DA || current_function  == FUNC_VA )   
      {
      if ( (BitstringASCII = (char *)malloc(num_XMR_SHD_bytes * 8 * sizeof(char))) == NULL )
         { printf("ERROR: SaveDBBitstringInfo(): Failed to allocated storage for BitstringASCII!\n"); exit(EXIT_FAILURE); }
      ConvertBinVecMaskToASCII(num_XMR_SHD_bytes * 8, XMR_SHD, BitstringASCII);

// Sanity check
      if ( strlen(BitstringASCII) != num_XMR_SHD_bytes * 8 )
         { 
         printf("ERROR: SaveDBBitstringInfo(): String len of VA or TA BitstringASCII %d NOT equal to requirement %d!\n", 
            (int)strlen(BitstringASCII), num_XMR_SHD_bytes * 8); exit(EXIT_FAILURE); 
         }
      }

// Session key generation in this PUF-Cash V3.0 version uses FSB mode of KEK so we really don't need 'FUNC_LL_ENROLL' below.
   else if ( current_function == FUNC_SE )
      {
      if ( (BitstringASCII = (char *)malloc(SAP_ptr->SE_target_num_key_bits * sizeof(char))) == NULL )
         { printf("ERROR: SaveDBBitstringInfo(): Failed to allocated storage for BitstringASCII!\n"); exit(EXIT_FAILURE); }
      ConvertBinVecMaskToASCII(SAP_ptr->SE_target_num_key_bits, SAP_ptr->SE_final_key, BitstringASCII);
      }

// We do NOT have the KEK key on the verifier, ONLY ON THE DEVICE. So I'll need to transmit it over from the device in order to save it here.
// Also, regeneration regenerates the SAME key over and over again, so unless we do enrollment over and over again, no sense saving it here.
   else if ( current_function == FUNC_LL_ENROLL )
      {
      if ( (BitstringASCII = (char *)malloc(SAP_ptr->KEK_target_num_key_bits * sizeof(char))) == NULL )
         { printf("ERROR: SaveDBBitstringInfo(): Failed to allocated storage for BitstringASCII!\n"); exit(EXIT_FAILURE); }
      ConvertBinVecMaskToASCII(SAP_ptr->KEK_target_num_key_bits, SAP_ptr->KEK_final_enroll_key, BitstringASCII);
      }
   else
      { printf("ERROR: SaveDBBitstringInfo(): Unknown 'current_function' %d\n", current_function); exit(EXIT_FAILURE); }

// Sanity check. We need 1000 additional bytes for the rest of the SQL_BStrings_insert_into_cnd below.
   if ( strlen(BitstringASCII) > MAX_BSTRING_CMD_LEN - 1000 )
      { printf("ERROR: SaveDBBitstringInfo(): SQL insert command %d is LARGER than MAX %d!\n", (int)strlen(SQL_BStrings_insert_into_cmd), MAX_BSTRING_CMD_LEN); exit(EXIT_FAILURE); }

   sprintf(SQL_BStrings_insert_into_cmd, "INSERT INTO Bitstrings (DesignIndex, NetlistName, SynthesisName, InstanceName, Dev, Placement, PUFInstanceID, ChallengeSetName, \
      CreationDate, SecurityFunction, FixParams, LFSRSeedLow, LFSRSeedHigh, RangeConstant, SpreadConstant, Threshold, Bitstring) VALUES \
      (%d, '%s', '%s', '%s', '%s', '%s', %d, '%s', '%s', '%s', '%s', %d, %d, %d, %d, %d, '%s')", 
      SAP_ptr->design_index, SAP_ptr->Netlist_name, SAP_ptr->Synthesis_name, InstanceName, Dev, Placement, PUF_instance_index_struct.int_arr[SAP_ptr->chip_num], 
      SAP_ptr->ChallengeSetName_NAT, date_str, cur_func_str, fix_params_str, SAP_ptr->param_LFSR_seed_low, SAP_ptr->param_LFSR_seed_high, 
      SAP_ptr->param_RangeConstant, SAP_ptr->param_SpreadConstant, SAP_ptr->param_Threshold, BitstringASCII);

#ifdef DEBUG
printf("SQL cmd: '%s'\n", SQL_BStrings_insert_into_cmd); fflush(stdout);
#endif

// No callback for insert operation.
   fc = sqlite3_exec(SAP_ptr->database_RT, SQL_BStrings_insert_into_cmd, NULL, 0, &zErrMsg);
   if ( fc != SQLITE_OK )
      { printf("ERROR: SaveDBBitstringInfo(): Function %s\tSQL ERROR: %s\n", cur_func_str, zErrMsg); sqlite3_free(zErrMsg); exit(EXIT_FAILURE); }

#ifdef DEBUG
printf("SaveDBBitstringInfo(): DONE!\n"); fflush(stdout); 
#endif

   if ( BitstringASCII != NULL )
      free(BitstringASCII); 
   if ( PUF_instance_index_struct.int_arr != NULL )
      free(PUF_instance_index_struct.int_arr); 

   return;
   }


