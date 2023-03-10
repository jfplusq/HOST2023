// ========================================================================================================
// ========================================================================================================
// ******************************************* device_common.c ********************************************
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
#include "device_hardware.h"
#include "device_common.h"
#include "device_regen_funcs.h"

// ========================================================================================================
// ========================================================================================================
// Called from GoGetVectors below (from CommonCore). Get the vectors and masks to be applied to the functional 
// unit. Verifier will send number of rising vectors (inspects vectors as it reads them) and indicate whether 
// masks will also be sent.

int ReceiveVectors(int max_string_len, int verifier_socket_desc, unsigned char ***first_vecs_b_ptr, 
   unsigned char ***second_vecs_b_ptr, int num_PIs, int *num_rise_vecs_ptr, int *has_masks_ptr, int num_POs, 
   unsigned char ***masks_b_ptr)
   {
   int num_vecs, vec_pair_num, vec_num;
   char num_vecs_str[max_string_len];
   unsigned char *vec_ptr;

// Get the number of vectors that verifier intends to send.
   if ( SockGetB((unsigned char *)num_vecs_str, max_string_len, verifier_socket_desc) < 0 )
      { printf("ERROR: ReceiveVectors(): Failed to receive 'num_vecs_str'!\n"); exit(EXIT_FAILURE); }

   if ( sscanf(num_vecs_str, "%d %d %d", &num_vecs, num_rise_vecs_ptr, has_masks_ptr) != 3 )
      { printf("ERROR: ReceiveVectors(): Expected 'num_vecs', 'num_rise_vecs' and 'has_masks' in '%s'\n", num_vecs_str); exit(EXIT_FAILURE); }

#ifdef DEBUG
printf("ReceiveVectors(): 'num_vecs_str' received from verifier '%s'\tNum vecs %d\tNum rise vectors %d\tHas masks %d\n", 
   num_vecs_str, num_vecs, *num_rise_vecs_ptr, *has_masks_ptr); fflush(stdout);
#endif

// Allocate the base arrays based on the number of vectors we will receive.
   if ( (*first_vecs_b_ptr = (unsigned char **)malloc(sizeof(unsigned char *) * num_vecs)) == NULL )
      { printf("ERROR: ReceiveVectors(): Failed to allocate storage for first_vecs_b array!\n"); exit(EXIT_FAILURE); }
   if ( (*second_vecs_b_ptr = (unsigned char **)malloc(sizeof(unsigned char *) * num_vecs)) == NULL )
      { printf("ERROR: ReceiveVectors(): Failed to allocate storage for second_vecs_b array!\n"); exit(EXIT_FAILURE); }
   if ( *has_masks_ptr == 1 )
      if ( (*masks_b_ptr = (unsigned char **)malloc(sizeof(unsigned char *) * num_vecs)) == NULL )
         { printf("ERROR: ReceiveVectors(): Failed to allocate storage for masks_b array!\n"); exit(EXIT_FAILURE); }

// Receive the first_vecs and second_vecs sent by the verifier. 
   vec_num = 0;
   vec_pair_num = 0;
   while ( vec_num != num_vecs )
      {

// Allocate space to store the binary vectors
      if ( vec_pair_num == 0 )
         {
         if ( ((*first_vecs_b_ptr)[vec_num] = (unsigned char *)malloc(sizeof(char)*num_PIs/8)) == NULL )
            { printf("ERROR: ReceiveVectors(): Failed to allocate storage for first_vecs_b element!\n"); exit(EXIT_FAILURE); }
         vec_ptr = (*first_vecs_b_ptr)[vec_num];
         }
      else if ( vec_pair_num == 1 )
         {
         if ( ((*second_vecs_b_ptr)[vec_num] = (unsigned char *)malloc(sizeof(char)*num_PIs/8)) == NULL )
            { printf("ERROR: ReceiveVectors(): Failed to allocate storage for second_vecs_b element!\n"); exit(EXIT_FAILURE); }
         vec_ptr = (*second_vecs_b_ptr)[vec_num];
         }
      else 
         if ( ((*masks_b_ptr)[vec_num] = (unsigned char *)malloc(sizeof(char)*num_POs/8)) == NULL )
            { printf("ERROR: ReceiveVectors(): Failed to allocate storage for masks_b element!\n"); exit(EXIT_FAILURE); }

// Get the binary vector data
      if ( vec_pair_num <= 1 )
         {
         if ( SockGetB(vec_ptr, num_PIs/8, verifier_socket_desc) != num_PIs/8 )
            { printf("ERROR: ReceiveVectors(): number of vector bytes received is not equal to %d\n", num_PIs/8); exit(EXIT_FAILURE); }
         }
      else if ( SockGetB((*masks_b_ptr)[vec_num], num_POs/8, verifier_socket_desc) != num_POs/8 )
         { printf("ERROR: ReceiveVectors(): number of mask bytes received is not equal to %d\n", num_POs/8); exit(EXIT_FAILURE); }

#ifdef DEBUG
printf("Vector %d\n\t", vec_num);
int i;
for ( i = 0; i < num_PIs/8; i++ )
   printf("%02X ", vec_ptr[i]);
printf("\n");
#endif

// Increment to next vector after both vectors (first and second), and potentially the mask, have been received.
      if ( (*has_masks_ptr == 0 && vec_pair_num == 1) || (*has_masks_ptr == 1 && vec_pair_num == 2) )
         {
         vec_num++;
         vec_pair_num = 0;
         }
      else
         vec_pair_num++; 
      }

#ifdef DEBUG
printf("ReceiveVectors(): %d vector pairs received from verifier!\n", vec_num); fflush(stdout);
#endif

   return num_vecs;
   }


// ========================================================================================================
// ========================================================================================================
// Called from device_provision.c. Get the CHALLENGES and masks to be applied to the functional unit (NOT the
// VECTORS as above, which are the challenges split into two equal-sized pieces). Verifier will send number 
// of rising challenges (inspects challenges as it reads them) and indicate whether masks will also be sent.

int ReceiveChlngsAndMasks(int max_string_len, int verifier_socket_desc, unsigned char ***challenges_b_ptr, 
   int num_chlng_bits, int *num_rise_chlngs_ptr, int *has_masks_ptr, int num_POs, unsigned char ***masks_b_ptr)
   {
   int num_chlngs, chlng_mask_num, chlng_num;
   char num_chlngs_str[max_string_len];
   unsigned char *chlng_ptr;

// Get the number of challenges that verifier intends to send.
   if ( SockGetB((unsigned char *)num_chlngs_str, max_string_len, verifier_socket_desc) < 0 )
      { printf("ERROR: ReceiveChlngsAndMasks(): Failed to receive 'num_chlngs_str'!\n"); fflush(stdout); exit(EXIT_FAILURE); }

// DEBUG
#ifdef DEBUG
printf("ReceiveChlngsAndMasks(): 'num_chlngs_str' received from verifier '%s'\n", num_chlngs_str); fflush(stdout);
#endif

   if ( sscanf(num_chlngs_str, "%d %d %d", &num_chlngs, num_rise_chlngs_ptr, has_masks_ptr) != 3 )
      { printf("ERROR: ReceiveChlngsAndMasks(): Expected 'num_chlngs', 'num_rise_chlngs' and 'has_masks' in '%s'\n", num_chlngs_str); fflush(stdout); exit(EXIT_FAILURE); }

// Allocate the base arrays based on the number of challenges we will receive.
   if ( (*challenges_b_ptr = (unsigned char **)malloc(sizeof(unsigned char *) * num_chlngs)) == NULL )
      { printf("ERROR: ReceiveChlngsAndMasks(): Failed to allocate storage for challenges_b array!\n"); fflush(stdout); exit(EXIT_FAILURE); }
   if ( *has_masks_ptr == 1 )
      if ( (*masks_b_ptr = (unsigned char **)malloc(sizeof(unsigned char *) * num_chlngs)) == NULL )
         { printf("ERROR: ReceiveChlngsAndMasks(): Failed to allocate storage for masks_b array!\n"); fflush(stdout); exit(EXIT_FAILURE); }

// Receive the challenges and masks sent by the verifier. 
   chlng_num = 0;
   chlng_mask_num = 0;
   while ( chlng_num != num_chlngs )
      {

// Allocate space to store the binary challenges
      if ( chlng_mask_num == 0 )
         {
         if ( ((*challenges_b_ptr)[chlng_num] = (unsigned char *)malloc(sizeof(char)*num_chlng_bits/8)) == NULL )
            { printf("ERROR: ReceiveChlngsAndMasks(): Failed to allocate storage for challenges_b element!\n"); fflush(stdout); exit(EXIT_FAILURE); }
         chlng_ptr = (*challenges_b_ptr)[chlng_num];
         }
      else 
         if ( ((*masks_b_ptr)[chlng_num] = (unsigned char *)malloc(sizeof(char)*num_POs/8)) == NULL )
            { printf("ERROR: ReceiveChlngsAndMasks(): Failed to allocate storage for masks_b element!\n"); fflush(stdout); exit(EXIT_FAILURE); }

// Get the binary challenge data
      if ( chlng_mask_num == 0 )
         {
         if ( SockGetB(chlng_ptr, num_chlng_bits/8, verifier_socket_desc) != num_chlng_bits/8 )
            { printf("ERROR: ReceiveChlngsAndMasks(): number of challenge bytes received is not equal to %d\n", num_chlng_bits/8); fflush(stdout); exit(EXIT_FAILURE); }
         }
      else if ( SockGetB((*masks_b_ptr)[chlng_num], num_POs/8, verifier_socket_desc) != num_POs/8 )
         { printf("ERROR: ReceiveChlngsAndMasks(): number of mask bytes received is not equal to %d\n", num_POs/8); fflush(stdout); exit(EXIT_FAILURE); }

// DEBUG
// printf("Vector %d\n\t", chlng_num);
// int i;
// for ( i = 0; i < num_chlng_bits/8; i++ )
//    printf("%02X ", chlng_ptr[i]);
// printf("\n");

// Increment to next challenge, and potentially the mask, have been received.
      if ( (*has_masks_ptr == 0 && chlng_mask_num == 0) || (*has_masks_ptr == 1 && chlng_mask_num == 1) )
         {
         chlng_num++;
         chlng_mask_num = 0;
         }
      else
         chlng_mask_num++; 
      }

// DEBUG
// printf("ReceiveChlngsAndMasks(): %d challenges received from verifier!\n", chlng_num); fflush(stdout);

   return num_chlngs;
   }


// ========================================================================================================
// ========================================================================================================
// Transfer a challenge plus optionally a mask through the GPIO to the VHDL side.

void LoadChlngAndMask(int max_string_len, volatile unsigned int *CtrlRegA, volatile unsigned int *DataRegA, int chlng_num, 
   unsigned char **challenges_b, int ctrl_mask, int num_chlng_bits, int chlng_chunk_size, int has_masks, int num_POs, 
   unsigned char **masks_b)
   {
   int word_num, iter_num, load_iterations, bit_len;
   unsigned char *chlng_ptr;
   int chlng_val_chunk;

// Sanity check
   if ( (num_chlng_bits % chlng_chunk_size) != 0 )
      { printf("ERROR: LoadChlngAndMask(): Challenge size %d must be evenly divisible by %d!\n", num_chlng_bits, chlng_chunk_size); exit(EXIT_FAILURE); }

// Reset the VHDL pointers to the challenge buffers. 
   *CtrlRegA = ctrl_mask | (1 << OUT_CP_DTO_RESTART);
   usleep(1000);
   *CtrlRegA = ctrl_mask;

#ifdef DEBUG
printf("LoadChlngAndMask(): Reset VHDL challenge pointers!\n"); fflush(stdout);
#endif

   if ( has_masks == 0 )
      load_iterations = 1;
   else
      load_iterations = 2;

// Send a binary challenge, 16-bits at a time, starting with the low order to high order bits. 
   for ( iter_num = 0; iter_num < load_iterations; iter_num++ )
      {
  
// Set size of data transfer.
      if ( iter_num == 0 )
         bit_len = num_chlng_bits;
      else
         bit_len = num_POs;

#ifdef DEBUG
printf("LoadChlngAndMask(): Current control mask '%08X'\n", ctrl_mask); fflush(stdout);
#endif

// Iterate over each of the 16-bit chunks. Verifier or data orders the data from low order to high order, i.e., the exact format that we need to load it up by 
// in the VHDL. This is done in ConvertASCIIVecMaskToBinary called by enrollDB.c in DATABASE directory.
      for ( word_num = 0; word_num < bit_len/chlng_chunk_size; word_num++ )
         {

// Add 2 bytes at a time to the pointer. 
         if ( iter_num == 0 )
            chlng_ptr = challenges_b[chlng_num] + word_num*2;
         else 
            chlng_ptr = masks_b[chlng_num] + word_num*2;

         chlng_val_chunk = (chlng_ptr[1] << 8) + chlng_ptr[0]; 

#ifdef DEBUG
printf("LoadChlngAndMask(): 16-bit chunk %d in hex '%04X'\n", word_num, chlng_val_chunk); fflush(stdout);
#endif

// Four step protocol
// 1) Assert 'data_ready' while putting the 16-bit binary value on the low order bits of CtrlReg
//printf("LoadChlngAndMask(): Writing 'data_ready' with 16-bit binary value in hex '%04X'\n", chlng_val_chunk); fflush(stdout);
         *CtrlRegA = ctrl_mask | (1 << OUT_CP_DTO_DATA_READY) | chlng_val_chunk;

// 2) Wait for 'done_reading to go to 1 (it is low by default). State machine latches data in 2 clk cycles. 
//    Maintain 1 on 'data_ready' and continue to hold 16-bit binary chunk.
// printf("LoadChlngAndMask(): Waiting state machine 'done_reading' to be set to '1'\n"); fflush(stdout);
         while ( (*DataRegA & (1 << IN_SM_DTO_DONE_READING)) == 0 );

// 3) Once 'done_reading' goes to 1, set 'data_ready' to 0 and remove chunk;
// printf("LoadChlngAndMask(): De-asserting 'data_ready'\n"); fflush(stdout);
         *CtrlRegA = ctrl_mask;

// 4) Wait for 'done_reading to go to 0.
// printf("LoadChlngAndMask(): Waiting state machine 'done_reading' to be set to '0'\n"); fflush(stdout);
         while ( (*DataRegA & (1 << IN_SM_DTO_DONE_READING)) != 0 );

// printf("LoadChlngAndMask(): Done handshake associated with challenge chunk transfer\n"); fflush(stdout);
         }
      }

// 6/1/2016: Tell CollectPNs (if it is waiting) that challenge and possibly a mask have been loaded.
   *CtrlRegA = ctrl_mask | (1 << OUT_CP_DTO_VEC_LOADED);
   *CtrlRegA = ctrl_mask;

   return;
   }


// ========================================================================================================
// ========================================================================================================
// DEBUG ROUTINE. Check that the vectors received are precisely the same as the ones stored in the file on 
// the server. Write an ASCII file. 

void SaveASCIIVectors(int max_string_len, int num_vecs, unsigned char **first_vecs_b, unsigned char **second_vecs_b, 
   int num_PIs, int has_masks, int num_POs, unsigned char **masks_b)
   {
   int byte_cnter, bit_cnter;
   unsigned char *vec_ptr;
   int vec_num, vec_pair;
   FILE *OUTFILE;

   if ( (OUTFILE = fopen("ReceivedVectors.txt", "w")) == NULL )
      { printf("ERROR: SaveASCIIVectors(): Could not open ReceivedVectors.txt for writing!\n"); exit(EXIT_FAILURE); }

   for ( vec_num = 0; vec_num < num_vecs; vec_num++ )
      {
      for ( vec_pair = 0; vec_pair < 2; vec_pair++ )
         {

         if ( vec_pair == 0 )
            vec_ptr = first_vecs_b[vec_num];
         else
            vec_ptr = second_vecs_b[vec_num];

// Print ASCII version of bitstring in high order to low order. 
         for ( byte_cnter = num_PIs/8 - 1; byte_cnter >= 0; byte_cnter-- )
            {
            for ( bit_cnter = 7; bit_cnter >= 0; bit_cnter-- )
               {

// Check binary bit for 0 or 1
               if ( (vec_ptr[byte_cnter] & (unsigned char)(1 << bit_cnter)) == 0 )
                  fprintf(OUTFILE, "0");
               else
                  fprintf(OUTFILE, "1");
               }
            }
         fprintf(OUTFILE, "\n");
         }

// Extra <cr> between vector pairs.
      fprintf(OUTFILE, "\n");
      }
   fclose(OUTFILE);


// Do the same for the received masks.
   if ( has_masks == 1 )
      {
      if ( (OUTFILE = fopen("ReceivedMasks.txt", "w")) == NULL )
         { printf("ERROR: SaveASCIIVectors(): Could not open ReceivedMasks.txt for writing!\n"); exit(EXIT_FAILURE); }

      for ( vec_num = 0; vec_num < num_vecs; vec_num++ )
         {

// Print ASCII version of bitstring in high order to low order. 
         for ( byte_cnter = num_POs/8 - 1; byte_cnter >= 0; byte_cnter-- )
            {
            for ( bit_cnter = 7; bit_cnter >= 0; bit_cnter-- )
               {

// Check binary bit for 0 or 1
               if ( (masks_b[vec_num][byte_cnter] & (unsigned char)(1 << bit_cnter)) == 0 )
                  fprintf(OUTFILE, "0");
               else
                  fprintf(OUTFILE, "1");
               }
            }
         fprintf(OUTFILE, "\n");
         }
      fclose(OUTFILE);
      }

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Fetch the seed that will be used to extract the set of vectors and path select masks from the database.

unsigned int GetChallengeGenSeed(int max_string_len, int verifier_socket_desc)
   {
   char DB_ChallengeGen_seed_str[max_string_len];
   unsigned int DB_ChallengeGen_seed;

   if ( SockGetB((unsigned char *)DB_ChallengeGen_seed_str, max_string_len, verifier_socket_desc) < 0 )
      { printf("ERROR: GetChallengeGenSeed(): Error receiving 'DB_ChallengeGen_seed_str' from verifier!\n"); exit(EXIT_FAILURE); }
   sscanf(DB_ChallengeGen_seed_str, "%u", &DB_ChallengeGen_seed);

#ifdef DEBUG
printf("GetChallengeGenSeed(): Got %u as ChallengeGen_seed from server!\n", DB_ChallengeGen_seed); fflush(stdout);
#endif

   return DB_ChallengeGen_seed;
   }


// ========================================================================================================
// ========================================================================================================
// Send 'GO' and get vectors and masks. NOTE: For SiRF, there is ONLY one vector, a configuration challenge.
// But I am storing two vectors in the database on the verifier side to make it compatible with the previous
// database structure.
// 11_1_2021: Added a Challenge.db to the device and TTP now so we can generate vectors locally.

int GoGetVectors(int max_string_len, int num_POs, int num_PIs, int verifier_socket_desc, int *num_rise_vecs_ptr, 
   int *has_masks_ptr, unsigned char ***first_vecs_b_ptr, unsigned char ***second_vecs_b_ptr, 
   unsigned char ***masks_b_ptr, int send_GO, int use_database_chlngs, sqlite3 *DB, int DB_design_index,
   char *DB_ChallengeSetName, int gen_or_use_challenge_seed, unsigned int *DB_ChallengeGen_seed_ptr, 
   pthread_mutex_t *GenChallenge_mutex_ptr, int debug_flag)
   {
   int num_vecs;

   struct timeval t0, t1;
   long elapsed; 

// ****************************************
// ***** Send "GO" to the verifier 
   if ( send_GO == 1 )
      {
      if ( debug_flag == 1 )
         {
         printf("GGV.1) Sending 'GO' to verifier\n");
         gettimeofday(&t0, 0);
         }
      if ( SockSendB((unsigned char *)"GO", strlen("GO")+1, verifier_socket_desc) < 0 )
         { printf("ERROR: GoGetVectors(): Send 'GO' request failed\n"); exit(EXIT_FAILURE); }
      if ( debug_flag == 1 )
         { gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed); }
      }

// ****************************************
// ***** Get the vectors and select masks from verifier. 
   if ( debug_flag == 1 )
      {
      printf("GGV.2) Receiving vectors and masks from verifier\n");
      gettimeofday(&t0, 0);
      }

// Read all the vectors from the verifier into a set of string arrays. Verifier will send number of rising vectors (inspects vectors 
// as it reads them) and indicate whether masks will also be sent.
   if ( use_database_chlngs == 0 )
      {

// When the device/TTP requests actual vectors from the server, also get and store the DB_ChallengeGen_seed so we can use it locally
// to reproduce the vectors in the future if needed.
      *DB_ChallengeGen_seed_ptr = GetChallengeGenSeed(max_string_len, verifier_socket_desc);

// Get the actual vectors from the server.
      num_vecs = ReceiveVectors(max_string_len, verifier_socket_desc, first_vecs_b_ptr, second_vecs_b_ptr, num_PIs, num_rise_vecs_ptr,
         has_masks_ptr, num_POs, masks_b_ptr);
      }
   else
      {
      VecPairPOStruct *challenge_vecpair_id_PO_arr = NULL;
      int num_challenge_vecpair_id_PO = 0;

// Optionally get the challenge seed from the server. Usually (maybe always) when 'send_GO' is 0, we want to use the existing challenge
// seed, but these two flags mean something different so keeping them both (for now).
      if ( gen_or_use_challenge_seed == 0 )
         *DB_ChallengeGen_seed_ptr = GetChallengeGenSeed(max_string_len, verifier_socket_desc);

#ifdef DEBUG
printf("GoGetVectors(): use_database_chlngs is 1! Calling GenChallengeDB with Seed %u\n", *DB_ChallengeGen_seed_ptr);
fflush(stdout);
#endif

// This routine generates additional, pseudo-randomly selected challenge sets from special challenges added by add_challengeDB that use '0', 
// '1', 'u' and 'q' designators. '0' means output has no transition, '1' means we MUST use that path, 'u' (unqualifying) means the path 
// has a transition but is not 'compatible' with the elements that are marked 'q' (for qualifying). We assume the challenge set specified 
// on the command line exists already in the database (added by add_challengeDB.c). It MUST be a challenge set that has 'u' and 'q' 
// designators otherwise it is fully specified by add_challengeDB.c and there is nothing this routine can do to pseudo-randomly select
// challenges. It returns a set of binary vectors and masks as well as a data structure that allows the enrollment timing values 
// that are tested by these vectors to be looked up by the caller.
      GenChallengeDB(max_string_len, DB, DB_design_index, DB_ChallengeSetName, *DB_ChallengeGen_seed_ptr, 0, NULL, NULL, 
         first_vecs_b_ptr, second_vecs_b_ptr, masks_b_ptr, &num_vecs, num_rise_vecs_ptr, GenChallenge_mutex_ptr,
         &num_challenge_vecpair_id_PO, &challenge_vecpair_id_PO_arr);

// We always generate masks during the database vector selection process.
      *has_masks_ptr = 1;

#ifdef DEBUG
printf("GoGetVectors(): DATABASE selected vecs/masks with Seed %d\n", *DB_ChallengeGen_seed_ptr);
PrintHeaderAndHexVals("\t\n", 49, (*first_vecs_b_ptr)[0], 49);
fflush(stdout);
#endif

// Free up the challenge_vecpair_id_PO_arr. We'll free the vectors and timing data in the caller if it isn't needed again for something else.
      if ( challenge_vecpair_id_PO_arr != NULL )
         free(challenge_vecpair_id_PO_arr);
#ifdef INCLUDE_DATABASE
#endif
      }

   if ( debug_flag == 1 )
      { gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed); }

// 9_18_2022: Timing tests for FSB KEK for SiRF paper. GenLLK from device_regeneration is called and then this routine.
// 9_18_2022: Taking this out temporarily for SiRF paper timing operation.
printf("\tNumber of vectors received %d\tNumber of rising vectors %d\tHas masks ? %d\n\n", num_vecs, *num_rise_vecs_ptr, *has_masks_ptr); 
fflush(stdout);
#ifdef DEBUG
#endif

#ifdef DEBUG
SaveASCIIVectors(max_string_len, num_vecs, *first_vecs_b_ptr, *second_vecs_b_ptr, num_PIs, *has_masks_ptr, num_POs, *masks_b_ptr);
#endif

   return num_vecs;
   }


// ========================================================================================================
// ========================================================================================================
// Read unsigned char data (helper data/SpreadFactors/nonces) from file. Data is expected to be in packed ASCII 
// hex format (NO spaces between pairs of consecutive hex digits) but can be on multiple lines. Allocates 
// space as needed.

int ReadFileHexASCIIToUnsignedChar(int max_string_len, char *file_name, unsigned char **bin_arr_ptr)
   {
   unsigned int tot_usc_bytes, read_num_bytes;
   char line[max_string_len+1], *char_ptr;
   int temp, HEX_TO_BYTE = 2;
   unsigned int i, j;
   FILE *INFILE;

printf("ReadFileHexASCIIToUnsignedChar(): Filename '%s'\n", file_name); fflush(stdout);
#ifdef DEBUG
#endif

   if ( (INFILE = fopen(file_name, "r")) == NULL )
      { printf("ERROR: ReadFileHexASCIIToUnsignedChar(): Failed to open file '%s'\n", file_name); exit(EXIT_FAILURE); }

// If bin_arr_ptr is NOT NULL, free up the data
   if ( *bin_arr_ptr != NULL )
      free(*bin_arr_ptr); 
   *bin_arr_ptr = NULL;

// Keep reading data a line at a time.
   tot_usc_bytes = 0;
   while ( fgets(line, max_string_len - 1, INFILE) != NULL )
      {

// Find the newline and eliminate it.
      if ((char_ptr = strrchr(line, '\n')) != NULL)
         *char_ptr = '\0';

// Skip blank lines.
      if ( strlen(line) == 0 )
         continue;

// Sanity check -- number of hex digits needs to be even.
      if ( (strlen(line) % 2) != 0 )
         { printf("ERROR: ReadFileHexASCIIToUnsignedChar(): Number of hex digits on a line in the file MUST be even!\n"); exit(EXIT_FAILURE); }

// Compute the number of unsigned char bytes needed to store the data.
      read_num_bytes = strlen(line)/HEX_TO_BYTE;

// Keep increasing the size of the array.
      if ( (*bin_arr_ptr = (unsigned char *)realloc(*bin_arr_ptr, sizeof(unsigned char) * (tot_usc_bytes + read_num_bytes))) == NULL )
         { printf("ERROR: ReadFileHexASCIIToUnsignedChar(): Failed to reallocate larger space!\n"); exit(EXIT_FAILURE); }

// Write out the data from low order to high order in the array.
      j = 0;
      for ( i = 0; i < read_num_bytes; i++ )
         {

// Convert 2-char instances of the data read into unsigned char bytes.
         sscanf(&(line[j]), "%2X", &temp);
         (*bin_arr_ptr)[tot_usc_bytes + i] = (unsigned char)temp;
         j += HEX_TO_BYTE;
         }

      tot_usc_bytes += read_num_bytes;
      }

   fclose(INFILE);

   return tot_usc_bytes;
   }


// ========================================================================================================
// ========================================================================================================
// Write unsigned char data (helper data/SpreadFactors/nonces) to file. Data will be packed as ASCII hex characters
// with NO spaces between pairs of consecutive hex digits -- all on one line. 

void WriteFileHexASCIIToUnsignedChar(int max_string_len, char *file_name, int num_bytes, unsigned char *bin_arr, 
   int overwrite_or_append)
   {
   FILE *OUTFILE = NULL;
   int byte_num;

printf("WriteFileHexASCIIToUnsignedChar(): Filename '%s'\tNumber bytes %d\n", file_name, num_bytes); fflush(stdout);
#ifdef DEBUG
#endif

   if ( overwrite_or_append == 0 && (OUTFILE = fopen(file_name, "w")) == NULL )
      { printf("ERROR: WriteFileHexASCIIToUnsignedChar(): Failed to open file '%s'\n", file_name); exit(EXIT_FAILURE); }
   else if ( overwrite_or_append == 1 && (OUTFILE = fopen(file_name, "a")) == NULL )
      { printf("ERROR: WriteFileHexASCIIToUnsignedChar(): Failed to open file '%s'\n", file_name); exit(EXIT_FAILURE); }

// Left-to-right processing of the bits makes the left-most bit in line the high order bit in the binary value. 
   for ( byte_num = 0; byte_num < num_bytes; byte_num++ )
      fprintf(OUTFILE, "%02X", bin_arr[byte_num]);
   fprintf(OUTFILE, "\n");

   fclose(OUTFILE);

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Read nonces/SHD from file. If user sets 'num_bytes' to a positive value (not -1), then assume each line
// that is read has exactly 'num_bytes'/2 (number of hex digits per byte) -- assume file is hex encoded ASCII.
// If user specifies 'alloc_arr' as 1, then allocate space for 'bin_arr_ptr' otherwise, assume it exists.
// One possible file structure is SHD on first line, number of RpXORn2s on second line, number of bitstrings 
// as a string on the third line and then RpXORn2s on subsequent lines, with last line storing XOR_nonce (n3).

int ReadFileHexASCIIToUnsignedCharSpecial(int max_string_len, char *file_name, int num_bytes, int alloc_arr, unsigned char **bin_arr_ptr,
   FILE *INFILE)
   {
   unsigned int tot_usc_bytes, read_num_bytes;
   char line[max_string_len+1], *char_ptr;
   int line_cnt, close_file;
   int HEX_TO_BYTE = 2;
   unsigned int temp;
   unsigned int i, j;

printf("ReadFileHexASCIIToUnsignedCharSpecial(): Filename '%s'\tNumber of bytes %d\tAlloc arr? %d\n", file_name, num_bytes, alloc_arr); fflush(stdout);
#ifdef DEBUG
#endif

// If the user passes in an open file (INFILE NOT NULL), the do NOT close it. Only close files that are opened by this routine.
   close_file = 1;
   if ( INFILE == NULL && (INFILE = fopen(file_name, "r")) == NULL )
      { printf("ERROR: ReadFileHexASCIIToUnsignedCharSpecial(): Failed to open file '%s'\n", file_name); exit(EXIT_FAILURE); }
   else
      close_file = 0;

// Assume nonce lines NEVER get longer than max_string_len, which is set to 2048 currently. If data stored as
// ASCII binary, then this allows for upto 2048 bits or 256 bytes per line.
   tot_usc_bytes = 0;
   line_cnt = 0;
   while ( fgets(line, max_string_len - 1, INFILE) != NULL )
      {

// Find the newline and eliminate it.
      if ((char_ptr = strrchr(line, '\n')) != NULL)
         *char_ptr = '\0';

// Skip blank lines.
      if ( strlen(line) == 0 )
         continue;

// When requested, check that the nonce size is consistent with input parameter. We assume 1 line here (for n3).
      if ( num_bytes != -1 && strlen(line)/HEX_TO_BYTE != (unsigned int)num_bytes )
         { 
         printf("ERROR: ReadFileHexASCIIToUnsignedCharSpecial(): Length of nonce in bytes %d is NOT equal to the required length %d!\n", (int)strlen(line)/HEX_TO_BYTE, num_bytes);
         exit(EXIT_FAILURE); 
         }

      read_num_bytes = strlen(line)/HEX_TO_BYTE;

// Allocate/reallocate storage in bin_arr if requested.
      if ( alloc_arr == 1 )
         {
         if ( tot_usc_bytes == 0 )
            *bin_arr_ptr = (unsigned char *)malloc(read_num_bytes * sizeof(unsigned char));
         else
            *bin_arr_ptr = (unsigned char *)realloc(*bin_arr_ptr, (tot_usc_bytes + read_num_bytes) * sizeof(unsigned char));
         }

// Left-to-right processing of the bits makes the left-most bit in line the high order bit in the binary value. 
      j = 0;
      for ( i = 0; i < read_num_bytes; i++ )
         {
         sscanf(&(line[j]), "%2X", &temp);
         (*bin_arr_ptr)[tot_usc_bytes + i] = (unsigned char)temp;
         j += HEX_TO_BYTE;
         }

      tot_usc_bytes += read_num_bytes;
      line_cnt++; 

// If the call specifies the number of bytes to read (they must be on one line), and we read them and exit loop.
      if ( num_bytes != -1 )
         break;
      }

   if ( close_file == 1 )
      fclose(INFILE);

printf("ReadFileHexASCIIToUnsignedCharSpecial(): Total number of nonce bytes read %d\n", tot_usc_bytes); fflush(stdout);
#ifdef DEBUG
#endif

   return tot_usc_bytes;
   }


