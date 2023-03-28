// ========================================================================================================
// ========================================================================================================
// **************************************** device_regeneration.c *****************************************
// ========================================================================================================
// ========================================================================================================
//
//--------------------------------------------------------------------------------
// Company: IC-Safety, LLC and University of New Mexico
// Engineer: Professor Jim Plusquellic
// Exclusive License: IC-Safety, LLC
// Copyright: Univ. of New Mexico
//--------------------------------------------------------------------------------


#include <signal.h>
#include "common.h"
#include "device_hardware.h"
#include "device_common.h"
#include "device_regen_funcs.h"

// ====================== DATABASE STUFF =========================
#include <sqlite3.h>
#include "commonDB.h"

extern int usleep (__useconds_t __useconds);
extern int getpagesize (void)  __THROW __attribute__ ((__const__));

// ========================================================================================================
// ========================================================================================================
// ========================================================================================================
SRFHardwareParamsStruct SHP;

int main(int argc, char *argv[])
   {
   volatile unsigned int *CtrlRegA;
   volatile unsigned int *DataRegA;
   unsigned int ctrl_mask;

   char *MyName;
   char *Bank_IP;
   char *My_IP;
   char *IP_list_filename;
   char *temp_str;

   int Bank_socket_desc = 0;
 
   int port_number;

   int nonce_base_address;
   int num_eCt_nonce_bytes;
   int num_KEK_authen_nonce_bytes; 

   int fix_params; 
   int num_sams;

   int num_PIs;
   int num_POs;

   int DUMP_BITSTRINGS;
   int DEBUG_FLAG;

   int PCR_or_PBD_or_PO; 

// The PL-side TRNG_LFSR is 64 bits. Note that we currently only suport loading the low-order 8-bits of the seed register below.
   unsigned char TRNG_LFSR_seed;

// ====================== DATABASE STUFF =========================
   sqlite3 *DB_Challenges;
   int rc;
   char *DB_name_Challenges;
   Allocate1DString(&DB_name_Challenges, MAX_STRING_LEN);
   int use_database_chlngs; 

   char *Netlist_name;
   char *Synthesis_name;
   char *ChallengeSetName;

   int design_index;
   int num_PIs_DB, num_POs_DB;

   int ChallengeGen_seed; 

   float command_line_SC;

   Allocate1DString(&MyName, MAX_STRING_LEN);
   Allocate1DString(&My_IP, MAX_STRING_LEN);
   Allocate1DString(&Bank_IP, MAX_STRING_LEN);

// ======================================================================================================================
// COMMAND LINE
// ======================================================================================================================
   if ( argc != 4 )
      {
      printf("Parameters: MyName (Alice/Bob/Jim/Cyrus/George) -- Device IP (192.168.1.10) -- Bank IP (192.168.1.20)\n");
      exit(EXIT_FAILURE);
      }

   strcpy(MyName, argv[1]);
   strcpy(My_IP, argv[2]);
   strcpy(Bank_IP, argv[3]);

   fix_params = 0;
   num_sams = 4;
   PCR_or_PBD_or_PO = 0;
   command_line_SC = 1.0;

// Sanity checks
   if ( fix_params != 0 && fix_params != 1 )
      { printf("ERROR: 'fix_params' MUST be 0 or 1!\n"); exit(EXIT_FAILURE); }

   if ( num_sams != 1 && num_sams != 4 && num_sams != 8 && num_sams != 16 )
      { printf("ERROR: 'num_sams' MUST be 1, 4, 8 or 16!\n"); exit(EXIT_FAILURE); }

   if ( PCR_or_PBD_or_PO != 0 && PCR_or_PBD_or_PO != 1 && PCR_or_PBD_or_PO != 2 )
      { printf("ERROR: 'PCR_or_PBD_or_PO' MUST be 0, 1 or 2!\n"); exit(EXIT_FAILURE); }

// Upper limit is arbitrary -- they never get this big -- 3.2 looks to be the max.
   if ( command_line_SC <= 0.0 || command_line_SC > (float)MAX_SCALING_VALUE )
      { printf("ERROR: 'command_line_SC' MUST be >= 0.0 and <= %f -- FIX ME!\n", (float)MAX_SCALING_VALUE); exit(EXIT_FAILURE); }

   Allocate1DString(&IP_list_filename, MAX_STRING_LEN);
   Allocate1DString(&temp_str, MAX_STRING_LEN);

   Allocate1DString((char **)(&Netlist_name), MAX_STRING_LEN);
   Allocate1DString((char **)(&Synthesis_name), MAX_STRING_LEN);
   Allocate1DString((char **)(&ChallengeSetName), MAX_STRING_LEN);

// ====================================================== PARAMETERS ====================================================
   strcpy(DB_name_Challenges, "Challenges.db");
   strcpy(Netlist_name, "SR_RFM_V4_TDC");
   strcpy(Synthesis_name, "SRFSyn1");
   strcpy(ChallengeSetName, "Master1_OptKEK_TVN_0.00_WID_1.75");

// Must be set to 0 until I fully integrate this into all of the primitives.
   use_database_chlngs = 0;
   ChallengeGen_seed = 1;

// SET TO WHATEVER bitstream you program with.
   int my_bitstream;
   my_bitstream = 0;

   char AES_IV[AES_IV_NUM_BYTES] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};

// The PL-side TRNG_LFSR is 64 bits. 
   TRNG_LFSR_seed = 1;

// NOTE: ASSUMPTION:
//    NUM_XOR_NONCE_BYTES   <=  num_eCt_nonce_bytes   <=   SE_TARGET_NUM_KEY_BITS/8   <=   NUM_REQUIRED_PNDIFFS/8
//           8                         16                            32                              256
// We always use 8 bytes for the XOR_nonce (NUM_XOR_NONCE_BYTES) to SelectParams. My plan is to use 16 byte nonces
// (num_eCt_nonce_bytes), 32 bytes AES session keys (256 bits for SE_TARGET_NUM_KEY_BITS) and NUM_REQUIRED_PNDIFFS are always 
// 2048/16 = 256. 
   num_eCt_nonce_bytes = ECT_NUM_BYTES;
   num_KEK_authen_nonce_bytes = KEK_AUTHEN_NUM_NONCE_BITS/8; 

// Base address, can be eliminated -- always 0. 
   nonce_base_address = 0;

   port_number = 8888;

// These depend on the function unit. For SR_RFM, it's 784 and 64
   num_PIs = NUM_PIS;
   num_POs = NUM_POS;

// Enable/disable debug information.
   DUMP_BITSTRINGS = 0;
   DEBUG_FLAG = 0;
// ====================================================== PARAMETERS ====================================================
// Sanity check MAKE PROTOCOL. We also assume that the SHA-3 hash input and output are the same size as the AK_A/MHK_As, 
// which must be the same size as the KEK key (since we use KEK_Regen() below to regenerate it). 
   if ( HASH_IN_LEN_BITS != KEK_TARGET_NUM_KEY_BITS || HASH_OUT_LEN_BITS != KEK_TARGET_NUM_KEY_BITS )
      { 
      printf("ERROR: main(): HASH_IN_LEN_BITS %d MUST be equal to HASH_OUT_LEN_BIT %d MUST be equal to KEK_TARGET_NUM_KEY_BITS %d\n", 
         HASH_IN_LEN_BITS, HASH_OUT_LEN_BITS, KEK_TARGET_NUM_KEY_BITS); exit(EXIT_FAILURE); 
      }

// Sanity check, constraint must be honored because of space allocations.
//    NUM_XOR_NONCE_BYTES   <=  num_eCt_nonce_bytes   <=   SE_TARGET_NUM_KEY_BITS/8   <=   NUM_REQUIRED_PNDIFFS/8
//           8                         16                            32                              256
   if ( !(NUM_XOR_NONCE_BYTES <= num_eCt_nonce_bytes && num_eCt_nonce_bytes <= SE_TARGET_NUM_KEY_BITS/8 && 
      SE_TARGET_NUM_KEY_BITS/8 <= NUM_REQUIRED_PNDIFFS/8) )
      { 
      printf("ERROR: Constraint violated: NUM_XOR_NONCE_BYTES %d <= num_eCt_nonce_bytes %d && \n\
         num_eCt_nonce_bytes %d <= SE_TARGET_NUM_KEY_BITS/8 %d <= NUM_REQUIRED_PNDIFFS/8 %d\n",
         NUM_XOR_NONCE_BYTES, num_eCt_nonce_bytes, num_eCt_nonce_bytes, SE_TARGET_NUM_KEY_BITS/8, NUM_REQUIRED_PNDIFFS/8);
      exit(EXIT_FAILURE);
      }

   printf("Parameters: This Device IP %s\tBank IP %s\tFIX PARAMS %d\tNum Sams %d\tPCR/PBD/PO %d\n", My_IP, Bank_IP, fix_params, num_sams, PCR_or_PBD_or_PO); fflush(stdout);

// The number of samples is set BELOW after CtrlRegA is given an address.
   ctrl_mask = 0;

// For handling Ctrl-C. We MUST exit gracefully to keep the hardware from quitting at a point where the
// fine phase of the MMCM is has not be set back to 0. If it isn't, then re-running this program will
// likely fail because my local fine phase register (which is zero initially after a RESET) is out-of-sync 
// with the MMCM phase (which is NOT zero).
   signal(SIGINT, intHandler);

// When we save output file, this tells us what we used.
   printf("PARAMETERS: PCR/PBD %d\tSE Target Num Bits %d\n\n", PCR_or_PBD_or_PO, SE_TARGET_NUM_KEY_BITS); fflush(stdout);

// Open up the memory mapped device so we can access the GPIO registers.
   int fd = open("/dev/mem", O_RDWR|O_SYNC);
   if (fd < 0) 
      { printf("ERROR: /dev/mem could NOT be opened!\n"); exit(EXIT_FAILURE); }

// Add 2 for the DataReg (for an SpreadFactor of 8 bytes for 32-bit integer variables)
   DataRegA = (volatile unsigned int *)mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, GPIO_0_BASE_ADDR);
   CtrlRegA = DataRegA + 2;

// ********************************************************************************************************** 
   *CtrlRegA = ctrl_mask | (1 << OUT_CP_RESET); 
   *CtrlRegA = ctrl_mask;
   usleep(10000);

// Set the number of samples
   if ( num_sams == 1 )
      ctrl_mask = (0 << OUT_CP_NUM_SAM1) | (0 << OUT_CP_NUM_SAM0);
   else if ( num_sams == 4 )
      ctrl_mask = (0 << OUT_CP_NUM_SAM1) | (1 << OUT_CP_NUM_SAM0);
   else if ( num_sams == 8 )
      ctrl_mask = (1 << OUT_CP_NUM_SAM1) | (0 << OUT_CP_NUM_SAM0);
   else if ( num_sams == 16 )
      ctrl_mask = (1 << OUT_CP_NUM_SAM1) | (1 << OUT_CP_NUM_SAM0);
   else
      { printf("ERROR: Number of samples MUST be 1, 4, 8 or 16!\n"); exit(EXIT_FAILURE); }

   *CtrlRegA = ctrl_mask;

// ====================== DATABASE STUFF =========================
   rc = sqlite3_open(":memory:", &DB_Challenges);
   if ( rc != 0 )
      { printf("Failed to open Challenge Database: %s\n", sqlite3_errmsg(DB_Challenges)); sqlite3_close(DB_Challenges); exit(EXIT_FAILURE); }
   printf("Reading filesystem database '%s' into memory!\n", DB_name_Challenges); fflush(stdout);
   if ( LoadOrSaveDb(DB_Challenges, DB_name_Challenges, 0) != 0 )
      { printf("Failed to open and copy into memory '%s': ERR: %s\n", DB_name_Challenges, sqlite3_errmsg(DB_Challenges)); sqlite3_close(DB_Challenges); exit(EXIT_FAILURE); }

// Get the PUFDesign parameters from the database. 
   if ( GetPUFDesignParams(MAX_STRING_LEN, DB_Challenges, Netlist_name, Synthesis_name, &design_index, &num_PIs_DB, &num_POs_DB) != 0 )
      { printf("ERROR: PUFDesign index NOT found for '%s', '%s'!\n", Netlist_name, Synthesis_name); exit(EXIT_FAILURE); }

// Sanity check
   if ( num_PIs_DB != num_PIs || num_POs_DB != num_POs )
      { 
      printf("ERROR: Number of PIs %d or POs %d in database do NOT match those in common.h %d and %d!\n", num_PIs_DB, num_POs_DB, num_PIs, num_POs); 
      exit(EXIT_FAILURE); 
      }

// =========================
// Set some of the params in the data structure.
   SHP.CtrlRegA = CtrlRegA;
   SHP.DataRegA = DataRegA;
   SHP.ctrl_mask = ctrl_mask;

// 10_11_2022: For testing COBRA and RangeConstant -- added this field. Can be used in other places for PUF-Cash too.
   StringCreateAndCopy(&(SHP.My_IP), My_IP);

// 11_1_2021: Adding this for MAKE and PeerTrust protocol. After device authenticates successfully with IA, IA sends 
// its ID from the NAT database to the device. The device will use this as it's ID.
   SHP.chip_num = -1;

// 7_2_2022: Adding this for the PUF-Cash V3.0 AliceWithdrawal process. This is also filled in by GenLLK(). THIS CAN BE DONE
// during device provisioning where the challenge are drawn from the ANONYMOUS DB, or by doing an anonymous authentication
// at any time with the server.
   SHP.anon_chip_num = -1;

   SHP.DB_Challenges = DB_Challenges;
   SHP.DB_name_Challenges = DB_name_Challenges;

   SHP.use_database_chlngs = use_database_chlngs;
   SHP.DB_design_index = design_index;
   SHP.DB_ChallengeSetName = ChallengeSetName;
   SHP.DB_ChallengeGen_seed = ChallengeGen_seed; 
#ifdef INCLUDE_DATABASE
#endif

   SHP.DB_MAKE_PT_AT = NULL;
   SHP.DB_name_MAKE_PT_AT = NULL;
   SHP.DB_PUFCash_V3 = NULL;
   SHP.DB_name_PUFCash_V3 = NULL;
   SHP.eCt_num_bytes = ECT_NUM_BYTES;

// Alice's withdrawal amount
   SHP.Alice_EWA = NULL;
   SHP.Alice_K_AT = NULL;

// 11_1_2021: MAKE protocol. This must also match the length of KEK_TARGET_NUM_KEY_BITS/8. Might make more sense to just set it to that even 
// though we use KEK session key generation to generate the MAT_LLK.
   SHP.MAT_LLK_num_bytes = SE_TARGET_NUM_KEY_BITS/8;

// 11_21_2021: PeerTrust protocol. This must also match the length of KEK_TARGET_NUM_KEY_BITS/8. Might make more sense to just set it to that even 
// though we use KEK session key generation to generate the PHK_A_nonce.
   SHP.PHK_A_num_bytes = SE_TARGET_NUM_KEY_BITS/8;

// For GenLLK()
   SHP.KEK_LLK_num_bytes = KEK_TARGET_NUM_KEY_BITS/8;

// For POP
   SHP.POP_LLK_num_bytes = KEK_TARGET_NUM_KEY_BITS/8;

// These we will eventually come from the verifier via a message. 
   SHP.num_PIs = num_PIs;
   SHP.num_POs = num_POs;

   SHP.fix_params = fix_params;

   SHP.num_required_PNDiffs = NUM_REQUIRED_PNDIFFS;

   SHP.num_SF_bytes = NUM_REQUIRED_PNDIFFS * SF_WORDS_TO_BYTES_MULT;
   SHP.num_SF_words = NUM_REQUIRED_PNDIFFS; 

// 1_1_2022: If TRIMCODE_CONSTANT is <= 32, then we can preserve on precision bit in the iSpreadFactors for the device, else we cannot preserve any.
   if ( TRIMCODE_CONSTANT <= 32 )
      SHP.iSpreadFactorScaler = 2;
   else
      SHP.iSpreadFactorScaler = 1;

   if ( (SHP.iSpreadFactors = (signed char *)calloc(SHP.num_SF_words, sizeof(signed char))) == NULL )
      { printf("ERROR: Failed to allocate storage for iSpreadFactors!\n"); exit(EXIT_FAILURE); }

   if ( (SHP.verifier_SHD = (unsigned char *)calloc(SHP.num_required_PNDiffs/8, sizeof(unsigned char))) == NULL )
      { printf("ERROR: Failed to allocate storage for verifier_SHD!\n"); exit(EXIT_FAILURE); }
   if ( (SHP.verifier_SBS = (unsigned char *)calloc(SHP.num_required_PNDiffs/8, sizeof(unsigned char))) == NULL )
      { printf("ERROR: Failed to allocate storage for verifier_SBS!\n"); exit(EXIT_FAILURE); }
   if ( (SHP.device_SHD = (unsigned char *)calloc(SHP.num_required_PNDiffs/8, sizeof(unsigned char))) == NULL )
      { printf("ERROR: Failed to allocate storage for device_SHD!\n"); exit(EXIT_FAILURE); }
   if ( (SHP.device_SBS = (unsigned char *)calloc(SHP.num_required_PNDiffs/8, sizeof(unsigned char))) == NULL )
      { printf("ERROR: Failed to allocate storage for device_SBS!\n"); exit(EXIT_FAILURE); }
   SHP.verifier_SHD_num_bytes = 0;
   SHP.verifier_SBS_num_bytes = 0;
   SHP.device_SHD_num_bytes = 0;
   SHP.device_SBS_num_bits = 0; 

// Note: MAX_GENERATED_NONCE_BYTES MUST BE LARGER THAN NUM_XOR_NONCE_BYTES.
   SHP.nonce_base_address = nonce_base_address;
   SHP.max_generated_nonce_bytes = MAX_GENERATED_NONCE_BYTES; 
   SHP.num_required_nonce_bytes = NUM_XOR_NONCE_BYTES; 

// This is filled in by CollectPNs as the hardware reads nonce bytes.
   SHP.num_device_n1_nonces = 0;
   if ( (SHP.device_n1 = (unsigned char *)calloc(SHP.max_generated_nonce_bytes, sizeof(unsigned char))) == NULL )
      { printf("ERROR: Failed to allocate storage for verifier_n2!\n"); exit(EXIT_FAILURE); }
   if ( (SHP.verifier_n2 = (unsigned char *)calloc(SHP.num_required_nonce_bytes, sizeof(unsigned char))) == NULL )
      { printf("ERROR: Failed to allocate storage for verifier_n2!\n"); exit(EXIT_FAILURE); }
   if ( (SHP.XOR_nonce = (unsigned char *)calloc(SHP.num_required_nonce_bytes, sizeof(unsigned char))) == NULL )
      { printf("ERROR: Failed to allocate storage for XOR_nonce!\n"); exit(EXIT_FAILURE); }

   SHP.vec_chunk_size = CHLNG_CHUNK_SIZE; 
   SHP.XMR_val = XMR_VAL;

   memcpy((char *)SHP.AES_IV, (char *)AES_IV, AES_IV_NUM_BYTES);

   SHP.SE_target_num_key_bits = SE_TARGET_NUM_KEY_BITS; 
   SHP.SE_final_key = NULL;
   SHP.authen_min_bitstring_size = AUTHEN_MIN_BITSTRING_SIZE;

// KEK information presumably stored in NVM for regeneration, preserved here in separate fields.
   SHP.KEK_target_num_key_bits = KEK_TARGET_NUM_KEY_BITS;
   SHP.KEK_final_enroll_key = NULL;
   SHP.KEK_final_regen_key = NULL;
   SHP.KEK_final_XMR_SHD = NULL;

// 5_11_2021: For tracking the number of minority bit flips with KEK FSB mode (NOT NE mode).
   SHP.KEK_BS_regen_arr = NULL;

   SHP.KEK_final_SpreadFactors_enroll = NULL;

   SHP.KEK_num_vecs = 0;
   SHP.KEK_num_rise_vecs = 0;;
   SHP.KEK_has_masks = 1;
   SHP.KEK_first_vecs_b = NULL;
   SHP.KEK_second_vecs_b = NULL;
   SHP.KEK_masks_b = NULL;
   if ( (SHP.KEK_XOR_nonce = (unsigned char *)calloc(SHP.num_required_nonce_bytes, sizeof(unsigned char))) == NULL )
      { printf("ERROR: Failed to allocate storage for XOR_nonce!\n"); exit(EXIT_FAILURE); }
   SHP.num_direction_chlng_bits = NUM_DIRECTION_CHLNG_BITS;

// For Special KEK mode data from hardware. Will eventually be eliminated once I change the VHDL to do this in hardware.
   SHP.KEK_num_iterations = 0;

// Allocate space for the authentication nonce received from server during device authentication or generated locally
// for transmission to server for server authentication.
   if ( (SHP.KEK_authentication_nonce = (unsigned char *)calloc(num_KEK_authen_nonce_bytes, sizeof(unsigned char))) == NULL )
      { printf("ERROR: Failed to allocate storage for KEK_authentication_nonce!\n"); exit(EXIT_FAILURE); }
   SHP.num_KEK_authen_nonce_bits = num_KEK_authen_nonce_bytes*8;
   SHP.num_KEK_authen_nonce_bits_remaining = SHP.num_KEK_authen_nonce_bits;
   SHP.DA_cobra_key = NULL;

// XMR_SHD that is generated during KEK_DeviceAuthentication during each iteration (to be concatenated to a larger blob and 
// sent to server).
   if ( (SHP.KEK_authen_XMR_SHD_chunk = (unsigned char *)calloc(SHP.num_required_PNDiffs/8, sizeof(unsigned char))) == NULL )
      { printf("ERROR: Failed to allocate storage for KEK_authen_XMR_SHD_chunk!\n"); exit(EXIT_FAILURE); }

   SHP.num_vecs = 0;
   SHP.num_rise_vecs = 0;;
   SHP.has_masks = 1;
   SHP.first_vecs_b = NULL;
   SHP.second_vecs_b = NULL;
   SHP.masks_b = NULL;

   SHP.PeerTrust_LLK = NULL;

   SHP.param_LFSR_seed_low = 0;
   SHP.param_LFSR_seed_high = 0;
   SHP.param_RangeConstant = RANGE_CONSTANT;
   SHP.param_SpreadConstant = SPREAD_CONSTANT;
   SHP.param_Threshold = THRESHOLD_CONSTANT;
   SHP.param_TrimCodeConstant = TRIMCODE_CONSTANT;
   SHP.param_PCR_or_PBD_or_PO = PCR_or_PBD_or_PO;

// 10_28_2022: Get the personalized ScalingConstant from the command line. NOTE: This is passed into the state machine as a FIXED POINT value
// with SCALING_PRECISION_NB bits of precision (currently 11 bits), xxxxx.xxxxxxxxxxx. Convert from floating point to scaled integer. So a 
// scaling value of 1.0 will be equal to 1 << SCALING_PRECISION_NB, which is 2^11 = 2048 (0000100000000000). NOTE: MyScalingConstant VALUE MUST BE POSITIVE and 
// between 1.0 and x.0 (current 5.0) above. So values here are between 4096 and 20480.
// 11_12_2022: Adding this 'do_scaling' flag, and initializing it to 0. COBRA and possibly SKE (PARCE) are the only functions that set it to 1.
   SHP.do_scaling = 0;
   SHP.MyScalingConstant = (int)(command_line_SC * pow(2.0, (float)SCALING_PRECISION_NB));

// Sanity check
   if ( SHP.MyScalingConstant < 0 || SHP.MyScalingConstant > (MAX_SCALING_VALUE << SCALING_PRECISION_NB) )
      { printf("ERROR: MyScalingConstant MUST be >= 0 and <= %d\n", MAX_SCALING_VALUE << SCALING_PRECISION_NB); exit(EXIT_FAILURE); }

if ( SHP.MyScalingConstant == (1 << SCALING_PRECISION_NB) )
   { printf("NO SCALING WILL OCCUR: ScalingConstant IS 1.0\n"); fflush(stdout); }
else
   { printf("ScalingConstant: %f\tScaled FixedPoint %d\n", command_line_SC, SHP.MyScalingConstant); fflush(stdout); }
#ifdef DEBUG
#endif

// The PL-side TRNG_LFSR is 64 bits. Note that we currently only suport loading the low-order 16-bits of the seed register. 
   SHP.TRNG_LFSR_seed = TRNG_LFSR_seed;

// For frequency statistics of the TRNG. Need to declare these here for the TTP -- can NOT make them static in multi-threaded apps.
   SHP.num_ones = 0; 
   SHP.total_bits = 0; 
   SHP.iteration = 0;

// 10_31_2021: We are now using a seed to specify the vector sequence on the device, TTP and verifier. When challenges are selected, we depend
// on the sequence returned by rand() to be the same no matter where this routine runs, device, TT or verifier. The verifier and TTP are multi-threaded
// and therefore it is possible that multiple threads call this routine simultaneously, interrupting the sequence generated by rand() (rand is NOT re-entrant).
// If this occurs, then the vector challenges used by the, e.g., verifier and device will be different and the security function will fail. When the device
// calls this function, the mutex is NULL. 
   SHP.GenChallenge_mutex_ptr = NULL;

   SHP.do_COBRA = DO_COBRA;

   SHP.DUMP_BITSTRINGS = DUMP_BITSTRINGS;
   SHP.DEBUG_FLAG = DEBUG_FLAG;

   char my_info_str[MAX_STRING_LEN];
   char ack_str[MAX_STRING_LEN];
   int authen_num, load_seed; 

   SHP.use_database_chlngs = 1;

   load_seed = 0;
   authen_num = 0;
   printf("\nAUTHENTICATION NUMBER %d\n", authen_num); fflush(stdout);

   SHP.do_COBRA = 0;
   while ( OpenSocketClient(MAX_STRING_LEN, Bank_IP, port_number, &Bank_socket_desc) < 0 )
      { printf("INFO: Waiting to connect to Bank to send MY ID!\n"); fflush(stdout); usleep(200000); }

   if ( SockSendB((unsigned char *)"CLIENT-AUTHENTICATION", strlen("CLIENT-AUTHENTICATION") + 1, Bank_socket_desc) < 0 )
      { printf("ERROR: Failed to send 'CLIENT-AUTHENTICATION' to Bank!\n"); exit(EXIT_FAILURE); }

   sprintf(my_info_str, "%d %f %s %d", SHP.chip_num, command_line_SC, My_IP, my_bitstream);
   if ( SockSendB((unsigned char *)my_info_str, strlen(my_info_str) + 1, Bank_socket_desc) < 0 )
      { printf("ERROR: main(): Failed to send my IP and bitstream number to Bank!\n"); exit(EXIT_FAILURE); }
   if ( SockGetB((unsigned char *)ack_str, MAX_STRING_LEN, Bank_socket_desc) != 4  )
      { printf("ERROR: Failed to get 'ACK' from Bank!\n"); exit(EXIT_FAILURE); }
   if ( strcmp(ack_str, "ACK") != 0 )
      { printf("ERROR: Failed to match 'ACK' string from Bank!\n"); exit(EXIT_FAILURE); }
// -------------------

   TRNG(MAX_STRING_LEN, &SHP, FUNC_INT_TRNG, load_seed, 0, NULL);

   if ( KEK_ClientServerAuthen(MAX_STRING_LEN, &SHP, Bank_socket_desc) == 0 )
      { printf("ERROR: SKE MODE: FAILED TO AUTHENICATE!\n"); exit(EXIT_FAILURE); }

   close(Bank_socket_desc);

// The Challenges DB is read-only.
   sqlite3_close(DB_Challenges);

   fflush(stdout);
   system("sync");
   return 0;
   }
