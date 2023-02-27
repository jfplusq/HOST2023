// ========================================================================================================
// ========================================================================================================
// ******************************************** commonDB.c ************************************************
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

#include "commonDB.h"

// SQL commands depend on the structure of the tables in the database. Keeping these all in one place where possible.
const char *SQL_PUFDesign_get_index_cmd = "SELECT id FROM PUFDesign WHERE netlist_name = ? AND synthesis_name = ?;";
const char *SQL_PUFDesign_insert_into_cmd = "INSERT INTO PUFDesign (netlist_name, synthesis_name, num_PIs, num_POs) VALUES (?, ?, ?, ?);";

const char *SQL_PUFInstance_get_index_cmd = "SELECT id FROM PUFInstance WHERE Instance_name = ? AND Dev = ? AND Placement = ?;";
const char *SQL_PUFInstance_insert_into_cmd = "INSERT INTO PUFInstance (Instance_name, Dev, Placement, EnrollDate, PUFDesign_id) VALUES (?, ?, ?, ?, ?);";
const char *SQL_PUFInstance_delete_cmd = "DELETE FROM PUFInstance WHERE id = ?;";

const char *SQL_Vectors_insert_into_cmd = "INSERT INTO Vectors (vector) VALUES (?);";
const char *SQL_Vectors_read_vector_cmd = "SELECT vector FROM Vectors WHERE id = ?;";
const char *SQL_Vectors_get_index_cmd = "SELECT id FROM Vectors WHERE vector = ?;";

const char *SQL_VecPairs_insert_into_cmd = "INSERT INTO VecPairs (R_F_str, VA, VB, NumPNs, PUFDesign_id) VALUES (?, ?, ?, ?, ?);";
const char *SQL_VecPairs_get_index_cmd = "SELECT id FROM VecPairs WHERE VA = ? AND VB = ? AND PUFDesign_id = ?;";

const char *SQL_TimingVals_insert_into_cmd = "INSERT INTO TimingVals (VecPair, PO, Ave, TSig, PUFInstance) VALUES (?, ?, ?, ?, ?);";

const char *SQL_PathSelectMasks_insert_into_cmd = "INSERT INTO PathSelectMasks (vector_str) VALUES (?);";
const char *SQL_PathSelectMasks_get_index_cmd = "SELECT id FROM PathSelectMasks WHERE vector_str = ?;";

const char *SQL_Challenges_insert_into_cmd = "INSERT INTO Challenges (Name, NumVecs, NumRiseVecs, NumPNs, NumRisePNs, PUFDesign_id) VALUES (?, ?, ?, ?, ?, ?);";
const char *SQL_Challenges_get_index_cmd = "SELECT id FROM Challenges WHERE Name = ?;";

const char *SQL_ChallengeVecPairs_insert_into_cmd = "INSERT INTO ChallengeVecPairs (Chlng, VecPair, PSM) VALUES (?, ?, ?);";
const char *SQL_ChallengeVecPairs_get_index_cmd = "SELECT id FROM ChallengeVecPairs WHERE Chlng = ? AND VecPair = ? AND PSM = ?;";


// ===========================================================================================================
// ===========================================================================================================
// Got this from https://www.sqlite.org/backup.html. This function is used to load the contents of a database 
// file on disk into the "main" database of open database connection pInMemory, or to save the current contents 
// of the database opened by pInMemory into database file on disk. pInMemory is probably an in-memory database, 
// but this function will also work fine if it is not. Parameter zFilename points to a null-terminated string 
// containing the name of the database file on disk to load from or save to. If parameter isSave is non-zero, 
// then the contents of the file zFilename are overwritten with the contents of the database opened by pInMemory. 
// If parameter isSave is zero, then the contents of the database opened by pInMemory are replaced by data loaded 
// from the file zFilename. If the operation is successful, SQLITE_OK is returned. Otherwise, if an error occurs, 
// an SQLite error code is returned. 

int LoadOrSaveDb(sqlite3 *pInMemory, const char *zFilename, int isSave)
   {
   int rc;                   // Function return code 
   sqlite3 *pFile;           // Database connection opened on zFilename
   sqlite3_backup *pBackup;  // Backup object used to copy data
   sqlite3 *pTo;             // Database to copy to (pFile or pInMemory)
   sqlite3 *pFrom;           // Database to copy from (pFile or pInMemory)

// Open the database file identified by zFilename. Exit early if this fails for any reason.
   rc = sqlite3_open(zFilename, &pFile);
   if ( rc == SQLITE_OK )
      {

// If this is a 'load' operation (isSave==0), then data is copied from the database file just opened to database pInMemory. 
// Otherwise, if this is a 'save' operation (isSave==1), then data is copied from pInMemory to pFile. Set the variables pFrom and
// pTo accordingly.
      pFrom = (isSave ? pInMemory : pFile);
      pTo   = (isSave ? pFile     : pInMemory);

// Set up the backup procedure to copy from the "main" database of connection pFile to the main database of connection pInMemory.
// If something goes wrong, pBackup will be set to NULL and an error code and message left in connection pTo. If the backup object 
// is successfully created, call backup_step() to copy data from pFile to pInMemory. Then call backup_finish() to release resources 
// associated with the pBackup object.  If an error occurred, then an error code and message will be left in connection pTo. If no 
// error occurred, then the error code belonging to pTo is set to SQLITE_OK.
      pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
      if ( pBackup )
         {
         (void)sqlite3_backup_step(pBackup, -1);
         (void)sqlite3_backup_finish(pBackup);
         }
      rc = sqlite3_errcode(pTo);
      }

// Close the database connection opened on database file zFilename and return the result of this function.
   (void)sqlite3_close(pFile);

   return rc;
   }


// ========================================================================================================
// ========================================================================================================
// Get all IDs from the table. 

void Get_IDs(int max_string_len, sqlite3 *db, char *table_name, SQLIntStruct *index_struct_ptr)
   {
   char sql_command_str[max_string_len];

   sprintf(sql_command_str, "SELECT ID FROM %s;", table_name);
   GetAllocateListOfInts(max_string_len, db, sql_command_str, index_struct_ptr);

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Delete from 'table_name' the ID given as parameter.

void Delete_ForID(int max_string_len, sqlite3 *db, char *table_name, int index)
   {
   char sql_command_str[max_string_len];
   char *zErrMsg = 0;
   int fc;

   sprintf(sql_command_str, "DELETE FROM %s WHERE ID = %d;", table_name, index);

// No callback for delete operation.
   fc = sqlite3_exec(db, sql_command_str, NULL, 0, &zErrMsg);
   if ( fc != SQLITE_OK )
      { printf("Delete_ForID(): SQL ERROR: %s\n", zErrMsg); sqlite3_free(zErrMsg); exit(EXIT_FAILURE); }

   return;
   }


// ========================================================================================================
// ========================================================================================================
// This callback invoked for each result row coming out of the evaluated SQL statements. First arg is
// user-specified arg, 2nd arg is number of columns in the result, 3rd arg is an array of pointers to
// strings RESULTS obtained from something like sqlite3_column_text() and 4th arg are pointers to strings
// of column names.

int SQL_callback(void *UserSpecifiedData, int argc, char **argv, char **azColName)
   {
   int i;

printf("SQL_callback() called!\n"); fflush(stdout);

for( i = 0; i < argc; i++ )
   { printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL"); }
printf("\n");

   return 0;
   }


// ========================================================================================================
// ========================================================================================================
// A utility routine that does sanity checks and gets the integer row result.

void GetRowResultFloat(SQLRowStringsStruct *row_strings_struct_ptr, char *calling_routine_str, int num_cols, 
   int col_index, char *field_name_str, float *field_val_float_ptr)
   {
   if ( row_strings_struct_ptr->num_cols != num_cols )
      { printf("ERROR: %s: Expected %d column names in return row_strings_struct, found %d!\n", calling_routine_str, num_cols, row_strings_struct_ptr->num_cols); exit(EXIT_FAILURE); }
   if ( strcmp(row_strings_struct_ptr->ColNames[col_index], field_name_str) != 0 )
      { printf("ERROR: %s: Unexpected column names: Got %s instead of %s\n", calling_routine_str, row_strings_struct_ptr->ColNames[col_index], field_name_str); exit(EXIT_FAILURE); }
   if ( sscanf(row_strings_struct_ptr->ColStringVals[col_index], "%f", field_val_float_ptr) != 1 )
      { printf("ERROR: %s: Failed to convert to numeric values for %s!\n", calling_routine_str, row_strings_struct_ptr->ColStringVals[col_index]); exit(EXIT_FAILURE); }
   return;
   }


// ========================================================================================================
// ========================================================================================================
// A utility routine that does sanity checks and gets the integer row result data.

void GetRowResultInt(SQLRowStringsStruct *row_strings_struct_ptr, char *calling_routine_str, int num_cols, 
   int col_index, char *field_name_str, int *field_val_int_ptr)
   {
   if ( row_strings_struct_ptr->num_cols != num_cols )
      { printf("ERROR: %s: Expected %d column names in return row_strings_struct, found %d!\n", calling_routine_str, num_cols, row_strings_struct_ptr->num_cols); exit(EXIT_FAILURE); }
   if ( strcmp(row_strings_struct_ptr->ColNames[col_index], field_name_str) != 0 )
      { printf("ERROR: %s: Unexpected column names: Got %s instead of %s\n", calling_routine_str, row_strings_struct_ptr->ColNames[col_index], field_name_str); exit(EXIT_FAILURE); }
   if ( sscanf(row_strings_struct_ptr->ColStringVals[col_index], "%d", field_val_int_ptr) != 1 )
      { printf("ERROR: %s: Failed to convert to numeric values for %s!\n", calling_routine_str, row_strings_struct_ptr->ColStringVals[col_index]); exit(EXIT_FAILURE); }
   return;
   }


// ========================================================================================================
// ========================================================================================================
// A utility routine that does sanity checks and gets the string row result data.

void GetRowResultString(SQLRowStringsStruct *row_strings_struct_ptr, char *calling_routine_str, int num_cols, 
   int col_index, char *field_name_str, int required_string_len, char *field_val_str)
   {
   if ( row_strings_struct_ptr->num_cols != num_cols )
      { printf("ERROR: %s: Expected %d column names in return row_strings_struct, found %d!\n", calling_routine_str, num_cols, row_strings_struct_ptr->num_cols); exit(EXIT_FAILURE); }
   if ( strcmp(row_strings_struct_ptr->ColNames[col_index], field_name_str) != 0 )
      { printf("ERROR: %s: Unexpected column names: Got %s instead of %s\n", calling_routine_str, row_strings_struct_ptr->ColNames[col_index], field_name_str); exit(EXIT_FAILURE); }
   if ( required_string_len != -1 && strlen(row_strings_struct_ptr->ColStringVals[col_index]) != required_string_len )
      { printf("ERROR: %s: Length of string %d expected to be %d\n", calling_routine_str, (int)strlen(row_strings_struct_ptr->ColStringVals[col_index]), required_string_len); exit(EXIT_FAILURE); }
   strcpy(field_val_str, row_strings_struct_ptr->ColStringVals[col_index]);

   return;
   }


// ========================================================================================================
// ========================================================================================================
// This callback invoked for SQL queries that return EXACTLY ONE row. We simply transfer the string data from
// the argc and argv incoming data to the UserSpecifiedData argument. First arg is user-specified arg, 2nd arg 
// is number of columns in the result, 3rd arg is an array of pointers to strings RESULTS obtained from something 
// like sqlite3_column_text() and 4th arg are pointers to strings of column names.

int SQL_GetStringsDataForRow_callback(void *UserSpecifiedData, int argc, char **argv, char **azColName)
   {
   SQLRowStringsStruct *SQL_row_strings_struct_ptr = (SQLRowStringsStruct *)UserSpecifiedData;
   int col_num;

#ifdef DEBUG
printf("SQL_GetStringsDataForRow_callback(): called!\n"); fflush(stdout);
#endif

// Sanity check. Check to make sure the SQLRowStringsStruct is NOT NULL;
   if ( SQL_row_strings_struct_ptr == NULL )
      { printf("ERROR: SQL_GetStringsDataForRow_callback(): UserSpecified data MUST NOT BE NULL!\n"); exit(EXIT_FAILURE); }

// Sanity check. Make sure this callback was called ONLY ONCE.
   if ( SQL_row_strings_struct_ptr->num_cols != 0 )
      { printf("ERROR: SQL_GetStringsDataForRow_callback(): More than 1 row matched in table -- SQLRowStringsStruct already has data!\n"); exit(EXIT_FAILURE); }

// Force realloc to behave like malloc
   SQL_row_strings_struct_ptr->ColNames = NULL;
   SQL_row_strings_struct_ptr->ColStringVals = NULL;

// Transfer the string data to our data structure, creating storage as needed.
   for ( col_num = 0; col_num < argc; col_num++ )
      {

#ifdef DEBUG
printf("Creating SQLRowStringsStruct element for column number %d\n", col_num); fflush(stdout);
#endif

// Copy column name first and then value.
      if ( (SQL_row_strings_struct_ptr->ColNames = (char **)realloc(SQL_row_strings_struct_ptr->ColNames, sizeof(char *)*(SQL_row_strings_struct_ptr->num_cols + 1))) == NULL )
         { printf("ERROR: SQL_GetStringsDataForRow_callback(): Failed to (re)allocate storage for SQL_row_strings_struct_ptr.ColNames pointer!\n"); exit(EXIT_FAILURE); }
      if ( (SQL_row_strings_struct_ptr->ColStringVals = (char **)realloc(SQL_row_strings_struct_ptr->ColStringVals, sizeof(char *)*(SQL_row_strings_struct_ptr->num_cols + 1))) == NULL )
         { printf("ERROR: SQL_GetStringsDataForRow_callback(): Failed to (re)allocate storage for SQL_row_strings_struct_ptr.ColStringVals pointer!\n"); exit(EXIT_FAILURE); }

      if ( (SQL_row_strings_struct_ptr->ColNames[col_num] = (char *)malloc(sizeof(char)*(strlen(azColName[col_num]) + 1))) == NULL )
         { printf("ERROR: SQL_GetStringsDataForRow_callback(): Failed to allocate storage for SQL_row_strings_struct_ptr.ColNames string!\n"); exit(EXIT_FAILURE); }
      strcpy(SQL_row_strings_struct_ptr->ColNames[col_num], azColName[col_num]);

      if ( (SQL_row_strings_struct_ptr->ColStringVals[col_num] = (char *)malloc(sizeof(char)*(strlen(argv[col_num]) + 1))) == NULL )
         { printf("ERROR: SQL_GetStringsDataForRow_callback(): Failed to allocate storage for SQL_row_strings_struct_ptr.ColStringVals string!\n"); exit(EXIT_FAILURE); }
      strcpy(SQL_row_strings_struct_ptr->ColStringVals[col_num], argv[col_num]);

      SQL_row_strings_struct_ptr->num_cols++; 
      }

#ifdef DEBUG
int i;
for( i = 0; i < argc; i++ )
   { printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL"); }
printf("\n");
#endif

#ifdef DEBUG
printf("SQL_GetStringsDataForRow_callback(): DONE!\n"); fflush(stdout);
#endif

   return 0;
   }


// ========================================================================================================
// ========================================================================================================
// Get generic string data for the entire row of some table.

void GetStringsDataForRow(int max_string_len, sqlite3 *db, char *sql_command_str, 
   SQLRowStringsStruct *row_strings_struct_ptr)
   {
   char *zErrMsg = 0;
   int fc;

#ifdef DEBUG
printf("GetStringsDataForRow(): called!\n"); fflush(stdout);
#endif

// Initialize the number of columns to 0.
   row_strings_struct_ptr->num_cols = 0;

   fc = sqlite3_exec(db, sql_command_str, SQL_GetStringsDataForRow_callback, row_strings_struct_ptr, &zErrMsg);
   if ( fc != SQLITE_OK )
      { printf("SQL ERROR: %s\n", zErrMsg); sqlite3_free(zErrMsg); exit(EXIT_FAILURE); }

   return; 
   }


// ========================================================================================================
// ========================================================================================================
// Free up the strings allocated in SQL_GetStringsDataForRow_callback.

void FreeStringsDataForRow(SQLRowStringsStruct *row_strings_struct_ptr)
   {
   int col_num;

#ifdef DEBUG
printf("FreeStringsDataForRow(): called!\n"); fflush(stdout);
#endif

   for ( col_num = 0; col_num < row_strings_struct_ptr->num_cols; col_num++ )
      {
      free(row_strings_struct_ptr->ColNames[col_num]);
      free(row_strings_struct_ptr->ColStringVals[col_num]);
      }
   free(row_strings_struct_ptr->ColNames);
   free(row_strings_struct_ptr->ColStringVals);

   row_strings_struct_ptr->ColNames = NULL;
   row_strings_struct_ptr->ColStringVals = NULL;
   row_strings_struct_ptr->num_cols = 0;
   }


// ========================================================================================================
// ========================================================================================================
// This callback is called once for each matching row in a table referred to by the SQL query. It adds 1 
// element to the float array in UserSpecifiedData on each call dynamically. 

int SQL_GetAllocateListOfFloats_callback(void *UserSpecifiedData, int argc, char **argv, char **azColName)
   {
   SQLFloatStruct *SQL_float_struct_ptr = (SQLFloatStruct *)UserSpecifiedData;

#ifdef DEBUG
printf("SQL_GetAllocateListOfFloats_callback(): called!\n"); fflush(stdout);
#endif

// Sanity check. The SQL query MUST restrict the number of fields retrieved to ONLY 1. 
   if ( argc != 1 )
      { printf("ERROR: SQL_GetAllocateListOfFloats_callback(): Expected argc to be 1!\n"); exit(EXIT_FAILURE); }

// Sanity check. Check to make sure the SQLFloatStruct is NOT NULL;
   if ( SQL_float_struct_ptr == NULL )
      { printf("ERROR: SQL_GetAllocateListOfFloats_callback(): UserSpecified data MUST NOT BE NULL!\n"); exit(EXIT_FAILURE); }

// Allocate or re-allocate another element to the array. THIS ROUTINE GETS CALLED MULTIPLE TIMES.
   if ( SQL_float_struct_ptr->num_floats == 0 )
      {
      if ( (SQL_float_struct_ptr->float_arr = (float *)malloc(sizeof(float))) == NULL )
         { printf("ERROR: SQL_GetAllocateListOfFloats_callback(): Failed to allocate storage for SQLFloatStruct.float_arr!\n"); exit(EXIT_FAILURE); }
      }
   else
      {
      if ( (SQL_float_struct_ptr->float_arr = (float *)realloc(SQL_float_struct_ptr->float_arr, sizeof(float)*(SQL_float_struct_ptr->num_floats + 1))) == NULL )
         { printf("ERROR: SQL_GetAllocateListOfFloats_callback(): Failed to reallocate storage for SQLFloatStruct.float_arr!\n"); exit(EXIT_FAILURE); }
      }

   sscanf(argv[0], "%f", &(SQL_float_struct_ptr->float_arr[SQL_float_struct_ptr->num_floats]));
   (SQL_float_struct_ptr->num_floats)++;

#ifdef DEBUG
printf("SQL_GetAllocateListOfFloats_callback(): New number of elements in float array %d\n", SQL_float_struct_ptr->num_floats); fflush(stdout);
#endif

#ifdef DEBUG
int i;
for( i = 0; i < argc; i++ )
   { printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL"); }
printf("\n");
#endif

#ifdef DEBUG
printf("SQL_GetAllocateListOfFloats_callback(): DONE!\n"); fflush(stdout);
#endif

   return 0;
   }


// ========================================================================================================
// ========================================================================================================
// This routine will return an array of floats associated with a database table that match the SQL_query_str
// passed in as an argument. The array in the struct is allocated dynamically so the caller is responsible for 
// freeing it once it is used.

void GetAllocateListOfFloats(int max_string_len, sqlite3 *db, char *sql_command_str, SQLFloatStruct *float_struct)
   {
   char *zErrMsg = 0;
   int fc;

#ifdef DEBUG
printf("GetAllocateListOfFloats(): Called!\n"); fflush(stdout);
#endif

// Initialize the fields before the callback is called.
   float_struct->float_arr = NULL;
   float_struct->num_floats = 0;

// The callback will be called multiple times, once for each matching database element to sql_command_str. 
   fc = sqlite3_exec(db, sql_command_str, SQL_GetAllocateListOfFloats_callback, float_struct, &zErrMsg);

   if ( fc != SQLITE_OK )
      { printf("SQL ERROR: %s\n", zErrMsg); sqlite3_free(zErrMsg); exit(EXIT_FAILURE); }

   return;
   }


// ========================================================================================================
// ========================================================================================================
// This callback is called once for each matching row in a table referred to by the SQL query. It adds 1 
// element to the integer array in UserSpecifiedData on each call dynamically. 

int SQL_GetAllocateListOfInts_callback(void *UserSpecifiedData, int argc, char **argv, char **azColName)
   {
   SQLIntStruct *SQL_int_struct_ptr = (SQLIntStruct *)UserSpecifiedData;

#ifdef DEBUG
printf("SQL_GetAllocateListOfInts_callback(): BEGIN\n"); fflush(stdout);
#endif

// Sanity check. The SQL query MUST restrict the number of fields retrieved to ONLY 1. 
   if ( argc != 1 )
      { printf("ERROR: SQL_GetAllocateListOfInts_callback(): Expected argc to be 1!\n"); exit(EXIT_FAILURE); }

// Sanity check. Check to make sure the SQLIntStruct is NOT NULL;
   if ( SQL_int_struct_ptr == NULL )
      { printf("ERROR: SQL_GetAllocateListOfInts_callback(): UserSpecified data MUST NOT BE NULL!\n"); exit(EXIT_FAILURE); }

// Allocate or re-allocate another element to the array. THIS ROUTINE GETS CALLED MULTIPLE TIMES.
   if ( SQL_int_struct_ptr->num_ints == 0 )
      {
      if ( (SQL_int_struct_ptr->int_arr = (int *)malloc(sizeof(int))) == NULL )
         { printf("ERROR: SQL_GetAllocateListOfInts_callback(): Failed to allocate storage for SQLIntStruct.int_arr!\n"); exit(EXIT_FAILURE); }
      }
   else
      {
      if ( (SQL_int_struct_ptr->int_arr = (int *)realloc(SQL_int_struct_ptr->int_arr, sizeof(int)*(SQL_int_struct_ptr->num_ints + 1))) == NULL )
         { printf("ERROR: SQL_GetAllocateListOfInts_callback(): Failed to reallocate storage for SQLIntStruct.int_arr!\n"); exit(EXIT_FAILURE); }
      }

   sscanf(argv[0], "%d", &(SQL_int_struct_ptr->int_arr[SQL_int_struct_ptr->num_ints]));
   (SQL_int_struct_ptr->num_ints)++;

#ifdef DEBUG
printf("SQL_GetAllocateListOfInts_callback(): New number of elements in integer array %d\n", SQL_int_struct_ptr->num_ints); fflush(stdout);
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
// This routine will return an array of integers associated with a database table that match the SQL_query_str
// passed in as an argument. The array in the struct is allocated dynamically so the caller is responsible for 
// freeing it once it is used.

void GetAllocateListOfInts(int max_string_len, sqlite3 *db, char *sql_command_str, SQLIntStruct *int_struct)
   {
   char *zErrMsg = 0;
   int fc;

#ifdef DEBUG
printf("GetAllocateListOfInts(): Called!\n"); fflush(stdout);
#endif

// Initialize the fields before the callback is called.
   int_struct->int_arr = NULL;
   int_struct->num_ints = 0;

// The callback will be called multiple times, once for each matching database element to sql_command_str. 
   fc = sqlite3_exec(db, sql_command_str, SQL_GetAllocateListOfInts_callback, int_struct, &zErrMsg);

   if ( fc != SQLITE_OK )
      { printf("SQL ERROR: %s\n", zErrMsg); sqlite3_free(zErrMsg); exit(EXIT_FAILURE); }

   return;
   }


// ========================================================================================================
// ========================================================================================================
// ========================================================================================================
// Update NumPNs field in the VecPairs record with 'num_vals_per_vecpair'. Called from enrollDB.c

void UpdateVecPairsNumPNsField(int max_string_len, sqlite3 *db, int num_vals_per_vecpair, int vecpair_index)
   {
   char sql_command_str[max_string_len];
   char *zErrMsg = 0;
   int fc;

#ifdef DEBUG
printf("UpdateVecPairsNumPNsField(): Replacing NumPNs for vecpair_index %d with %d\n", vecpair_index, num_vals_per_vecpair); fflush(stdout);
#endif

   sprintf(sql_command_str, "UPDATE VecPairs SET NumPNs = %d WHERE id = %d;", num_vals_per_vecpair, vecpair_index);

// 'sqlite3_exec' calls sqlite3_prepare_v2(), sqlite3_step(), and sqlite3_finalize(), and can process multiple ';' separated SQL statements. 
// 'callback' function is called for each row produced in the result set. 4th arg is user-specified data. If an error occurs, 'zErrMesg' is 
// filled in, otherwise is NULL (if error, it must be freed).

// NOTE: There is no need to have a callback function (is NULL here) because 'UPDATE' does NOT return any result rows.
   fc = sqlite3_exec(db, sql_command_str, NULL, 0, &zErrMsg);
   if ( fc != SQLITE_OK )
      { printf("SQL ERROR: %s\n", zErrMsg); sqlite3_free(zErrMsg); exit(EXIT_FAILURE); }

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Get NumPNs field in the VecPair record.

int GetVecPairsNumPNsField(int max_string_len, sqlite3 *db, int vecpair_index)
   {
   SQLRowStringsStruct row_strings_struct;
   char sql_command_str[max_string_len];
   int num_PNs_per_vecpair = -1;
   char *col1_name = "NumPNs";

   sprintf(sql_command_str, "SELECT %s FROM VecPairs WHERE id = %d;", col1_name, vecpair_index);
   GetStringsDataForRow(max_string_len, db, sql_command_str, &row_strings_struct);
   GetRowResultInt(&row_strings_struct, "GetVecPairsNumPNsField()", 1, 0, col1_name, &num_PNs_per_vecpair);
   FreeStringsDataForRow(&row_strings_struct);

#ifdef DEBUG
printf("GetVecPairsNumPNsField(): Got %d for NumPNs for vecpair_index %d\n", num_PNs_per_vecpair, vecpair_index); fflush(stdout);
#endif

   return num_PNs_per_vecpair;
   }


// ========================================================================================================
// ========================================================================================================
// Get R_F_str field in the VecPair record. Return 0 for 'R' and 1 for 'F'.

int GetVecPairsRiseFallStrField(int max_string_len, sqlite3 *db, int vecpair_index)
   {
   SQLRowStringsStruct row_strings_struct;
   char sql_command_str[max_string_len];
   char rise_fall_str[max_string_len];
   char *col1_name = "R_F_str";

   sprintf(sql_command_str, "SELECT %s FROM VecPairs WHERE id = %d;", col1_name, vecpair_index);
   row_strings_struct.num_cols = 0;
   GetStringsDataForRow(max_string_len, db, sql_command_str, &row_strings_struct);
   GetRowResultString(&row_strings_struct, "GetVecPairsRiseFallStrField()", 1, 0, col1_name, 1, rise_fall_str);

#ifdef DEBUG
printf("GetVecPairsNumPNsField(): Got %s for R_F_str for vecpair_index %d\n", row_strings_struct.ColStringVals[0], vecpair_index); fflush(stdout);
#endif

   FreeStringsDataForRow(&row_strings_struct);

// Sanity check on the expected value for this field.
   if ( strcmp(rise_fall_str, "R") != 0 && strcmp(rise_fall_str, "F") != 0 )
      { 
      printf("ERROR: GetVecPairsNumPNsField(): Expected to find 'R' or 'F', found %s instead!\n", rise_fall_str); 
      exit(EXIT_FAILURE); 
      }

   if ( strcmp(rise_fall_str, "R") == 0 )
      return 0;
   else
      return 1;
   }


// ========================================================================================================
// ========================================================================================================
// Update NumVecs and NumRiseVecs fields in the Challenges record with parameters. Called from add_challengeDB.c

void UpdateChallengesNumVecFields(int max_string_len, sqlite3 *db, int challenge_index, int tot_vecs, int tot_rise_vecs)
   {
   char sql_command_str[max_string_len];
   char *zErrMsg = 0;
   int fc;

#ifdef DEBUG
printf("UpdateChallengesNumVecFields(): Replacing NumVecs and NumRiseVecs for challenge %d with %d and %d\n", challenge_index, 
   tot_vecs, tot_rise_vecs); fflush(stdout);
#endif

   sprintf(sql_command_str, "UPDATE Challenges SET NumVecs = %d, NumRiseVecs = %d WHERE id = %d;", tot_vecs, tot_rise_vecs, 
      challenge_index);

// NOTE: There is no need to have a callback function (is NULL here) because 'UPDATE' does NOT return any result rows.
   fc = sqlite3_exec(db, sql_command_str, NULL, 0, &zErrMsg);
   if ( fc != SQLITE_OK )
      { printf("SQL ERROR: %s\n", zErrMsg); sqlite3_free(zErrMsg); exit(EXIT_FAILURE); }

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Update NumPNs and NumRisePNs fields in the Challenges record with parameters. Called from add_challengeDB.c

void UpdateChallengesNumPNFields(int max_string_len, sqlite3 *db, int challenge_index, int tot_PNs, int tot_rise_PNs)
   {
   char sql_command_str[max_string_len];
   char *zErrMsg = 0;
   int fc;

#ifdef DEBUG
printf("UpdateChallengesNumPNFields(): Replacing NumPNs and NumRisePNs for challenge %d with %d and %d\n", challenge_index, 
   tot_PNs, tot_rise_PNs); fflush(stdout);
#endif

   sprintf(sql_command_str, "UPDATE Challenges SET NumPNs = %d, NumRisePNs = %d WHERE id = %d;", tot_PNs, tot_rise_PNs, 
      challenge_index);

// NOTE: There is no need to have a callback function (is NULL here) because 'UPDATE' does NOT return any result rows.
   fc = sqlite3_exec(db, sql_command_str, NULL, 0, &zErrMsg);
   if ( fc != SQLITE_OK )
      { printf("SQL ERROR: %s\n", zErrMsg); sqlite3_free(zErrMsg); exit(EXIT_FAILURE); }

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Get the field values associated with a challenge_index.

void GetChallengeNumVecsNumPNs(int max_string_len, sqlite3 *db, int *num_vecpairs_ptr, int *num_rising_vecpairs_ptr,
   int *num_PNs_ptr, int *num_rising_PNs_ptr, int challenge_index)
   {
   SQLRowStringsStruct row_strings_struct;
   char sql_command_str[max_string_len];
   char *col1_name = "NumVecs";
   char *col2_name = "NumRiseVecs";
   char *col3_name = "NumPNs";
   char *col4_name = "NumRisePNs";

   sprintf(sql_command_str, "SELECT %s, %s, %s, %s FROM Challenges WHERE id = %d;", col1_name, col2_name, col3_name, col4_name, 
      challenge_index);
   GetStringsDataForRow(max_string_len, db, sql_command_str, &row_strings_struct);
   GetRowResultInt(&row_strings_struct, "GetChallengeNumVecsNumPNs()", 4, 0, col1_name, num_vecpairs_ptr);
   GetRowResultInt(&row_strings_struct, "GetChallengeNumVecsNumPNs()", 4, 1, col2_name, num_rising_vecpairs_ptr);
   GetRowResultInt(&row_strings_struct, "GetChallengeNumVecsNumPNs()", 4, 2, col3_name, num_PNs_ptr);
   GetRowResultInt(&row_strings_struct, "GetChallengeNumVecsNumPNs()", 4, 3, col4_name, num_rising_PNs_ptr);
   FreeStringsDataForRow(&row_strings_struct);

#ifdef DEBUG
printf("GetChallengeNumVecsNumPNs(): Got num_vecpairs %d, num_rising_vecpairs %d, num_PNs %d and num_rising_PNs %d for Challenge index %d\n", 
   *num_vecpairs_ptr, *num_rising_vecpairs_ptr, *num_PNs_ptr, *num_rising_PNs_ptr, challenge_index); fflush(stdout);
#endif

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Look up the Challenge and get it's parameters.

void GetChallengeParams(int max_string_len, sqlite3 *db, char *ChallengeSetName, int *challenge_index_ptr, 
   int *num_vecpairs_ptr, int *num_rising_vecpairs_ptr, int *num_qualified_PNs_ptr, int *num_rise_qualified_PNs_ptr)
   {

// Get the index for this challenge. 
   if ( (*challenge_index_ptr = GetIndexFromTable(max_string_len, db, "Challenges", SQL_Challenges_get_index_cmd, NULL, 0, ChallengeSetName, 
      NULL, NULL, NULL, -1, -1, -1)) == -1 )
      {
      printf("ERROR: GetChallengeParams(): Failed to find challenge index for ChallengeSetName '%s' in Challenge table!\n", ChallengeSetName); 
      exit(EXIT_FAILURE); 
      }

// Get the NumVecs and NumPNs fields from challenge
   GetChallengeNumVecsNumPNs(max_string_len, db, num_vecpairs_ptr, num_rising_vecpairs_ptr, num_qualified_PNs_ptr, 
      num_rise_qualified_PNs_ptr, *challenge_index_ptr);

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Get Ave field in the TimingVals record. I started storing the Ave and TSig fields as integers, multiplied
// by 16 (fixed point) on 9/28/2018. Here we fetch the integer value as a floating point and then scale it
// to an actual floating point by dividing by 16.

float GetTimingValsAveField(int max_string_len, sqlite3 *db, int PUF_instance_index, int vecpair_index, int PO_num)
   {
   SQLRowStringsStruct row_strings_struct;
   char sql_command_str[max_string_len];
   char *col1_name = "Ave";
   float ave_val; 

   sprintf(sql_command_str, "SELECT %s FROM TimingVals WHERE PUFInstance = %d AND VecPair = %d AND PO = %d;", col1_name, 
      PUF_instance_index, vecpair_index, PO_num);

   GetStringsDataForRow(max_string_len, db, sql_command_str, &row_strings_struct);
   GetRowResultFloat(&row_strings_struct, "GetTimingValsAveField()", 1, 0, col1_name, &ave_val);
   FreeStringsDataForRow(&row_strings_struct);

// FIXED POINT
   ave_val /= 16.0;

#ifdef DEBUG
printf("GetTimingValsAveField(): Got %f for Ave for PUFInstance ID %d, vecpair index %d, PO %d\n", ave_val, PUF_instance_index,
   vecpair_index, PO_num); fflush(stdout);
#endif

   return ave_val;
   }


// ========================================================================================================
// ========================================================================================================
// Get TSig field in the TimingVals record. I started storing the Ave and TSig fields as integers, multiplied
// by 16 (fixed point) on 9/28/2018. Here we fetch the integer value as a floating point and then scale it
// to an actual floating point by dividing by 16.

float GetTimingValsTSigField(int max_string_len, sqlite3 *db, int PUF_instance_index, int vecpair_index, int PO_num)
   {
   SQLRowStringsStruct row_strings_struct;
   char sql_command_str[max_string_len];
   float tsig_val = -1.0;
   char *col1_name = "TSig";

   sprintf(sql_command_str, "SELECT %s FROM TimingVals WHERE PUFInstance = %d AND VecPair = %d AND PO = %d;", col1_name, 
      PUF_instance_index, vecpair_index, PO_num);
   row_strings_struct.num_cols = 0;
   GetStringsDataForRow(max_string_len, db, sql_command_str, &row_strings_struct);
   GetRowResultFloat(&row_strings_struct, "GetTimingValsTSigField()", 1, 0, col1_name, &tsig_val);
   FreeStringsDataForRow(&row_strings_struct);

   tsig_val /= 16.0;

#ifdef DEBUG
printf("GetTimingValsTSigField(): Got %f for TSig for PUFInstance ID %d, vecpair index %d, PO %d\n", tsig_val, PUF_instance_index,
   vecpair_index, PO_num); fflush(stdout);
#endif

   return tsig_val;
   }


// ========================================================================================================
// ========================================================================================================
// Get PUFInstances that match search criteria. Trying to get database table names out of application code.
// Note, wildcard '*' can be used for PUF_instance_name to get all elements.

void GetPUFInstanceIDsForInstanceName(int max_string_len, sqlite3 *db, SQLIntStruct *PUF_instance_index_struct_ptr, 
   char *PUF_instance_name)
   {
   char sql_command_str[max_string_len];

//printf("PUF_instance_name to match '%s'\n", PUF_instance_name); fflush(stdout);

// NOTE: Use '%' for * and '_' for ?, e.g., '%' matches everything, 'C%' matches everything that starts with a
// 'c' or 'C' (unless you set 'PRAGMA case_sensitive_like = true;').
   sprintf(sql_command_str, "SELECT ID FROM PUFInstance WHERE Instance_name LIKE '%s' ORDER BY ID ASC;", PUF_instance_name);
   GetAllocateListOfInts(max_string_len, db, sql_command_str, PUF_instance_index_struct_ptr);

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Retrieve the string information about the PUFInstance ID given as parameter.

void GetPUFInstanceInfoForID(int max_string_len, sqlite3 *db, int PUFInstance_id, char *Instance_name, char *Dev, 
   char *Placement)
   {
   SQLRowStringsStruct row_strings_struct;
   char sql_command_str[max_string_len];
   char *col1_name = "Instance_name";
   char *col2_name = "Dev";
   char *col3_name = "Placement";

   sprintf(sql_command_str, "SELECT %s, %s, %s FROM PUFInstance WHERE id = %d;", col1_name, col2_name, col3_name, PUFInstance_id);
   GetStringsDataForRow(max_string_len, db, sql_command_str, &row_strings_struct);

   GetRowResultString(&row_strings_struct, "GetPUFInstanceInfoForID", 3, 0, col1_name, -1, Instance_name);
   GetRowResultString(&row_strings_struct, "GetPUFInstanceInfoForID", 3, 1, col2_name, -1, Dev);
   GetRowResultString(&row_strings_struct, "GetPUFInstanceInfoForID", 3, 2, col3_name, -1, Placement);
   FreeStringsDataForRow(&row_strings_struct);

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Get num_PIs and num_POs fields associated with the PUFDesign index.

void GetPUFDesignNumPIPOFields(int max_string_len, sqlite3 *db, int *num_PIs_ptr, int *num_POs_ptr, int design_index)
   {
   SQLRowStringsStruct row_strings_struct;
   char sql_command_str[max_string_len];
   char *col1_name = "num_PIs";
   char *col2_name = "num_POs";

   sprintf(sql_command_str, "SELECT %s, %s FROM PUFDesign WHERE id = %d;", col1_name, col2_name, design_index);
   GetStringsDataForRow(max_string_len, db, sql_command_str, &row_strings_struct);
   GetRowResultInt(&row_strings_struct, "GetPUFDesignNumPIPOFields()", 2, 0, col1_name, num_PIs_ptr);
   GetRowResultInt(&row_strings_struct, "GetPUFDesignNumPIPOFields()", 2, 1, col2_name, num_POs_ptr);
   FreeStringsDataForRow(&row_strings_struct);

#ifdef DEBUG
printf("GetPUFDesignNumPIPOFields(): Got num_PIs %d and num_POs %d for PUFDesign index %d\n", *num_PIs_ptr, *num_POs_ptr, design_index); fflush(stdout);
#endif

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Look up the PUFDesign and get it's parameters.

int GetPUFDesignParams(int max_string_len, sqlite3 *db, char *Netlist_name, char *Synthesis_name, int *design_index_ptr, 
   int *num_PIs_ptr, int *num_POs_ptr)
   {

   if ( (*design_index_ptr = GetIndexFromTable(max_string_len, db, "PUFDesign", SQL_PUFDesign_get_index_cmd, NULL, 0, 
      Netlist_name, Synthesis_name, NULL, NULL, -1, -1, -1)) == -1 )
      { 
//      printf("INFO: GetPUFDesignParams(): PUFDesign index NOT found for '%s', '%s'!\n", Netlist_name, Synthesis_name); 
         return -1;
      }

// Get the num_PIs and num_POs values for this design.
   GetPUFDesignNumPIPOFields(max_string_len, db, num_PIs_ptr, num_POs_ptr, *design_index_ptr);

   return 0;
   }


// ========================================================================================================
// ========================================================================================================
// Read blobs, e.g., binary vectors from DB. Note, space MUST be allocated in the caller for 'binary_vec'. 
// Added 'allocate_storage' on 5/3/2019 for HOST.

int ReadBinaryBlob(sqlite3 *db, const char *sql_command_str, int id, unsigned char *binary_blob, int expected_size, 
   int allocate_storage, unsigned char **binary_blob_ptr)
   {
   int num_bytes_binary_blob = 0;
   sqlite3_stmt *pStmt;
   int rc;


// If allocate_storage is 0, then storage has already been allocated. In case there is no table entry for id or an error occurs.
   if ( allocate_storage == 0 )
      {

// Sanity check
      if ( binary_blob == NULL )
         { printf("ERROR: ReadBinaryBlob(): UNEXPECTED: Incoming binary_blob is NULL\n"); exit(EXIT_FAILURE); }
      binary_blob[0] = '\0';
      }

// Compile the SELECT statement into a virtual machine.
   do 
      {
      rc = sqlite3_prepare_v2(db, sql_command_str, strlen(sql_command_str) + 1, &pStmt, 0);
      if( rc != SQLITE_OK )
         { printf("ERROR: ReadBinaryBlob(): 'sqlite3_prepare_v2' failed\n"); exit(EXIT_FAILURE); }

// Bind the Blob, e.g., binary vector, to the SQL variable.
      sqlite3_bind_int(pStmt, 1, id);

// Run the virtual machine. The SQL statement prepared MUST return at most 1 row of data since we call sqlite3_step() ONLY once
// here. Normally, we would keep calling sqlite3_step until it returned something other than SQLITE_ROW, e.g., SQLITE_DONE. 
// Multiple kinds of errors can occur as well -- see doc.
      rc = sqlite3_step(pStmt);
      if ( rc == SQLITE_ROW )
         {

// The pointer returned by sqlite3_column_blob() points to memory that is owned by the statement handle (pStmt). It is only good
// until the next call to an sqlite3_XXX() function (e.g. the sqlite3_finalize() below) that involves the statement handle. 
// So we need to make a copy of the blob into memory obtained from malloc() to return to the caller.
         num_bytes_binary_blob = sqlite3_column_bytes(pStmt, 0);

// Added 5/3/2019 for HOST
         if ( allocate_storage == 0 )
            {
            if ( num_bytes_binary_blob != expected_size )
               { printf("ERROR: ReadBinaryBlob(): UNEXPECTED return size from vector! %d vs expected %d\n", num_bytes_binary_blob, expected_size); exit(EXIT_FAILURE); }
            memcpy(binary_blob, sqlite3_column_blob(pStmt, 0), num_bytes_binary_blob);
            }
         else 
            {
            if ( (*binary_blob_ptr = (unsigned char *)calloc(num_bytes_binary_blob, sizeof(unsigned char))) == NULL )
               { printf("ReadBinaryBlob(): ERROR: Failed to allocate storage for binary_blob of %d bytes\n", num_bytes_binary_blob); exit(EXIT_FAILURE); }
            memcpy(*binary_blob_ptr, sqlite3_column_blob(pStmt, 0), num_bytes_binary_blob);
            }
         }

// NOT the only possibility, SQLITE_BUSY, SQLITE_DONE, etc. are possible.
      else
         { printf("ReadBinaryBlob(): ERROR: UNEXPECTED return value for sqlite3_step() -- More than one row in the results perhaps? %d\n", rc); exit(EXIT_FAILURE); }

// Finalize the statement (this releases resources allocated by sqlite3_prepare() ).
      rc = sqlite3_finalize(pStmt);

// If sqlite3_finalize() returned SQLITE_SCHEMA, then try to execute the statement all over again.
      } while( rc == SQLITE_SCHEMA );

   if ( rc != SQLITE_OK )
      { printf("ReadBinaryBlob(): ERROR: Some other error condition %d\n", rc); exit(EXIT_FAILURE); }

   return num_bytes_binary_blob;
   }


// ========================================================================================================
// ========================================================================================================
// Search a table for a match to the input args. Return its index.

int GetIndexFromTable(int max_string_len, sqlite3 *db, char *Table, const char *SQL_cmd, unsigned char *blob, 
   int blob_size_bytes, char *text1, char *text2, char *text3, char *text4, int int1, int int2, int int3)
   {
   sqlite3_stmt *pStmt;
   int rc, fc;
   int index;

#ifdef DEBUG
printf("GetIndexFromTable(): SQL cmd %s\n", SQL_cmd); fflush(stdout);
#endif

// Prepare 'pStmt' with SQL query.
   rc = sqlite3_prepare_v2(db, SQL_cmd, strlen(SQL_cmd) + 1, &pStmt, 0);
   if( rc != SQLITE_OK )
      { printf("ERROR: GetIndexFromTable(): 'sqlite3_prepare_v2' failed with %d\n", rc); exit(EXIT_FAILURE); }

// Bind the variables to '?' in 'SQL_cmd'.
   if ( strcmp(Table, "PUFDesign") == 0 )
      {
      sqlite3_bind_text(pStmt, 1, text1, -1, SQLITE_STATIC);
      sqlite3_bind_text(pStmt, 2, text2, -1, SQLITE_STATIC);
      }
   else if ( strcmp(Table, "PUFInstance") == 0 )
      {
      sqlite3_bind_text(pStmt, 1, text1, -1, SQLITE_STATIC);
      sqlite3_bind_text(pStmt, 2, text2, -1, SQLITE_STATIC);
      sqlite3_bind_text(pStmt, 3, text3, -1, SQLITE_STATIC);
      }
   else if ( strcmp(Table, "VecPairs") == 0 )
      {
      sqlite3_bind_int(pStmt, 1, int1);
      sqlite3_bind_int(pStmt, 2, int2);
      sqlite3_bind_int(pStmt, 3, int3);
      }
   else if ( strcmp(Table, "Vectors") == 0 )
      sqlite3_bind_blob(pStmt, 1, blob, blob_size_bytes, SQLITE_STATIC);
   else if ( strcmp(Table, "PathSelectMasks") == 0 )
      sqlite3_bind_text(pStmt, 1, text1, -1, SQLITE_STATIC);
   else if ( strcmp(Table, "Challenges") == 0 )
      sqlite3_bind_text(pStmt, 1, text1, -1, SQLITE_STATIC);
   else if ( strcmp(Table, "ChallengeVecPairs") == 0 )
      {
      sqlite3_bind_int(pStmt, 1, int1);
      sqlite3_bind_int(pStmt, 2, int2);
      sqlite3_bind_int(pStmt, 3, int3);
      }
   else
      { printf("ERROR: GetIndexFromTable(): Unknown Table '%s'\n", Table); exit(EXIT_FAILURE); }

// Run the virtual machine. The SQL statement prepared MUST return at most 1 row of data since we call sqlite3_step() ONLY once
// here. Normally, we would keep calling sqlite3_step until it returned something other than SQLITE_ROW. Multiple kinds of errors 
// can occur as well -- see doc.
   rc = sqlite3_step(pStmt);

#ifdef DEBUG
printf("GetIndexFromTable(): Table '%s', Column cnt %d\n", Table, sqlite3_column_count(pStmt)); fflush(stdout);
printf("GetIndexFromTable(): Table '%s', Column type %d => Integer type %d\n", Table, sqlite3_column_type(pStmt, 0), SQLITE_INTEGER); fflush(stdout);
#endif

   index = sqlite3_column_int64(pStmt, 0);

   if ( (fc = sqlite3_finalize(pStmt)) != 0 )
      { printf("ERROR: GetIndexFromTable(): Finalize failed %d for Table '%s'\n", fc, Table); exit(EXIT_FAILURE); }

// Classify the return code. 'DONE' means search failed while 'ROW' means it found the item.
   if ( rc == SQLITE_DONE )
      { 
//      printf("WARNING: GetIndexFromTable(): Search string NOT FOUND in Table '%s': Return code %d\n", Table, rc); fflush(stdout); 
      index = -1;
      }
   else if ( rc != SQLITE_ROW )
      { printf("ERROR: GetIndexFromTable(): Return code for 'sqlite3_step' not SQLITE_DONE or SQLITE_ROW => %d for Table '%s'\n", rc, Table); exit(EXIT_FAILURE); }

#ifdef DEBUG
printf("GetIndexFromTable(): Index %d for Table '%s'\n", index, Table); fflush(stdout);
#endif

   return index;
   }


// ========================================================================================================
// ========================================================================================================
// Insert an element into a table given by the input arguments.

int InsertIntoTable(int max_string_len, sqlite3 *db, char *Table, const char *SQL_cmd, unsigned char *vector,
   int vector_size_bytes, char *text1, char *text2, char *text3, char *text4, char *text5, int int1, int int2, 
   int int3, int int4, int int5, float float1, float float2)
   {
   sqlite3_stmt *pStmt;
   int rc, fc;

#ifdef DEBUG
printf("InsertIntoTable(): SQL cmd %s\n", SQL_cmd); fflush(stdout);
#endif

// Prepare 'pStmt' with SQL query.
   rc = sqlite3_prepare_v2(db, SQL_cmd, strlen(SQL_cmd) + 1, &pStmt, 0);
   if( rc != SQLITE_OK )
      { printf("ERROR: InsertIntoTable(): 'sqlite3_prepare_v2' failed with %d\n", rc); exit(EXIT_FAILURE); }

// Insert the Netlist_name into PUFDesign
   if ( strcmp(Table, "PUFDesign") == 0 )
      {
      sqlite3_bind_text(pStmt, 1, text1, -1, SQLITE_STATIC);
      sqlite3_bind_text(pStmt, 2, text2, -1, SQLITE_STATIC);
      sqlite3_bind_int(pStmt, 3, int1);
      sqlite3_bind_int(pStmt, 4, int2);
      }
   else if ( strcmp(Table, "PUFInstance") == 0 )
      {
      sqlite3_bind_text(pStmt, 1, text1, -1, SQLITE_STATIC);
      sqlite3_bind_text(pStmt, 2, text2, -1, SQLITE_STATIC);
      sqlite3_bind_text(pStmt, 3, text3, -1, SQLITE_STATIC);
      sqlite3_bind_text(pStmt, 4, text4, -1, SQLITE_STATIC);
      sqlite3_bind_int(pStmt, 5, int1);
      }
   else if ( strcmp(Table, "VecPairs") == 0 )
      {
      sqlite3_bind_text(pStmt, 1, text1, -1, SQLITE_STATIC);
      sqlite3_bind_int(pStmt, 2, int1);
      sqlite3_bind_int(pStmt, 3, int2);
      sqlite3_bind_int(pStmt, 4, int3);
      sqlite3_bind_int(pStmt, 5, int4);
      }

// FIXED POINT. On 9/28/2018, I converted the 'real' fields (double) here to scaled integer to save space. Note that there
// was a bug prior to this where I wasn't actually storing the 3 sigma value (TSig) but rather the Ave value twice...
   else if ( strcmp(Table, "TimingTable") == 0 )
      {
      sqlite3_bind_int(pStmt, 1, int1);
      sqlite3_bind_int(pStmt, 2, int2);
      sqlite3_bind_int(pStmt, 3, (int)(float1*16.0));
      sqlite3_bind_int(pStmt, 4, (int)(float2*16.0));
      sqlite3_bind_int(pStmt, 5, int3);
      }
   else if ( strcmp(Table, "Challenges") == 0 )
      {
      sqlite3_bind_text(pStmt, 1, text1, -1, SQLITE_STATIC);
      sqlite3_bind_int(pStmt, 2, int1);
      sqlite3_bind_int(pStmt, 3, int2);
      sqlite3_bind_int(pStmt, 4, int3);
      sqlite3_bind_int(pStmt, 5, int4);
      sqlite3_bind_int(pStmt, 6, int5);
      }
   else if ( strcmp(Table, "Vectors") == 0 )
      sqlite3_bind_blob(pStmt, 1, vector, vector_size_bytes, SQLITE_STATIC);
   else if ( strcmp(Table, "PathSelectMasks") == 0 )
      sqlite3_bind_text(pStmt, 1, text1, -1, SQLITE_STATIC);
   else if ( strcmp(Table, "ChallengeVecPairs") == 0 )
      {
      sqlite3_bind_int(pStmt, 1, int1);
      sqlite3_bind_int(pStmt, 2, int2);
      sqlite3_bind_int(pStmt, 3, int3);
      }
   else
      { printf("ERROR: InsertIntoTable(): Unknown Table '%s'\n", Table); exit(EXIT_FAILURE); }

   rc = sqlite3_step(pStmt);

   if ( (fc = sqlite3_finalize(pStmt)) != 0 && fc != SQLITE_CONSTRAINT )
      { printf("ERROR: InsertIntoTable(): Finalize failed %d for Table '%s'\n", fc, Table); exit(EXIT_FAILURE); }

// FIX ME AT SOME POINT. Return 1 if successful and 0 if unsuccessful (flip these around -- did this for 
// InsertIntoTable_RT in PUFCash V3.0).
   if ( rc == SQLITE_DONE )
      return 0;
   else if ( rc == SQLITE_CONSTRAINT )
      {
#ifdef DEBUG
      printf("\t\tINFO: InsertIntoTable(): Element already exists in Table '%s'!\n", Table); fflush(stdout);
#endif
      return 1;
      }
   else 
      { printf("ERROR: InsertIntoTable(): Return code => %d for Table '%s'\n", rc, Table); exit(EXIT_FAILURE); }

   return 1;
   }


// ========================================================================================================
// ========================================================================================================
// Utility routine that takes a mask_asc and counts the number of POs that are selected. '0' in the mask_asc 
// increments no_trans, '1' increments hard_selected, 'u' increments unqual_selected, 'q' increments qual_selected.

void DetermineNumSelectedPOsInMask(char *mask_asc, int num_POs, int *num_no_trans_ptr, int *num_hard_selected_ptr,
   int *num_unqual_selected_ptr, int *num_qual_selected_ptr)
   {
   int PO_num;
   int sum_of_mask_bits;

   (*num_no_trans_ptr) = (*num_hard_selected_ptr) = (*num_unqual_selected_ptr) = (*num_qual_selected_ptr) = 0;

   for ( PO_num = 0; PO_num < num_POs; PO_num++ )
      {
      if ( mask_asc[PO_num] == '0' )
         (*num_no_trans_ptr)++; 
      else if ( mask_asc[PO_num] == '1' ) 
         (*num_hard_selected_ptr)++; 
      else if ( mask_asc[PO_num] == 'u' ) 
         (*num_unqual_selected_ptr)++;
      else if ( mask_asc[PO_num] == 'q' ) 
         (*num_qual_selected_ptr)++;

// Error will be treated below.
      else 
         { printf("ERROR: DetermineSelectedPOsInMask(): Mask bit not equal to one of '0', '1', 'u' or 'q' at pos %d\n", PO_num); fflush(stdout); } 
      }

// Sanity check
   sum_of_mask_bits = *num_no_trans_ptr + *num_hard_selected_ptr + *num_unqual_selected_ptr + *num_qual_selected_ptr; 
   if ( sum_of_mask_bits != num_POs )
      { 
      printf("ERROR: DetermineSelectedPOsInMask(): Sum of '0', '1', 'u' and 'q' %d NOT equal to num_POs %d\n", sum_of_mask_bits, num_POs); 
      exit(EXIT_FAILURE); 
      }

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Utility routine that takes a mask_asc and returns an array of integers that give the PO numbers tested by 
// the ASCII mask_asc. Note that typically this is called with mask with only '0' and '1' but it works for 
// 'compatibility masks with 'u' and 'q' as well. NOTE: ONLY '1's and 'q's add elements to the list of 
// integers when the 'only_qualified' parameter is set to 1. 'u's are unqualified paths so they are NOT 
// included even though a transition occurs on them. If 'only_qualified' is 0, then 'u's are included as
// well.
//
// NOTE: the mask_asc is parsed from right to left, i.e., high address to low address. The mask bits at the 
// highest address correspond to the low order POs. The list of integers then gives the POs in increasing 
// order, similar to the way the enrollment timing data files are given when enrollment data is collected 
// by the hardware. NOTE: this array is allocated in the caller for efficiently reasons. 

void GetSelectedPOsInMask(char *mask_asc, int num_POs, int *list_of_PO_nums, int *num_POs_found_ptr, 
   int only_qualified)
   {
   int PO_num;

   *num_POs_found_ptr = 0;
   for ( PO_num = 0; PO_num < num_POs; PO_num++ )
      {
      if ( mask_asc[num_POs - PO_num - 1] == '1' || mask_asc[num_POs - PO_num - 1] == 'q' || 
         (only_qualified == 0 && mask_asc[num_POs - PO_num - 1] == 'u') )
         {
         list_of_PO_nums[*num_POs_found_ptr] = PO_num;
         (*num_POs_found_ptr)++;
         }

// Sanity check
      else if ( mask_asc[num_POs - PO_num - 1] != '0' && mask_asc[num_POs - PO_num - 1] != 'u' )
         { printf("ERROR: GetSelectedPOsInMask(): Mask bit not equal to one of '0', '1', 'u' or 'q' at pos %d\n", num_POs - PO_num - 1); fflush(stdout); } 
      }

   return;
   }


// ========================================================================================================
// ========================================================================================================
// This routine compares a string constructed by inspecting the PO numbers in the TimingVals table for a
// PUFInstance/VecPair ('all_PO_trans_mask_from_timing_data') to a challenge_mask read from a file (that is 
// about to be written to the PathSelectMasks table). If it is NOT consistent, then error. Note that the 
// challenge_mask can have '1' or 'q' for paths with transitions, so these all match '1's that occur in 
// the 'all_PO_trans_mask_from_timing_data'

void ComparePOGenMaskWithChallengeMask(int max_string_len, sqlite3 *db, int PUF_instance_index, int vecpair_index, 
   char *all_PO_trans_mask_from_timing_data, char *challenge_mask, int num_POs)
   {
   int PO_num;

// Sanity check.
   if ( strlen(all_PO_trans_mask_from_timing_data) != num_POs )
      {
      printf("ERROR: ComparePOGenMaskWithChallengeMask(): Length of 'all_PO_trans_mask_from_timing_data' is %d => Expected %d\n", 
         (int)strlen(all_PO_trans_mask_from_timing_data), num_POs); exit(EXIT_FAILURE);
      }

// Sanity check.
   if ( strlen(challenge_mask) != num_POs )
      {
      printf("ERROR: ComparePOGenMaskWithChallengeMask(): Length of 'challenge_mask' is %d => Expected %d\n", 
         (int)strlen(challenge_mask), num_POs); exit(EXIT_FAILURE);
      }

// Consistency check. Match 0 to 0, and then 1 to 1, u or q.
   for ( PO_num = 0; PO_num < num_POs; PO_num++ )
      {

// For cases in which the bits do NOT match, then when the challenge mask is '1', 'u' or 'q', we MUST also have a 1 in the
// all_PO_trans_mask_from_timing_data. If the PO-generated mask does NOT have a transition and the challenge selects this path with
// a '1', 'u' or 'q', then ERROR. 9/30/2018: Path marked with 'u' (unqualified) do NOT have to have timing data in the database.
//      if ( all_PO_trans_mask_from_timing_data[PO_num] == '0' && (challenge_mask[PO_num] == '1' || challenge_mask[PO_num] == 'u' || challenge_mask[PO_num] == 'q') )
      if ( all_PO_trans_mask_from_timing_data[PO_num] == '0' && (challenge_mask[PO_num] == '1' || challenge_mask[PO_num] == 'q') )
         {
         printf("ERROR: ComparePOGenMaskWithChallengeMask(): (PUFInstance index %d, vecpair_index %d) PO-constructed mask at PO_num %d has NO transition (%c) \
but challenge mask selects this PO with %c\n", PUF_instance_index, vecpair_index, PO_num, all_PO_trans_mask_from_timing_data[PO_num], challenge_mask[PO_num]); 
         exit(EXIT_FAILURE);
         }
      }

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Read in the enrollment data from a Chip/placement file, e.g., C1_V1_KG_FU_... Modified 8/3/2019 to handle
// TDC enrollment files.

int ReadChipEnrollPNs(int max_string_len, char ChipEnrollDatafile[max_string_len], float **PNX_ptr, 
   float **PNX_Tsig_ptr, int **rise_fall_ptr, int **vec_pairs_ptr, int **POs_ptr, int *num_PNX_ptr, 
   int num_POs, int has_masks, int num_vec_pairs, char **master_masks, int debug_flag)
   {
   int rise_fall, num_sams, sam_num, num_PNR, vec_pair, PO;
   int num_allocated_PN_vals = 0;
   char line[max_string_len];
   float PN_mean, PN_Tsig;
   float *PN_vals = NULL;
   int first_skip_MPS;
   char *char_ptr;
   FILE *INFILE;

   if ( (INFILE = fopen(ChipEnrollDatafile, "r")) == NULL )
      { printf("ERROR: ReadChipEnrollPNs(): Could not open PNs database file %s\n", ChipEnrollDatafile); exit(EXIT_FAILURE); }

   rise_fall = 0;
   num_sams = 0;
   num_PNR = 0;

// Make sure all of these are set to NULL since we realloc depends on it below.
   *PNX_ptr = NULL;
   *PNX_Tsig_ptr = NULL;
   *rise_fall_ptr = NULL;
   *vec_pairs_ptr = NULL;
   *POs_ptr = NULL;
   *num_PNX_ptr = 0;

   first_skip_MPS = 1;
   int state = 0;
   while ( fgets(line, max_string_len, INFILE) != NULL )
      {

//printf("ReadChipEnrollPNs(): Processing '%s'\n", line); fflush(stdout);

// 8/3/2019: Added this to handle TDC enrollment files which contain calibration data at the beginning of the file.
// ONLY process lines that begin with 'V:' AND '0:' AND 'C:'. This should skip ALL of the calibration data lines.
// 	'V: 0	O: 0	C: 0	MPS1: 3	MPS2: 3	 316 316 316 316 316 316 316 316 316 316 316 316 316 316 316 316'
      if ( state == 0 && (strstr(line, "V:") == NULL || strstr(line, "O:") == NULL || strstr(line, "C:") == NULL) )
         continue;

// First time we see a 'V:' data line, set the state so we process the rest of the file (which has an empty line
// mid-way through it to separate rise and fall).
      else
         state = 1;

// There are blank lines between the rise and fall PNs. 
      if ( strlen(line) == 0 || strlen(line) == 1 )
         {

// DEBUG
//printf("ReadChipEnrollPNs(): Flipping current value of rise_fall flag %d\n", rise_fall); fflush(stdout);

// Invert rise_fall flag. Note that we now use separate arrays for the rise and fall PNs.
         rise_fall++; 
         continue;
         }

// If the while condition above does not fail after reading the blank line at the end of the file, then there's more data.
// This routine assumes each file has data for ONE CHIP.
      if ( rise_fall > 1 )
         { printf("ERROR: ReadChipEnrollPNs(): Datafile has data for more than 1 chip!\n"); exit(EXIT_FAILURE); }

// Header information is ignored (for now)
      if ((char_ptr = strtok(line, " \t")) == NULL )
         { printf("ERROR: ReadChipEnrollPNs(): No ' ' found in data line for 'V:'!\n"); exit(EXIT_FAILURE); }
      if ( strcmp(char_ptr, "V:") != 0 )
         { printf("ERROR: ReadChipEnrollPNs(): Expected 'V:' as first token!\n"); exit(EXIT_FAILURE); }

      if ((char_ptr = strtok(NULL, " \t")) == NULL )
         { printf("ERROR: ReadChipEnrollPNs(): No ' ' found in data line for vector number!\n"); exit(EXIT_FAILURE); }
      sscanf(char_ptr, "%d", &vec_pair);

      if ((char_ptr = strtok(NULL, " \t")) == NULL )
         { printf("ERROR: ReadChipEnrollPNs(): No ' ' found in data line for 'O:'!\n"); exit(EXIT_FAILURE); }
      if ( strcmp(char_ptr, "O:") != 0 )
         { printf("ERROR: ReadChipEnrollPNs(): Expected 'O:' as third token => '%s'!\n", char_ptr); exit(EXIT_FAILURE); }

      if ((char_ptr = strtok(NULL, " \t")) == NULL )
         { printf("ERROR: ReadChipEnrollPNs(): No ' ' found in data line for output number!\n"); exit(EXIT_FAILURE); }
      sscanf(char_ptr, "%d", &PO);

      if ((char_ptr = strtok(NULL, " \t")) == NULL )
         { printf("ERROR: ReadChipEnrollPNs(): No ' ' found in data line for 'C:'!\n"); exit(EXIT_FAILURE); }
      if ( strcmp(char_ptr, "C:") != 0 )
         { printf("ERROR: ReadChipEnrollPNs(): Expected 'C:' as fifth token => '%s'!\n", char_ptr); exit(EXIT_FAILURE); }

      if ((char_ptr = strtok(NULL, " \t")) == NULL )
         { printf("ERROR: ReadChipEnrollPNs(): No ' ' found in data line for PN cnter!\n"); exit(EXIT_FAILURE); }

// Sanity check
      if ( PO < 0 || PO >= num_POs )
         { printf("ERROR: ReadChipEnrollPNs(): PO number read from file %d is outside range of 0 to num_POs - 1 %d!\n", PO, num_POs - 1); exit(EXIT_FAILURE); }

// 9/30/2018: Skip paths that are not selected by the masks, if the mask file exists. This is to allow smaller databases to be created with
// fewer than the number of enrollment PNs.
      if ( has_masks == 1 )
         {

// Sanity check
         if ( vec_pair >= num_vec_pairs )
            { printf("ERROR: ReadChipEnrollPNs(): Vector number recorded in file %d larger than number of masks %d!\n", vec_pair, num_vec_pairs); exit(EXIT_FAILURE); }
   
// Check if this path is selected, continue if not (either 0 or 'u' for unqualitied'. Be sure to reverse the numbering, low order addresses in masks store high order 
// mask bits.
         if ( master_masks[vec_pair][num_POs - PO - 1] == '0' || master_masks[vec_pair][num_POs - PO - 1] == 'u' )
            continue;
         }

// Assume each line in one of these formats with multiple sams
// 	'V: 11	O: 0	C: 2705	 319 320 320 320 320 320 320 320 320 320 323 320 320 320 320 320'
// 	'V: 0	O: 0	C: 0	MPS1: 3	MPS2: 3	 316 316 316 316 316 316 316 316 316 316 316 316 316 316 316 316'

// Retrieve the samples, one at a time, to store to temporary array.
      sam_num = 0;
      while ( (char_ptr = strtok(NULL, " \t")) != NULL )
         {

// 8/3/2019: Skip the MPSx if they exist.
         if ( strstr(char_ptr, "MPS1:") != NULL )
            {
            if ( first_skip_MPS == 1 )
               { printf("Skipping MPSx data on line!\n"); fflush(stdout); }
            first_skip_MPS = 0;
            if ( (char_ptr = strtok(NULL, " \t")) == NULL )
               { printf("ERROR: ReadChipEnrollPNs(): Expected MPS data!\n"); exit(EXIT_FAILURE); }
            if ( (char_ptr = strtok(NULL, " \t")) == NULL )
               { printf("ERROR: ReadChipEnrollPNs(): Expected MPS data!\n"); exit(EXIT_FAILURE); }
            if ( strstr(char_ptr, "MPS2:") == NULL )
               { printf("ERROR: ReadChipEnrollPNs(): Expected 'MPS2:' !\n"); exit(EXIT_FAILURE); }
            if ( (char_ptr = strtok(NULL, " \t")) == NULL )
               { printf("ERROR: ReadChipEnrollPNs(): Expected MPS data!\n"); exit(EXIT_FAILURE); }
            if ( (char_ptr = strtok(NULL, " \t")) == NULL )
               { printf("ERROR: ReadChipEnrollPNs(): Expected sample data!\n"); exit(EXIT_FAILURE); }
            }

// Allocate storage ONLY when processing the first line. We will re-use this storage over-and-over again for each subsequent line.
         if ( sam_num == num_allocated_PN_vals )
            {
            if ( (PN_vals = (float *)realloc(PN_vals, sizeof(float) * (num_allocated_PN_vals + 1))) == NULL )
               { printf("ERROR: ReadChipEnrollPNs(): Failed to re-allocate storage for PN_vals!\n"); exit(EXIT_FAILURE); }
            num_allocated_PN_vals++; 
            }

         sscanf(char_ptr, "%f", &(PN_vals[sam_num]));
         sam_num++;
         }

// Sanity check. Number of samples MUST be consistent across all chips and PN lines. 'num_sams' is initially 0 so after first line of data
// is processed, it gets set here.
      if ( num_sams == 0 )
         {
         num_sams = sam_num;
         if ( num_sams == 0 )
            { printf("ERROR: ReadChipEnrollPNs(): Number of samples is 0!\n"); exit(EXIT_FAILURE); }
         }
      else if ( num_sams != sam_num )
         { printf("ERROR: ReadChipEnrollPNs(): Sample number mismatch across PN lines!\n"); exit(EXIT_FAILURE); }

// Compute the mean and three sig.
      PN_mean = ComputeMean(num_sams, PN_vals);
      if ( num_sams > 1 )
         PN_Tsig = 3*ComputeStdDev(num_sams, PN_mean, PN_vals);
      else
         PN_Tsig = 0.0;

// Flag PNs with large variations
      if ( PN_Tsig >= MAX_TSIG )
         {
         printf("\t\tINFO: ReadChipEnrollPNs(): Rise or Fall? %d, PN (C: %d), Three Sig %f is greater than threshold %f!\n",
            rise_fall, *num_PNX_ptr, PN_Tsig, MAX_TSIG); fflush(stdout); 
         }

// NULL assignment above forces realloc to behave as malloc on the first call. 
      if ( (*PNX_ptr = (float *)realloc(*PNX_ptr, sizeof(float)*(*num_PNX_ptr + 1))) == NULL )
         { printf("ERROR: ReadChipEnrollPNs(): Failed to re-allocate storage for PNX!\n"); exit(EXIT_FAILURE); }
      if ( (*PNX_Tsig_ptr = (float *)realloc(*PNX_Tsig_ptr, sizeof(float)*(*num_PNX_ptr + 1))) == NULL )
         { printf("ERROR: ReadChipEnrollPNs(): Failed to re-allocate storage for PNX!\n"); exit(EXIT_FAILURE); }
      if ( (*rise_fall_ptr = (int *)realloc(*rise_fall_ptr, sizeof(int)*(*num_PNX_ptr + 1))) == NULL )
         { printf("ERROR: ReadChipEnrollPNs(): Failed to re-allocate storage for rise_fall!\n"); exit(EXIT_FAILURE); }
      if ( (*vec_pairs_ptr = (int *)realloc(*vec_pairs_ptr, sizeof(int)*(*num_PNX_ptr + 1))) == NULL )
         { printf("ERROR: ReadChipEnrollPNs(): Failed to re-allocate storage for vec_pairs!\n"); exit(EXIT_FAILURE); }
      if ( (*POs_ptr = (int *)realloc(*POs_ptr, sizeof(int)*(*num_PNX_ptr + 1))) == NULL )
         { printf("ERROR: ReadChipEnrollPNs(): Failed to re-allocate storage for POs!\n"); exit(EXIT_FAILURE); }

// Compute the average value and store in the array.
      (*PNX_ptr)[*num_PNX_ptr] = PN_mean;
      if ( num_sams > 1 )
         (*PNX_Tsig_ptr)[*num_PNX_ptr] = PN_Tsig;
      else
         (*PNX_Tsig_ptr)[*num_PNX_ptr] = 0.0;

      (*rise_fall_ptr)[*num_PNX_ptr] = rise_fall;
      (*vec_pairs_ptr)[*num_PNX_ptr] = vec_pair;
      (*POs_ptr)[*num_PNX_ptr] = PO;

      (*num_PNX_ptr)++;
      if ( rise_fall == 0 )
         num_PNR++;
      }

   fclose(INFILE);

printf("\tReadChipEnrollPNs(): Num PNR %d\tNum PNF %d\tNum sams %d\n", num_PNR, *num_PNX_ptr - num_PNR, num_sams); fflush(stdout);

   return num_PNR;
   }


// ========================================================================================================
// ========================================================================================================
// Delete a PUF Instance. NOTE: All of it's timing data will also be deleted because we have 'ON DELETE CASCADE'
// set on the TimingVals Table for the PUFInstance field.

int DeletePUFInstance(int max_string_len, sqlite3 *db, const char *SQL_cmd, int instance_index)
   {
   sqlite3_stmt *pStmt;
   int rc, fc;

// Prepare 'pStmt' with SQL query.
   rc = sqlite3_prepare_v2(db, SQL_cmd, strlen(SQL_cmd) + 1, &pStmt, 0);
   if( rc != SQLITE_OK )
      { printf("ERROR: DeletePUFInstance(): 'sqlite3_prepare_v2' failed with %d\n", rc); exit(EXIT_FAILURE); }

   sqlite3_bind_int(pStmt, 1, instance_index);

   rc = sqlite3_step(pStmt);
   if ( (fc = sqlite3_finalize(pStmt)) != 0 && fc != SQLITE_CONSTRAINT )
      { printf("ERROR: DeletePUFInstance(): Finalize failed %d\n", fc); exit(EXIT_FAILURE); }

   if ( rc == SQLITE_DONE )
      return 0;
   else if ( rc == SQLITE_CONSTRAINT )
      {
      printf("WARNING: DeletePUFInstance(): Instance already exists! Return code => %d\n", rc); fflush(stdout);
      return 1;
      }
   else 
      {
      printf("ERROR: DeletePUFInstance(): Return code => %d\n", rc); exit(EXIT_FAILURE);
      return -1;
      }
   }


// ========================================================================================================
// ========================================================================================================
// Get/create the PUFDesign index and PUFInstance index.

void GetCreatePUFDesignAndInstance(int max_string_len, sqlite3 *db, char *Netlist_name, char *Synthesis_name, 
   char *Chip_name, char *Device_name, char *Placement_name, int *design_index_ptr, int *instance_index_ptr, 
   int num_PIs, int num_POs)
   {
   int existing_num_PIs, existing_num_POs;
   char PUF_response_str[MAX_STRING_LEN];
   char date_str[max_string_len];
   struct tm *tmp;
   time_t t;

printf("\n\nAdding PUFDesign\tNetlist name '%s': Synthesis name '%s'\tChip name '%s'\n", Netlist_name, Synthesis_name, Chip_name); fflush(stdout);

   t = time(NULL);
   tmp = localtime(&t);
   if (tmp == NULL) 
      { printf("localtime FAILED!\n"); exit(EXIT_FAILURE); }
   if ( strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M", tmp) == 0 ) 
      { printf("strftime FAILED!\n"); exit(EXIT_FAILURE); }

printf("\tSetting 'PRAGMA foreign_keys = ON'\n"); fflush(stdout);

// Be sure 'PRAGMA foreign_keys = ON' is set to enable CASCADE mode.
   int fc;
   char *zErrMsg = 0;
   fc = sqlite3_exec(db, "PRAGMA foreign_keys = ON", SQL_callback, 0, &zErrMsg);
   if ( fc != SQLITE_OK )
      { printf("SQL error: %s\n", zErrMsg); sqlite3_free(zErrMsg); }

// Create a PUFDesign entry if it's not already present. 
   if ( GetPUFDesignParams(MAX_STRING_LEN, db, Netlist_name, Synthesis_name, design_index_ptr, &existing_num_PIs, &existing_num_POs) != 0 )
      {
      InsertIntoTable(MAX_STRING_LEN, db, "PUFDesign", SQL_PUFDesign_insert_into_cmd, NULL, 0, Netlist_name, Synthesis_name, NULL, NULL, NULL, 
         num_PIs, num_POs, -1, -1, -1, -1.0, -1.0);

// Get the index for the new PUFDesign
      if ( GetPUFDesignParams(MAX_STRING_LEN, db, Netlist_name, Synthesis_name, design_index_ptr, &existing_num_PIs, &existing_num_POs) != 0 )
         { printf("ERROR: GetCreatePUFDesignAndInstance(): Failed to find '%s' '%s' in PUFDesign Table!\n", Netlist_name, Synthesis_name); exit(EXIT_FAILURE); }
      }

printf("\tPUFDesign index %d\n\n", *design_index_ptr); fflush(stdout);

// Sanity check. Check that the specified values agree with the existing values.
   if ( existing_num_PIs != num_PIs || existing_num_POs != num_POs )
      { 
      printf("ERROR: GetCreatePUFDesignAndInstance(): PUFDesign Netlist '%s', Synthesis '%s' EXISTS and num_PIs and/or num_POs do NOT agree with number specified!\n", 
         Netlist_name, Synthesis_name); exit(EXIT_FAILURE); 
      }

// ----------------------------------
// Check if the PUFInstance already exists. If PUF instance already exists, give user chance to delete it (and ALL timing data) or abort. This call repeated below.
   if ( (*instance_index_ptr = GetIndexFromTable(MAX_STRING_LEN, db, "PUFInstance", SQL_PUFInstance_get_index_cmd, NULL, 0, Chip_name, Device_name, Placement_name, 
      NULL, -1, -1, -1)) != -1 )
      {
      strcpy(PUF_response_str, ""); 

      printf("\tPUF Instance ALREADY exists for '%s', '%s', '%s' with PUFDesign '%s', '%s': Do you want to delete the PUF Instance and all of its Timing Data (Y/N)? ", 
         Chip_name, Device_name, Placement_name, Netlist_name, Synthesis_name); fflush(stdout);
      scanf("%s", PUF_response_str);

// If user specifies 'Y' or 'y', then do it, else any other response abort.
      if ( strcmp(PUF_response_str, "Y") == 0 || strcmp(PUF_response_str, "y") == 0 )
         {

// If the PUF instance exists and the user chooses to overwrite it's data, then delete it and then recreate it. Deleting it will delete its previous timestamp
// and ALL timing data because of the ON UPDATE CASCADE property on the TimingVals table.
         if ( DeletePUFInstance(MAX_STRING_LEN, db, SQL_PUFInstance_delete_cmd, *instance_index_ptr) == -1 )
            { 
            printf("ERROR: GetCreatePUFDesignAndInstance(): Failed to delete '%s', '%s', '%s' in PUFInstance Table!\n", Chip_name, Device_name, Placement_name); 
            sqlite3_close(db);
            exit(EXIT_FAILURE); 
            }
         }
      else
         {
         printf("Aborting operation!\n"); 
         sqlite3_close(db);
         exit(EXIT_SUCCESS);
         }
      }

// Else it does NOT exist OR the user choose to overwrite the data -- create it and get the index for the PUFInstance.
   InsertIntoTable(MAX_STRING_LEN, db, "PUFInstance", SQL_PUFInstance_insert_into_cmd, NULL, 0, Chip_name, Device_name, Placement_name, 
      date_str, NULL, *design_index_ptr, -1, -1, -1, -1, -1.0, -1.0);

   if ( (*instance_index_ptr = GetIndexFromTable(MAX_STRING_LEN, db, "PUFInstance", SQL_PUFInstance_get_index_cmd, NULL, 0, Chip_name, Device_name, Placement_name, 
      NULL, -1, -1, -1)) == -1 )
      {
      printf("ERROR: GetCreatePUFDesignAndInstance(): Failed to find '%s', '%s', '%s' in PUFInstance Table!\n", Chip_name, Device_name, Placement_name); 
      exit(EXIT_FAILURE); 
      }

printf("\n\tPUFInstance index %d\n", *instance_index_ptr); fflush(stdout);
printf("\tNew Chip enroll date '%s'\n\n", date_str);

   return; 
   }


// ========================================================================================================
// ========================================================================================================
// Add all the TimingVals data associated with the vector pair. NOTE: This should never fail because the 
// element already exists because I've set ON DELETE CASCADE on the TimingVals table. The user must choose to 
// delete and overwrite the PUFInstance at the beginning of the processing done in this enrollDB code. If he/she 
// chooses 'Y', the the PUFInstance is deleted and so are all the TimingVals associated with the PUFInstance.

int AddTimingDataToDB(int max_string_len, sqlite3 *db, const char *SQL_TimingVals_insert_into_cmd, int vec_pair_num, 
   int instance_index, int vecpair_index, float *PNX, float *PNX_Tsig, int *rise_fall, int *vec_pairs, 
   int *POs, int num_PNR, int num_PNX, int master_num_rise_vec_pairs, int num_POs)
   {
   int num_PNs_per_vecpair, PN_num;

// 'vec_pair_num' corresponds to the number stored in the enrollment data file under 'V:', which is stored in the 'vec_pairs' 
// array.  Keep adding timing values until the value for an array element in 'vecpairs' is not equal to 'vec_pair_num'.
// First skip forward until we find the first match.
   PN_num = 0;
   while ( PN_num < num_PNX && vec_pair_num != vec_pairs[PN_num] )
      PN_num++;

   num_PNs_per_vecpair = 0;
   while ( PN_num < num_PNX && vec_pair_num == vec_pairs[PN_num] )
      {

// Sanity check. We depend on no negative values in GetVecPairsAllTimingValsFields (commonDB.c).
      if ( POs[PN_num] < 0 || POs[PN_num] >= num_POs )
         { 
         printf("ERROR: AddTimingDataToDB(): PO for vec_pair index %d and timing index %d is %d, which is outside range of 0 to num_POs - 1 %d!\n", 
            vec_pair_num, PN_num, POs[PN_num], num_POs - 1); exit(EXIT_FAILURE); 
         }

// Sanity check. We depend on no negative values in GetVecPairsAllTimingValsFields (commonDB.c).
      if ( PNX[PN_num] < 0.0 || PNX_Tsig[PN_num] < 0.0 )
         { 
         printf("ERROR: AddTimingDataToDB(): Average timing value or threesig for vec_pair index %d and timing index %d is less than 0 => %f and %f!\n", 
            vec_pair_num, PN_num, PNX[PN_num], PNX_Tsig[PN_num]); exit(EXIT_FAILURE); 
         }

// Store the timing value as an element in the TimingTable. NOTE: THIS routine scales the floating point value to a integer (FIXED POINT) by
// multiplying by 16 before storing the data into the TABLE.
      InsertIntoTable(max_string_len, db, "TimingTable", SQL_TimingVals_insert_into_cmd, NULL, 0, NULL, NULL, NULL, NULL, NULL, vecpair_index, 
         POs[PN_num], instance_index, -1, -1, PNX[PN_num], PNX_Tsig[PN_num]);

#ifdef DEBUG
printf("AddTimingDataToDB(): InsertIntoTimingTable return val %d\n", rc);
#endif

// Sanity check
      if ( vec_pair_num < master_num_rise_vec_pairs && rise_fall[PN_num] != 0 )
         { printf("ERROR: AddTimingDataToDB(): Inconsistency between rising and falling for vec_pair_num %d!\n", vec_pair_num); exit(EXIT_FAILURE); }
      if ( vec_pair_num >= master_num_rise_vec_pairs && rise_fall[PN_num] != 1 )
         { printf("ERROR: AddTimingDataToDB(): Inconsistency between rising and falling for vec_pair_num %d!\n", vec_pair_num); exit(EXIT_FAILURE); }

      PN_num++;
      num_PNs_per_vecpair++;
      }

#ifdef DEBUG
printf("AddTimingDataToDB(): Number of timing values added for vec_pair_num %d is %d\n", vec_pair_num, num_PNs_per_vecpair); fflush(stdout);
#endif

   return num_PNs_per_vecpair;
   }


// ========================================================================================================
// ========================================================================================================
// Add master vectors to the Vectors table, and then vector pairs to the VecPairs table, and then the timing 
// data to the TimingVals table. It is common for this routine to issue constraint violations because vectors 
// or vector pairs already exist in the tables. Timing values are deleted and then re-added if they already 
// exist b/c the user must first decide if the PUFInstance is to be preserved or replaced. This is 
// accomplished because a foreign key and ON DELETE CASCADE is set on the TimingVals database to the PUFInstance 
// table.

void ProcessMasterVecsAndTimingData(int max_string_len, sqlite3 *db, int design_index, int instance_index,
   int master_num_vec_pairs, int master_num_rise_vec_pairs, unsigned char **master_first_vecs_b, 
   unsigned char **master_second_vecs_b, float *PNX, float *PNX_Tsig, int *rise_fall, int *vec_pairs, 
   int *POs, int num_PNR, int num_PNX, int num_PIs, int num_POs)
   {
   int vecpair_index, num_PNs_per_vecpair;
   char rise_fall_str[200];
   int vec_num, vec_len_bytes;

   int first_vec_index, second_vec_index;

   int tot_TVs;

   vec_len_bytes = num_PIs/8;

#ifdef DEBUG
struct timeval t1, t2;
long elapsed; 
#endif

printf("ENROLL:\tPUFDesign %d: PUFInstance %d\tFor Num Vectors %d\n", design_index, instance_index, master_num_vec_pairs); fflush(stdout);
#ifdef DEBUG
#endif

// ---------------------------------------------------------
   tot_TVs = 0;
   for ( vec_num = 0; vec_num < master_num_vec_pairs; vec_num++ )
      {
#ifdef DEBUG
gettimeofday(&t2, 0);
#endif

#ifdef DEBUG
printf("\n\nPROCESSING vector %d of %d\n", vec_num, master_num_vec_pairs); fflush(stdout);
#endif

#ifdef DEBUG
printf("SQL Query '%s'\n", SQL_Vectors_insert_into_cmd);
#endif

#ifdef DEBUG
printf("\tWriting 1st vector number %d\n", vec_num);
#endif

// Insert first vector of vector pair into Vectors table if it does already exist. It is common for vectors to already exist
// as enrollment data for new chips is added.
      InsertIntoTable(max_string_len, db, "Vectors", SQL_Vectors_insert_into_cmd, master_first_vecs_b[vec_num], vec_len_bytes, 
         NULL, NULL, NULL, NULL, NULL, -1, -1, -1, -1, -1, -1.0, -1.0);

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
#endif

#ifdef DEBUG
printf("\tWriting 2nd vector number %d\n", vec_num);
#endif

// Insert second vector of vector pair.
      InsertIntoTable(max_string_len, db, "Vectors", SQL_Vectors_insert_into_cmd, master_second_vecs_b[vec_num], vec_len_bytes, 
         NULL, NULL, NULL, NULL, NULL, -1, -1, -1, -1, -1, -1.0, -1.0);

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
#endif

// ------------------------------------------------------------------------------------------------------------
// For each pair of vectors that have been added, add a row to the VecPairs table. First find the vectors in the
// Vectors table (that were either just added or already existed). Since only unique vectors are added, these number 
// can jump around because the calls above may not add a vector (if it is not unique).

#ifdef DEBUG
printf("\nProcessMasterVecsAndTimingData(): Find first vector!\n"); fflush(stdout);
#endif

// Get index for first vector of the vector pair from the Vectors table.
      if ( (first_vec_index = GetIndexFromTable(max_string_len, db, "Vectors", SQL_Vectors_get_index_cmd, master_first_vecs_b[vec_num], 
         vec_len_bytes, NULL, NULL, NULL, NULL, -1, -1, -1)) == -1 )
         { 
         printf("ERROR: ProcessMasterVecsAndTimingData(): Failed to find first_vec_index for vec_num %d in Vectors table!\n", vec_num); 
         exit(EXIT_FAILURE); 
         }

#ifdef DEBUG
printf("Vec %d, First vector index %d\n", vec_num, first_vec_index); fflush(stdout);
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
#endif

#ifdef DEBUG
// Sanity check. Read back the first vector and check it against array values.
      int byte_num;
      unsigned char temp_vec[vec_len_bytes];
      ReadBinaryBlob(db, SQL_Vectors_read_vector_cmd, first_vec_index, temp_vec, vec_len_bytes, 0, NULL);

// printf("Stored and retrieved vector %d => size is %d:\n\t\t", vec_num, vec_len_bytes); fflush(stdout);
      for ( byte_num = 0; byte_num < vec_len_bytes; byte_num++ ) 
         {
         printf("%02X <=> %02X  ", master_first_vecs_b[vec_num][byte_num], temp_vec[byte_num]); 
         if ( (byte_num + 1) % 16 == 0 )
            printf("\n\t\t");
         if ( master_first_vecs_b[vec_num][byte_num] != temp_vec[byte_num] )
            {
            printf("ERROR: ProcessMasterVecsAndTimingData(): Master Vector byte %02X does NOT equal stored vector byte %02X!\n", 
               master_first_vecs_b[vec_num][byte_num], temp_vec[byte_num]); 
            exit(EXIT_FAILURE);
            }
         }
      printf("\n"); fflush(stdout);
#endif

#ifdef DEBUG
printf("\nProcessMasterVecsAndTimingData(): Find second vector!\n"); fflush(stdout);
#endif

// Get index for second vector of the vector pair from the Vectors table.
      if ( (second_vec_index = GetIndexFromTable(max_string_len, db, "Vectors", SQL_Vectors_get_index_cmd, master_second_vecs_b[vec_num], 
         vec_len_bytes, NULL, NULL, NULL, NULL, -1, -1, -1)) == -1 )
         { 
         printf("ERROR: ProcessMasterVecsAndTimingData(): Failed to find second_vec_index for vec_num %d in Vectors table!\n", vec_num); 
         exit(EXIT_FAILURE); 
         }

#ifdef DEBUG
printf("Vec %d, Second vector index %d\n", vec_num, second_vec_index); fflush(stdout);
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
#endif

#ifdef DEBUG
printf("\tWrote binary vectors into Vector Table for %d with 1st vector index %d and 2nd vector index %d\n", vec_num, first_vec_index, second_vec_index); fflush(stdout);
#endif

#ifdef DEBUG
// Sanity check. Read back the second vector and check it against array values.
      ReadBinaryBlob(db, SQL_Vectors_read_vector_cmd, second_vec_index, temp_vec, vec_len_bytes, 0, NULL);

// printf("Stored and retrieved vector %d => size is %d:\n\t\t", vec_num, vec_len_bytes); fflush(stdout);
      for ( byte_num = 0; byte_num < vec_len_bytes; byte_num++ ) 
         {
         printf("%02X <=> %02X  ", master_second_vecs_b[vec_num][byte_num], temp_vec[byte_num]); 
         if ( (byte_num + 1) % 16 == 0 )
            printf("\n\t\t");
         if ( master_second_vecs_b[vec_num][byte_num] != temp_vec[byte_num] )
            {
            printf("ERROR: ProcessMasterVecsAndTimingData(): Master Vector byte %02X does NOT equal stored vector byte %02X!\n", 
               master_second_vecs_b[vec_num][byte_num], temp_vec[byte_num]); 
            exit(EXIT_FAILURE);
            }
         }
      printf("\n"); fflush(stdout);
#endif

// We automatically detected when reading vectors and mask files which vectors are rising and counted them (and also made sure they
// were listed as the first set of vectors in the file). This number is passed in here as 'master_num_rise_vec_pairs'. We record in the
// VecPairs table whether the vector is rising or falling.
      if ( vec_num < master_num_rise_vec_pairs )
         strcpy(rise_fall_str, "R");
      else
         strcpy(rise_fall_str, "F");

#ifdef DEBUG
printf("Creating/Adding VecPair for vec_num %d, R/F %s, 1st vec index %d, 2nd vec index %d and design index %d!\n",
   vec_num, rise_fall_str, first_vec_index, second_vec_index, design_index); fflush(stdout);
#endif

// Add a vector pair to VecPair table. NOTE: a 'unique' index is setup on VecPair that uses 'first_vec_index', 'second_vec_index' and
// 'design_index', so if the VecPair already exists, it is NOT added again. ALSO NOTE: We will fill in NumPNs after we add and count the 
// timing values below.
      InsertIntoTable(max_string_len, db, "VecPairs", SQL_VecPairs_insert_into_cmd, NULL, 0, rise_fall_str, NULL, NULL, NULL, NULL, first_vec_index, 
         second_vec_index, -1, design_index, -1, -1.0, -1.0);

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
printf("Getting index from VecPair Table\n"); fflush(stdout);
#endif

// Get the index for this newly added VecPair.
      if ( (vecpair_index = GetIndexFromTable(max_string_len, db, "VecPairs", SQL_VecPairs_get_index_cmd, NULL, 0, NULL, NULL, NULL, NULL, 
         first_vec_index, second_vec_index, design_index)) == -1 )
         {
         printf("ERROR: ProcessMasterVecsAndTimingData(): Failed to find vecpair_index for vec_num %d in VecPairs table!\n", vec_num); 
         exit(EXIT_FAILURE); 
         }

#ifdef DEBUG
printf("\tWrote VecPair with index %d\n", vecpair_index); fflush(stdout);
#endif

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
printf("Saving timing data to TimingVals table\n"); fflush(stdout);
#endif

// Add the timing data for this PUFInstance and VecPair. Unlike above, this should never find that the TimingVal exists. 
      num_PNs_per_vecpair = AddTimingDataToDB(max_string_len, db, SQL_TimingVals_insert_into_cmd, vec_num, instance_index, vecpair_index, 
         PNX, PNX_Tsig, rise_fall, vec_pairs, POs, num_PNR, num_PNX, master_num_rise_vec_pairs, num_POs);

// Update NumPNs field in the VecPairs record with 'num_PNs_per_vecpair'.
      UpdateVecPairsNumPNsField(max_string_len, db, num_PNs_per_vecpair, vecpair_index);

#ifdef DEBUG
printf("\tWrote TimingVals with %d elements\n", num_PNs_per_vecpair); fflush(stdout);
#endif
      tot_TVs += num_PNs_per_vecpair;

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed);
fflush(stdout);
#endif
      }

printf("\tWrote Total TimingVals %d\n", tot_TVs); fflush(stdout);
#ifdef DEBUG
#endif

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Check that the PathSelectMask that is about to be added to the database, that is associated with the VecPair,
// is 'consistent' with the TimingVals database. The mask can have '0', '1', 'u' and 'q' designators. '0' means 
// output has no transition, '1' means path has transition, 'u' (unqualifying) means the path has a transition 
// but is not 'compatible' with the elements that are marked 'q' (for qualifying).

void CheckMaskIsConsistentWithVecPairTimingVals(int max_string_len, sqlite3 *db, int vecpair_index, 
   char *challenge_mask, int num_POs)
   {
   char first_PO_string_from_timing_data[num_POs + 1]; 
   char PO_string_from_timing_data[num_POs + 1]; 
   int PUF_instance_num, PUF_instance_index;
   SQLIntStruct PUF_instance_index_struct; 
   char sql_command_str[max_string_len];
   SQLIntStruct PO_nums_struct;
   int PO_num;

// Note that the Timing values table has both PUFInstance and VecPair fields, i.e., the timing values for any given
// vecpair_index are repeated for each PUF_instance_index. We check the TimingVals for each PUF instance in this
// routine. Get a list of PUFInstance 'ids' from the PUFInstance table.
//   strcpy(sql_command_str, "SELECT ID FROM PUFInstance;");
//   GetAllocateListOfInts(max_string_len, db, sql_command_str, &PUF_instance_index_struct);
   GetPUFInstanceIDsForInstanceName(max_string_len, db, &PUF_instance_index_struct, "%");

#ifdef DEBUG
for ( PUF_instance_num = 0; PUF_instance_num < PUF_instance_index_struct.num_ints; PUF_instance_num++ )
   printf("CheckMaskIsConsistentWithVecPairTimingVals(): PUF_instance_index %d\n", PUF_instance_index_struct.int_arr[PUF_instance_num]); fflush(stdout);
#endif

// Sanity check. Must be at least one enrolled PUFInstance.
   if ( PUF_instance_index_struct.num_ints == 0 )
      { printf("ERROR: CheckMaskIsConsistentWithVecPairTimingVals(): No PUFInstances found -- MUST be at least one!\n"); exit(EXIT_FAILURE); }

// Check each PUFinstance
   for ( PUF_instance_num = 0; PUF_instance_num < PUF_instance_index_struct.num_ints; PUF_instance_num++ )
      {

// This is the 'id' field of the PUFInstance array -- is it always 1 to n?
      PUF_instance_index = PUF_instance_index_struct.int_arr[PUF_instance_num]; 

// Parse the TimingVals database elements associated with the VecPair index, collecting the PO information as a set
// of integers. The PO_arr is dynamically allocated so be sure to free it here after we are done.
      sprintf(sql_command_str, "SELECT PO FROM TimingVals WHERE PUFInstance = %d AND VecPair = %d;", PUF_instance_index, vecpair_index);
      GetAllocateListOfInts(max_string_len, db, sql_command_str, &PO_nums_struct);

// Initialize the mask string constructed with PO numbers.
//      for ( PO_num = 0; PO_num < num_POs; PO_num++ )
//         PO_string_from_timing_data[PO_num] = '0';
      memset(PO_string_from_timing_data, '0', num_POs);

      PO_string_from_timing_data[num_POs] = '\0';

// Add '1's to the proper position. If we have a functional unit with 4 outputs, then finding a PO in the TimingVals database numbered 
// 2 would update the string in the following fashion "0100". So string is constructed with the PO 0 position on the RIGHT. This makes 
// it consistent with the challenge_mask order. 
      for ( PO_num = 0; PO_num < PO_nums_struct.num_ints; PO_num++ )
         {

#ifdef DEBUG
printf("PO %d has transition and has a TimingVals entry, and is associated with (PUF_instance_index %d, vecpair_index %d)\n", 
   PO_nums_struct.int_arr[PO_num], PUF_instance_index, vecpair_index); fflush(stdout);
#endif

         PO_string_from_timing_data[num_POs - PO_nums_struct.int_arr[PO_num] - 1] = '1';
         }

#ifdef DEBUG
printf("(PUF_instance_index %d, vecpair_index %d) \n\tPO-con '%s'\n\tChlngM '%s'\n", PUF_instance_index, vecpair_index, 
   PO_string_from_timing_data, challenge_mask); fflush(stdout);
#endif

// Now make sure the PO-constructed string is consistent with the mask, taking into account that the mask can contain '1', 'u' or 'q'
// for '1' in the PO-constructed string. NOTE: We only need to make this comparison with the first PUFInstance we process. Afterwards,
// just make sure ALL PUFInstance data has the same PO-constructed mask.
      if ( PUF_instance_num == 0 )
         {
         ComparePOGenMaskWithChallengeMask(max_string_len, db, PUF_instance_index, vecpair_index, PO_string_from_timing_data, challenge_mask, 
            num_POs);

// Save PO-generated mask for first PUFInstance for comparisons with other PUFInstances.
         strcpy(first_PO_string_from_timing_data, PO_string_from_timing_data);
         }
      else if ( strcmp(first_PO_string_from_timing_data, PO_string_from_timing_data) != 0 )
         { 
         printf("ERROR: CheckMaskIsConsistentWithVecPairTimingVals(): TimingVals data incorrect for PUFInstances %d for vecpair index %d\n", 
            PUF_instance_index, vecpair_index); exit(EXIT_FAILURE); 
         }

// Free the int array of PO numbers allocated by GetAllocListOfInts.
      free(PO_nums_struct.int_arr);
      }

// Free up the array of PUFInstance ids.
   free(PUF_instance_index_struct.int_arr); 

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Reads in the vector file. Format is one configuration vector per line. Space for first_vecs and second_vecs is 
// allocated in the caller. NOTE: We split the configuration vector into two pieces and store it as a vector
// pair. *** VEC_CHLNG ***
//
// (NOTE: ../COMMON/verifier_common.c defines another very similar version that reads BINARY version of the mask 

int ReadVectorAndASCIIMaskFiles(int max_string_len, char *vec_path, int *num_rise_vecs_ptr, unsigned char ***first_vecs_b_ptr, 
   unsigned char ***second_vecs_b_ptr, int has_masks, char *mask_file_path, char ***masks_asc_ptr, int num_PIs, int num_POs,
   int rise_fall_bit_pos)
   {
// Sanity checks. *** VEC_CHLNG ***
   char first_vec[num_PIs+1], second_vec[num_PIs+1];
   char line[max_string_len], *char_ptr;
   unsigned char *vec_ptr;
   int rise_or_fall;
   int vec_cnt;

   FILE *INFILE;

   if ( (INFILE = fopen(vec_path, "r")) == NULL )
      { printf("ERROR: ReadVectorAndASCIIMaskFiles(): Could not open Vector file %s\n", vec_path); exit(EXIT_FAILURE); }

// Sanity check. Make sure 'num_PIs' is evenly divisible by 16 otherwise the binary conversion routine below will NOT work.
// *** VEC_CHLNG ***
   if ( (num_PIs % 8) != 0 )
      { printf("ERROR: ReadVectorAndASCIIMaskFiles(): 'num_PIs' %d MUST be divisible by 8!\n", num_PIs); exit(EXIT_FAILURE); }

   if ( has_masks == 1 && (num_POs % 8) != 0 )
      { printf("ERROR: ReadVectorAndASCIIMaskFiles(): 'num_POs' %d MUST be divisible by 8!\n", num_POs); exit(EXIT_FAILURE); }

// Force realloc to behave like malloc. DO NOT DO 'masks_asc_ptr' here because we set it to NULL sometimes in the caller. Done below.
   *first_vecs_b_ptr = NULL;
   *second_vecs_b_ptr = NULL;
   *masks_asc_ptr = NULL;

   vec_cnt = 0;
   *num_rise_vecs_ptr = 0;
   rise_or_fall = -1;

// max_string_len is set to 2048, big enough to handle the 784 ASCII chars for the challenges.
   while ( fgets(line, max_string_len, INFILE) != NULL )
      {

// Find the newline and eliminate it.
      if ((char_ptr = strrchr(line, '\n')) != NULL)
         *char_ptr = '\0';

// Sanity checks. *** VEC_CHLNG ***
      if ( strlen(line) != num_PIs*2 )
         { printf("ERROR: ReadVectorAndASCIIMaskFiles(): Unexpected Chlng size %d -- expected %d!\n", (int)strlen(line), num_PIs*2); exit(EXIT_FAILURE); }

// Sanity checks. *** VEC_CHLNG ***
      if ( (strlen(line) % 16) != 0 )
         { printf("ERROR: ReadVectorAndASCIIMaskFiles(): Chlng size %d MUST be divisable by 2*8 = 16!\n", (int)strlen(line)); exit(EXIT_FAILURE); }

// Copy the first half of the vector into the first_vec and the second half into the second_vec. Need to explicity NULL terminate. 
// *** VEC_CHLNG ***
      strncpy(first_vec, line, num_PIs);
      strncpy(second_vec, line + num_PIs, num_PIs);
      first_vec[num_PIs] = '\0';
      second_vec[num_PIs] = '\0';

#ifdef DEBUG
printf("First Len %u\tVec '%s'\n", (unsigned int)strlen(first_vec), first_vec);
printf("Second Len %u\tVec '%s'\n", (unsigned int)strlen(second_vec), second_vec);
fflush(stdout);
#endif

// Determine if vector is rising or falling. Use the 15th bit from the left side to determine transition direction. All rising vectors must preceed all falling vectors. 
      if ( first_vec[rise_fall_bit_pos] == '1' )
         {

// Check that we are not intermixing rising and falling vectors. If this is ever 1, then we processed a falling vector on a previous iteration,
// which is an error. This requires rising vectors to preceed falling vectors.
         if ( rise_or_fall == 1 )
            { printf("ERROR: ReadVectorAndASCIIMaskFiles(): Intermixing of rising and falling vectors illegal!\n"); exit(EXIT_FAILURE); }
         rise_or_fall = 0;

// Add 1 to rising vectors 
         (*num_rise_vecs_ptr)++;
         }

// Same checks for falling transitions
      else
         {

// Indicate that we are processing falling vectors.
         if ( rise_or_fall == -1 )
            { printf("ERROR: ReadVectorAndASCIIMaskFiles(): Expected rising vectors to be first in vector file!\n"); exit(EXIT_FAILURE); }
         rise_or_fall = 1;
         }

#ifdef DEBUG
printf("ReadVectorAndASCIIMaskFiles(): BEFORE ALLOCATION!\n"); fflush(stdout);
#endif

// =============================================
// Convert from ASCII to binary. Allocate space for binary unsigned char version of vectors. 
      if ( (*first_vecs_b_ptr = (unsigned char **)realloc(*first_vecs_b_ptr, sizeof(unsigned char *)*(vec_cnt + 1))) == NULL )
         { printf("ERROR: ReadVectorAndASCIIMaskFiles(): Failed to reallocate storage for first_vecs_b_ptr array!\n"); exit(EXIT_FAILURE); }
      if ( ((*first_vecs_b_ptr)[vec_cnt] = (unsigned char *)malloc(sizeof(unsigned char)*num_PIs/8)) == NULL )
         { printf("ERROR: ReadVectorAndASCIIMaskFiles(): Failed to allocate storage for first_vecs_b_ptr string!\n"); exit(EXIT_FAILURE); }
      if ( (*second_vecs_b_ptr = (unsigned char **)realloc(*second_vecs_b_ptr, sizeof(unsigned char *)*(vec_cnt + 1))) == NULL )
         { printf("ERROR: ReadVectorAndASCIIMaskFiles(): Failed to reallocate storage for second_vecs_b_ptr array!\n"); exit(EXIT_FAILURE); }
      if ( ((*second_vecs_b_ptr)[vec_cnt] = (unsigned char *)malloc(sizeof(unsigned char)*num_PIs/8)) == NULL )
         { printf("ERROR: ReadVectorAndASCIIMaskFiles(): Failed to allocate storage for second_vecs_b_ptr string!\n"); exit(EXIT_FAILURE); }

// Convert ASCII version of vector to binary, reversing the order as appropriate for the hardware that uses these vectors.
// We MUST reverse the first and second vecs TOO since these two will be concatenated to define 1 configuration vector and
// MUST BE reversed in order from high to low! NOTE: 'second_vec' is used to load 'first_vecs_b_ptr'
// *** VEC_CHLNG ***
      vec_ptr = (*first_vecs_b_ptr)[vec_cnt];
      ConvertASCIIVecMaskToBinary(num_PIs, second_vec, vec_ptr);
      vec_ptr = (*second_vecs_b_ptr)[vec_cnt];
      ConvertASCIIVecMaskToBinary(num_PIs, first_vec, vec_ptr);

#ifdef DEBUG
printf("ReadVectorAndASCIIMaskFiles(): Binary vec for vec_cnt %d\n\t", vec_cnt); fflush(stdout);
#endif

      vec_cnt++;
      }
   fclose(INFILE);

// Read in the mask file if requested.
   if ( has_masks == 1 )
      {
      int mask_vec_num;
      if ( (INFILE = fopen(mask_file_path, "r")) == NULL )
         { printf("ERROR: ReadVectorAndASCIIMaskFiles(): Could not open Mask file %s\n", mask_file_path); exit(EXIT_FAILURE); }

// Force realloc to behave like malloc.
      *masks_asc_ptr = NULL;

// We MUST find a mask for every vector read above.
      for ( mask_vec_num = 0; mask_vec_num < vec_cnt; mask_vec_num++ )
         {

// Sanity check
         if ( fgets(line, max_string_len, INFILE) == NULL )
            { printf("ERROR: ReadVectorAndASCIIMaskFiles(): A mask file is requested but number of masks is less than the number of vectors!\n"); exit(EXIT_FAILURE); }

// Find the newline and eliminate it.
         if ((char_ptr = strrchr(line, '\n')) != NULL)
            *char_ptr = '\0';

// Sanity check
         if ( strlen(line) != num_POs )
            { printf("ERROR: ReadVectorAndASCIIMaskFiles(): Mask line expected to have %d bits, found => %d!\n", num_POs, (int)strlen(line)); exit(EXIT_FAILURE); }

// Allocate space for the mask. NOTE: WE DO NOT PACK mask bits into unsigned char because mask bits can be 0, 1, u, or q. Add 1 character so we can NULL terminate
// and treat the masks as strings.
         if ( (*masks_asc_ptr = (char **)realloc(*masks_asc_ptr, sizeof(unsigned char *)*(mask_vec_num + 1))) == NULL )
            { printf("ERROR: ReadVectorAndASCIIMaskFiles(): Failed to reallocate storage for mask_ptr array!\n"); exit(EXIT_FAILURE); }
         if ( ((*masks_asc_ptr)[mask_vec_num] = (char *)malloc(sizeof(char)*(num_POs + 1))) == NULL )
            { printf("ERROR: ReadVectorAndASCIIMaskFiles(): Failed to allocate storage for mask_ptr string!\n"); exit(EXIT_FAILURE); }

// DO NOT reverse the order of ASCII file masks in ASCII array because when we print it, we want to see it as it is shown in the
// ASCII file, i.e., from high-order-to-low-order. Later when we fetch this ASCII version and send to the chip as a unsigned binary
// string, we'll reverse it.
         strcpy((*masks_asc_ptr)[mask_vec_num], line);
         }

// Sanity check. Mask files do NOT have an extra empty line at the end of the file. If more lines exist, something is wrong. Probably
// empty line at end of vector file is missing (in which case vec_cnt is 1 less than it should be).
      if ( fgets(line, max_string_len, INFILE) != NULL )
         { printf("ERROR: ReadVectorAndASCIIMaskFiles(): Mask file has MORE masks then vector file has vectors! Is empty line present at end of vector file?!\n"); exit(EXIT_FAILURE); }

      fclose(INFILE);
      }

   return vec_cnt;
   }


// ===========================================================================================================
// ===========================================================================================================
// Find qualifying paths from the 'q' in the masks associated with the Challenge. The return PathInfo struct 
// and 'xxx_qualified_PNs' indicates how many we found. 

void FindQualifyingPaths(int max_string_len, sqlite3 *db, PathInfoStruct **tested_path_info_ptr, 
   int num_rising_vecpairs, int num_falling_vecpairs, int *num_rise_tested_PNs_ptr, int *num_fall_tested_PNs_ptr, 
   int num_POs, int num_rise_required_PNs, int num_fall_required_PNs, int num_rise_qualified_PNs_expected, 
   int num_fall_qualified_PNs_expected, PathInfoStruct **qualified_path_info_ptr, int challenge_index, 
   int **vecpair_ids_ptr)
   {
   int PO_num, PN_tested_num, num_tested_PNs, PN_num_qualified; 
   SQLIntStruct challenge_vecpair_index_struct; 
   char sql_command_str[max_string_len];
   int cvp_num;

   SQLRowStringsStruct row_strings_struct;
   char *CVP_PSM_name = "PSM";
   char *CVP_VecPair_name = "VecPair";
   char *PSM_name = "vector_str";

   char PSM_mask[num_POs + 1];
   int PSM_index;

   int num_rise_qualified_PNs;
   int num_fall_qualified_PNs;

// Initialize the total number of tested PNs variables. These are not really used...
   *num_rise_tested_PNs_ptr = 0;
   *num_fall_tested_PNs_ptr = 0;

// Mask computed number of qualified PN -- need to match expected number specified as parameters.
   num_rise_qualified_PNs = 0;;
   num_fall_qualified_PNs = 0;

// Masks of the form, with 'u' meaning path has transition but did NOT qualify, 'q' meaning qualified path, '1'
// meaning must include and '0' no transition. Be sure to assign path number from right-to-left since that's the 
// way the hardware collects the data.
// uuuquuuuuuuuuuuuuquuuquu0q0qq0u0u0q00q0uuquuuuuuuquu0uuu00000000

// The ChallengeVecPairs Table has a set of records associated with the challenge_index (added by add_challengeDB.c).
// This call will return the indexes of these database records (hopefully in the same order they were added since we
// must process rising vectors BEFORE falling vectors). This list of ChallengeVecPairs is the starting point for
// the random generation of a challenge.
   sprintf(sql_command_str, "SELECT id FROM ChallengeVecPairs WHERE Chlng = %d;", challenge_index);
   GetAllocateListOfInts(max_string_len, db, sql_command_str, &challenge_vecpair_index_struct);

// Allocate storage for the VecPair ids (to be used later to identify the vectors associated with this newly constructed challenge).
   if ( (*vecpair_ids_ptr = (int *)malloc(sizeof(int) * challenge_vecpair_index_struct.num_ints)) == NULL )
      { printf("ERROR: FindQualifyingPaths(): Failed to allocate storage for 'vecpair_ids_ptr'\n"); exit(EXIT_FAILURE); }

   num_tested_PNs = 0;

// Force realloc to behave like malloc on the first call.
   *tested_path_info_ptr = NULL;

// Parse each ChallengeVecPair record and get the VecPair and PathSelectMask it refers to.
   for ( cvp_num = 0; cvp_num < challenge_vecpair_index_struct.num_ints; cvp_num++ )
      {

#ifdef DEBUG
printf("\tFindQualifyingPaths(): ChallengeVecPair index %d\n", challenge_vecpair_index_struct.int_arr[cvp_num]); fflush(stdout);
#endif

// Get VecPair field from ChallengeVecPairs. We will use this later to identify the vectors for the new challenge constructed using this routine.
      sprintf(sql_command_str, "SELECT %s FROM ChallengeVecPairs WHERE id = %d;", CVP_VecPair_name, challenge_vecpair_index_struct.int_arr[cvp_num]);
      GetStringsDataForRow(max_string_len, db, sql_command_str, &row_strings_struct);
      GetRowResultInt(&row_strings_struct, "FindQualifyingPaths()", 1, 0, CVP_VecPair_name, &((*vecpair_ids_ptr)[cvp_num]));
      FreeStringsDataForRow(&row_strings_struct);

// Get PSM field from ChallengeVecPairs.
      sprintf(sql_command_str, "SELECT %s FROM ChallengeVecPairs WHERE id = %d;", CVP_PSM_name, challenge_vecpair_index_struct.int_arr[cvp_num]);
      GetStringsDataForRow(max_string_len, db, sql_command_str, &row_strings_struct);
      GetRowResultInt(&row_strings_struct, "FindQualifyingPaths()", 1, 0, CVP_PSM_name, &PSM_index);
      FreeStringsDataForRow(&row_strings_struct);

// Get vector field from PathSelectMasks using the key stored in the ChallengeVecPair, which is a string of the form shown above. 
// Should only ever be one match because we store the id field from PhaseSelectMasks in the PSM field of the ChallengeVecPair table.
      sprintf(sql_command_str, "SELECT %s FROM PathSelectMasks WHERE id = %d;", PSM_name, PSM_index);
      row_strings_struct.num_cols = 0;
      GetStringsDataForRow(max_string_len, db, sql_command_str, &row_strings_struct);
      GetRowResultString(&row_strings_struct, "FindQualifyingPaths()", 1, 0, PSM_name, num_POs, PSM_mask);
      FreeStringsDataForRow(&row_strings_struct);

#ifdef DEBUG
printf("\tFindQualifyingPaths(): PSM_mask '%s' for PathSelectMask index %d\n\n", PSM_mask, PSM_index); fflush(stdout);
#endif

// Read it from right-to-left (in ASCII file, largest address is low order bit) so that path information is stored in the order in which it 
// is collected by the hardware experiments.
      for ( PO_num = num_POs - 1; PO_num >= 0; PO_num-- )
         {

// If PO is marked with a 'q' or 'u' than it has a transition -- store path information.
         if ( PSM_mask[PO_num] == 'q' || PSM_mask[PO_num] == 'u' || PSM_mask[PO_num] == '1' )
            {

// Allocate storage for the PathInfo structure.
            if ( (*tested_path_info_ptr = (PathInfoStruct *)realloc(*tested_path_info_ptr, sizeof(PathInfoStruct) * (num_tested_PNs + 1))) == NULL )
               { printf("ERROR: FindQualifyingPaths(): Failed to reallocate storage for PathInfo array!\n"); exit(EXIT_FAILURE); }

// Zero out the malloced structure. This is IMPORTANT because some fields of the structure are NOT initialized here but MUST be assigned zero initially.
            memset(&((*tested_path_info_ptr)[num_tested_PNs]), 0, sizeof(PathInfoStruct));

            (*tested_path_info_ptr)[num_tested_PNs].path_num = num_tested_PNs;
            (*tested_path_info_ptr)[num_tested_PNs].vecpair_num = cvp_num;

// Store this so that we can create masks later to select specific paths to use for each vector. 
            (*tested_path_info_ptr)[num_tested_PNs].PO_num = num_POs - PO_num - 1;

            if ( (*tested_path_info_ptr)[num_tested_PNs].vecpair_num < num_rising_vecpairs )
               (*tested_path_info_ptr)[num_tested_PNs].rise_or_fall = 0;
            else
               (*tested_path_info_ptr)[num_tested_PNs].rise_or_fall = 1;

// Increment the number of tested paths (only some of which are qualified).
            if ( (*tested_path_info_ptr)[num_tested_PNs].vecpair_num < num_rising_vecpairs )
               (*num_rise_tested_PNs_ptr)++; 
            else
               (*num_fall_tested_PNs_ptr)++; 

// Indicate whether the path qualifies.
            if ( PSM_mask[PO_num] == 'q' || PSM_mask[PO_num] == '1' )
               {
               (*tested_path_info_ptr)[num_tested_PNs].path_qualifies = 1;

// The number of rising PNs is bounded by the number of rising vectors
               if ( (*tested_path_info_ptr)[num_tested_PNs].vecpair_num < num_rising_vecpairs )
                  num_rise_qualified_PNs++;
               else
                  num_fall_qualified_PNs++;
               }
            else
               (*tested_path_info_ptr)[num_tested_PNs].path_qualifies = 0;
            num_tested_PNs++;
            }
         }
      }

   free(challenge_vecpair_index_struct.int_arr); 

// Sanity checks
   if ( cvp_num != num_rising_vecpairs + num_falling_vecpairs )
      { printf("ERROR: FindQualifyingPaths(): Expected number of vecpairs %d to be %d!\n", cvp_num, num_rising_vecpairs + num_falling_vecpairs); exit(EXIT_FAILURE); }

   if ( num_rise_qualified_PNs + num_fall_qualified_PNs != num_rise_qualified_PNs_expected + num_fall_qualified_PNs_expected )
      { 
      printf("ERROR: FindQualifyingPaths(): Expected total number of qualified paths to be %d => read %d!\n", 
         num_rise_qualified_PNs_expected + num_fall_qualified_PNs_expected, num_rise_qualified_PNs + num_fall_qualified_PNs);
      exit(EXIT_FAILURE); 
      }

   if ( num_rise_qualified_PNs < num_rise_required_PNs || num_fall_qualified_PNs < num_fall_required_PNs )
      { 
      printf("ERROR: FindQualifyingPaths(): Number of required rise %d or fall %d is less than the required number for HELP %d and %d!\n", 
         num_rise_qualified_PNs, num_fall_qualified_PNs, num_rise_required_PNs, num_fall_required_PNs); exit(EXIT_FAILURE); 
      }

#ifdef DEBUG
printf("\tNumber of vecpairs processed %d\tNumber of tested PNs found %d\tNumber of qualified rise PN %d and fall PN %d (ALWAYS greater than number needed %d and %d)\n", 
   cvp_num, num_tested_PNs, num_rise_qualified_PNs, num_fall_qualified_PNs, num_rise_required_PNs, num_fall_required_PNs); fflush(stdout);
#endif

// Force realloc to behave as malloc.
   *qualified_path_info_ptr = NULL;

// Copy the PathInfo structures that 'qualify' into a QualifiedPathInfo structure for random selection on return.
   PN_num_qualified = 0;
   for ( PN_tested_num = 0; PN_tested_num < num_tested_PNs; PN_tested_num++ )
      if ( (*tested_path_info_ptr)[PN_tested_num].path_qualifies == 1 )
         {
         if ( (*qualified_path_info_ptr = (PathInfoStruct *)realloc(*qualified_path_info_ptr, sizeof(PathInfoStruct) * (PN_num_qualified + 1))) == NULL )
            { printf("ERROR: FindQualifyingPaths(): Failed to reallocate storage for PathInfo array!\n"); exit(EXIT_FAILURE); }

// Note that not needed here because of the assignment but use 'memset(&((*qualified_path_info_ptr)[PN_num_qualified]), 0, sizeof(PathInfoStruct));' if needed to zero.
         (*qualified_path_info_ptr)[PN_num_qualified] = (*tested_path_info_ptr)[PN_tested_num];
         PN_num_qualified++;
         }

// Sanity check
   if ( PN_num_qualified != num_rise_qualified_PNs + num_fall_qualified_PNs )
      { printf("ERROR: FindQualifyingPaths(): Expected number of qualified paths to be %d!\n", PN_num_qualified); exit(EXIT_FAILURE); }

#ifdef DEBUG
printf("\n\tTested PN %d\tWith %d qualified rise paths and %d qualified fall paths\tTotal Qualified %d\n", num_tested_PNs, num_rise_qualified_PNs, 
   num_fall_qualified_PNs, num_rise_qualified_PNs + num_fall_qualified_PNs); 
fflush(stdout);
#endif

   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// ===========================================================================================================
// PathInfo compare function for C lib qsort function -- to get back to the default order. Return -x if ele1 < ele2, 
// 0 if ele1 == ele2 and +x if ele1 > ele2. TVNoise major sort and reverse sort on WithinDie

int PathInfoPathNumCompareFunc(const void *v1, const void *v2)
   {
   return (*(PathInfoStruct *)v1).path_num - (*(PathInfoStruct *)v2).path_num; 
   }


// ===========================================================================================================
// ===========================================================================================================
// PathInfo compare function for C lib qsort function -- to get back to the default order. Return -x if ele1 < ele2, 
// 0 if ele1 == ele2 and +x if ele1 > ele2. TVNoise major sort and reverse sort on WithinDie

int PathInfoVecPairNumCompareFunc(const void *v1, const void *v2)
   {
   return (*(PathInfoStruct *)v1).vecpair_num - (*(PathInfoStruct *)v2).vecpair_num; 
   }


// ===========================================================================================================
// ===========================================================================================================
// PathInfo compare function for C lib qsort function -- to get back to the default order. Return -x if ele1 < ele2, 
// 0 if ele1 == ele2 and +x if ele1 > ele2. TVNoise major sort and reverse sort on WithinDie. Used in
// GenMaskFromQualifyingPaths.c

int PathInfoRandomOrderNumberCompareFunc(const void *v1, const void *v2)
   {
   return (*(PathInfoStruct *)v1).random_order_number - (*(PathInfoStruct *)v2).random_order_number; 
   }


// ============================================================================================================================
// ============================================================================================================================
// Original brute force method that randomly chooses PN without regard to how many vecpairs are needed to generate those qualified
// PN. When the set of qualified PN is only slightly larger than the number needed, e.g., 3500 qualified and need 2048, then
// this algorithm can produce a smaller set of required vecpairs then OptVec above.

void SelectRandomBruteForce(int max_string_len, int num_rise_qualified_PNs, int num_fall_qualified_PNs, 
   PathInfoStruct *qualified_path_info, int num_rise_required_PNs, int num_fall_required_PNs, int *rise_indexes, int *fall_indexes, 
   int num_rising_vecpairs, int num_falling_vecpairs, int *bruteforce_num_rise_vecpairs_ptr, int *bruteforce_num_fall_vecpairs_ptr)
   {
   int selected_falling_vectors[num_falling_vecpairs];
   int selected_rising_vectors[num_rising_vecpairs];
   int num_selected_falling_vectors;
   int num_selected_rising_vectors;
//   int vector_savings;
   int PN_num, i, j;
   int temp_rand;

// Randomly select a set of rising PN qualified_path_info structures of size 'num_rise_required_PNs'. 
// These used by the caller to decide if optvec or the brute_force method produces smaller set of vectors.
   *bruteforce_num_rise_vecpairs_ptr = 0;
   *bruteforce_num_fall_vecpairs_ptr = 0;

// ======================================================================================================================
// DO NOT SORT IN THESE ROUTINES. Current sort order of qualified_path_info should be on vecpair_num. Assume that ALL qualified 
// rise PNs preceed ALL qualified fall PNs in qualified_path_info structure (in other words, the rising vectors preceed the 
// falling vectors). Sanity check is done below.
//   srand(seed);
   PN_num = 0;
   num_selected_rising_vectors = 0;
   while ( PN_num < num_rise_required_PNs )
      {
      temp_rand = rand() % num_rise_qualified_PNs;

// Sanity check
      if ( temp_rand >= num_rise_qualified_PNs + num_fall_qualified_PNs )
         { printf("PROGRAM ERROR: SelectRandomBruteForce(): temp_rand %d >= %d!\n", temp_rand, num_rise_qualified_PNs + num_fall_qualified_PNs); exit(EXIT_FAILURE); }

// Check to make sure this index (path) is NOT already selected.
      for ( i = 0; i < PN_num; i++ )
         if ( rise_indexes[i] == temp_rand )
            break;

// If the list is empty or we did NOT find temp_rand, assign it to the array and increment.
      if ( i == PN_num )
         {

// Check if this vector is already saved in the array.
         for ( j = 0; j < num_selected_rising_vectors; j++ )
            if ( selected_rising_vectors[j] == qualified_path_info[temp_rand].vecpair_num )
               break;

// If not found, add it to the array and increment the counter.
         if ( j == num_selected_rising_vectors )
            {
            selected_rising_vectors[j] = qualified_path_info[temp_rand].vecpair_num;
            num_selected_rising_vectors++;
            }

#ifdef DEBUG
printf("Rise random index choosen %d (Max is %d)\n", temp_rand, num_rise_qualified_PNs - 1);
#endif
         rise_indexes[PN_num] = temp_rand;
         PN_num++;
         }
      }

   PN_num = 0;
   num_selected_falling_vectors = 0;
   while ( PN_num < num_fall_required_PNs )
      {
      temp_rand = (rand() % num_fall_qualified_PNs) + num_rise_qualified_PNs;

// Sanity check
      if ( temp_rand >= num_rise_qualified_PNs + num_fall_qualified_PNs )
         { printf("PROGRAM ERROR: SelectRandomBruteForce(): temp_rand %d >= %d!\n", temp_rand, num_rise_qualified_PNs + num_fall_qualified_PNs); exit(EXIT_FAILURE); }

// Check to make sure this index (path) is NOT already selected.
      for ( i = 0; i < PN_num; i++ )
         if ( fall_indexes[i] == temp_rand )
            break;

// If the list is empty or we did NOT find temp_rand, assign it to the array and increment.
      if ( i == PN_num )
         {

// Check if this vector is already saved in the array.
         for ( j = 0; j < num_selected_falling_vectors; j++ )
            if ( selected_falling_vectors[j] == qualified_path_info[temp_rand].vecpair_num )
               break;

// If not found, add it to the array and increment the counter.
         if ( j == num_selected_falling_vectors )
            {
            selected_falling_vectors[j] = qualified_path_info[temp_rand].vecpair_num;
            num_selected_falling_vectors++;
            }

#ifdef DEBUG
printf("Fall random index choosen %d (Min is %d and Max is %d)\n", temp_rand, num_rise_qualified_PNs, num_rise_qualified_PNs + num_fall_qualified_PNs - 1);
#endif
         fall_indexes[PN_num] = temp_rand;
         PN_num++;
         }
      }

   *bruteforce_num_rise_vecpairs_ptr = num_selected_rising_vectors;
   *bruteforce_num_fall_vecpairs_ptr = num_selected_falling_vectors;

//   vector_savings = num_rising_vecpairs + num_falling_vecpairs - num_selected_rising_vectors - num_selected_falling_vectors;

#ifdef DEBUG
printf("\nSUMMARY: SelectRandomBruteForce: Num selected rising vecpairs %d\tNum selected falling vecpairs %d\tTotal initial vectors %d\tVector savings %d\n\n", 
   *bruteforce_num_rise_vecpairs_ptr, *bruteforce_num_fall_vecpairs_ptr, num_rising_vecpairs + num_falling_vecpairs, vector_savings);
#endif

   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// 8/31/2018: New algorithm that is designed to optimally select qualifying paths. This algorithm attempts to
// minimize the number of vectors selected, but fails sometimes, especially if the number of qualifying
// paths is close to the number required, e.g., 3500 qualifying and 2048 need to be selected.

int SelectRandomOptVec(int max_string_len, int num_rise_qualified_PNs, int num_fall_qualified_PNs, 
   PathInfoStruct *qualified_path_info, int num_rise_required_PNs, int num_fall_required_PNs, int *rise_indexes, 
   int *fall_indexes, int num_rising_vecpairs, int num_falling_vecpairs, int *optvec_num_rise_vecpairs_ptr, 
   int *optvec_num_fall_vecpairs_ptr, int NUM_QUAL_PATH_LOWER_BOUND, int FRACTION_TO_SELECT_LOWER_BOUND, 
   int FRACTION_NUM_QUAL_PATH_LOWER_BOUND, int num_POs)
   {
   int qpi_low_index, qpi_high_index, num_qualifying_for_vecpair, fraction_PNs_needed_for_vecpair;
   int PN_num, vec_pair, random_fraction, random_PN, succeed, i;

// Hopefully, the number of vectors does NOT get terribly large otherwise stack allocation may fail.
   int selected_falling_vectors[num_falling_vecpairs];
   int selected_rising_vectors[num_rising_vecpairs];
   int num_randomly_selected_for_vecpair;
   int num_selected_falling_vectors;
   int num_selected_rising_vectors;
//   int vector_savings;

   int num_rise_qualifying_PN_found, num_fall_qualifying_PN_found; 

// Randomly select a set of rising PN qualified_path_info structures of size 'num_rise_required_PNs'. 8/31/2018: Need to do this 
// intelligently so that we don't end up with the majority of the vectors being used. Idea is to sort on vecpair_num (probably
// the same as sorting on path_num) and then random select vectors. For each vector, randomly choose among those paths tested
// by the vector pair. The goal is to include at least, e.g., 25% of the paths upto 100% of the paths (also randomly selected).
// Repeat for falling vectors. Stop when we meet the 2048 PN requirement.

// Sanity check
// This parameter is used to set a firm lower bound on the number of 'q's for any given vector-pair-mask. Masks
// with fewer than this number are skipped, and so is the corresponding vector pair. Setting this too large
// makes it difficult for the algorithm to find 2048 rise and 2048 fall PNs, and may cause this algorithm to
// fail (in which case, the vector pairs selected by the brute force algorithm is used). Increasing this reduces
// the number of vector pairs that are selected (which speeds up the HELP algorithm), as long as the OptVec
// is successful in finding 2048 rise and 2048 fall PNs.
   if ( NUM_QUAL_PATH_LOWER_BOUND <= 0 || NUM_QUAL_PATH_LOWER_BOUND > num_POs ) 
      { 
      printf("ERROR: SelectRandomOptVec(): NUM_QUAL_PATH_LOWER_BOUND MUST be a value between 1 and number of POs %d => %d!\n", 
         num_POs, NUM_QUAL_PATH_LOWER_BOUND);
      exit(EXIT_FAILURE);
      }

// Sanity check
// This parameter sets a lower bound on the fraction, e.g., 70% when 70 is specified, of 'q's that are selected
// from a mask with more than 'NUM_QUAL_PATH_LOWER_BOUND' 'q's. This parameter MUST be between 0 and 100. Setting
// this smaller, allows FEWER 'q's to be randomly select from among those that exist in each mask. This enables
// more diversity among the set of PN selected when this routine is run over and over again for different bitstring
// generations. But it also makes it harder for the algorithm to find 2048 rise and 2048 fall PN because, e.g., a
// mask that has 10 'q's (10 compatible PN) and say this is set to 10 (10%), then the random number generator may
// generate the lower bound of 10% and only allow 1 of the 10 PN to be used from this vector. Setting it to 50
// restricts the random number generator to generate a random percentage between 50% and 100%, forcing at least
// 5 of the PN to be used (as a lower bound).
   if ( FRACTION_TO_SELECT_LOWER_BOUND <= 0 || FRACTION_TO_SELECT_LOWER_BOUND > 100 ) 
      { 
      printf("ERROR: SelectRandomOptVec(): FRACTION_TO_SELECT_LOWER_BOUND MUST be a value between 1 and 100 => %d!\n", FRACTION_TO_SELECT_LOWER_BOUND); 
      exit(EXIT_FAILURE);
      }

// Sanity check
// If you use a small number for 'FRACTION_TO_SELECT_LOWER_BOUND', then you can discard masks where the number of
// actual PN selected (based on the percentage) is less than this absolute value. Setting is to a smaller value
// allows masks (vector pairs) to be used that have a smaller number of tested paths (PN) that are actually used.
// Setting it larger will force each vector pair to test at least this number of PN. Note that masks/vector pairs 
// that are eliminated because the random number generates too small a number using 'FRACTION_TO_SELECT_LOWER_BOUND' 
// may be tried again later -- see the algorithm (the vector pair is NOT eliminated as is true for violating the 
// above two parameters). Setting this too large may cause the algorithm to run a very long time so care must be 
// taken here.
   if ( FRACTION_NUM_QUAL_PATH_LOWER_BOUND <= 0 || FRACTION_NUM_QUAL_PATH_LOWER_BOUND > num_POs ) 
      { 
      printf("ERROR: SelectRandomOptVec(): FRACTION_NUM_QUAL_PATH_LOWER_BOUND MUST be a value between 1 and number of POs %d => %d!\n", 
         num_POs, FRACTION_NUM_QUAL_PATH_LOWER_BOUND);
      exit(EXIT_FAILURE);
      }

// These used by the caller to decide if optvec or the brute_force method produces smaller set of vectors.
   *optvec_num_rise_vecpairs_ptr = 0;
   *optvec_num_fall_vecpairs_ptr = 0;

   PN_num = 0;
   num_selected_rising_vectors = 0;
   succeed = 1;
   num_rise_qualifying_PN_found = num_fall_qualifying_PN_found = 0;
   while ( PN_num < num_rise_required_PNs )
      {

// Sanity check. Exit if we ran out of vectors.
      if ( num_selected_rising_vectors == num_rising_vecpairs )
         { 
#ifdef DEBUG
printf("INFO: SelectRandomOptVec(): RISING: Ran out of rising vecpairs before we could meeting path number requirement %d!\n", num_rise_required_PNs); fflush(stdout); 
#endif
         succeed = 0;
         break;
         }

// Randomly select a rising vector.
      vec_pair = rand() % num_rising_vecpairs;

// Check if this vector is already being used. If so, try another.
      for ( i = 0; i < num_selected_rising_vectors; i++ )
         if ( selected_rising_vectors[i] == vec_pair )
            break;
      if ( i != num_selected_rising_vectors )
         continue;

// Save the vector number selected so we dont use it again. NOTE: It is important that we record vectors that don't have ANY qualifying PNs (as we do below)
// and those that don't meet the minimum number of qualified paths (also done below). If both of these are false, AND the fraction requirements are not met
// below, then this vector will be removed from the list so it can be considered again but at a higher fraction. This will lead to potentically long run times
// so I may end up removing it (leaving it in the list) for this case too. Do not move this code further down in the code because we definitely want this 
// vector removed (kept in the list) for the first two cases.
      selected_rising_vectors[num_selected_rising_vectors] = vec_pair;
      num_selected_rising_vectors++;

// Get the index bounds on the number of PNs tested by this vector.
      qpi_low_index = 0;
      qpi_high_index = 0;

// Find first PN with vecpair that matches vec_pair, if any.
      while ( qpi_low_index < num_rise_qualified_PNs && qualified_path_info[qpi_low_index].vecpair_num != vec_pair )
         qpi_low_index++; 

#ifdef DEBUG
printf("RISE: Stopped at qpi_low_index %d with num_rise_qualified_PNs %d and qualified_path_info[qpi_low_index].vecpair_num %d !=? vec_pair %d\n",
   qpi_low_index, num_rise_qualified_PNs, qualified_path_info[qpi_low_index].vecpair_num, vec_pair); fflush(stdout);
#endif

// If we exit while loop because we ran out of elements, then qpi_low_index is one beyond max available. No match in this case. If we exit
// because the current vecpair_num matches vec_pair, then we sit on the first element.
      if ( qpi_low_index == num_rise_qualified_PNs )
         continue;

// Find last PN with vecpair that matches vec_pair. STOP if we reach the last element or the vecpair_num changes.
      qpi_high_index = qpi_low_index;
      while ( qpi_high_index < num_rise_qualified_PNs - 1 && qualified_path_info[qpi_high_index].vecpair_num == vec_pair )
         qpi_high_index++; 

// If we exit while loop because we are sitting on the last element then no adjustments are needed. If this is NOT true, then we exited the loop 
// because the vecpair_num does NOT match vec_pair. In this case, we have gone 1 too far, decrement.
//      if ( qpi_high_index != num_rise_qualified_PNs - 1 )
      if ( qualified_path_info[qpi_high_index].vecpair_num != vec_pair )
         qpi_high_index--; 

// Compute total number of qualifying paths for this vecpair.
      num_qualifying_for_vecpair = qpi_high_index - qpi_low_index + 1;
      if ( num_qualifying_for_vecpair <= 0 )
         { printf("PROGRAM ERROR: SelectRandomOptVec(): RISE: num_qualifying_for_vecpair is less than 0 %d!\n", num_qualifying_for_vecpair); exit(EXIT_FAILURE); }

#ifdef DEBUG
printf("SelectRandomOptVec(): RISING: Vecpair %d has %d qualifying paths starting at index %d to %d!\n", vec_pair, num_qualifying_for_vecpair, qpi_low_index, qpi_high_index); fflush(stdout);
#endif

// Sanity check
      if ( qpi_low_index >= num_rise_qualified_PNs || qpi_high_index >= num_rise_qualified_PNs )
         { 
         printf("PROGRAM ERROR: SelectRandomOptVec(): RISE: Indexes qpi_low_index %d or qpi_high_index %d is >= than number available %d!\n", 
            qpi_low_index, qpi_high_index, num_rise_qualified_PNs); exit(EXIT_FAILURE);
         }

// Sanity check
      if ( qualified_path_info[qpi_low_index].vecpair_num != vec_pair || qualified_path_info[qpi_high_index].vecpair_num != vec_pair )
         { 
         printf("PROGRAM ERROR: SelectRandomOptVec(): RISE: Indexes qpi_low_index %d selecting vec_pair %d OR qpi_high_index %d selecting vec_pair %d do NOT match vec_pair %d\n",
            qpi_low_index, qualified_path_info[qpi_low_index].vecpair_num, qpi_high_index, qualified_path_info[qpi_high_index].vecpair_num, vec_pair); exit(EXIT_FAILURE);
         }

// Check to make sure at least some paths exist that are qualified for this vector.
      if ( num_qualifying_for_vecpair < NUM_QUAL_PATH_LOWER_BOUND )
         { 
#ifdef DEBUG
printf("INFO: SelectRandomOptVec(): RISING: Vecpair %d has %d qualifying paths -- TOO FEW!\n", vec_pair, num_qualifying_for_vecpair); fflush(stdout);
#endif
         continue;
         }

// Randomly choose a percentage.
      random_fraction = (rand() % (100 - FRACTION_TO_SELECT_LOWER_BOUND)) + FRACTION_TO_SELECT_LOWER_BOUND;
      fraction_PNs_needed_for_vecpair = (int)(num_qualifying_for_vecpair*(float)random_fraction/100);

#ifdef DEBUG
printf("SelectRandomOptVec(): RISING: Random fraction of qualified paths to use for this vecpair %d%%. Number of paths to be randomly selected %d!\n",
   random_fraction, fraction_PNs_needed_for_vecpair);
fflush(stdout);
#endif

// Apply a similar lower bound constraint on the number that will actually be chosen from this vector.
      if ( fraction_PNs_needed_for_vecpair < FRACTION_NUM_QUAL_PATH_LOWER_BOUND )
         { 
#ifdef DEBUG
printf("INFO: SelectRandomOptVec(): RISING: Vecpair %d has %d qualifying paths randomly choosen -- TOO FEW!\n", vec_pair, fraction_PNs_needed_for_vecpair); fflush(stdout);
#endif

// Remove this vector from those marked as 'used'. Might need it, in which case, we will try again later and wait until the number of randomly selected paths is larger 
// than minimum required. Do NOT REMOVE THIS -- it adds too much runtime and this routine needs to be fast.
//         num_selected_rising_vectors--;
         continue;
         }

// Sanity check.
      if ( fraction_PNs_needed_for_vecpair > num_qualifying_for_vecpair )
         { 
         printf("PROGRAM ERROR: SelectRandomOptVec(): RISING: Fraction of PNs chosen %d is LARGER than number available %d!\n", 
            fraction_PNs_needed_for_vecpair, num_qualifying_for_vecpair); exit(EXIT_FAILURE); 
         }

// If we get here, then the vecpair qualifies. Add 1 to the number of rising vecpairs that will be part of the challenge.
      (*optvec_num_rise_vecpairs_ptr)++;

// Randomly select a set of 'fraction_PNs_needed_for_vecpair' PNs from the range of PNs valid for this vec_pair. This is guaranteed to always finish
// (no infinite loops) because of checks above.
      num_randomly_selected_for_vecpair = 0;
      while ( PN_num < num_rise_required_PNs && num_randomly_selected_for_vecpair < fraction_PNs_needed_for_vecpair )
         {
         random_PN = (rand() % num_qualifying_for_vecpair) + qpi_low_index;

// Check to make sure this index (path) is NOT already selected.
         for ( i = 0; i < PN_num; i++ )
            if ( rise_indexes[i] == random_PN )
               break;

// If the list is empty or we did NOT find random_PN, assign it to the array and increment.
         if ( i == PN_num )
            {
            rise_indexes[PN_num] = random_PN;
            PN_num++;
            num_randomly_selected_for_vecpair++;

#ifdef DEBUG
printf("RISING: Vecpair %d\tRandomly chosen rise PN is %d\tSelecting %d of total number available %d\tTotal number selected so far %d\n", 
   vec_pair, random_PN, fraction_PNs_needed_for_vecpair, num_qualifying_for_vecpair, PN_num);
#endif
            }
         }
      num_rise_qualifying_PN_found = PN_num;
      }

#ifdef DEBUG
printf("\n\n"); fflush(stdout);
#endif

// ============================================================================================================================
// Do the same for falling vectors, even if we fail above, for status information.
   PN_num = 0;
   num_selected_falling_vectors = 0;
   while ( PN_num < num_fall_required_PNs )
      {

// Sanity check. Exit if we ran out of vectors.
      if ( num_selected_falling_vectors == num_falling_vecpairs )
         { 
#ifdef DEBUG
printf("INFO: SelectRandomOptVec(): FALLING: Ran out of falling vecpairs before we could meeting path number requirement %d!\n", num_fall_required_PNs); 
#endif
         succeed = 0;
         break;
         }

// Randomly select a falling vector. Note that falling vectors following rising vectors and do NOT start at 0 but rather continue numbering.
      vec_pair = (rand() % num_falling_vecpairs) + num_rising_vecpairs;

// Check if this vector is already being used. If so, try another.
      for ( i = 0; i < num_selected_falling_vectors; i++ )
         if ( selected_falling_vectors[i] == vec_pair )
            break;
      if ( i != num_selected_falling_vectors )
         continue;

// Save the vector number selected so we dont use it again. NOTE: It is important that we record vectors that don't have ANY qualifying PNs (as we do below)
// and those that don't meet the minimum number of qualified paths (also done below). If both of these are false, AND the fraction requirements are not met
// below, then this vector will be removed from the list so it can be considered again but at a higher fraction. This will lead to potentically long run times
// so we may end up removing it (leaving it in the list) for this case too. Do not move this code further down in the code because we definitely want this 
// vector removed (kept in the list) for the first two cases.
      selected_falling_vectors[num_selected_falling_vectors] = vec_pair;
      num_selected_falling_vectors++;

// Get the index bounds on the number of PNs tested by this vector.
      qpi_low_index = 0;
      qpi_high_index = 0;

// Find first PN with vecpair that matches vec_pair. 
      while ( qpi_low_index < num_rise_qualified_PNs + num_fall_qualified_PNs && qualified_path_info[qpi_low_index].vecpair_num != vec_pair )
         qpi_low_index++; 

#ifdef DEBUG
printf("FALL: Stopped at qpi_low_index %d with num_rise_qualified_PNs %d and qualified_path_info[qpi_low_index].vecpair_num %d !=? vec_pair %d\n",
   qpi_low_index, num_rise_qualified_PNs, qualified_path_info[qpi_low_index].vecpair_num, vec_pair); fflush(stdout);
#endif

// If we exit while loop because we ran out of elements, then qpi_low_index is one beyond max available. No match in this case. If we exit
// because the current vecpair_num matches vec_pair, then we sit on the first element.
      if ( qpi_low_index == num_rise_qualified_PNs + num_fall_qualified_PNs )
         continue;

// Find last PN with vecpair that matches vec_pair.
      qpi_high_index = qpi_low_index;
      while ( qpi_high_index < num_rise_qualified_PNs + num_fall_qualified_PNs - 1 && qualified_path_info[qpi_high_index].vecpair_num == vec_pair )
         qpi_high_index++; 

// If we exit while loop because we are sitting on the last element then no adjustments are needed. If this is NOT true, then we exited the loop 
// because the vecpair_num does NOT match vec_pair. In this case, we have gone 1 too far, decrement.
//      if ( qpi_high_index != num_rise_qualified_PNs + num_fall_qualified_PNs - 1 )
      if ( qualified_path_info[qpi_high_index].vecpair_num != vec_pair )
         qpi_high_index--;

// Compute total number of qualifying paths for this vecpair.
      num_qualifying_for_vecpair = qpi_high_index - qpi_low_index + 1;
      if ( num_qualifying_for_vecpair <= 0 )
         { printf("PROGRAM ERROR: SelectRandomOptVec(): FALL: num_qualifying_for_vecpair is less than 0 %d!\n", num_qualifying_for_vecpair); exit(EXIT_FAILURE); }

#ifdef DEBUG
printf("SelectRandomOptVec(): FALLING: Vecpair %d has %d qualifying paths starting at index %d to %d!\n", vec_pair, num_qualifying_for_vecpair, qpi_low_index, qpi_high_index); fflush(stdout);
#endif

// Sanity check
      if ( qpi_low_index < num_rise_qualified_PNs || qpi_low_index >= num_rise_qualified_PNs + num_fall_qualified_PNs || qpi_high_index >= num_rise_qualified_PNs + num_fall_qualified_PNs )
         { 
         printf("PROGRAM ERROR: SelectRandomOptVec(): FALL: Indexes qpi_low_index %d or qpi_high_index %d are outside range < %d or >= %d!\n", 
            qpi_low_index, qpi_high_index, num_rise_qualified_PNs, num_rise_qualified_PNs + num_fall_qualified_PNs); exit(EXIT_FAILURE);
         }

// Sanity check
      if ( qualified_path_info[qpi_low_index].vecpair_num != vec_pair || qualified_path_info[qpi_high_index].vecpair_num != vec_pair )
         { 
         printf("PROGRAM ERROR: SelectRandomOptVec(): FALL: Indexes qpi_low_index %d selecting vec_pair %d OR qpi_high_index %d selecting vec_pair %d do NOT match vec_pair %d\n",
            qpi_low_index, qualified_path_info[qpi_low_index].vecpair_num, qpi_high_index, qualified_path_info[qpi_high_index].vecpair_num, vec_pair); exit(EXIT_FAILURE);
         }

// Check to make sure at least some paths exist that are qualified for this vector.
      if ( num_qualifying_for_vecpair < NUM_QUAL_PATH_LOWER_BOUND )
         { 
#ifdef DEBUG
printf("INFO: SelectRandomOptVec(): FALLING: Vecpair %d has %d qualifying paths -- TOO FEW!\n", vec_pair, num_qualifying_for_vecpair); fflush(stdout);
#endif
         continue;
         }

// Randomly choose a percentage.
      random_fraction = (rand() % (100 - FRACTION_TO_SELECT_LOWER_BOUND)) + FRACTION_TO_SELECT_LOWER_BOUND;
      fraction_PNs_needed_for_vecpair = (int)(num_qualifying_for_vecpair*(float)random_fraction/100);

#ifdef DEBUG
printf("SelectRandomOptVec(): FALLING: Random fraction of qualified paths to use for this vecpair %d%%. Number of paths to be randomly selected %d!\n",
   random_fraction, fraction_PNs_needed_for_vecpair);
fflush(stdout);
#endif

// Apply a similar lower bound constraint on the number that will actually be chosen from this vector.
      if ( fraction_PNs_needed_for_vecpair < FRACTION_NUM_QUAL_PATH_LOWER_BOUND )
         { 
#ifdef DEBUG
printf("INFO: SelectRandomOptVec(): FALLING: Vecpair %d has %d qualifying paths randomly choosen -- TOO FEW!\n", vec_pair, fraction_PNs_needed_for_vecpair); fflush(stdout);
#endif

// Remove this vector from those marked as 'used'. Might need it, in which case, we will try again later and wait until the number of randomly selected paths is larger 
// than minimum required. Do NOT REMOVE THIS -- it adds too much runtime and this routine needs to be fast.
//         num_selected_falling_vectors--;
         continue;
         }

// Sanity check.
      if ( fraction_PNs_needed_for_vecpair > num_qualifying_for_vecpair )
         { 
         printf("PROGRAM ERROR: SelectRandomOptVec(): FALLING: Fraction of PNs chosen %d is LARGER than number available %d!\n", 
            fraction_PNs_needed_for_vecpair, num_qualifying_for_vecpair); exit(EXIT_FAILURE); 
         }

// If we get here, then the vecpair qualifies. Add 1 to the number of falling vecpairs that will be part of the challenge.
      (*optvec_num_fall_vecpairs_ptr)++;

// Randomly select a set of 'fraction_PNs_needed_for_vecpair' PNs from the range of PNs valid for this vec_pair. This is guaranteed to always finish
// (no infinite loops) because of checks above.
      num_randomly_selected_for_vecpair = 0;
      while ( PN_num < num_fall_required_PNs && num_randomly_selected_for_vecpair < fraction_PNs_needed_for_vecpair )
         {

// qpi_low_index is an index of a qualified_path_info element and handles the offset needed.
         random_PN = (rand() % num_qualifying_for_vecpair) + qpi_low_index; 

// Check to make sure this index (path) is NOT already selected.
         for ( i = 0; i < PN_num; i++ )
            if ( fall_indexes[i] == random_PN )
               break;

// If the list is empty or we did NOT find random_PN, assign it to the array and increment.
         if ( i == PN_num )
            {
            fall_indexes[PN_num] = random_PN;
            PN_num++;
            num_randomly_selected_for_vecpair++;

#ifdef DEBUG
printf("FALLING: Vecpair %d\tRandomly chosen fall PN is %d\tSelecting %d of total number available %d\tTotal number selected so far %d\n", 
   vec_pair, random_PN, fraction_PNs_needed_for_vecpair, num_qualifying_for_vecpair, PN_num);
#endif
            }
         }

      num_fall_qualifying_PN_found = PN_num;
      }

// Sanity check
   if ( (num_rise_qualifying_PN_found != num_rise_required_PNs || num_fall_qualifying_PN_found != num_fall_required_PNs) && succeed == 1 )
      { 
      printf("PROGRAM ERROR: SelectRandomOptVec(): Succeed is true but number of rising or falling qualified PN found is NOT equal to %d and %d!\n", 
         num_rise_required_PNs, num_fall_required_PNs); exit(EXIT_FAILURE); 
      }

//   vector_savings = num_rising_vecpairs + num_falling_vecpairs - *optvec_num_rise_vecpairs_ptr - *optvec_num_fall_vecpairs_ptr;

#ifdef DEBUG
printf("\nSUMMARY: SelectRandomOptVec: Did we succeed in finding a qualifying set? %d\tNum qualified rising PN found %d\tNum qualified falling PN found %d\n", 
   succeed, num_rise_qualifying_PN_found, num_fall_qualifying_PN_found);
printf("\tNum selected rising vecpairs %d\tNum selected falling vecpairs %d\tTotal initial vectors %d\tVector savings %d\n\n\n", 
   *optvec_num_rise_vecpairs_ptr, *optvec_num_fall_vecpairs_ptr, num_rising_vecpairs + num_falling_vecpairs, vector_savings);
#endif

   return succeed;
   }


// ===========================================================================================================
// ===========================================================================================================
// Randomly select a subset of 'num_xxx_required_PNs' from the number that is 'qualified' using a Seed parameter
// to the C rand function.

int SelectRandomSubset(int max_string_len, unsigned int Seed, int num_rise_qualified_PNs, int num_fall_qualified_PNs, 
   PathInfoStruct *qualified_path_info, int num_rise_required_PNs, int num_fall_required_PNs, int *rise_indexes1, 
   int *fall_indexes1, int *rise_indexes2, int *fall_indexes2, int num_rising_vecpairs, int num_falling_vecpairs, 
   int num_tested_PNs, PathInfoStruct *tested_path_info, int NUM_QUAL_PATH_LOWER_BOUND, int FRACTION_TO_SELECT_LOWER_BOUND, 
   int FRACTION_NUM_QUAL_PATH_LOWER_BOUND, int num_POs)
   {
   int bruteforce_num_rise_vecpairs, bruteforce_num_fall_vecpairs;
   int optvec_num_rise_vecpairs, optvec_num_fall_vecpairs;
   int *rise_indexes_final, *fall_indexes_final;
   int PN_num_tested, PN_num_qualified;
   int optvec_succeed = 0;

#ifdef DEBUG
struct timeval t1, t2;
long elapsed; 
#endif

// Sanity check. The number of qualified MUST be larger than the number that we will select, otherwise, the hardware masks
// will NOT have enough rising and falling edge PNs selected.
   if ( num_rise_qualified_PNs < num_rise_required_PNs || num_fall_qualified_PNs < num_fall_required_PNs )
      { printf("ERROR: SelectRandomSubset(): Number of 'qualified_rise/fall_PNs LESS THAN the number required!\n"); exit(EXIT_FAILURE); }

   srand(Seed);

#ifdef DEBUG
gettimeofday(&t2, 0);
#endif

// Sort here -- DO NOT SORT INSIDE OF THE TWO calls below to SelectRandomBruteForce and SelectRandomOptVec since the indexes returned
// in the arrays are given with repect to THIS order.
   qsort(qualified_path_info, num_rise_qualified_PNs + num_fall_qualified_PNs, sizeof(PathInfoStruct), PathInfoVecPairNumCompareFunc);

// Sanity check. Hope this always holds -- NOT sure if this is important since sorting on vecpair_num brings all qualified_path_info records
// together. Plus we use indexes into qualified_path_info below to record paths that , NOT the path_num. So this check can be removed if it 
// becomes a problem.
   for ( PN_num_qualified = 0; PN_num_qualified < num_rise_qualified_PNs + num_fall_qualified_PNs - 1; PN_num_qualified++ )
      if ( qualified_path_info[PN_num_qualified].path_num >= qualified_path_info[PN_num_qualified+1].path_num )
         { printf("ERROR: SelectRandomSubset(): Expected sorted list on 'vecpair_num' to be in 'path_num' order!\n"); exit(EXIT_FAILURE); }

// This is the original brute force algorithm that does NOT track vecpair usage. 
   SelectRandomBruteForce(max_string_len, num_rise_qualified_PNs, num_fall_qualified_PNs, qualified_path_info, num_rise_required_PNs, num_fall_required_PNs, 
      rise_indexes1, fall_indexes1, num_rising_vecpairs, num_falling_vecpairs, &bruteforce_num_rise_vecpairs, &bruteforce_num_fall_vecpairs);
   rise_indexes_final = rise_indexes1;
   fall_indexes_final = fall_indexes1;

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tELAPSED TIME: BruteForce algo %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
#endif

// This is one of the algorithms that is designed to optimally select qualifying paths. This algorithm attempts to minimize the number 
// of vectors selected, but fails sometimes, especially if the number of qualifying paths is close to the number required, e.g., 
// 3500 qualifying and 2048 need to be selected. 
   if ( (optvec_succeed = SelectRandomOptVec(max_string_len, num_rise_qualified_PNs, num_fall_qualified_PNs, qualified_path_info, num_rise_required_PNs, 
      num_fall_required_PNs, rise_indexes2, fall_indexes2, num_rising_vecpairs, num_falling_vecpairs, &optvec_num_rise_vecpairs,
      &optvec_num_fall_vecpairs, NUM_QUAL_PATH_LOWER_BOUND, FRACTION_TO_SELECT_LOWER_BOUND, FRACTION_NUM_QUAL_PATH_LOWER_BOUND, num_POs)) == 1 )
      {

// If we succeed, then check if number of vectors is smaller than brute force method. If so, use OptVec selected vectors.
      if ( optvec_num_rise_vecpairs + optvec_num_fall_vecpairs < bruteforce_num_rise_vecpairs + bruteforce_num_fall_vecpairs )
         {
         rise_indexes_final = rise_indexes2;
         fall_indexes_final = fall_indexes2;
         }
      }

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tELAPSED TIME: OptVec algo %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
#endif

// ============================================================================================================================
// Set the 'path_selected' field for the paths that are selected. Note that the rise/fall_indexes randomly refer to elements in
// the sorted (by vecpair_num) qualified_path_info array. Once the 'path_selected' field is set, it is safe to sort 
// qualified_path_info. Note that the 'random_order_number' field records the random order of the selection process carried out
// on qualified_path_info (truely random for BruteForce, semi-random for OptVec). This is used ONLY for analysis not done here.
   int num_updates = 0;
   for ( PN_num_qualified = 0; PN_num_qualified < num_rise_required_PNs; PN_num_qualified++ )
      {
      if ( qualified_path_info[rise_indexes_final[PN_num_qualified]].rise_or_fall != 0 )
         { 
         printf("ERROR: SelectRandomSubset(): Found qualified_path_info structure at index %d that is NOT classified as storing a rising PN!\n",
            rise_indexes_final[PN_num_qualified]); exit(EXIT_FAILURE); 
         }
      qualified_path_info[rise_indexes_final[PN_num_qualified]].path_selected = 1;
      qualified_path_info[rise_indexes_final[PN_num_qualified]].random_order_number = PN_num_qualified;
      num_updates++;
      }

#ifdef DEBUG
printf("Number of rising updates to 'path_selected' field in qualified_path_info is %d\n", num_updates); fflush(stdout);
#endif

   num_updates = 0;
   for ( PN_num_qualified = 0; PN_num_qualified < num_fall_required_PNs; PN_num_qualified++ )
      {
      if ( qualified_path_info[fall_indexes_final[PN_num_qualified]].rise_or_fall != 1 )
         { 
         printf("ERROR: SelectRandomSubset(): Found qualified_path_info structure at index %d that is NOT classified as storing a falling PN!\n",
            fall_indexes_final[PN_num_qualified]); exit(EXIT_FAILURE); 
         }
      qualified_path_info[fall_indexes_final[PN_num_qualified]].path_selected = 1;
      qualified_path_info[fall_indexes_final[PN_num_qualified]].random_order_number = PN_num_qualified;
      num_updates++;
      }
#ifdef DEBUG
printf("Number of falling updates to 'path_selected' field in qualified_path_info is %d\n", num_updates); fflush(stdout);
#endif

// Sort qualified_path_info on 'random_order_num' since qualified_path_info is currently sorted on 'vecpair_num'. This randomizes the order of
// the qualified_path_info array according to the randomized, semi-randomized selection process carried out in BruteForce or OptVec selection 
// routines. This is needed ONLY for the evaluation process carried out on the file data written next. Sort on each set separately.
//   qsort(qualified_path_info, num_rise_qualified_PNs, sizeof(PathInfoStruct), PathInfoRandomOrderNumberCompareFunc); 
//   qsort(&(qualified_path_info[num_rise_qualified_PNs]), num_fall_qualified_PNs, sizeof(PathInfoStruct), PathInfoRandomOrderNumberCompareFunc); 

// Save this information to a file so we can evaluate security properties using other programs. This is printed out 'random_order_number' ORDER 
// because we randomly selected the indexes above. Commented this out 8/30/2018.
//   FILE *OUTFILE;
//   printf("\nWriting '%s'\n", out_tested_path_info_fullpath);
//   sprintf(out_tested_path_info_fullpath, "output_random_qualifyingpaths/GenMasks/%s_%s_%s_OutTestedPathInfo_%s_Seed_%d.txt", 
//      Netlist_name, Synthesis_name, ChallengeSetName, Seed);
//   if ( (OUTFILE = fopen(out_tested_path_info_fullpath, "w")) == NULL )
//      { printf("ERROR: SelectRandomSubset(): Path information file '%s' open failed for writing!\n", out_tested_path_info_fullpath); exit(EXIT_FAILURE); }
//   i = 0;
//   for ( PN_num_qualified = 0; PN_num_qualified < num_rise_qualified_PNs + num_fall_qualified_PNs; PN_num_qualified++ )
//      if ( qualified_path_info[PN_num_qualified].path_selected == 1 )
//         {
//         fprintf(OUTFILE, "%4d) PathNum %5d\tVecNum %d\tRiseOrFall %d\n", i, qualified_path_info[PN_num_qualified].path_num, 
//            qualified_path_info[PN_num_qualified].vecpair_num, qualified_path_info[PN_num_qualified].rise_or_fall);
//         i++;
//         }
//   fclose(OUTFILE);


// Sort qualified_path_info on 'path_num'. ASSUME tested_path_info is sorted on 'path_num'. This reduces time to update tested_path_info
// below with qualified_path_info. SORT HERE IF NEEDED!
   qsort(qualified_path_info, num_rise_qualified_PNs + num_fall_qualified_PNs, sizeof(PathInfoStruct), PathInfoPathNumCompareFunc); 

#ifdef DEBUG
int i = 0;
for ( PN_num_qualified = 0; PN_num_qualified < num_rise_qualified_PNs + num_fall_qualified_PNs; PN_num_qualified++ )
   if ( qualified_path_info[PN_num_qualified].path_selected == 1 )
      {
      printf("%4d) PathNum %5d\tVecNum %d\tSelected %d\tRise/Fall %d\n", i, qualified_path_info[PN_num_qualified].path_num, qualified_path_info[PN_num_qualified].vecpair_num, 
         qualified_path_info[PN_num_qualified].path_selected, qualified_path_info[PN_num_qualified].vecpair_num >= num_rising_vecpairs);
      i++;
      }
#endif

// Update path_info structure with qualified_path_info 'path_selected' value. The path_info structure stores path information for ALL TESTED PATHS 
// (num_tested_PNs) while num_rise/fall_qualified_PNs is a subset of these. Note only 4096 of the qualified_PNs are identified as 'selected'. 
   PN_num_tested = 0;
   for ( PN_num_qualified = 0; PN_num_qualified < num_rise_qualified_PNs + num_fall_qualified_PNs; PN_num_qualified++ )
      {

// Move forward in the tested_path_info structure until we find a path to 'path_num' in the qualified_path_info structure.
      while ( PN_num_tested < num_tested_PNs )
         {
         if ( qualified_path_info[PN_num_qualified].path_num == tested_path_info[PN_num_tested].path_num )
            {

// Sanity check
            if ( qualified_path_info[PN_num_qualified].vecpair_num != tested_path_info[PN_num_tested].vecpair_num )
               { 
               printf("ERROR: SelectRandomSubset(): Vector num mismatch between qualified_path_info and tested_path_info for path_num %d!\n", 
                  qualified_path_info[PN_num_qualified].path_num); exit(EXIT_FAILURE); 
               }

// Copy entire structure.
            tested_path_info[PN_num_tested] = qualified_path_info[PN_num_qualified];
            break;
            }
         PN_num_tested++; 
         }

// We MUST always find a match for every qualified_path_info element in the tested_path_info array.
      if ( PN_num_tested == num_tested_PNs )
         { printf("ERROR: SelectRandomSubset(): Failed to find qualified_path_info path_num %d in path_info structure!\n", qualified_path_info[PN_num_qualified].path_num); exit(EXIT_FAILURE); }
      }

   return optvec_succeed;
   }


// ===========================================================================================================
// ===========================================================================================================
// Create the masks from the information stored in tested_path_info. The elements in tested_path_info store
// data ONLY for POs that transition. This routine fills in the masks with '0's POs that do NOT transition. 
// Note that the qualified_path_info data has been transferred to tested_path_info before this routine is
// called and therefore, mask bits corresponding to tested_path_info elements that do NOT have the 'path_selected' 
// set to 1 also generate '0's in the masks.

void CreateMasksFromQualifiedPathInfo(int max_string_len, sqlite3 *db, char *outfile_vecs, char *outfile_masks, 
   int num_tested_PNs, int num_PIs, int num_POs, PathInfoStruct *tested_path_info, int num_vecpairs, 
   int num_required_PNs, char ***masks_ptr, int *num_masks_created_ptr, int *vecpair_is_selected)
   {
   int PN_num_tested, PO_num, num_masks_processed, num_masks_allocated;
   int tot_written_1_bits, cur_vec;
   int has_transition; 

#ifdef DEBUG
printf("\tCreating masks from subset of large set of tested paths by qualifying vectors %d\n\n", num_tested_PNs); fflush(stdout);
#endif

// Sanity check
   tot_written_1_bits = 0;
   for ( PN_num_tested = 0; PN_num_tested < num_tested_PNs; PN_num_tested++ )
      if ( tested_path_info[PN_num_tested].path_selected == 1 )
         tot_written_1_bits++;
   if ( tot_written_1_bits != num_required_PNs ) 
      { 
      printf("ERROR: CreateMasksFromQualifiedPathInfo(): Number of paths selected %d NOT equal to number required %d\n", 
         tot_written_1_bits, num_required_PNs); exit(EXIT_FAILURE); 
      }

#ifdef DEBUG
printf("\tNumber of selected paths %d is equal to the number required %d\n", tot_written_1_bits, num_required_PNs); fflush(stdout);
#endif

// The tested_path_info struct is filled in with data from FindQualifyingPaths in the order that the vectors are applied in the hardware.
// Vector order is sequential
   PN_num_tested = 0;
   PO_num = 0;
   cur_vec = 0;
   num_masks_processed = 0;
   *num_masks_created_ptr = 0;
   tot_written_1_bits = 0;
   has_transition = 0;

// Setting *masks_ptr to NULL allows realloc to behave as malloc on the first call.
   *masks_ptr = NULL;
   num_masks_allocated = 0;

// It is possible that some vectors have NO transitions. Update 'cur_vec' if this occurs for the first vector. 
   while ( cur_vec < tested_path_info[PN_num_tested].vecpair_num )
      cur_vec++;

   while (1)
      {

#ifdef DEBUG
printf("Processing PN_num_tested %d with tested_path_info PO_num %d\tSelected? %d\tWith tested_path_info vector number %d\tCurrent vec_num %d and PO_num %d\n", 
   PN_num_tested, tested_path_info[PN_num_tested].PO_num, tested_path_info[PN_num_tested].path_selected, tested_path_info[PN_num_tested].vecpair_num, cur_vec, PO_num);
#endif

// Create the next mask when the number of masks allocated is equal to the number of masks created. We MUST allocate storage in advance of the next mask 
// to be created (if any). This while loop can iterate multiple times for each mask that is created. 
      if ( num_masks_allocated == *num_masks_created_ptr )
         {
         if ( ((*masks_ptr) = (char **)realloc(*masks_ptr, sizeof(char *) * (num_masks_allocated + 1))) == NULL )
            { printf("ERROR: CreateMasksFromQualifiedPathInfo(): Failed to reallocate storage for masks_ptr!\n"); exit(EXIT_FAILURE); }
         if ( ((*masks_ptr)[num_masks_allocated] = (char *)malloc(sizeof(char) * (num_POs + 1))) == NULL )
            { printf("ERROR: CreateMasksFromQualifiedPathInfo(): Failed to reallocate storage for mask!\n"); exit(EXIT_FAILURE); }
         memset((*masks_ptr)[num_masks_allocated], 0, num_POs + 1);
         num_masks_allocated++;
         }

// The tested_path_info struct stores data ONLY for paths that have transitions. We need to add '0's to the mask for all paths that HAVE NO TRANSITIONS as well.
// Add '0's until the 'PO_num' is equal to the 'PO_num' recorded for the current path, or 'PO_num' reaches max value. REMEMBER TO WRITE THE STRING FROM LAST TO 
// FIRST because the first bit in the string is the HIGH ORDER BIT IN THE MASK, NOT THE LOW ORDER BIT which is where you want to put the value when PO_num is 0. 
// NOTE: THIS CODE REPEATED WITH SLIGHT VARIATIONS in WriteQualifyingPathMasks which is used by TVCharacterization to write the special mask files.
      while ( PO_num < num_POs && (PO_num != tested_path_info[PN_num_tested].PO_num || cur_vec != tested_path_info[PN_num_tested].vecpair_num) )
         {
#ifdef DEBUG
printf("\tWriting '0' to mask for PO_num %d\n", PO_num);
#endif
         (*masks_ptr)[*num_masks_created_ptr][num_POs - PO_num - 1] = '0';
         PO_num++;
         }

#ifdef DEBUG
printf("\tChecking is current PO_num %d is less then number of POs (num_POs) %d\n", PO_num, num_POs);
#endif

// Write mask according to tested_path_info information for this 'PO_num', i.e., was this path selected or not? 
      if ( PO_num < num_POs )
         {
         if ( tested_path_info[PN_num_tested].path_selected == 1 )
            {
            (*masks_ptr)[*num_masks_created_ptr][num_POs - PO_num - 1] = '1';
            tot_written_1_bits++; 
#ifdef DEBUG
printf("\tSelected Vec %d\tPath %d\tPO %d\n", tested_path_info[PN_num_tested].vecpair_num, tested_path_info[PN_num_tested].path_num, tested_path_info[PN_num_tested].PO_num);
#endif

// Sanity check.
            if ( tested_path_info[PN_num_tested].vecpair_num < 0 || tested_path_info[PN_num_tested].vecpair_num >= num_vecpairs )
               { printf("ERROR: CreateMasksFromQualifiedPathInfo(): Vector number of selected path OUTSIDE valid range!\n"); exit(EXIT_FAILURE); }

// Sanity check. Vectors must be processed in order of 0 to n since we are writing vector and mask output files.
            if ( cur_vec != tested_path_info[PN_num_tested].vecpair_num )
               { 
               printf("ERROR: CreateMasksFromQualifiedPathInfo(): Unexpected vector number in current tested_path_info structure %d -- expected %d!\n", 
                  tested_path_info[PN_num_tested].vecpair_num, cur_vec); exit(EXIT_FAILURE); 
               }

// If any PO for this vector HAS a transition, indicate that the vector needs to be saved to the output file.
            vecpair_is_selected[tested_path_info[PN_num_tested].vecpair_num] = 1;
            has_transition = 1;
            }
         else
            (*masks_ptr)[*num_masks_created_ptr][num_POs - PO_num - 1] = '0';
         PO_num++;
         PN_num_tested++;
         }

// If we just processed the last path, i.e., PN_num_tested == num_tested_PNs, finish mask off with '0's. 
      if ( PN_num_tested == num_tested_PNs )
         while ( PO_num < num_POs )
            {
            (*masks_ptr)[*num_masks_created_ptr][num_POs - PO_num - 1] = '0';
            PO_num++;
            }

#ifdef DEBUG
printf("\tIs PO_num %d == num_POs %d\n", PO_num, num_POs);
#endif

// As soon as 'PO_num' is equal to the number of POs, we are done with this mask.
      if ( PO_num == num_POs )
         {

// NULL-terminate the mask so we can write it as a string to the file.
         (*masks_ptr)[*num_masks_created_ptr][PO_num] = '\0';

// DO NOT WRITE the mask line if this vector has NO transitions.
         if ( has_transition == 1 )
            {
//            fprintf(OUTFILE_MASKS, "%s\n", mask);
            (*num_masks_created_ptr)++; 
            }

// Re-initialize 'PO_num' and add 1 to the number of masks that have been processed (with or without a transition) so we can do a 
// sanity check below.
         PO_num = 0;
         num_masks_processed++; 

#ifdef DEBUG
printf("\tIncrementing old value of cur_vec %d to new value %d\n", cur_vec, cur_vec+1);
#endif

// Re-initialize state for next vector and update vector number sanity checking variable.
         has_transition = 0;
         cur_vec++; 

// It is possible that some vectors have NO transitions. Update 'cur_vec' in this case. PN_num_tested was updated above and points to 
// the next PN now (if there is one). 
         if ( PN_num_tested < num_tested_PNs )
            while ( cur_vec < tested_path_info[PN_num_tested].vecpair_num )
               cur_vec++;

#ifdef DEBUG
printf("\tMay have updated cur_vec to skip vectors with NO transitions -> value at exit %d\n", cur_vec);
#endif
         }

// Exit if we processed all tested_path_info records.
      if ( PN_num_tested == num_tested_PNs )
         break;
      }

// Sanity check. 
   if ( num_masks_allocated < *num_masks_created_ptr )
      {
      printf("ERROR: CreateMasksFromQualifiedPathInfo(): UNEXPECTED: Number of allocated masks %d less than number filled in %d!\n", num_masks_allocated, *num_masks_created_ptr); 
      exit(EXIT_FAILURE);
      }

// Sanity check. 
   if ( num_masks_allocated != *num_masks_created_ptr )
      {

// If the number allocated is one extra, then this can happen in the above loop. Free the extra mask to prevent a memory leak. 
      if ( num_masks_allocated == *num_masks_created_ptr + 1 )
         free((*masks_ptr)[num_masks_allocated-1]);

// Technically this is just a memory leak, but indicates something is wrong, so I'm generating an error.
      else
         {
         printf("WARNING: CreateMasksFromQualifiedPathInfo(): UNEXPECTED: Number of allocated masks %d greater than to number filled in %d!\n", num_masks_allocated, *num_masks_created_ptr); 
         exit(EXIT_FAILURE);
         }
      }

// Number of mask processed (NOT created) must equal the number of vectors.
   if ( num_masks_processed != num_vecpairs )
      { 
      printf("ERROR: CreateMasksFromQualifiedPathInfo(): Number of masks created is %d, but is not equal to number of vectors %d\n", 
         num_masks_processed, num_vecpairs); 
      exit(EXIT_FAILURE); 
      }

#ifdef DEBUG
printf("\tTOTAL number of '1's written to the mask file => %d\tNumber of vector masks written to the outputfile %d of original number %d\n", 
   tot_written_1_bits, *num_masks_created_ptr, num_vecpairs); fflush(stdout);
#endif

   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// Get the binary versions of the vectors from the database for the newly created challenge. 

void GetChallengeBinaryVecsFromDB(int max_string_len, sqlite3 *db, int num_PIs, int num_vecpairs, 
   int *vecpair_is_selected, int *vecpair_ids, unsigned char ***vecs1_bin_ptr, unsigned char ***vecs2_bin_ptr, 
   int *num_vecs_masks_ptr, int *num_rise_vecs_masks_ptr)
   {
   SQLRowStringsStruct row_strings_struct;
   char sql_command_str[max_string_len];
   int num_challenge_vectors, vec_num;
   int VA_index, VB_index;

   char *VP_VA_name = "VA";
   char *VP_VB_name = "VB";
   char *Vectors_name = "vector";

   int doing_rise_vectors;

// ==============================================
// Vector processing: 

// Force realloc to behave like malloc on the first call.
   *vecs1_bin_ptr = NULL;
   *vecs2_bin_ptr = NULL;

   *num_vecs_masks_ptr = 0;
   *num_rise_vecs_masks_ptr = 0;
   doing_rise_vectors = 1;
   num_challenge_vectors = 0;
   for ( vec_num = 0; vec_num < num_vecpairs; vec_num++ )
      if ( vecpair_is_selected[vec_num] == 1 )
         {

// Allocate memory for the vector pair.
         if ( (*vecs1_bin_ptr = (unsigned char **)realloc(*vecs1_bin_ptr, sizeof(unsigned char *) * (num_challenge_vectors + 1))) == NULL )
            { printf("ERROR: GetChallengeBinaryVecsFromDB(): Failed to reallocate storage for vecs1_bin_ptr!\n"); exit(EXIT_FAILURE); }
         if ( ((*vecs1_bin_ptr)[num_challenge_vectors] = (unsigned char *)malloc(sizeof(unsigned char) * num_PIs/8)) == NULL )
            { printf("ERROR: GetChallengeBinaryVecsFromDB(): Failed to reallocate storage for vecs1_bin!\n"); exit(EXIT_FAILURE); }

         if ( (*vecs2_bin_ptr = (unsigned char **)realloc(*vecs2_bin_ptr, sizeof(unsigned char *) * (num_challenge_vectors + 1))) == NULL )
            { printf("ERROR: GetChallengeBinaryVecsFromDB(): Failed to reallocate storage for vecs2_bin_ptr!\n"); exit(EXIT_FAILURE); }
         if ( ((*vecs2_bin_ptr)[num_challenge_vectors] = (unsigned char *)malloc(sizeof(unsigned char) * num_PIs/8)) == NULL )
            { printf("ERROR: GetChallengeBinaryVecsFromDB(): Failed to reallocate storage for vecs2_bin!\n"); exit(EXIT_FAILURE); }

// Get the vector index for the first vector from VecPairs. 
         sprintf(sql_command_str, "SELECT %s FROM VecPairs WHERE id = %d;", VP_VA_name, vecpair_ids[vec_num]);
         GetStringsDataForRow(max_string_len, db, sql_command_str, &row_strings_struct);
         GetRowResultInt(&row_strings_struct, "GetChallengeBinaryVecsFromDB()", 1, 0, VP_VA_name, &VA_index);
         FreeStringsDataForRow(&row_strings_struct);

// Determine if its a rising or falling vector. Only need to do this on the first call. NOTE: ALL rise vectors MUST preceed ALL fall vectors.
         if ( GetVecPairsRiseFallStrField(max_string_len, db, vecpair_ids[vec_num]) == 0 )
            {
            (*num_rise_vecs_masks_ptr)++; 
            if ( doing_rise_vectors == 0 )
               { printf("ERROR: GetChallengeBinaryVecsFromDB(): ALL Rise vectors MUST preceed ALL Fall vectors!\n"); exit(EXIT_FAILURE); }
            }
         else 
            doing_rise_vectors = 0;

         sprintf(sql_command_str, "SELECT %s FROM Vectors WHERE id = ?;", Vectors_name);
         ReadBinaryBlob(db, sql_command_str, VA_index, (*vecs1_bin_ptr)[num_challenge_vectors], num_PIs/8, 0, NULL);

// Get the vector index for the second vector from VecPairs. 
         sprintf(sql_command_str, "SELECT %s FROM VecPairs WHERE id = %d;", VP_VB_name, vecpair_ids[vec_num]);
         GetStringsDataForRow(max_string_len, db, sql_command_str, &row_strings_struct);
         GetRowResultInt(&row_strings_struct, "GetChallengeBinaryVecsFromDB()", 1, 0, VP_VB_name, &VB_index);
         FreeStringsDataForRow(&row_strings_struct);

         sprintf(sql_command_str, "SELECT %s FROM Vectors WHERE id = ?;", Vectors_name);
         ReadBinaryBlob(db, sql_command_str, VB_index, (*vecs2_bin_ptr)[num_challenge_vectors], num_PIs/8, 0, NULL);

         num_challenge_vectors++;
         }

#ifdef DEBUG
printf("\tTOTAL number of vectors fetched from database => %d: Original number %d\n\n", num_challenge_vectors, num_vecpairs); fflush(stdout);
#endif

// Return the number of vectors that have been selected and binary versions created for.
   *num_vecs_masks_ptr = num_challenge_vectors;
   
   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// Convert new created ASCII masks for challenge to binary.

void ConvertChallengeASCIIMasksToBinary(int num_POs, int num_masks_created, char **masks_asc, 
   unsigned char ***masks_bin_ptr)
   {
   int mask_num, PO_num;

// Create the base array of pointers.
   if ( (*masks_bin_ptr = (unsigned char **)malloc(sizeof(unsigned char *) * num_masks_created)) == NULL )
      { printf("ERROR: ConvertChallengeASCIIMasksToBinary(): Failed to reallocate storage for masks_bin_ptr!\n"); exit(EXIT_FAILURE); }

   for ( mask_num = 0; mask_num < num_masks_created; mask_num++ )
      {
      if ( ((*masks_bin_ptr)[mask_num] = (unsigned char *)malloc(sizeof(unsigned char) * num_POs/8)) == NULL )
         { printf("ERROR: ConvertChallengeASCIIMasksToBinary(): Failed to reallocate storage for masks_bin!\n"); exit(EXIT_FAILURE); }

// This routine takes a char (a byte), a bit value and a bit position between 0 and 7 and overwrites the bit in the byte with the new bit. 
      for ( PO_num = 0; PO_num < num_POs; PO_num++ )
         SetBitInByte(&((*masks_bin_ptr)[mask_num][PO_num/8]), masks_asc[mask_num][num_POs - PO_num - 1] == '1' ? 1 : 0, PO_num % 8);
      }

#ifdef DEBUG
printf("\tTOTAL number of masks converted to binary %d\n\n", num_masks_created); fflush(stdout);
#endif

   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// Save the binary vector pairs and masks assocated with a challenge as ASCII to a file. 

void SaveChallengeBinaryVectorsAndMasks(int max_string_len, char *outfile_vecs, char *outfile_masks, int num_PIs, 
   int num_POs, unsigned char **vecs1_bin, unsigned char **vecs2_bin, unsigned char **masks_bin, int num_vecs_masks, 
   int num_rise_vecs_masks)
   {
   char vec1_asc[num_PIs+1];
   char vec2_asc[num_PIs+1];
   char mask_asc[num_POs+1];
   int vec_num, mask_num;

   FILE *OUTFILE_VECS;
   FILE *OUTFILE_MASKS;

// Open up output file and write vectors. 
   printf("\tWriting selected vectors to '%s'\n", outfile_vecs); fflush(stdout);
   if ( (OUTFILE_VECS = fopen(outfile_vecs, "w")) == NULL )
      { printf("ERROR: SaveChallengeVectorsAndMasks(): Data file '%s' open failed for writing!\n", outfile_vecs); exit(EXIT_FAILURE); }

// Convert binary vectors to ASCII.
   for ( vec_num = 0; vec_num < num_vecs_masks; vec_num++ )
      {
      ConvertBinVecMaskToASCII(num_PIs, vecs1_bin[vec_num], vec1_asc);
      ConvertBinVecMaskToASCII(num_PIs, vecs2_bin[vec_num], vec2_asc);
      fprintf(OUTFILE_VECS, "%s\n%s\n\n", vec1_asc, vec2_asc);
      }
   fclose(OUTFILE_VECS);

// Write ASCII masks to file 
   printf("\tWriting masks to '%s'\n\n", outfile_masks); fflush(stdout);
   if ( (OUTFILE_MASKS = fopen(outfile_masks, "w")) == NULL )
      { printf("ERROR: SaveChallengeVectorsAndMasks(): Data file '%s' open failed for writing!\n", outfile_masks); exit(EXIT_FAILURE); }

// Convert binary masks to ASCII. 
   for ( mask_num = 0; mask_num < num_vecs_masks; mask_num++ )
      {
      ConvertBinVecMaskToASCII(num_POs, masks_bin[mask_num], mask_asc);
      fprintf(OUTFILE_MASKS, "%s\n", mask_asc);
      }

   fclose(OUTFILE_MASKS);

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Callback for optimized TimingVal retrieval.

int SQL_GetTimingValsOpt_callback(void *UserSpecifiedData, int argc, char **argv, char **azColName)
   {
   float *float_val_ptr = (float *)UserSpecifiedData;

// Sanity check. Make sure this callback was called ONLY ONCE.
   if ( *float_val_ptr != -50000.0 )
      { printf("ERROR: SQL_GetTimingValsOpt_callback(): More than 1 row matched in table!\n"); exit(EXIT_FAILURE); }

   if ( sscanf(argv[0], "%f", float_val_ptr) != 1 )
      { printf("ERROR: SQL_GetTimingValsOpt_callback(): Failed to Ave value %s!\n", argv[0]); exit(EXIT_FAILURE); }

// Divide the database stored integer value (returned in argv[0]) by 16 to make it a FIXED POINT value before returning.
   *float_val_ptr /= 16.0;

   return 0;
   }


// ===========================================================================================================
// ===========================================================================================================
// This routine fetches the the timing data for a PUFInstance given by the index parameter. The timing data
// is stored in a dynamically allocated array in the order given by the VecPairPO structure. Each element
// of this structure contains a vecpair-PO combination. Note that vecpair is repeated for multiple PO as
// dictated by the challenge.

void GetPUFInstanceTimingInfoUsingVecPairPOStruct(int max_string_len, sqlite3 *db, int PUF_instance_index, int timing_or_tsig,
   VecPairPOStruct *vecpair_id_PO, int num_VPPO_eles, int allocate_float_arrs, float **PNR_TSig_ptr, float **PNF_TSig_ptr,
   TimingValCacheStruct *TVC_arr, int num_TVC_arr, int use_TVC_cache, int TVC_chip_num)
   {
   int vppo_num, num_rise_PNs, num_fall_PNs, rise_fall_vec, doing_rise_PNs;
   int TVC_arr_num;

// Illegal combo
   if ( timing_or_tsig == 1 && use_TVC_cache == 1 )
      { 
      printf("ERROR: GetPUFInstanceTimingInfoUsingVecPairPOStruct(): Fetching Tsig with cache enabled is not supported! Disable cache or add code to handle this\n"); 
      exit(EXIT_FAILURE); 
      }

#ifdef DEBUG
printf("Getting %d PNR/PNF (or tsig) data for PUF_instance_index %d\n", num_VPPO_eles, PUF_instance_index); fflush(stdout);
#endif

// Allocate storage for results. ASSUME half of the values are PNR and half are PNF.
   if ( allocate_float_arrs == 1 )
      {
      if ( (*PNR_TSig_ptr = (float *)malloc(sizeof(float) * num_VPPO_eles/2)) == NULL )
         { printf("ERROR: GetPUFInstanceTimingInfoUsingVecPairPOStruct(): Failed to allocate storage PNR_TSig_ptr pointer!\n"); exit(EXIT_FAILURE); }
      if ( (*PNF_TSig_ptr = (float *)malloc(sizeof(float) * num_VPPO_eles/2)) == NULL )
         { printf("ERROR: GetPUFInstanceTimingInfoUsingVecPairPOStruct(): Failed to allocate storage PNF_TSig_ptr pointer!\n"); exit(EXIT_FAILURE); }
      }

// Get one timing value for each element in the stucture.
   num_rise_PNs = 0;
   num_fall_PNs = 0;
   doing_rise_PNs = 1;
   TVC_arr_num = 0;
   for ( vppo_num = 0; vppo_num < num_VPPO_eles; vppo_num++ )
      {

// This is true when all data is to be derived from the database directly (and not the PN cache).
      if ( use_TVC_cache == 0 )
         {
// Get rise_fall status of vecpair_id. Note that GetChallengeBinaryVecsFromDB above already checked that all rise vectors preceed all fall vectors.
         if ( (rise_fall_vec = GetVecPairsRiseFallStrField(max_string_len, db, vecpair_id_PO[vppo_num].vecpair_id)) == 0 )
            {
            num_rise_PNs++;

// Sanity check
            if ( doing_rise_PNs == 0 )
               { printf("ERROR: GetPUFInstanceTimingInfoUsingVecPairPOStruct(): ALL Rise PNS MUST preceed ALL Fall PNS!\n"); exit(EXIT_FAILURE); }
            }
         else
            {
            num_fall_PNs++;
            doing_rise_PNs = 0;
            }

// Sanity check
         if ( num_rise_PNs > num_VPPO_eles/2 || num_fall_PNs > num_VPPO_eles/2 )
            { 
            printf("ERROR: GetPUFInstanceTimingInfoUsingVecPairPOStruct(): Number of rise PNs %d or fall PNs %d larger than expected %d!\n", num_rise_PNs, num_fall_PNs, num_VPPO_eles/2); 
            exit(EXIT_FAILURE); 
            }

// Get timing value or three sig from database. Split into two arrays.
         if ( timing_or_tsig == 0 )
            {

// Tried a couple things here to speed up the direct database access method but none of my attempts resulted in any speedup. Returning all timing values 
// associated with a vector pair using GetAllocateListOfFloats also isn't going to work since we would then need to select a small subset from those returned
// (see bckup/extra).
            char sql_command_str[max_string_len];
            char *zErrMsg = 0;
            float ave_val;
            int fc;

// For error check in callback.
            ave_val = -50000.0;
            sprintf(sql_command_str, "SELECT Ave FROM TimingVals WHERE PUFInstance = %d AND VecPair = %d AND PO = %d;", 
               PUF_instance_index, vecpair_id_PO[vppo_num].vecpair_id, vecpair_id_PO[vppo_num].PO_num);
            fc = sqlite3_exec(db, sql_command_str, SQL_GetTimingValsOpt_callback, &ave_val, &zErrMsg);
            if ( fc != SQLITE_OK )
               { printf("SQL ERROR: %s\n", zErrMsg); sqlite3_free(zErrMsg); exit(EXIT_FAILURE); }

            if ( doing_rise_PNs == 1 )
               (*PNR_TSig_ptr)[num_rise_PNs - 1] = ave_val;
            else
               (*PNF_TSig_ptr)[num_fall_PNs - 1] = ave_val;

#ifdef DEBUG
printf("PI %d\tVP %d\tPO %d\tAve %f\n", PUF_instance_index, vecpair_id_PO[vppo_num].vecpair_id, vecpair_id_PO[vppo_num].PO_num, ave_val);
#endif

// Original method
//         if ( doing_rise_PNs == 1 )
//            (*PNR_TSig_ptr)[num_rise_PNs - 1] = GetTimingValsAveField(max_string_len, db, PUF_instance_index, vecpair_id_PO[vppo_num].vecpair_id, 
//               vecpair_id_PO[vppo_num].PO_num);
//         else
//            (*PNF_TSig_ptr)[num_fall_PNs - 1] = GetTimingValsAveField(max_string_len, db, PUF_instance_index, vecpair_id_PO[vppo_num].vecpair_id, 
//               vecpair_id_PO[vppo_num].PO_num);
            }

// Three Sig values
         else
            {
            if ( doing_rise_PNs == 1 )
               (*PNR_TSig_ptr)[num_rise_PNs - 1] = GetTimingValsTSigField(max_string_len, db, PUF_instance_index, vecpair_id_PO[vppo_num].vecpair_id, 
                  vecpair_id_PO[vppo_num].PO_num);
            else
               (*PNF_TSig_ptr)[num_fall_PNs - 1] = GetTimingValsTSigField(max_string_len, db, PUF_instance_index, vecpair_id_PO[vppo_num].vecpair_id, 
                  vecpair_id_PO[vppo_num].PO_num);
            }
         }

// The data stored in the TVC_arr is a super-set of the 2048 rise and 2048 fall PNs that are identifed by the vecpair_id_PO elements as participating
// in the current challenge. The data in both arrays is ordered in the same fashion, from vecpair_id and then PO in ascending order. So we just need
// to keep moving forward in the TVC_arr until we find the next element, i.e., we don't need to start the search from the beginning of the TVC_arr
// on every iteration of this loop.
      else
         {

// Increment TVC_arr_num forward until we find a match between vecpair_ids and PO_nums in the two data structures.
         while ( TVC_arr_num < num_TVC_arr && 
            !(TVC_arr[TVC_arr_num].vecpair_id == vecpair_id_PO[vppo_num].vecpair_id && TVC_arr[TVC_arr_num].PO_num == vecpair_id_PO[vppo_num].PO_num) )
            TVC_arr_num++;

// Program error if this occurs.
         if ( TVC_arr_num == num_TVC_arr )
            { 
            printf("PROGRAM ERROR: GetPUFInstanceTimingInfoUsingVecPairPOStruct(): Failed to find vecpair_id %d and PO_num %d in TVC_arr (cache)!\n",
               vecpair_id_PO[vppo_num].vecpair_id, vecpair_id_PO[vppo_num].PO_num); 
            exit(EXIT_FAILURE); 
            }

// Split the values into rise and fall timing value arrays.
         if ( TVC_arr[TVC_arr_num].rise_or_fall == 0 )
            {
            num_rise_PNs++;

// Sanity check
            if ( doing_rise_PNs == 0 )
               { printf("ERROR: GetPUFInstanceTimingInfoUsingVecPairPOStruct(): ALL Rise PNS MUST preceed ALL Fall PNS!\n"); exit(EXIT_FAILURE); }
            }
         else
            {
            num_fall_PNs++;
            doing_rise_PNs = 0;
            }

// Sanity check
         if ( num_rise_PNs > num_VPPO_eles/2 || num_fall_PNs > num_VPPO_eles/2 )
            { 
            printf("ERROR: GetPUFInstanceTimingInfoUsingVecPairPOStruct(): Number of rise PNs %d or fall PNs %d larger than expected %d!\n", num_rise_PNs, num_fall_PNs, num_VPPO_eles/2); 
            exit(EXIT_FAILURE); 
            }

// Transfer the timing value to the output arrays. NOTE: PUF_instance_index IS NOT a zero based index (counter).
         if ( doing_rise_PNs == 1 )
            (*PNR_TSig_ptr)[num_rise_PNs - 1] = TVC_arr[TVC_arr_num].PNs[TVC_chip_num];
         else
            (*PNF_TSig_ptr)[num_fall_PNs - 1] = TVC_arr[TVC_arr_num].PNs[TVC_chip_num];
         }
      }

#ifdef DEBUG
printf("GetPUFInstanceTimingInfoUsingVecPairPOStruct(): DONE\n"); fflush(stdout);
#endif

// Sanity check
   if ( num_rise_PNs != num_VPPO_eles/2 || num_fall_PNs != num_VPPO_eles/2 )
      { 
      printf("ERROR: GetPUFInstanceTimingInfoUsingVecPairPOStruct(): Number of rise PNs %d or fall PNs %d not equal to expected %d!\n", num_rise_PNs, num_fall_PNs, num_VPPO_eles/2); 
      exit(EXIT_FAILURE); 
      }

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Get a subset of the timing data for all (or a subset) of PUFInstances. The specific timing values are 
// identified by an array of challenge_vecpair_id_PO_arr structures with (vecpair, PO) elements. These
// are constructed by GenChallengeDB as the random challenge is generated and are guaranteed to match
// the PN tested by these challenge vectors/masks.

void GetAllPUFInstanceTimingValsForChallenge(int max_string_len, sqlite3 *db, VecPairPOStruct *challenge_vecpair_id_PO_arr, 
   int num_challenge_vecpair_id_PO, char *PUF_instance_name_to_match, float ***PNR_ptr, float ***PNF_ptr, int *num_chips_ptr,
   TimingValCacheStruct *TVC_arr, int num_TVC_arr, int use_TVC_cache)
   {
   SQLIntStruct PUF_instance_index_struct;
   int chip_num;

// We need to do this to compute population offsets below. Get timing data for all PUFInstances (or a subset).
// First get the a list of PUFInstance IDs that match the string 'PUF_instance_name_to_match', which can be '%' to
// match all. Use '%' for * and '_' for ?
   GetPUFInstanceIDsForInstanceName(max_string_len, db, &PUF_instance_index_struct, PUF_instance_name_to_match);

// Sanity check
   if ( PUF_instance_index_struct.num_ints == 0 )
      { printf("ERROR: GetAllPUFInstanceTimingValsForChallenge(): No PUFInstances match search string %s!\n", PUF_instance_name_to_match); exit(EXIT_FAILURE); }

#ifdef DEBUG
printf("GetAllPUFInstanceTimingValsForChallenge(): Number of PUFInstances fetched from database %d\n", 
   PUF_instance_index_struct.num_ints); fflush(stdout);
#endif

// Allocate arrays to add the new dynamically allocated subarrays, one pointer for each PUFInstance (chip).
   if ( (*PNR_ptr = (float **)malloc(sizeof(float *) * PUF_instance_index_struct.num_ints)) == NULL )
      { printf("ERROR: GetAllPUFInstanceTimingValsForChallenge(): Failed to allocate storage for PNR!\n"); exit(EXIT_FAILURE); }
   if ( (*PNF_ptr = (float **)malloc(sizeof(float *) * PUF_instance_index_struct.num_ints)) == NULL )
      { printf("ERROR: GetAllPUFInstanceTimingValsForChallenge(): Failed to allocate storage for PNF!\n"); exit(EXIT_FAILURE); }

struct timeval t0, t1;
long elapsed; 
gettimeofday(&t0, 0);
#ifdef DEBUG
#endif

// Get dynamically allocated arrays, one for each PUF instance and add to PNR and PNF arrays.
   for ( chip_num = 0; chip_num < PUF_instance_index_struct.num_ints; chip_num++ )
      GetPUFInstanceTimingInfoUsingVecPairPOStruct(max_string_len, db, PUF_instance_index_struct.int_arr[chip_num],
         0, challenge_vecpair_id_PO_arr, num_challenge_vecpair_id_PO, 1, &((*PNR_ptr)[chip_num]), &((*PNF_ptr)[chip_num]),
         TVC_arr, num_TVC_arr, use_TVC_cache, chip_num);
         
#ifdef DEBUG
printf("HERE\n");
int i;
for ( i = 0; i < 10; i++ )
   printf("\tTIMING VAL %f\n", (*PNR_ptr)[0][i]);
#endif

gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tElapsed: Time to get PNs: %ld us\n\n", (long)elapsed);
#ifdef DEBUG
#endif

#ifdef DEBUG
printf("GetAllPUFInstanceTimingValsForChallenge(): Fetched %d PNR and PNF TimingVals for all PUFInstances\n", 
   num_challenge_vecpair_id_PO/2); fflush(stdout);
#endif

// Return the number of timing data sets fetched from the database.
   *num_chips_ptr = PUF_instance_index_struct.num_ints;

   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// This routine generates additional, randomly selected challenge sets from special challenges added by add_challengeDB
// that use '0', '1', 'u' and 'q' designators. '0' means output has no transition, '1' means we MUST use that path,
// 'u' (unqualifying) means the path has a transition but is not 'compatible' with the elements that are marked 'q' 
// (for qualifying).
//
// In this routine, we assume the challenge set specified on the command line exists already in the database (added
// by add_challengeDB.c). It MUST be a challenge set that has 'u' and 'q' designators otherwise it is fully 
// specified by add_challengeDB.c and there is nothing we can do here to create additional challenges.
//
// It returns a set of binary vectors and masks as well as a data structure that allows the enrollment timing values 
// that are tested by these vectors to be looked up by the caller, who can call GetPUFInstanceTimingInfoUsingVecPairPOStruct 
// defined above.

int GenChallengeDB(int max_string_len, sqlite3 *db, int design_index, char *ChallengeSetName, unsigned int Seed, int save_vecs_masks, 
   char *outfile_vecs, char *outfile_masks, unsigned char ***vecs1_bin_ptr, unsigned char ***vecs2_bin_ptr, 
   unsigned char ***masks_bin_ptr, int *num_vecs_masks_ptr, int *num_rise_vecs_masks_ptr, pthread_mutex_t *GenChallenge_mutex_ptr,
   int *num_challenge_vecpair_id_PO_ptr, VecPairPOStruct **challenge_vecpair_id_PO_ptr)
   {
   int challenge_index;

   int num_vecpairs, num_rising_vecpairs, num_falling_vecpairs; 
   int num_tested_PNs, num_rise_tested_PNs, num_fall_tested_PNs;
   int num_qualified_PNs, num_rise_qualified_PNs, num_fall_qualified_PNs;
   int num_PIs, num_POs;

   PathInfoStruct *tested_path_info = NULL;
   PathInfoStruct *qualified_path_info = NULL;

   int *rise_indexes1, *fall_indexes1;
   int *rise_indexes2, *fall_indexes2;
//   int optvec_succeed; 

   int NUM_QUAL_PATH_LOWER_BOUND, FRACTION_TO_SELECT_LOWER_BOUND, FRACTION_NUM_QUAL_PATH_LOWER_BOUND;

   int *vecpair_is_selected = NULL;
   int *vecpair_ids = NULL;
   int num_masks_created = 0;
   char **masks_asc = NULL;

   int vec_num, sel_vec_num, rec_num, PO_num, num_rise_PNs, rise_fall_vec; 

#ifdef DEBUG
struct timeval t1, t2, t3;
long elapsed; 
#endif

#ifdef DEBUG
gettimeofday(&t3, 0);
#endif

#ifdef DEBUG
printf("\nGenChallengeDB():\tPUFDesign index %d\tSeed %u\n\n", design_index, Seed); fflush(stdout);
#endif

// ===========================================================================================================
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! PARAMETERS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ===========================================================================================================
// One of the PN selection routines implemented in SelectRandomSubset attempts to minimize the number of vector
// pairs that are used to generate the 4096 PN required to generate a bitstring or key. This routine uses a 
// database challenge that has special masks that 'qualify' paths with a 'q'. These paths are identified
// in the TVCharacterization routine as compatible. The special masks used by these special challenges in the
// database identify a larger set of PNs (more than 4096) with 'q's (otherwise, SelectRandomSubset fails). So
// the goal of SelectRandomSubset is to select a subset of 4096 PNs (2048 rise and 2048 fall) by using only the
// mask-encoded 'q's. 
//
// We implement two separate algorithms in SelectRandomSubset, one that simply randomly selects 2048 rise and 
// 2048 fall with no regard to which vector pairs the 'q's are included in (BruteForce algorithm), and a second
// routine that attempts to select random set of 'q's while minimizing the number of vector pairs (OptVec). Keeping 
// the number of vector pairs small speeds up the bitstring algorithm because most of the time is associated
// with measuring the path delays. If fewer vectors need to be processed to generate the bitstring/key, then fewer 
// number of timing operations need to be carried out. These parameters relates to this second vector-pair-optimizing 
// algorithm.

// This parameter is used to set a firm lower bound on the number of 'q's for any given vector-pair-mask. Masks
// with fewer than this number are skipped, and so is the corresponding vector pair. Setting this too large
// makes it difficult for the algorithm to find 2048 rise and 2048 fall PNs, and may cause this algorithm to
// fail (in which case, the vector pairs selected by the brute force algorithm is used). Increasing this reduces
// the number of vector pairs that are selected (which speeds up the HELP algorithm), as long as the OptVec
// is successful in finding 2048 rise and 2048 fall PNs.
   NUM_QUAL_PATH_LOWER_BOUND = 4;

// This parameter sets a lower bound on the fraction, e.g., 70% when 70 is specified, of 'q's that are selected
// from a mask with more than 'NUM_QUAL_PATH_LOWER_BOUND' 'q's. This parameter MUST be between 0 and 100. Setting
// this smaller, allows FEWER 'q's to be randomly select from among those that exist in each mask. This enables
// more diversity among the set of PN selected when this routine is run over and over again for different bitstring
// generations. But it also makes it harder for the algorithm to find 2048 rise and 2048 fall PN because, e.g., a
// mask that has 10 'q's (10 compatible PN) and say this is set to 10 (10%), then the random number generator may
// generate the lower bound of 10% and only allow 1 of the 10 PN to be used from this vector. Setting it to 50
// restricts the random number generator to generate a random percentage between 50% and 100%, forcing at least
// 5 of the PN to be used (as a lower bound).
   FRACTION_TO_SELECT_LOWER_BOUND = 70;

// If you use a small number for 'FRACTION_TO_SELECT_LOWER_BOUND', then you can discard masks where the number of
// actual PN selected (based on the percentage) is less than this absolute value. Setting is to a smaller value
// allows masks (vector pairs) to be used that have a smaller number of tested paths (PN) that are actually used.
// Setting it larger will force each vector pair to test at least this number of PN. Note that masks/vector pairs 
// that are eliminated because the random number generates too small a number using 'FRACTION_TO_SELECT_LOWER_BOUND' 
// may be tried again later -- see the algorithm (the vector pair is NOT eliminated as is true for violating the 
// above two parameters). Setting this too large may cause the algorithm to run a very long time so care must be 
// taken here.
   FRACTION_NUM_QUAL_PATH_LOWER_BOUND = 3;

// ===========================================================================================================

#ifdef DEBUG
gettimeofday(&t2, 0);
#endif

// Get the PUFDesign informatiion. IT MUST ALREADY exist assumption here is the enrollDB has already been run.
   GetPUFDesignNumPIPOFields(max_string_len, db, &num_PIs, &num_POs, design_index);

#ifdef DEBUG
printf("\nPUFDesign index %d\tNum PIs %d\tNum POs %d\n", design_index, num_PIs, num_POs); fflush(stdout);
#endif

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tELAPSED TIME: Design info lookup %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
#endif

   GetChallengeParams(max_string_len, db, ChallengeSetName, &challenge_index, &num_vecpairs, &num_rising_vecpairs, &num_qualified_PNs, 
      &num_rise_qualified_PNs);

// Compute falling number of vecpairs and PNs from returned database parameters.
   num_falling_vecpairs = num_vecpairs - num_rising_vecpairs;
   num_fall_qualified_PNs = num_qualified_PNs - num_rise_qualified_PNs;

#ifdef DEBUG
printf("\tChallenge '%s' with index %d has num_vecpairs %d, num_rising_vecpairs %d, num_qualified_PNs %d and num_rise_qualified_PNs %d\n", 
   ChallengeSetName, challenge_index, num_vecpairs, num_rising_vecpairs, num_qualified_PNs, num_rise_qualified_PNs); fflush(stdout);
#endif

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tELAPSED TIME: Challenge info lookup %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
#endif

// ===========================================================================================================
// Find qualifying paths from the 'q' in the masks associated with the Challenge. The tested_path_info array records information about ALL paths 
// tested by the challenge vector pairs (both 'u' and 'q'). The PN marked 'q' in the masks are copied into the 'qualified_path_info' structure, 
// and the number of them are returned in the two parameters. 'vecpair_ids' is dynamically allocated and filled in with the VecPair id for the 
// VecPairs table. It will be used below to assign/get the vectors for the newly created challenge.
#ifdef DEBUG
printf("Getting qualified paths\n");
#endif
   FindQualifyingPaths(max_string_len, db, &tested_path_info, num_rising_vecpairs, num_falling_vecpairs, &num_rise_tested_PNs, &num_fall_tested_PNs, 
      num_POs, NUM_RISE_REQUIRED_PNS, NUM_FALL_REQUIRED_PNS, num_rise_qualified_PNs, num_fall_qualified_PNs, &qualified_path_info, challenge_index, 
      &vecpair_ids);
   num_tested_PNs = num_rise_tested_PNs + num_fall_tested_PNs;

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tELAPSED TIME: Find qualifying paths %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
#endif

// Allocated storage for the results for the two algorithms in SelectRandomSubset.
   if ( (rise_indexes1 = (int *)malloc(sizeof(int)*NUM_RISE_REQUIRED_PNS)) == NULL )
      { printf("ERROR: GenChallengeDB(): Failed to allocate storage for rise_indexes1 array!\n"); exit(EXIT_FAILURE); }
   if ( (fall_indexes1 = (int *)malloc(sizeof(int)*NUM_FALL_REQUIRED_PNS)) == NULL )
      { printf("ERROR: GenChallengeDB(): Failed to allocate storage for fall_indexes1 array!\n"); exit(EXIT_FAILURE); }
   if ( (rise_indexes2 = (int *)malloc(sizeof(int)*NUM_RISE_REQUIRED_PNS)) == NULL )
      { printf("ERROR: GenChallengeDB(): Failed to allocate storage for rise_indexes2 array!\n"); exit(EXIT_FAILURE); }
   if ( (fall_indexes2 = (int *)malloc(sizeof(int)*NUM_FALL_REQUIRED_PNS)) == NULL )
      { printf("ERROR: GenChallengeDB(): Failed to allocate storage for fall_indexes2 array!\n"); exit(EXIT_FAILURE); }

// Randomly select a subset of 'NUM_XXX_REQUIRED_PNS' from those that are 'qualified' using a Seed parameter. Allocate storage for the indexes. We use 
// two algorithms, BruteForce and OptVec. Brute force results stored in rise/fall_indexes1 while OptVec in rise/fall_indexes2. Not that OptVec can fail. 
// Best results (smallest set of vector pairs) are indicated by optvec_succeed return value (not used currently b/c SelectRandomSubset sets 'path_selected' 
// field in tested_path_info array elements before returning so we know which tested_path_info are going to be used). 
//   optvec_succeed = SelectRandomSubset(max_string_len, Seed, num_rise_qualified_PNs, num_fall_qualified_PNs, qualified_path_info, NUM_RISE_REQUIRED_PNS, 

#ifdef DEBUG
printf("Setting ChallengeDB mutex!\n");
#endif

// 10_31_2021: We are now using a seed to specify the vector sequence on the device, TTP and verifier. When challenges are selected, we depend
// on the sequence returned by rand() to be the same no matter where this routine runs, device, TT or verifier. The verifier and TTP are multi-threaded
// and therefore it is possible that multiple threads call this routine simultaneously, interrupting the sequence generated by rand() (rand is NOT re-entrant).
// If this occurs, then the vector challenges used by the, e.g., verifier and device will be different and the security function will fail. When the device
// calls this function, the mutex is NULL. This function is the only place where rand() is called.
   if ( GenChallenge_mutex_ptr != NULL )
      pthread_mutex_lock(GenChallenge_mutex_ptr);

   SelectRandomSubset(max_string_len, Seed, num_rise_qualified_PNs, num_fall_qualified_PNs, qualified_path_info, NUM_RISE_REQUIRED_PNS, 
      NUM_FALL_REQUIRED_PNS, rise_indexes1, fall_indexes1, rise_indexes2, fall_indexes2, num_rising_vecpairs, num_falling_vecpairs, num_tested_PNs, tested_path_info, 
      NUM_QUAL_PATH_LOWER_BOUND, FRACTION_TO_SELECT_LOWER_BOUND, FRACTION_NUM_QUAL_PATH_LOWER_BOUND, num_POs);

   if ( GenChallenge_mutex_ptr != NULL )
      pthread_mutex_unlock(GenChallenge_mutex_ptr);

#ifdef DEBUG
printf("Releasing ChallengeDB mutex!\n");
#endif

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tELAPSED TIME: Select random subset %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
#endif

// Allocate and initialize (calloc) array that determines which vectors are selected as part of the challenge.
   if ( (vecpair_is_selected = (int *)calloc(sizeof(int), num_vecpairs)) == NULL )
      { printf("ERROR: GenChallengeDB(): Failed to allocate storage for 'vecpair_is_selected'!\n"); exit(EXIT_FAILURE); }

#ifdef DEBUG
printf("\nCreating vectors and masks: Optionally saving them to '%s' and masks '%s'\n", outfile_vecs, outfile_masks); fflush(stdout);
#endif

// Create the masks and vectors.
   CreateMasksFromQualifiedPathInfo(max_string_len, db, outfile_vecs, outfile_masks, num_tested_PNs, num_PIs, num_POs, tested_path_info, num_vecpairs, 
      NUM_REQUIRED_PNS, &masks_asc, &num_masks_created, vecpair_is_selected);

// Get the binary vectors from the database that define the challenge. Return the binary vectors in newly allocated space in vecsx_bin_ptr, along with the sizes.
   GetChallengeBinaryVecsFromDB(max_string_len, db, num_PIs, num_vecpairs, vecpair_is_selected, vecpair_ids, vecs1_bin_ptr, vecs2_bin_ptr, num_vecs_masks_ptr, 
      num_rise_vecs_masks_ptr);

// Sanity check. Number of vectors selected better equal the number of masks created.
   if ( *num_vecs_masks_ptr != num_masks_created )
      { printf("ERROR: Number of vectors and masks created are NOT equal: %d vs %d!\n", *num_vecs_masks_ptr, num_masks_created); exit(EXIT_FAILURE); }

// Convert the ASCII version of the mask to binary. 
   ConvertChallengeASCIIMasksToBinary(num_POs, num_masks_created, masks_asc, masks_bin_ptr);

// Save the vectors and masks as ASCII to files if requested.
   if ( save_vecs_masks == 1 )
      SaveChallengeBinaryVectorsAndMasks(max_string_len, outfile_vecs, outfile_masks, num_PIs, num_POs, *vecs1_bin_ptr, *vecs2_bin_ptr, *masks_bin_ptr, 
         *num_vecs_masks_ptr, *num_rise_vecs_masks_ptr);

// Create a list of records of the vecpair ids and POs so we can look up the TimingVals data for the challenge. This information plus the PUFInstance
// id will allow us to fetch timing data for authentication, etc. in the caller.
   if ( (*challenge_vecpair_id_PO_ptr = (VecPairPOStruct *)calloc(sizeof(VecPairPOStruct), NUM_REQUIRED_PNS)) == NULL )
      { printf("ERROR: GenChallengeDB(): Failed to allocate storage for 'challenge_vecpair_id_PO_ptr'!\n"); exit(EXIT_FAILURE); }

   sel_vec_num = 0;
   rec_num = 0;
   num_rise_PNs = 0;
   for ( vec_num = 0; vec_num < num_vecpairs; vec_num++ )
      if ( vecpair_is_selected[vec_num] == 1 )
         {

// Sanity check
         if ( sel_vec_num == num_masks_created )
            { printf("ERROR: GenChallengeDB(): Access to masks_asc that exceeds number available %d!\n", num_masks_created); exit(EXIT_FAILURE); }

// Get rise_fall status of vecpair_id. Note that GetChallengeBinaryVecsFromDB above already checked that all rise vectors preceed all fall vectors.
         rise_fall_vec = GetVecPairsRiseFallStrField(max_string_len, db, vecpair_ids[vec_num]);

// Loop through the ASCII mask setting POs
         for ( PO_num = 0; PO_num < num_POs; PO_num++ )
            {

// Skip POs that are not selected. High address in ASCII mask is low order bit. Load these from small PO_nums to large PO_nums.
// In other words, work right-to-left in the mask and count from 0 to num_POs - 1.
            if ( masks_asc[sel_vec_num][num_POs - PO_num - 1] == '0' )
               continue;

// Sanity check
            if ( rec_num == NUM_REQUIRED_PNS )
               { printf("ERROR: GenChallengeDB(): Number of challenge_vecpair_id_PO_ptr greater than max %d!\n", NUM_REQUIRED_PNS); exit(EXIT_FAILURE); }

// Record the vecpair_id for every PO that is selected in the mask, and the PO_num itself, that are part of the challenge.
            (*challenge_vecpair_id_PO_ptr)[rec_num].vecpair_id = vecpair_ids[vec_num];
            (*challenge_vecpair_id_PO_ptr)[rec_num].PO_num = PO_num;

// For sanity check below.
            if ( rise_fall_vec == 0 )
               num_rise_PNs++;

            rec_num++;
            }
         sel_vec_num++;
         }

// Sanity check
   if ( rec_num != NUM_REQUIRED_PNS )
      { printf("ERROR: GenChallengeDB(): Number of challenge_vecpair_id_PO_ptr %d NOT equal to expected %d!\n", rec_num, NUM_REQUIRED_PNS); exit(EXIT_FAILURE); }

// Sanity check
   if ( num_rise_PNs != NUM_RISE_REQUIRED_PNS )
      { printf("ERROR: GenChallengeDB(): Number of rise PNs %d expected to %d!\n", num_rise_PNs, NUM_RISE_REQUIRED_PNS); exit(EXIT_FAILURE); }

// Assign number of elements created. 
   *num_challenge_vecpair_id_PO_ptr = rec_num;

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tELAPSED TIME: Write/deliver vectors/masks %ld us\n\n", (long)elapsed);
fflush(stdout);
#endif

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t3.tv_sec)*1000000 + t1.tv_usec-t3.tv_usec; printf("\tTOTAL ELAPSED TIME %ld us\n\n", (long)elapsed);
fflush(stdout);
#endif

// Free array of pointers
   int mask_num;
   for ( mask_num = 0; mask_num < num_masks_created; mask_num++ )
      free(masks_asc[mask_num]);
   free(masks_asc);

   free(rise_indexes1); 
   free(fall_indexes1); 
   free(rise_indexes2); 
   free(fall_indexes2); 
   free(tested_path_info); 
   free(qualified_path_info);
   free(vecpair_ids);
   free(vecpair_is_selected); 

   return 0;
   }


// ===========================================================================================================
// ===========================================================================================================
// Given two binary vectors as input, look up their indexes in the Vectors table, and then with the design_index,
// find the VecPair index that corresponds to the design_index and binary vector pair. Return the 3 indexes.

void GetVectorAndVecPairIndexesForBinaryVectors(int max_string_len, sqlite3 *db, int design_index, int vec_len_bytes, 
   unsigned char *first_vecs_b, unsigned char *second_vecs_b, int *first_vec_index_ptr, int *second_vec_index_ptr, 
   int *vecpair_index_ptr, int vecpair_num)
   {

// Find the first vector of the vector pair in the Vector's table. If not present, error. WE MUST HAVE run enrollDB.c that recorded these
// vectors AND timing data BEFORE this routine will succeed.
   if ( (*first_vec_index_ptr = GetIndexFromTable(max_string_len, db, "Vectors", SQL_Vectors_get_index_cmd, first_vecs_b,
      vec_len_bytes, NULL, NULL, NULL, NULL, -1, -1, -1)) == -1 )
      { 
      printf("ERROR: GetVectorAndVecPairIndexesForBinaryVectors(): Failed to find first_vec_index for vec_num %d in Vectors table!\n", vecpair_num); 
      exit(EXIT_FAILURE); 
      }

// Find the second vector of the vector pair in the Vector's table. If not present, error.
   if ( (*second_vec_index_ptr = GetIndexFromTable(max_string_len, db, "Vectors", SQL_Vectors_get_index_cmd, second_vecs_b,
      vec_len_bytes, NULL, NULL, NULL, NULL, -1, -1, -1)) == -1 )
      { 
      printf("ERROR: GetVectorAndVecPairIndexesForBinaryVectors(): Failed to find second_vec_index for vec_num %d in Vectors table!\n", vecpair_num); 
      exit(EXIT_FAILURE); 
      }

#ifdef DEBUG
printf("\tFirst Vector Index %d\tSecond Vector Index %d\n", *first_vec_index_ptr, *second_vec_index_ptr); fflush(stdout);
#endif

// Now get the vecpair_index from the VecPairs table keying on the combination of two Vector id's (VecPairs does NOT allow duplicates for the
// combination (VA, VB). Note that we do NOT need PUFDesign_id here because matches to the Vectors produce a unique VecPair. Also, the rise/fall
// status of the VecPair is determined by the Vectors, so no chance of another PUFDesign_id matching on the (VA, VB) pair and being characterized
// as "F" when the existing VecPair records "R". If VecPair element not found, error.
   if ( (*vecpair_index_ptr = GetIndexFromTable(max_string_len, db, "VecPairs", SQL_VecPairs_get_index_cmd, NULL, 0, NULL, NULL, NULL, NULL, 
      *first_vec_index_ptr, *second_vec_index_ptr, design_index)) == -1 )
      {
      printf("ERROR: GetVectorAndVecPairIndexesForBinaryVectors(): Failed to find vecpair_index for vecpair_num %d in VecPairs table!\n", vecpair_num); 
      exit(EXIT_FAILURE); 
      }

   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// This routine is an alternative to GenChallengeDB. GenChallengeDB takes the name of a 'compatible' challenge,
// randomly selects a set of vector pairs and POs (via masks) and then returns them along with a data structure
// that allows the enrollment timing values that are tested by these vectors to be looked up by the caller
// via calling GetPUFInstanceTimingInfoUsingVecPairPOStruct defined above.
//
// Here, we take a set of binary vector pairs and masks as INPUT which are then used to generate the data structure 
// that allows the caller to look up the timing values. 

void GetVecPairPOStructForBinaryVecsMasks(int max_string_len, sqlite3 *db, int num_PIs, int num_POs, int design_index, 
   unsigned char **vecs1_bin, unsigned char **vecs2_bin, unsigned char **masks_bin, int num_vecs_masks, 
   int num_rise_vecs_masks, int *num_challenge_vecpair_id_PO_ptr, VecPairPOStruct **challenge_vecpair_id_PO_ptr)
   {
   int first_vec_index, second_vec_index, vecpair_index;
   int vecpair_num, num_PNs_per_vecpair, num_POs_found;
   int list_of_PO_nums[num_POs];
   int list_ele, tot_num_eles;
   char mask_asc[num_POs+1];
   int vec_len_bytes;

// Sanity check.
   if ( (num_PIs % 8) != 0 || (num_POs % 8) != 0 )
      { printf("ERROR: GetVecPairPOStructForBinaryVecsMasks(): num_PIs %d and num_POs %d MUST be a multiple of 8!\n", num_PIs, num_POs); exit(EXIT_FAILURE); }
   vec_len_bytes = num_PIs/8;

// Force realloc to behave like malloc on the first call.
   *challenge_vecpair_id_PO_ptr = NULL;
   tot_num_eles = 0;
   for ( vecpair_num = 0; vecpair_num < num_vecs_masks; vecpair_num++ )
      {

// Look up each vector in the Vectors table to get its id. For each pair of vectors, in particular, the design_index
// and the vector pair ids, search for the matching element in the VecPair table to get its index.
      GetVectorAndVecPairIndexesForBinaryVectors(max_string_len, db, design_index, vec_len_bytes, vecs1_bin[vecpair_num],
         vecs2_bin[vecpair_num], &first_vec_index, &second_vec_index, &vecpair_index, vecpair_num);

// Get the total number of PNs tested by this VecPair WITHOUT considering a mask. Used for sanity check below.
      num_PNs_per_vecpair = GetVecPairsNumPNsField(max_string_len, db, vecpair_index);

// Convert each masks_bin to ASCII.
      ConvertBinVecMaskToASCII(num_POs, masks_bin[vecpair_num], mask_asc);

// Get the list of POs that are tested by the '1' bits in the mask. The list is given in low order PO to high order PO. 
      GetSelectedPOsInMask(mask_asc, num_POs, list_of_PO_nums, &num_POs_found, 1);

// Sanity check
      if ( num_POs_found > num_PNs_per_vecpair || num_PNs_per_vecpair > num_POs )
         { 
         printf("ERROR: GetVecPairPOStructForBinaryVecsMasks(): num of tested POs %d larger than those available in vector %d or larger than max %d!\n", 
            num_POs_found, num_PNs_per_vecpair, num_POs); exit(EXIT_FAILURE); 
         }

// Add elements to the challenge_vecpair_id_PO structure that gives the vecpair index and PO. Note that the vecpair index is 
// likely repeated in multiple elements of the data structure because vecpairs typically generate timing data at multiple POs.
      for ( list_ele = 0; list_ele < num_POs_found; list_ele++ )
         {

// Reallocate another element. NOTE: this is likely to always be 'num_required_PNs' in size but not checking this here
// makes this routine more generic.
         if ( (*challenge_vecpair_id_PO_ptr = (VecPairPOStruct *)realloc(*challenge_vecpair_id_PO_ptr, sizeof(VecPairPOStruct) * (tot_num_eles + 1))) == NULL )
            { 
            printf("ERROR: GetVecPairPOStructForBinaryVecsMasks(): Failed to reallocate storage for 'challenge_vecpair_id_PO_ptr'!\n"); 
            exit(EXIT_FAILURE); 
            }
         (*challenge_vecpair_id_PO_ptr)[tot_num_eles].vecpair_id = vecpair_index;
         (*challenge_vecpair_id_PO_ptr)[tot_num_eles].PO_num = list_of_PO_nums[list_ele];

         tot_num_eles++;
         }
      }

   *num_challenge_vecpair_id_PO_ptr = tot_num_eles;

   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// This routine does what GenChallengeDB does initially, i.e., find all VecPairs that are 'qualified' by the
// vectors and masks stored for the ChallengeSetName. It then retrieves all the TimingVal data for all PUFInstances
// into an array for fast parsing by GetPUFInstanceTimingInfoUsingVecPairPOStruct routine, which appears to be
// the bottleneck to runtime performance of the protocol (takes about 2.3 seconds if the data is retrieved directly
// from the database).

int CreateTimingValsCacheFromChallengeSet(int max_string_len, sqlite3 *db, int design_index, char *ChallengeSetName, 
   char *PUF_instance_name_to_match, TimingValCacheStruct **TVC_arr_ptr, int *num_TVC_arr_ptr) 
   {
   int challenge_index;

   int num_vecpairs, num_rising_vecpairs, num_falling_vecpairs; 
   int num_qualified_PNs, num_rise_qualified_PNs, num_fall_qualified_PNs;
   int num_PIs, num_POs;

   int num_rise_tested_PNs, num_fall_tested_PNs;

   int *vecpair_ids = NULL;

   PathInfoStruct *tested_path_info = NULL;
   PathInfoStruct *qualified_path_info = NULL;

   SQLIntStruct PUF_instance_index_struct;

   int qPN_num, chip_num; 

#ifdef DEBUG
struct timeval t1, t2;
long elapsed; 
#endif

#ifdef DEBUG
gettimeofday(&t2, 0);
#endif

// Get the PUFDesign informatiion. IT MUST ALREADY exist assumption here is the enrollDB has already been run.
   GetPUFDesignNumPIPOFields(max_string_len, db, &num_PIs, &num_POs, design_index);

#ifdef DEBUG
printf("\nCreateTimingValsCacheFromChallengeSet(): PUFDesign index %d\tNum PIs %d\tNum POs %d\n", design_index, num_PIs, num_POs); fflush(stdout);
#endif

   GetChallengeParams(max_string_len, db, ChallengeSetName, &challenge_index, &num_vecpairs, &num_rising_vecpairs, &num_qualified_PNs, 
      &num_rise_qualified_PNs);

// Compute falling number of vecpairs and PNs from returned database parameters.
   num_falling_vecpairs = num_vecpairs - num_rising_vecpairs;
   num_fall_qualified_PNs = num_qualified_PNs - num_rise_qualified_PNs;

#ifdef DEBUG
printf("\tCreateTimingValsCacheFromChallengeSet(): Challenge '%s' with index %d has num_vecpairs %d, num_rising_vecpairs %d, num_qualified_PNs %d and num_rise_qualified_PNs %d\n", 
   ChallengeSetName, challenge_index, num_vecpairs, num_rising_vecpairs, num_qualified_PNs, num_rise_qualified_PNs); fflush(stdout);
#endif

// Find qualifying paths from the 'q' in the masks associated with the Challenge. The tested_path_info array records information about ALL paths 
// tested by the challenge vector pairs (both 'u' and 'q'). The PN marked 'q' in the masks are copied into the 'qualified_path_info' structure, 
// and the number of them are returned in the two parameters. 'vecpair_ids' is dynamically allocated and filled in with the VecPair id for the 
// VecPairs table. It will be used below to assign/get the vectors for the newly created challenge.
#ifdef DEBUG
printf("\tGetting qualified paths\n\n");
#endif

   FindQualifyingPaths(max_string_len, db, &tested_path_info, num_rising_vecpairs, num_falling_vecpairs, &num_rise_tested_PNs, &num_fall_tested_PNs, 
      num_POs, NUM_RISE_REQUIRED_PNS, NUM_FALL_REQUIRED_PNS, num_rise_qualified_PNs, num_fall_qualified_PNs, &qualified_path_info, challenge_index, 
      &vecpair_ids);

// Create the timing val cache structure, one element for each qualified PN.
   if ( (*TVC_arr_ptr = (TimingValCacheStruct *)malloc(sizeof(TimingValCacheStruct) * num_qualified_PNs)) == NULL )
      { printf("ERROR: CreateTimingValsCacheFromChallengeSet(): Failed to allocate storage for TVC structure array!\n"); exit(EXIT_FAILURE); }
   
// Get the a list of PUFInstance IDs that match the string 'PUF_instance_name_to_match', which can be '%' to match all. Note: The cache stores data
// in the order of the PUFInstance ID returned by this routine.
   GetPUFInstanceIDsForInstanceName(max_string_len, db, &PUF_instance_index_struct, PUF_instance_name_to_match);

// Sanity check
   if ( PUF_instance_index_struct.num_ints == 0 )
      { printf("ERROR: CreateTimingValsCacheFromChallengeSet(): No PUFInstances match search string %s!\n", PUF_instance_name_to_match); exit(EXIT_FAILURE); }

#ifdef DEBUG
printf("CreateTimingValsCacheFromChallengeSet(): Number of PUFInstances fetched from database %d\n", 
   PUF_instance_index_struct.num_ints); fflush(stdout);
#endif

// For each vecpair, allocate an array of floats, one float for each chip.
   for ( qPN_num = 0; qPN_num < num_qualified_PNs; qPN_num++ )
      if ( ((*TVC_arr_ptr)[qPN_num].PNs = (float *)malloc(sizeof(float) * PUF_instance_index_struct.num_ints)) == NULL )
         { printf("ERROR: CreateTimingValsCacheFromChallengeSet(): Failed to allocate storage for PNR!\n"); exit(EXIT_FAILURE); }

// Store information in the TVC array that allows us to get subsets of this data very quickly in GetPUFInstanceTimingInfoUsingVecPairPOStruct by
// parsing this array from top-to-bottom in vecpair_id followed by PO order, both low-to-high.
   for ( qPN_num = 0; qPN_num < num_qualified_PNs; qPN_num++ )
      {

if ( ((qPN_num + 1) % 100) == 0 )
printf("CreateTimingValsCacheFromChallengeSet(): Reading %d PNR/PNF from DB of %d\n", qPN_num, num_qualified_PNs); 
fflush(stdout);
#ifdef DEBUG
#endif

// FindQualifyingPaths creates one vecpair_id for each vector pair that is part of the challenge set. It actual vecpair id is found using the vecpair_num
// field of the qualified_path_info structure.
      (*TVC_arr_ptr)[qPN_num].vecpair_id = vecpair_ids[qualified_path_info[qPN_num].vecpair_num];
      (*TVC_arr_ptr)[qPN_num].PO_num = qualified_path_info[qPN_num].PO_num;
      (*TVC_arr_ptr)[qPN_num].rise_or_fall = qualified_path_info[qPN_num].rise_or_fall;
      for ( chip_num = 0; chip_num < PUF_instance_index_struct.num_ints; chip_num++ )
         {
         char sql_command_str[max_string_len];
         char *zErrMsg = 0;
         float ave_val;
         int fc;

// Look up the timing value for this PUF instance. This is the slow operation that we do ONLY once at the beginning of the protocol run
// for a given ChallengeSetName. NOTE: THIS ROUTINE SCALES the FIXED POINT data in the database by dividing by 16 to create a floating
// point value from the stored integer before returning.
//         (*TVC_arr_ptr)[qPN_num].PNs[chip_num] = GetTimingValsAveField(max_string_len, db, PUF_instance_index_struct.int_arr[chip_num], 
//            (*TVC_arr_ptr)[qPN_num].vecpair_id, (*TVC_arr_ptr)[qPN_num].PO_num);

         ave_val = -50000.0;
         sprintf(sql_command_str, "SELECT Ave FROM TimingVals WHERE PUFInstance = %d AND VecPair = %d AND PO = %d;",
            PUF_instance_index_struct.int_arr[chip_num], (*TVC_arr_ptr)[qPN_num].vecpair_id, (*TVC_arr_ptr)[qPN_num].PO_num);
         fc = sqlite3_exec(db, sql_command_str, SQL_GetTimingValsOpt_callback, &ave_val, &zErrMsg);
         if ( fc != SQLITE_OK )
            { printf("SQL ERROR: %s\n", zErrMsg); sqlite3_free(zErrMsg); exit(EXIT_FAILURE); }
         (*TVC_arr_ptr)[qPN_num].PNs[chip_num] = ave_val;
         }
      }

// Return the size of the array of TVC structures for sanity checks.
   *num_TVC_arr_ptr = num_qualified_PNs;

   free(tested_path_info); 
   free(qualified_path_info);
   free(vecpair_ids);

// Free up integer array that holds PUFInstanceIDs.
   if ( PUF_instance_index_struct.int_arr != NULL )
      free(PUF_instance_index_struct.int_arr);

printf("\n\nCreated PN cache with %d values for each of %d chips\n\n", *num_TVC_arr_ptr, PUF_instance_index_struct.num_ints); fflush(stdout);
#ifdef DEBUG
#endif

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tELAPSED TIME: Total time %ld us\n\n", (long)elapsed);
#endif

// Return number of chips.
   return PUF_instance_index_struct.num_ints;
   }
