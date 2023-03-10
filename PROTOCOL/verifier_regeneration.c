// ========================================================================================================
// ========================================================================================================
// **************************************** verifier_regeneration.c ***************************************
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
#include "verifier_common.h"
#include "verifier_regen_funcs.h"
#include "commonDB_RT.h"
#include <signal.h>

extern struct tm *localtime_r (const time_t *__restrict __timer,
			       struct tm *__restrict __tp) __THROW;
extern int usleep (__useconds_t __useconds);

// THIS MUST BE THE SAME as that defined in device_common.h

//static volatile int keepRunning = 1;

typedef struct
   {
   int task_num;
   int iteration_cnt;
   SRFAlgoParamsStruct *SAP_ptr;
   int TTP_request;
   int Device_socket_desc;
   int port_number;
   int RANDOM;
   int in_use;
   int client_index;
   int *client_sockets; 
   int TTP_num; 
   unsigned char **TTP_session_keys;
   int num_TTPs;
   char **TTP_IPs;
   int num_customers;
   char **customer_IPs;
   int ip_length;
   int max_string_len; 
   int num_preauth_n3_bytes; 
   int num_eCt_nonce_bytes;
   int *TTP_socket_descs;
   AccountStruct *Accounts_ptr;
   pthread_mutex_t Thread_mutex;
   pthread_cond_t Thread_cv;
   } ThreadDataType;


// ========================================================================================================
// ========================================================================================================
// Device or TTP sends it's ID, (IP and bitstream number, 1 to 4). 

void GetClientIDInformation(int max_string_len, SRFAlgoParamsStruct *SAP_ptr, int client_socket_desc, 
   int task_num, int iteration_cnt)
   {
   char my_info_str[max_string_len];

   if ( SockGetB((unsigned char *)my_info_str, max_string_len, client_socket_desc) < 0 )
      { printf("ERROR: GetClientIDInformation(): Error receiving 'my_info_str' from client!\n"); exit(EXIT_FAILURE); }

printf("ID str fetched from device %s\n", my_info_str); fflush(stdout); 
#ifdef DEBUG
#endif

   if ( SAP_ptr->my_IP != NULL )
      free(SAP_ptr->my_IP);

// The my_info_str length is longer than the IP because it has the bitstream number too but that's fine.
   if ( (SAP_ptr->my_IP = (char *)calloc(strlen(my_info_str) + 1, sizeof(char))) == NULL )
      { printf("ERROR: GetClientIDInformation(): Failed to allocated storage for IP!\n"); exit(EXIT_FAILURE); }

   if ( sscanf(my_info_str, "%d %f %s %d", &(SAP_ptr->my_chip_num), &(SAP_ptr->my_scaling_constant), SAP_ptr->my_IP, &(SAP_ptr->my_bitstream)) != 4 )
      { printf("ERROR: GetClientIDInformation(): Failed to sscanf the Chip num, ScalingConstant, IP, bitstream number!\n"); exit(EXIT_FAILURE); }

printf("Fetched Chip %3d\tScalingConstant %f\tIP %s and Bitstream number %d from client!\n", 
   SAP_ptr->my_chip_num, SAP_ptr->my_scaling_constant, SAP_ptr->my_IP, SAP_ptr->my_bitstream); fflush(stdout); 
#ifdef DEBUG
#endif

// Send ACK to the Alice/Bob to allow it to continue
   if ( SockSendB((unsigned char *)"ACK", strlen("ACK") + 1, client_socket_desc) < 0  )
      { printf("ERROR: GetClientIDInformation(): Failed to send 'ACK' to client!\n"); exit(EXIT_FAILURE); }

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Device thread.

void BankThread(ThreadDataType *ThreadDataPtr)
   {
   SRFAlgoParamsStruct *SAP_ptr;
   int TTP_request;
   int Device_socket_desc;
   int client_index;
   int *client_sockets;
   int max_string_len;
   int RANDOM;

   int task_num, iteration_cnt;

// Making this static here makes it global to all threads.
   static pthread_mutex_t RT_DB_mutex = PTHREAD_MUTEX_INITIALIZER;
   static pthread_mutex_t FileStat_mutex = PTHREAD_MUTEX_INITIALIZER;
   static pthread_mutex_t GenChallenge_mutex = PTHREAD_MUTEX_INITIALIZER;
   static pthread_mutex_t Authentication_mutex = PTHREAD_MUTEX_INITIALIZER;

   static pthread_mutex_t PUFCash_WRec_DB_mutex = PTHREAD_MUTEX_INITIALIZER;
   static pthread_mutex_t PUFCash_POP_DB_mutex = PTHREAD_MUTEX_INITIALIZER;

printf("BankThread: CALLED!\t(Task %d\tIterationCnt %d)\n", ThreadDataPtr->task_num, ThreadDataPtr->iteration_cnt); fflush(stdout);
#ifdef DEBUG
#endif

// Threads never return
   while (1)
      {

// Sleep waiting for the main program to receive connect request from Alice or a TTP, and assign a device_socket_desc
// No CPU cycles are wasted here in a busy wait, which is important when we query TTPs for performance information.
      pthread_mutex_lock(&(ThreadDataPtr->Thread_mutex));
      while ( ThreadDataPtr->in_use == 0 )
         pthread_cond_wait(&(ThreadDataPtr->Thread_cv), &(ThreadDataPtr->Thread_mutex));
      pthread_mutex_unlock(&(ThreadDataPtr->Thread_mutex));

      task_num = ThreadDataPtr->task_num;
      iteration_cnt = ThreadDataPtr->iteration_cnt;

// Get local copies/pointers from the data structure.
      SAP_ptr = ThreadDataPtr->SAP_ptr;
      TTP_request = ThreadDataPtr->TTP_request;
      Device_socket_desc = ThreadDataPtr->Device_socket_desc;
      client_index = ThreadDataPtr->client_index;
      client_sockets = ThreadDataPtr->client_sockets;
      max_string_len = ThreadDataPtr->max_string_len;
      RANDOM = ThreadDataPtr->RANDOM;

// If we set SAP_ptr->use_database_chlngs to 1, then we can NOT allow more than one thread to run GenChallengeDB() at the same time because we
// call rand(). If the device or TTP is going to get the same set of random numbers as this verifier is than after srand() is seeded with 
// SAP_ptr->DB_ChallengeGen_seed, we must block other threads until the vector sequence is completely generated, i.e., rand() is NOT re-entrant!
// We can do this in main() or here once we've initialized the GenChallenge_mutex mutex above (it is static and therefore global to all threads).
      SAP_ptr->RT_DB_mutex_ptr = &RT_DB_mutex;
      SAP_ptr->FileStat_mutex_ptr = &FileStat_mutex;
      SAP_ptr->GenChallenge_mutex_ptr = &GenChallenge_mutex;
      SAP_ptr->Authentication_mutex_ptr = &Authentication_mutex; 

      SAP_ptr->PUFCash_WRec_DB_mutex_ptr = &PUFCash_WRec_DB_mutex;
      SAP_ptr->PUFCash_POP_DB_mutex_ptr = &PUFCash_POP_DB_mutex;

// Get the request
      char client_request_str[max_string_len];
      int client_request;

struct timeval t1, t2;
long elapsed; 
gettimeofday(&t2, 0);
#ifdef DEBUG
#endif

      if ( SockGetB((unsigned char *)client_request_str, max_string_len, Device_socket_desc) < 0 )
         { printf("ERROR: BankThread(): Error receiving 'client_request_str' from Device or TTP (TTP_request? %d)!\n", TTP_request); exit(EXIT_FAILURE); }

printf("BankThread(): Client request '%s'\tIs TTP request %d\tIterationCnt %d\n", client_request_str, TTP_request, iteration_cnt); fflush(stdout);
#ifdef DEBUG
#endif

// ========================================================
// Supported client requests
      client_request = -1;
      if ( strcmp(client_request_str, "CLIENT-AUTHENTICATION") == 0 )
         client_request = 17;
      if ( client_request == -1 )
         { printf("ERROR: BankThread(): Unknown 'client_request' '%s'!\n", client_request_str); exit(EXIT_FAILURE); }

// ===============================================================
// Authentication only 
      if ( strcmp(client_request_str, "CLIENT-AUTHENTICATION") == 0 )
         {

// TESTING ONLY: Get chip information
         GetClientIDInformation(max_string_len, SAP_ptr, Device_socket_desc, task_num, iteration_cnt);

printf("CLIENT-AUTHENTICATION: BankThread(): Request from socket %d at index %d!\tIterationCnt %d\n", 
   Device_socket_desc, client_index, iteration_cnt); fflush(stdout);
#ifdef DEBUG
#endif
         int prev_udc = SAP_ptr->use_database_chlngs;
         SAP_ptr->use_database_chlngs = 1;

         KEK_ClientServerAuthen(max_string_len, SAP_ptr, Device_socket_desc, RANDOM);

         SAP_ptr->use_database_chlngs = prev_udc;
         }

// ===============================================================
// Unknown message
      else
         { printf("ERROR: BankThread(): Unknown client request '%s'!\n", client_request_str); exit(EXIT_FAILURE); }


// ===============================================================
// ===============================================================
// Close the socket descriptor if the request is from Alice (do NOT close TTP socket descriptors).
      if ( TTP_request == 0 )
         {
         close(Device_socket_desc);

// Make the socket descriptor processed by this thread available again to 'select' in OpenMultipleServerSocket.
// Note this is a shared array among the threads but no semaphore needed here because each thread updates a unique element
// given by client_index.
         client_sockets[client_index] = 0;
         }

// If a TTP request, then restore activity on this socket_descriptor? Note that I assign a TTP socket descriptor to Device_socket_desc
// when the request is from a TTP in main when this thread is spun up. I also 'disable' this TTP socket descriptor by assigning a -1
// while this thread executes. At least this is what I can deduce on 10_29_2021.
      else
         client_sockets[client_index] = Device_socket_desc;

gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tElapsed: For '%s' %ld us\tIterationCnt %d\n\n", 
   client_request_str, (long)elapsed, iteration_cnt);
#ifdef DEBUG
#endif

// Indicate to the parent that this thread is available for reassignment.
      pthread_mutex_lock(&(ThreadDataPtr->Thread_mutex));
      ThreadDataPtr->in_use = 0;
      pthread_mutex_unlock(&(ThreadDataPtr->Thread_mutex));
      }

// Exit and clean up resources. Nope -- this generates some type of library required message -- an error. I'm not destroying threads
// any longer -- they get created and run forever.
//   pthread_exit(NULL);

// NEVER GETS EXECUTED anyway
#ifdef DEBUG
printf("TTPThread: DONE!\t(Task %d\tIterationCnt %d)\n", task_num, iteration_cnt); fflush(stdout);
#endif
   return;
   }



// ========================================================================================================
// ========================================================================================================
// ========================================================================================================
// ========================================================================================================
#define MAX_TTPS 20
#define MAX_CUSTOMERS 20

int client_sockets[MAX_CLIENTS];

int TTP_socket_descs[MAX_TTPS]; 
int TTP_socket_indexes[MAX_TTPS]; 
unsigned char *TTP_session_keys[MAX_TTPS]; 

#define MAX_THREADS 20
SRFAlgoParamsStruct SAP_arr[MAX_THREADS];

ThreadDataType ThreadDataArr[MAX_THREADS] = {
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   {0, 0, NULL, 0, -1, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, NULL, NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER},
   };


// ============================================================================
// ============================================================================

int main(int argc, char *argv[])
   {
   char *Bank_server_IP;
   char *client_IP;

   char *TTP_IP_list_filename;
   char *customer_IP_list_filename;
   int num_chips;

   char *MasterDB_prefix;
   char *DB_name_NAT;
   char *DB_name_AT;
   char *DB_name_RunTime;
   char *Netlist_name;
   char *Synthesis_name;
   char *ChallengeSetName_NAT;
   char *ChallengeSetName_AT;

   sqlite3 *DB_NAT, *DB_AT, *DB_RunTime = NULL;

// PeerTrust
   char *DB_name_MAKE_PT_AT;
   sqlite3 *DB_MAKE_PT_AT;

// PUF-Cash V3.0
   char *DB_name_PUFCash_V3;
   sqlite3 *DB_PUFCash_V3;

   int rc;

// Declare sockaddr_in structures 
   int Bank_server_socket_desc = 0;
   int Device_socket_desc = 0;

   char **TTP_IPs = NULL;
   int num_TTPs;
   int TTP_num; 
   int TTP_request; 

// PeerTrust
   char **customer_IPs;
   int num_customers;

   int client_index;
   int first_time;
   int SD;

   int port_number;

   int RANDOM;

   int iteration, num_iterations;

   int fix_params; 

   int design_index;
   int num_PIs, num_POs;

   int use_database_chlngs;
   unsigned int ChallengeGen_seed;
   unsigned int SpreadFactor_random_seed;

   int num_eCt_nonce_bytes; 
   int num_KEK_authen_nonce_bytes; 

   int use_TVC_cache; 

   int gen_random_challenge; 

   int do_save_bitstrings_to_RT_DB;
   int do_save_PARCE_COBRA_file_stats;
   int do_save_COBRA_SHD;
   int do_save_SKE_SHD;

   int read_db_into_memory;

   int max_chips, chip_num;

   int thread_num; 

   int PCR_or_PBD_or_PO; 

   int DUMP_BITSTRINGS;
   int DEBUG_FLAG;

   int do_PO_dist_flip; 

   Allocate1DString((char **)(&Bank_server_IP), MAX_STRING_LEN);
   Allocate1DString((char **)(&client_IP), MAX_STRING_LEN);
   Allocate1DString((char **)(&customer_IP_list_filename), MAX_STRING_LEN);
   Allocate1DString((char **)(&TTP_IP_list_filename), MAX_STRING_LEN);
   Allocate1DString((char **)(&MasterDB_prefix), MAX_STRING_LEN);
   Allocate1DString((char **)(&DB_name_NAT), MAX_STRING_LEN);
   Allocate1DString((char **)(&DB_name_AT), MAX_STRING_LEN);
   Allocate1DString((char **)(&DB_name_RunTime), MAX_STRING_LEN);
   Allocate1DString((char **)(&Netlist_name), MAX_STRING_LEN);
   Allocate1DString((char **)(&Synthesis_name), MAX_STRING_LEN);
   Allocate1DString((char **)(&ChallengeSetName_NAT), MAX_STRING_LEN);
   Allocate1DString((char **)(&ChallengeSetName_AT), MAX_STRING_LEN);

// ===============================================================================
   if ( argc != 6 )
      { 
      printf("Parameters: Master Database prefix (Master_TDC) -- PUF Netlist name (SR_RFM_V4_TDC) -- PUF Synthesis name (SRFSyn1) -- Bank IP (192.168.1.20) -- ChallengeSetName (Master1_OptKEK_TVN_0.00_WID_1.75)\n"); 
      exit(EXIT_FAILURE); 
      }

   strcpy(MasterDB_prefix, argv[1]);
   strcpy(Netlist_name, argv[2]);
   strcpy(Synthesis_name, argv[3]);
   strcpy(Bank_server_IP, argv[4]);
//   strcpy(TTP_IP_list_filename, argv[5]);
   strcpy(ChallengeSetName_NAT, argv[5]);
//   sscanf(argv[7], "%d", &fix_params);
//   sscanf(argv[8], "%d", &gen_random_challenge);
//   sscanf(argv[9], "%d", &num_iterations);
//   sscanf(argv[10], "%d", &PCR_or_PBD_or_PO);

// Removed these command line args.
   fix_params = 0;
   gen_random_challenge = 1;
   num_iterations = 100000;
   PCR_or_PBD_or_PO = 0;

   printf("PARAMETERS:\n\tMasterDB '%s'\n\tPUFNetlistName '%s'\n\tPUF SynName '%s'\n\tBank IP '%s'\n\tChlngSetName '%s'\n\tFIXED %d\n\tRandomChlng %d\n\tNumIter %d\n\tPCR/PBD/PO %d\n\n", 
      MasterDB_prefix, Netlist_name, Synthesis_name, Bank_server_IP, ChallengeSetName_NAT, fix_params, gen_random_challenge, num_iterations, PCR_or_PBD_or_PO); fflush(stdout);

// ====================================================== PARAMETERS ====================================================
// PeerTrust. Set the name of the file that contains the device IPs
   strcpy(customer_IP_list_filename, "customer_IP_list.txt"); 

// Used to be a command line option.
   strcpy(TTP_IP_list_filename, "TTP_IP_list.txt"); 

// Flag used to enable server-side distribution flipping. NOTE: THE INITIAL VALUE assigned here does NOTHING since it is set
// and reset in the individual routines, e.g., it is asserted in KEK_VerifierAuthentication and KEK_SessionKeyGen, and deasserted
// in KEK_DeviceAuthentication_SKE, KEK_DeviceAuthentication_Cobra and KEK_EnrollProvisioning. For DA and LLK_Enroll, the device 
// needs to receive PURE PopOnly SF and it will modify them itself with the distribution flipping.
   do_PO_dist_flip = 0;

// Set this to 1 to create an in-memory version of the database. Memory limited operation but it at least 100 times faster.
   read_db_into_memory = 1;

// Setting this to 1 creates a PN cache. Here, the qualifing PNs (according to the ChallengeSetName) for all chips 
// are read out of the database and stored in the TimingValCacheStruct array for very quick access. USE THIS TOO with the
// in memory copy.
   use_TVC_cache = 1;

   char AES_IV[AES_IV_NUM_BYTES] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};

// Copying this for now since I'm copy the Master_NAT.db to the Master_AT.db but eventually this will become a command line 
// parameter for the AT_Master.db.
   strcpy(ChallengeSetName_AT, ChallengeSetName_NAT);

// Temporary Database used to store run time information. THIS DATABASE SHOULD BE EMPTY when this code starts up (although
// it doesn't hurt that it has information).
   strcpy(DB_name_RunTime, "RunTime.db");

// Peer-Trust
   Allocate1DString(&DB_name_MAKE_PT_AT, MAX_STRING_LEN);
   strcpy(DB_name_MAKE_PT_AT, "AuthenticationToken.db");

// PUF-Cash V3.0
   Allocate1DString((char **)(&DB_name_PUFCash_V3), MAX_STRING_LEN);
   strcpy(DB_name_PUFCash_V3, "PUFCash_V3.db");

// NORMALLY 0
// Setting this to 1 writes a lot of data to the RunTime database regarding the bitstrings produced during DA, VA and SE
// operations. FOR PERFORMANCE MEASUREMENTS, THIS SHOULD BE DISABLED. In this KEK version of the protocol, only XMR_SHD
// generated in SKE/FSB mode (Device and Verifier authentication) and XMR_SHD and session key generated in FSB mode 
// (SessionKeyGen) are available for saving. Not sure yet what can be done statistics-wise for the XMR_SHD.
   do_save_bitstrings_to_RT_DB = 0;

// NORMALLY 0
// Setting this to 1 writes a lot of data to an (x,y) file I can use for plotting to determine PARCE distinguishability.
// FOR PERFORMANCE MEASUREMENT, THIS SHOULD BE DISABLED. The stats files contains data sets for each authentication where
// I effectively create several files. The first one gives the num_mismatches for the authenticated chip (NOTE: Right now, 
// num_mismatches MUST be zero otherwise the authentication fails). The second file plots the num_minority_bit_flips 
// for the authenticated chip. The third file plots the non-authentic chip with the smallest num_mismatches. The fourth
// file plots the num_minority_bit_flips for this chip. The fifth file plots the num_mismatches + num_minority_bit_flips
// for the remaining chips.
   do_save_PARCE_COBRA_file_stats = 0;

// NORMALLY 0
// 10_10_2022: Set this 1 to save the COBRA SHD for statistical analysis. Writes data to ../PROTOCOL_V3.0_TDC/KEK_Authentication_SHD/
// for each chip.
   do_save_COBRA_SHD = 0;
   do_save_SKE_SHD = 0;

// NOTE: ASSUMPTION:
//    NUM_XOR_NONCE_BYTES   <=  num_eCt_nonce_bytes   <=   SE_TARGET_NUM_KEY_BITS/8   <=   NUM_REQUIRED_PNDIFFS/8
//           8                         16                            32                              256
// We always use 8 bytes for the XOR_nonce (NUM_XOR_NONCE_BYTES) to SelectParams. My plan is to use 16 byte nonces
// (num_eCt_nonce_bytes), 32 bytes AES session keys (256 bits for SE_TARGET_NUM_KEY_BITS) and NUM_REQUIRED_PNDIFFS are always 
// 2048/16 = 256. 
   num_eCt_nonce_bytes = SE_TARGET_NUM_KEY_BITS/8;
   num_KEK_authen_nonce_bytes = KEK_AUTHEN_NUM_NONCE_BITS/8; 

// If the device uses it's own database to get the challenges. Once I've converted everything over, I'll set this to 1.
   use_database_chlngs = 0;

// If gen_random_challenge is set to 1, then a random seed is selected for each of the calls to GenChallengeDB, otherwise, this
// value is used for ALL calls (which means the same challenge will be used over and over again). NOTE: Do not use 0 here,
// it generates the same output as 1 -- must be illegal.
   ChallengeGen_seed = 1;

// Default seed assignment used in PCR mode. We will occasionally need to reseed because there are occasions where we require
// the same SpreadFactors to be generated twice (VerifierAuthentication and SessionKeyGen).
   SpreadFactor_random_seed = 0;

   port_number = 8888;

// Set this to the maximum number of chips that are to be preserved in the 'in-memory' database. NOTE: 'read_db_into_memory'
// MUST be set to 1 for this to work. Setting to -1 disables any deletions, i.e., ALL chips from the database are kept
// in the in-memory version. DOES NOT WORK ANY LONGER (Must be set to -1) after adding the AT database. See note below.
   max_chips = -1;

// Debug parameters that control the amount of output generated.
   DUMP_BITSTRINGS = 0;
   DEBUG_FLAG = 0;
// ====================================================== PARAMETERS ====================================================
// Sanity check. With SF stored in (signed char) now, we can NOT allow TrimCodeConstant to be any larger than 64. See
// log notes on 1_1_2022.
   if ( TRIMCODE_CONSTANT > 64 )
      { printf("ERROR: main(): TRIMCODE_CONSTANT %d MUST be <= 64\n", TRIMCODE_CONSTANT); exit(EXIT_FAILURE); }

// Sanity check MAKE PROTOCOL. We also assume that the SHA-3 hash input and output are the same size as the AK_A/HK_A, 
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
         num_eCt_nonce_bytes %d == SE_TARGET_NUM_KEY_BITS/8 %d <= NUM_REQUIRED_PNDIFFS/8 %d\n",
         NUM_XOR_NONCE_BYTES, num_eCt_nonce_bytes, num_eCt_nonce_bytes, SE_TARGET_NUM_KEY_BITS/8, NUM_REQUIRED_PNDIFFS/8);
      exit(EXIT_FAILURE);
      }

// Allocate space and then read the list of TTP IPs. IP_LENGTH is 16 characters, 15 for xxx.xxx.xxx.xxx and 1 for NULL termination.
if (0)
   {
   TTP_IPs = Allocate2DCharArray(MAX_TTPS, IP_LENGTH);
   if ( (num_TTPs = ReadXFile(MAX_STRING_LEN, TTP_IP_list_filename, MAX_TTPS, TTP_IPs)) == 0 )
      ;
//      exit(EXIT_FAILURE);

// Create invalid sockets for all TTPs initially
   int i;
   for ( i = 0; i < MAX_TTPS; i++ )
      {
      TTP_socket_descs[i] = -1; 
      TTP_socket_indexes[i] = -1; 
      }

// Check for at least the correct number of characters.
   for ( i = 0; i < num_TTPs; i++ )
      {
      if ( strlen(TTP_IPs[i]) < 7 || strlen(TTP_IPs[i]) > 15 )
         { printf("ERROR: IP in '%s' MUST be between 7 characters and 15 characters => '%s'!\n", TTP_IP_list_filename, TTP_IPs[i]); exit(EXIT_FAILURE); }

// Make sure all TTP session keys are NULL initially
      TTP_session_keys[i] = NULL;
      }
   }
else
   {
   num_TTPs = 0;
   TTP_IPs = NULL;
   }

// Allocate space and then read the list of customer IPs. IP_LENGTH is 16 characters, 15 for xxx.xxx.xxx.xxx and 1 for NULL termination.
if (0)
   {
   customer_IPs = Allocate2DCharArray(MAX_CUSTOMERS, IP_LENGTH);
   if ( (num_customers = ReadXFile(MAX_STRING_LEN, customer_IP_list_filename, MAX_CUSTOMERS, customer_IPs)) == 0 )
      exit(EXIT_FAILURE);
#ifdef DEBUG
printf("Read %d customer IPs\n", num_customers); fflush(stdout);
int customer_num;
for ( customer_num = 0; customer_num < num_customers; customer_num++ )
   printf("\tCustomer IP '%s'\n", customer_IPs[customer_num]);
fflush(stdout);
#endif
   }
else
   {
   num_customers = 0;
   customer_IPs = NULL;
   }

// Create non-anonymous and anonymous transaction database names. We have not yet made the anonymous database truely anonymous yet -- just a copy of NAT
   strcpy(DB_name_NAT, "NAT_");
   strcat(DB_name_NAT, MasterDB_prefix);
   strcat(DB_name_NAT, ".db");

   strcpy(DB_name_AT, "AT_");
   strcat(DB_name_AT, MasterDB_prefix);
   strcat(DB_name_AT, ".db");

// Highly recommended that this is set to 1 unless database is too big to read into memory.
   if ( read_db_into_memory == 0 )
      {

// Third arg to sqlite3_open_v2 forced serialized mode, which makes it thread-safe with NO restrictions
      rc = sqlite3_open_v2(DB_name_NAT, &DB_NAT, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, NULL);
      if ( rc != 0 )
         { printf("Failed to open Database: %s\n", sqlite3_errmsg(DB_NAT)); sqlite3_close(DB_NAT); exit(EXIT_FAILURE); }

      rc = sqlite3_open_v2(DB_name_AT, &DB_AT, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, NULL);
      if ( rc != 0 )
         { printf("Failed to open Database: %s\n", sqlite3_errmsg(DB_AT)); sqlite3_close(DB_AT); exit(EXIT_FAILURE); }

// Peer-Trust
if (0)
   {
      rc = sqlite3_open_v2(DB_name_MAKE_PT_AT, &DB_MAKE_PT_AT, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, NULL);
      if ( rc != 0 )
         { printf("Failed to open Database: %s\n", sqlite3_errmsg(DB_MAKE_PT_AT)); sqlite3_close(DB_MAKE_PT_AT); exit(EXIT_FAILURE); }

// PUF-Cash V3.0
      rc = sqlite3_open_v2(DB_name_PUFCash_V3, &DB_PUFCash_V3, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, NULL);
      if ( rc != 0 )
         { printf("Failed to open Database: %s\n", sqlite3_errmsg(DB_PUFCash_V3)); sqlite3_close(DB_PUFCash_V3); exit(EXIT_FAILURE); }
   }
      }

// Using an in-memory version speeds this whole process up by about a factor of 100. 
   else
      {

// Open an in-memory database.
      rc = sqlite3_open_v2(":memory:", &DB_NAT, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, NULL);
      printf("Reading filesystem database '%s' into memory!\n", DB_name_NAT); fflush(stdout);
      if ( LoadOrSaveDb(DB_NAT, DB_name_NAT, 0) != 0 )
         { printf("Failed to open and copy into memory the Database: %s\n", sqlite3_errmsg(DB_NAT)); sqlite3_close(DB_NAT); exit(EXIT_FAILURE); }

      rc = sqlite3_open_v2(":memory:", &DB_AT, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, NULL);
      printf("Reading filesystem database '%s' into memory!\n", DB_name_AT); fflush(stdout);
      if ( LoadOrSaveDb(DB_AT, DB_name_AT, 0) != 0 )
         { printf("Failed to open and copy into memory the Database: %s\n", sqlite3_errmsg(DB_AT)); sqlite3_close(DB_AT); exit(EXIT_FAILURE); }

if (0)
   {
      rc = sqlite3_open_v2(":memory:", &DB_MAKE_PT_AT, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, NULL);
      printf("Reading filesystem database '%s' into memory!\n", DB_name_MAKE_PT_AT); fflush(stdout);
      if ( LoadOrSaveDb(DB_MAKE_PT_AT, DB_name_MAKE_PT_AT, 0) != 0 )
         { printf("Failed to open and copy into memory the Database: %s\n", sqlite3_errmsg(DB_MAKE_PT_AT)); sqlite3_close(DB_MAKE_PT_AT); exit(EXIT_FAILURE); }

      rc = sqlite3_open_v2(":memory:", &DB_PUFCash_V3, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, NULL);
      printf("Reading filesystem database '%s' into memory!\n", DB_name_PUFCash_V3); fflush(stdout);
      if ( LoadOrSaveDb(DB_PUFCash_V3, DB_name_PUFCash_V3, 0) != 0 )
         { printf("Failed to open and copy into memory the Database: %s\n", sqlite3_errmsg(DB_PUFCash_V3)); sqlite3_close(DB_PUFCash_V3); exit(EXIT_FAILURE); }
   }

// ========
// PERFORMANCE EVAL ONLY: Delete from the in-memory database ALL chips (PUF Instances) beyond a certain number so we can determine the timing scalability
// of this program. Be sure 'PRAGMA foreign_keys = ON' is set to enable CASCADE mode. NOTE: This will NOT WORK ANY longer after I added AT database
// on 5/20/2019 -- NEED A DIFFERENT METHOD to delete the chips since the data is scrambled in the AT database.
      if ( max_chips != -1 )
         {
         SQLIntStruct PUF_instance_index_struct;
         char *zErrMsg = 0;
         int fc;

         fc = sqlite3_exec(DB_NAT, "PRAGMA foreign_keys = ON", SQL_callback, 0, &zErrMsg);
         if ( fc != SQLITE_OK )
            { printf("SQL error: %s\n", zErrMsg); sqlite3_free(zErrMsg); }

// Get a list of all chip (PUFInstance_IDs) from the database.
         GetPUFInstanceIDsForInstanceName(MAX_STRING_LEN, DB_NAT, &PUF_instance_index_struct, "%");

         for ( chip_num = max_chips; chip_num < PUF_instance_index_struct.num_ints; chip_num++ )
            {
            printf("NAT DATABASE: Deleting chip %d with PUFInstance ID %d from 'in-memory' database\n", chip_num, PUF_instance_index_struct.int_arr[chip_num]); fflush(stdout);
            if ( DeletePUFInstance(MAX_STRING_LEN, DB_NAT, SQL_PUFInstance_delete_cmd, PUF_instance_index_struct.int_arr[chip_num]) == -1 )
               { 
               printf("ERROR: Failed to delete chip %d PUFInstance ID %d!\n", chip_num, PUF_instance_index_struct.int_arr[chip_num]); 
               sqlite3_close(DB_NAT);
               exit(EXIT_FAILURE); 
               }
            }

// Free up the list.
         if ( PUF_instance_index_struct.int_arr != NULL )
            free(PUF_instance_index_struct.int_arr);
         }
      }

// Open up the run-time database. Third arg to sqlite3_open_v2 forced serialized mode, which makes it thread-safe with NO restrictions
if (0)
   {
   rc = sqlite3_open_v2(DB_name_RunTime, &DB_RunTime, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, NULL);
   if ( rc != 0 )
      { printf("Failed to open RunTime Database: %s\n", sqlite3_errmsg(DB_RunTime)); sqlite3_close(DB_RunTime); exit(EXIT_FAILURE); }
   }


// Current parameters
   printf("PARAMETERS: NAT DB name '%s'\tAT DB name '%s'\tNetlist name '%s'\tSynthesis name '%s'\tChallengeSetName NAT %s ChallengeSetName AT %s\n\n", 
      DB_name_NAT, DB_name_AT, Netlist_name, Synthesis_name, ChallengeSetName_NAT, ChallengeSetName_AT); fflush(stdout);

// =====================================================================================================================================
// MISC
// Open up the random source for verifier to generate nonce bytes that will be used to SelectParams for authentication.
   if ( (RANDOM = open("/dev/urandom", O_RDONLY)) == -1 )
      { printf("ERROR: Could not open /dev/urandom\n"); exit(EXIT_FAILURE); }
   printf("\tSuccessfully open '/dev/urandom'\n");

// Initialize all client_sockets to 0. We do NOT DO THIS any longer in the call to OpenMultipleSocketServer below.
// We needed to remove this because ttp.c needs to set a client_socket after opening the connection to the Bank (this
// code). 
   int client_num;
   for ( client_num = 0; client_num < MAX_CLIENTS; client_num++) 
      { client_sockets[client_num] = 0; }

// =====================================================================================================================================
// =====================================================================================================================================
// ACCOUNT NUMBERS: DELETE ME -- Accounts are kept on the TTP in PUFCash V3.0
   SQLIntStruct PUF_instance_index_struct;
   AccountStruct Accounts;

// Get a list of all the chips (PUFInstance IDs) enrolled in the non-anonymous database. Use '%' for * and '_' for ?
   GetPUFInstanceIDsForInstanceName(MAX_STRING_LEN, DB_NAT, &PUF_instance_index_struct, "%");

// Sanity check
   if ( PUF_instance_index_struct.num_ints == 0 )
      { printf("ERROR: No PUFInstances in Master database!\n"); exit(EXIT_FAILURE); }

// 10_12_2022: This is used below to by ComputePersonalizedScalingConstants().
   num_chips = PUF_instance_index_struct.num_ints;

// Allocate storage for account numbers.
   Accounts.Accts_arr = (int *)malloc(sizeof(int) * num_chips);
   Accounts.PI_indexes = (int *)malloc(sizeof(int) * num_chips);
   Accounts.Amount_arr = (int *)malloc(sizeof(int) * num_chips);
   Accounts.Withdraw_ID = (int *)malloc(sizeof(int) * num_chips);

// Use the PUFInstance ID as the account number for now.
   for ( chip_num = 0; chip_num < num_chips; chip_num++ )
      {
      char Instance_name[MAX_STRING_LEN];
      char Dev[MAX_STRING_LEN];
      char Placement[MAX_STRING_LEN];

      Accounts.Accts_arr[chip_num] = chip_num;
      Accounts.PI_indexes[chip_num] = PUF_instance_index_struct.int_arr[chip_num];
      Accounts.Amount_arr[chip_num] = 100000000; 

// 11_14_2021: PUF-Cash 3.0
      Accounts.Withdraw_ID[chip_num] = 0; 

      GetPUFInstanceInfoForID(MAX_STRING_LEN, DB_NAT, Accounts.PI_indexes[chip_num], Instance_name, Dev, Placement);

printf("Created Account Number %3d for PUFInstance %3d for %5s, %10s, %5s, with $%d cents\n", Accounts.Accts_arr[chip_num], PUF_instance_index_struct.int_arr[chip_num], 
   Instance_name, Dev, Placement, Accounts.Amount_arr[chip_num]); fflush(stdout);
#ifdef DEBUG
#endif
      }
   Accounts.num_Accts = chip_num;

// Free up the list.
   if ( PUF_instance_index_struct.int_arr != NULL )
      free(PUF_instance_index_struct.int_arr);
   fflush(stdout);

// =====================================================================================================================================
// =====================================================================================================================================
// THREADS:
// Get the PUFDesign parameters from the NAT database. These MUST be identical to those stored in the AT_db.
   if ( GetPUFDesignParams(MAX_STRING_LEN, DB_NAT, Netlist_name, Synthesis_name, &design_index, &num_PIs, &num_POs) != 0 )
      { printf("ERROR: PUFDesign index NOT found for '%s', '%s'!\n", Netlist_name, Synthesis_name); exit(EXIT_FAILURE); }

   int check_design_index, check_num_PIs, check_num_POs;
   if ( GetPUFDesignParams(MAX_STRING_LEN, DB_AT, Netlist_name, Synthesis_name, &check_design_index, &check_num_PIs, &check_num_POs) != 0 )
      { printf("ERROR: PUFDesign index NOT found for '%s', '%s'!\n", Netlist_name, Synthesis_name); exit(EXIT_FAILURE); }
   if ( design_index != check_design_index || num_PIs != check_num_PIs || num_POs != check_num_POs )
      { printf("ERROR: Design information in the NAT and AT databases MUST be identcal!\n"); exit(EXIT_FAILURE); }


// -------------------------------------------
// Load up verifier data structure for the thread.
   for ( thread_num = 0; thread_num < MAX_THREADS; thread_num++ )
      {
      ThreadDataArr[thread_num].task_num = thread_num;

// Set the SAP_ptr to one of the static array elements. We need separate data structures for the SAP structure because it has fields
// that store data during authentication/session key generation. These don't need to be shared (and protected with a mutex).
      ThreadDataArr[thread_num].SAP_ptr = &SAP_arr[thread_num];

// Non-anonymous database
      ThreadDataArr[thread_num].SAP_ptr->database_NAT = DB_NAT;
      if ( (ThreadDataArr[thread_num].SAP_ptr->DB_name_NAT = (char *)malloc(sizeof(char) * strlen(DB_name_NAT) + 1)) == NULL )
         { printf("ERROR: Failed to allocate storage for DB_name_NAT!\n"); exit(EXIT_FAILURE); }
      strcpy(ThreadDataArr[thread_num].SAP_ptr->DB_name_NAT, DB_name_NAT);

// Anonymous database
      ThreadDataArr[thread_num].SAP_ptr->database_AT = DB_AT;
      if ( (ThreadDataArr[thread_num].SAP_ptr->DB_name_AT = (char *)malloc(sizeof(char) * strlen(DB_name_AT) + 1)) == NULL )
         { printf("ERROR: Failed to allocate storage for DB_name_AT!\n"); exit(EXIT_FAILURE); }
      strcpy(ThreadDataArr[thread_num].SAP_ptr->DB_name_AT, DB_name_AT);

// Runtime database for storing info during protocol runs.
      ThreadDataArr[thread_num].SAP_ptr->database_RT = NULL;

// PeerTrust
if (0)
   {
      ThreadDataArr[thread_num].SAP_ptr->DB_MAKE_PT_AT = DB_MAKE_PT_AT;
      if ( (ThreadDataArr[thread_num].SAP_ptr->DB_name_MAKE_PT_AT = (char *)malloc(sizeof(char) * strlen(DB_name_MAKE_PT_AT) + 1)) == NULL )
         { printf("ERROR: Failed to allocate storage for DB_name_MAKE_PT_AT!\n"); exit(EXIT_FAILURE); }
      strcpy(ThreadDataArr[thread_num].SAP_ptr->DB_name_MAKE_PT_AT, DB_name_MAKE_PT_AT);

// PUF-Cash V3.0
      ThreadDataArr[thread_num].SAP_ptr->DB_PUFCash_V3 = DB_PUFCash_V3;
      if ( (ThreadDataArr[thread_num].SAP_ptr->DB_name_PUFCash_V3 = (char *)malloc(sizeof(char) * strlen(DB_name_PUFCash_V3) + 1)) == NULL )
         { printf("ERROR: Failed to allocate storage for DB_name_PUFCash_V3!\n"); exit(EXIT_FAILURE); }
      strcpy(ThreadDataArr[thread_num].SAP_ptr->DB_name_PUFCash_V3, DB_name_PUFCash_V3);
   }
else
   {
      ThreadDataArr[thread_num].SAP_ptr->DB_MAKE_PT_AT = NULL;
      ThreadDataArr[thread_num].SAP_ptr->DB_PUFCash_V3 = NULL;
   }

      ThreadDataArr[thread_num].SAP_ptr->design_index = design_index;
      ThreadDataArr[thread_num].SAP_ptr->num_PIs = num_PIs;
      ThreadDataArr[thread_num].SAP_ptr->num_POs = num_POs;

      if ( (ThreadDataArr[thread_num].SAP_ptr->ChallengeSetName_NAT = (char *)malloc(sizeof(char) * strlen(ChallengeSetName_NAT) + 1)) == NULL )
         { printf("ERROR: Failed to allocate storage for ChallengeSetName_NAT!\n"); exit(EXIT_FAILURE); }
      strcpy(ThreadDataArr[thread_num].SAP_ptr->ChallengeSetName_NAT, ChallengeSetName_NAT);
      if ( (ThreadDataArr[thread_num].SAP_ptr->ChallengeSetName_AT = (char *)malloc(sizeof(char) * strlen(ChallengeSetName_AT) + 1)) == NULL )
         { printf("ERROR: Failed to allocate storage for ChallengeSetName_AT!\n"); exit(EXIT_FAILURE); }
      strcpy(ThreadDataArr[thread_num].SAP_ptr->ChallengeSetName_AT, ChallengeSetName_AT);

      ThreadDataArr[thread_num].SAP_ptr->gen_random_challenge = gen_random_challenge; 

      ThreadDataArr[thread_num].SAP_ptr->use_database_chlngs = use_database_chlngs;
      ThreadDataArr[thread_num].SAP_ptr->DB_ChallengeGen_seed = ChallengeGen_seed;
      ThreadDataArr[thread_num].SAP_ptr->SpreadFactor_random_seed = SpreadFactor_random_seed;

      ThreadDataArr[thread_num].SAP_ptr->fix_params = fix_params;

// Fill in the data structure with the parameters associated SRF algorithm. Get the PUF design characteristics from the database. 
      if ( (ThreadDataArr[thread_num].SAP_ptr->Netlist_name = (char *)malloc(sizeof(char) * strlen(Netlist_name) + 1)) == NULL )
         { printf("ERROR: Failed to allocate storage for Netlist_name!\n"); exit(EXIT_FAILURE); }
      strcpy(ThreadDataArr[thread_num].SAP_ptr->Netlist_name, Netlist_name);

      if ( (ThreadDataArr[thread_num].SAP_ptr->Synthesis_name = (char *)malloc(sizeof(char) * strlen(Synthesis_name) + 1)) == NULL )
         { printf("ERROR: Failed to allocate storage for Synthesis_name!\n"); exit(EXIT_FAILURE); }
      strcpy(ThreadDataArr[thread_num].SAP_ptr->Synthesis_name, Synthesis_name);

// SRF algorithm currently configured to process 2048 values.
      ThreadDataArr[thread_num].SAP_ptr->num_required_PNDiffs = NUM_REQUIRED_PNDIFFS;

// Allocated dynamically and freed during execution.
      ThreadDataArr[thread_num].SAP_ptr->PNR = NULL;
      ThreadDataArr[thread_num].SAP_ptr->PNF = NULL;

// Allocate permanent storage
      if ( (ThreadDataArr[thread_num].SAP_ptr->fPND = (float *)calloc(ThreadDataArr[thread_num].SAP_ptr->num_required_PNDiffs, sizeof(float))) == NULL )
         { printf("ERROR: Failed to allocate storage for fPND!\n"); exit(EXIT_FAILURE); }
      if ( (ThreadDataArr[thread_num].SAP_ptr->fPNDc = (float *)calloc(ThreadDataArr[thread_num].SAP_ptr->num_required_PNDiffs, sizeof(float))) == NULL )
         { printf("ERROR: Failed to allocate storage for fPNDc!\n"); exit(EXIT_FAILURE); }
      if ( (ThreadDataArr[thread_num].SAP_ptr->fPNDco = (float *)calloc(ThreadDataArr[thread_num].SAP_ptr->num_required_PNDiffs, sizeof(float))) == NULL )
         { printf("ERROR: Failed to allocate storage for fPNDco!\n"); exit(EXIT_FAILURE); }


// These are currently not used -- I do not support fast population SpreadFactor method. If I do eventually, we may need to add NAT 
// and AT versions here.
      ThreadDataArr[thread_num].SAP_ptr->MedianPNR = NULL;
      ThreadDataArr[thread_num].SAP_ptr->MedianPNF = NULL;
      ThreadDataArr[thread_num].SAP_ptr->num_qualified_rise_PNs = 0;
      ThreadDataArr[thread_num].SAP_ptr->num_qualified_fall_PNs = 0;


// Although the PopOnly SF are ALWAYS restricted to values less than the TrimCodeConstant, e.g., 5-bit for TrimCodeConstant of 32, the PCR SF can get
// larger and smaller (negative). When the TrimCodeConstant is 32, we will require one extra positive bit to represent numbers >= 32 and one extra bit 
// for negative numbers. So 7 bits will be needed, so only one bit of precision for TrimCodeConstant of 32. Max TrimCodeConstant is 64, which would 
// use all 8 bits of a 'signed char'.
      ThreadDataArr[thread_num].SAP_ptr->num_SF_bytes = NUM_REQUIRED_PNDIFFS * SF_WORDS_TO_BYTES_MULT;
      ThreadDataArr[thread_num].SAP_ptr->num_SF_words = NUM_REQUIRED_PNDIFFS; 

// 1_1_2022: If TRIMCODE_CONSTANT is <= 32, then we can preserve one precision bit in the iSpreadFactors for the device, else we cannot preserve any.
      if ( TRIMCODE_CONSTANT <= 32 )
         ThreadDataArr[thread_num].SAP_ptr->iSpreadFactorScaler = 2;
      else
         ThreadDataArr[thread_num].SAP_ptr->iSpreadFactorScaler = 1;

      if ( (ThreadDataArr[thread_num].SAP_ptr->fSpreadFactors = (float *)calloc(ThreadDataArr[thread_num].SAP_ptr->num_SF_words, sizeof(float))) == NULL )
         { printf("ERROR: Failed to allocate storage for fSpreadFactors!\n"); exit(EXIT_FAILURE); }
      if ( (ThreadDataArr[thread_num].SAP_ptr->iSpreadFactors = (signed char *)calloc(ThreadDataArr[thread_num].SAP_ptr->num_SF_words, sizeof(signed char))) == NULL )
         { printf("ERROR: Failed to allocate storage for iSpreadFactors!\n"); exit(EXIT_FAILURE); }

      if ( (ThreadDataArr[thread_num].SAP_ptr->verifier_SHD = (unsigned char *)calloc(ThreadDataArr[thread_num].SAP_ptr->num_required_PNDiffs/8, sizeof(unsigned char))) == NULL )
         { printf("ERROR: Failed to allocate storage for verifier_SHD!\n"); exit(EXIT_FAILURE); }
      if ( (ThreadDataArr[thread_num].SAP_ptr->verifier_SBS = (unsigned char *)calloc(ThreadDataArr[thread_num].SAP_ptr->num_required_PNDiffs/8, sizeof(unsigned char))) == NULL )
         { printf("ERROR: Failed to allocate storage for verifier_SBS!\n"); exit(EXIT_FAILURE); }
      if ( (ThreadDataArr[thread_num].SAP_ptr->device_SHD = (unsigned char *)calloc(ThreadDataArr[thread_num].SAP_ptr->num_required_PNDiffs/8, sizeof(unsigned char))) == NULL )
         { printf("ERROR: Failed to allocate storage for device_SHD!\n"); exit(EXIT_FAILURE); }
      if ( (ThreadDataArr[thread_num].SAP_ptr->device_SBS = (unsigned char *)calloc(ThreadDataArr[thread_num].SAP_ptr->num_required_PNDiffs/8, sizeof(unsigned char))) == NULL )
         { printf("ERROR: Failed to allocate storage for device_SBS!\n"); exit(EXIT_FAILURE); }
      ThreadDataArr[thread_num].SAP_ptr->verifier_SHD_num_bytes = 0;
      ThreadDataArr[thread_num].SAP_ptr->verifier_SBS_num_bytes = 0;
      ThreadDataArr[thread_num].SAP_ptr->device_SHD_num_bytes = 0;
      ThreadDataArr[thread_num].SAP_ptr->device_SBS_num_bytes = 0; 

      ThreadDataArr[thread_num].SAP_ptr->do_save_bitstrings_to_RT_DB = do_save_bitstrings_to_RT_DB;
      ThreadDataArr[thread_num].SAP_ptr->do_save_PARCE_COBRA_file_stats = do_save_PARCE_COBRA_file_stats;
      ThreadDataArr[thread_num].SAP_ptr->do_save_COBRA_SHD = do_save_COBRA_SHD;
      ThreadDataArr[thread_num].SAP_ptr->do_save_SKE_SHD = do_save_SKE_SHD;

      if ( (ThreadDataArr[thread_num].SAP_ptr->DHD_HD = (unsigned char *)calloc(ThreadDataArr[thread_num].SAP_ptr->num_required_PNDiffs/8, sizeof(unsigned char))) == NULL )
         { printf("ERROR: Failed to allocate storage for DHD_HD!\n"); exit(EXIT_FAILURE); }
      if ( (ThreadDataArr[thread_num].SAP_ptr->verifier_DHD_SBS = (unsigned char *)calloc(ThreadDataArr[thread_num].SAP_ptr->num_required_PNDiffs/8, sizeof(unsigned char))) == NULL )
         { printf("ERROR: Failed to allocate storage for verifier_DHD_SBS!\n"); exit(EXIT_FAILURE); }
      if ( (ThreadDataArr[thread_num].SAP_ptr->device_DHD_SBS = (unsigned char *)calloc(ThreadDataArr[thread_num].SAP_ptr->num_required_PNDiffs/8, sizeof(unsigned char))) == NULL )
         { printf("ERROR: Failed to allocate storage for device_DHD_SBS!\n"); exit(EXIT_FAILURE); }
      ThreadDataArr[thread_num].SAP_ptr->DHD_SBS_num_bits = 0;

      ThreadDataArr[thread_num].SAP_ptr->nonce_base_address = 0;

// Size of the nonce used to SRF's SelectParams, which is currently set to 8 bytes.
      ThreadDataArr[thread_num].SAP_ptr->num_required_nonce_bytes = NUM_XOR_NONCE_BYTES;
      if ( (ThreadDataArr[thread_num].SAP_ptr->verifier_n2 = (unsigned char *)calloc(ThreadDataArr[thread_num].SAP_ptr->num_required_nonce_bytes, sizeof(unsigned char))) == NULL )
         { printf("ERROR: Failed to allocate storage for verifier_n2!\n"); exit(EXIT_FAILURE); }
      if ( (ThreadDataArr[thread_num].SAP_ptr->XOR_nonce = (unsigned char *)calloc(ThreadDataArr[thread_num].SAP_ptr->num_required_nonce_bytes, sizeof(unsigned char))) == NULL )
         { printf("ERROR: Failed to allocate storage for XOR_nonce!\n"); exit(EXIT_FAILURE); }

      ThreadDataArr[thread_num].SAP_ptr->dist_range = DIST_RANGE; 
      ThreadDataArr[thread_num].SAP_ptr->range_low_limit = RANGE_LOW_LIMIT; 
      ThreadDataArr[thread_num].SAP_ptr->range_high_limit = RANGE_HIGH_LIMIT; 

      ThreadDataArr[thread_num].SAP_ptr->vec_chunk_size = CHLNG_CHUNK_SIZE; 
      ThreadDataArr[thread_num].SAP_ptr->XMR_val = XMR_VAL;

      memcpy((char *)ThreadDataArr[thread_num].SAP_ptr->AES_IV, (char *)AES_IV, AES_IV_NUM_BYTES);

// The key fields are assigned dynamic storage during execution and will be freed once the app finishes using the key.
      ThreadDataArr[thread_num].SAP_ptr->SE_target_num_key_bits = SE_TARGET_NUM_KEY_BITS; 
      ThreadDataArr[thread_num].SAP_ptr->SE_final_key = NULL;
      ThreadDataArr[thread_num].SAP_ptr->authen_min_bitstring_size = AUTHEN_MIN_BITSTRING_SIZE;
      ThreadDataArr[thread_num].SAP_ptr->DA_cobra_key = NULL;
      ThreadDataArr[thread_num].SAP_ptr->DA_nonce_reproduced = NULL;

// We do NOT have the KEK key on the verifier, ONLY ON THE DEVICE. But we can use this field to get statistics on the KEK key by having
// the device transmit it to the verifier and then store it in the statistics database.
      ThreadDataArr[thread_num].SAP_ptr->KEK_target_num_key_bits = KEK_TARGET_NUM_KEY_BITS;
      ThreadDataArr[thread_num].SAP_ptr->KEK_final_enroll_key = NULL;

      if ( (ThreadDataArr[thread_num].SAP_ptr->KEK_authentication_nonce = (unsigned char *)calloc(num_KEK_authen_nonce_bytes, sizeof(unsigned char))) == NULL )
         { printf("ERROR: Failed to allocate storage for KEK_authentication_nonce!\n"); exit(EXIT_FAILURE); }
      ThreadDataArr[thread_num].SAP_ptr->num_KEK_authen_nonce_bits = num_KEK_authen_nonce_bytes*8;

// XMR_SHD that is generated during KEK_VerifierAuthentication during each iteration (to be concatenated to a larger blob and sent to device).
      if ( (ThreadDataArr[thread_num].SAP_ptr->KEK_authen_XMR_SHD_chunk = (unsigned char *)calloc(ThreadDataArr[thread_num].SAP_ptr->num_required_PNDiffs/8, 
         sizeof(unsigned char))) == NULL )
         { printf("ERROR: Failed to allocate storage for KEK_authen_XMR_SHD_chunk!\n"); exit(EXIT_FAILURE); }

// These are filled in by the verifier. 
      ThreadDataArr[thread_num].SAP_ptr->num_chips = 0;
      ThreadDataArr[thread_num].SAP_ptr->num_vecs = 0;
      ThreadDataArr[thread_num].SAP_ptr->num_rise_vecs = 0;;

// THIS MUST BE 1
      ThreadDataArr[thread_num].SAP_ptr->has_masks = 1;

// Allocated dynamically and freed during execution.
      ThreadDataArr[thread_num].SAP_ptr->first_vecs_b = NULL;
      ThreadDataArr[thread_num].SAP_ptr->second_vecs_b = NULL;
      ThreadDataArr[thread_num].SAP_ptr->masks_b = NULL;

      ThreadDataArr[thread_num].SAP_ptr->chip_num = 0;

      ThreadDataArr[thread_num].SAP_ptr->param_LFSR_seed_low = 0;
      ThreadDataArr[thread_num].SAP_ptr->param_LFSR_seed_high = 0;
      ThreadDataArr[thread_num].SAP_ptr->param_RangeConstant = RANGE_CONSTANT;
      ThreadDataArr[thread_num].SAP_ptr->param_SpreadConstant = SPREAD_CONSTANT;
      ThreadDataArr[thread_num].SAP_ptr->param_Threshold = THRESHOLD_CONSTANT;
      ThreadDataArr[thread_num].SAP_ptr->param_TrimCodeConstant = TRIMCODE_CONSTANT;

      ThreadDataArr[thread_num].SAP_ptr->param_PCR_or_PBD_or_PO = PCR_or_PBD_or_PO;

      ThreadDataArr[thread_num].SAP_ptr->do_PO_dist_flip = do_PO_dist_flip;

      ThreadDataArr[thread_num].SAP_ptr->HBS_arr = NULL;

      ThreadDataArr[thread_num].SAP_ptr->first_chip_num = 0;
      ThreadDataArr[thread_num].SAP_ptr->first_num_match_bits = 0;
      ThreadDataArr[thread_num].SAP_ptr->first_num_mismatch_bits = 0;
      ThreadDataArr[thread_num].SAP_ptr->second_chip_num = 0;
      ThreadDataArr[thread_num].SAP_ptr->second_num_match_bits = 0;
      ThreadDataArr[thread_num].SAP_ptr->second_num_mismatch_bits = 0;

// Set to 0 to ENABLE SKE and 1 to ENABLE COBRA during device authentication. SKE compares the nonce reproduced using the device helper data with the one it sent
// to the device, counting bit-flips (failure) and minority bit flips. For COBRA, the device sends ONLY XMR SHD to the server, and where the server runs KEK FSB
// enrollment The XMR SHD bitstring from the device and server are then correlated, where we count the number of mismatches and threshold them to decide to accept
// or reject.
      ThreadDataArr[thread_num].SAP_ptr->do_COBRA = DO_COBRA;
      ThreadDataArr[thread_num].SAP_ptr->COBRA_AND_correlation_results = 0;
      ThreadDataArr[thread_num].SAP_ptr->COBRA_XNOR_correlation_results = 0;

// Only for testing the KEK SKE and FSB authentication primitives.
      ThreadDataArr[thread_num].SAP_ptr->my_chip_num = -1;
      ThreadDataArr[thread_num].SAP_ptr->my_scaling_constant = -1.0;
      ThreadDataArr[thread_num].SAP_ptr->my_IP = NULL;
      ThreadDataArr[thread_num].SAP_ptr->my_bitstream = -1;

// 11_1_2021: MAKE protocol. 
      ThreadDataArr[thread_num].SAP_ptr->MAT_LLK_num_bytes = SE_TARGET_NUM_KEY_BITS/8;

// 11_21_2021: PeerTrust protocol. This must also match the length of KEK_TARGET_NUM_KEY_BITS/8. Might make more sense to just set it to that even 
// though we use KEK session key generation to generate the PHK_A_nonce.
      ThreadDataArr[thread_num].SAP_ptr->PHK_A_num_bytes = SE_TARGET_NUM_KEY_BITS/8;

// 11_12_2021: PUF-Cash V3.0
      ThreadDataArr[thread_num].SAP_ptr->eCt_num_bytes = ECT_NUM_BYTES;

// 7_4_2022: Added when updating GenPOPLLKs
      ThreadDataArr[thread_num].SAP_ptr->POP_LLK_num_bytes = KEK_TARGET_NUM_KEY_BITS/8;

      ThreadDataArr[thread_num].SAP_ptr->DEBUG_FLAG = DEBUG_FLAG;
      ThreadDataArr[thread_num].SAP_ptr->DUMP_BITSTRINGS = DUMP_BITSTRINGS;

// If the user chooses to use the PN cache, then the qualifing PNs (according to the ChallengeSetName_NAT and ChallengeSetName_AT) for all chips are 
// read out of the database and stored in the TimingValCacheStruct array for very quick access. 
      ThreadDataArr[thread_num].SAP_ptr->use_TVC_cache = use_TVC_cache;
      ThreadDataArr[thread_num].SAP_ptr->TVC_arr_NAT = NULL;
      ThreadDataArr[thread_num].SAP_ptr->num_TVC_arr_NAT = 0;
      ThreadDataArr[thread_num].SAP_ptr->TVC_arr_AT = NULL;
      ThreadDataArr[thread_num].SAP_ptr->num_TVC_arr_AT = 0;

// Do this if the cache is enabled. YOU MUST DO THIS FOR THE AT database too.
      if ( ThreadDataArr[thread_num].SAP_ptr->use_TVC_cache == 1 )
         {
         int check_num_chips;

// Do this on the first iteration ONLY
         if ( thread_num == 0 )
            {
// Use '%' for * and '_' for ?
            ThreadDataArr[thread_num].SAP_ptr->num_chips = CreateTimingValsCacheFromChallengeSet(MAX_STRING_LEN, ThreadDataArr[thread_num].SAP_ptr->database_NAT, 
               ThreadDataArr[thread_num].SAP_ptr->design_index, ThreadDataArr[thread_num].SAP_ptr->ChallengeSetName_NAT, "%", &(ThreadDataArr[thread_num].SAP_ptr->TVC_arr_NAT), 
               &(ThreadDataArr[thread_num].SAP_ptr->num_TVC_arr_NAT));

            check_num_chips = CreateTimingValsCacheFromChallengeSet(MAX_STRING_LEN, ThreadDataArr[thread_num].SAP_ptr->database_AT, 
               ThreadDataArr[thread_num].SAP_ptr->design_index, ThreadDataArr[thread_num].SAP_ptr->ChallengeSetName_AT, "%", &(ThreadDataArr[thread_num].SAP_ptr->TVC_arr_AT), 
               &(ThreadDataArr[thread_num].SAP_ptr->num_TVC_arr_AT));

// Sanity check. These databases MUST have the same number of chips. They also must have the same SynthesisName and NetlistName, which is not checked here.
            if ( ThreadDataArr[thread_num].SAP_ptr->num_chips != check_num_chips )
               { printf("ERROR: NAT and AT databases must have the same number of chips %d vs %d\n", ThreadDataArr[thread_num].SAP_ptr->num_chips, check_num_chips); exit(EXIT_FAILURE); }
            }

// Else copy pointers to these cache data structures. They are READ-ONLY.
         else
            {

// As noted elsewhere, SAP_ptr->num_chips is used during challenge generation to record the number of DVR/DVF and is zero'ed out afterwards when the data is freed,
// so this assignment cannot be depended on to remain.
            ThreadDataArr[thread_num].SAP_ptr->num_chips = ThreadDataArr[0].SAP_ptr->num_chips; 
            ThreadDataArr[thread_num].SAP_ptr->TVC_arr_NAT = ThreadDataArr[0].SAP_ptr->TVC_arr_NAT;
            ThreadDataArr[thread_num].SAP_ptr->num_TVC_arr_NAT = ThreadDataArr[0].SAP_ptr->num_TVC_arr_NAT;
            ThreadDataArr[thread_num].SAP_ptr->TVC_arr_AT = ThreadDataArr[0].SAP_ptr->TVC_arr_AT;
            ThreadDataArr[thread_num].SAP_ptr->num_TVC_arr_AT = ThreadDataArr[0].SAP_ptr->num_TVC_arr_AT;
            }

// Force this to 1 if the timing data has been read into arrays because fast population SpreadFactor method requested (which NOT currently supported).
         ThreadDataArr[thread_num].SAP_ptr->use_TVC_cache = 1;
         }

// Else, no cache is being used. NULL out the fields.
      else
         {
         ThreadDataArr[thread_num].SAP_ptr->num_chips = 0;
         ThreadDataArr[thread_num].SAP_ptr->TVC_arr_NAT = NULL;
         ThreadDataArr[thread_num].SAP_ptr->num_TVC_arr_NAT = 0;
         ThreadDataArr[thread_num].SAP_ptr->TVC_arr_AT = NULL;
         ThreadDataArr[thread_num].SAP_ptr->num_TVC_arr_AT = 0;
         }

// ============================================================================
// Additional fields beyond SAP needed by the thread.
      ThreadDataArr[thread_num].TTP_request = 0;
      ThreadDataArr[thread_num].Device_socket_desc = -1;
      ThreadDataArr[thread_num].port_number = port_number;
      ThreadDataArr[thread_num].RANDOM = RANDOM;
      ThreadDataArr[thread_num].in_use = 0;
      ThreadDataArr[thread_num].client_index = -1;
      ThreadDataArr[thread_num].client_sockets = client_sockets;
      ThreadDataArr[thread_num].TTP_num = -1;
      ThreadDataArr[thread_num].TTP_session_keys = TTP_session_keys;
      ThreadDataArr[thread_num].num_TTPs = num_TTPs;
      ThreadDataArr[thread_num].TTP_IPs = TTP_IPs;
      ThreadDataArr[thread_num].num_customers = num_customers;
      ThreadDataArr[thread_num].customer_IPs = customer_IPs;
      ThreadDataArr[thread_num].ip_length = IP_LENGTH;

      ThreadDataArr[thread_num].max_string_len = MAX_STRING_LEN;
      ThreadDataArr[thread_num].num_eCt_nonce_bytes = num_eCt_nonce_bytes;

      ThreadDataArr[thread_num].TTP_socket_descs = TTP_socket_descs;

// This will need to be protected by a mutex.
      ThreadDataArr[thread_num].Accounts_ptr = &Accounts;

// ******************************************************
// Create a set of static threads -- thread memory management on the Cora/Zybo seems to have problems. Pass to each a copy
// of DeviceDataArr structure. The loop below will update Device_socket_desc, client_index and in_use if Alice/Bob else it
// will be a TTP. The Thread will use the Device_socket_desc or the TTP_socket_desc as the new communication channel. The 
// client_index refers to the element in the client_sockets array that is in use during the message processing -- the 
// processing thread will set this back to client_sockets[client_index] back to 0 before finishing. The Threads are in an 
// infinite loop testing the in_use field. The parent here sets this to 1 when a new message is to be processed.
//
// We need to call pthread_cancel() to have the pthread free resources (I think pthread_detach also frees resources). If I
// try to use pthread_cancel, I get a library complaint that some part of the thread library is missing -- need a newer version,
// or whatever. Since I create all these threads statically I don't need to create and destroy them so no need for these functions
// anyway.
      int err;
      pthread_t thread_id;
      if ( (err = pthread_create((pthread_t *)&thread_id, NULL, (void *)BankThread, (void *)&(ThreadDataArr[thread_num]))) != 0 )
         { printf("Failed to create thread: %d\n", err); fflush(stdout); }

// Detach thread since we don't need to synchronize with it (no 'join' required). Also allows resources to be freed when thread 
// terminates.
//      pthread_detach(thread_id);

#ifdef DEBUG
printf("Thread ID %lu\n", (unsigned long int)thread_id); fflush(stdout);
#endif

printf("Number of threads created: %d\n", thread_num + 1); fflush(stdout);
#ifdef DEBUG
#endif
      }

// NOT USED
   for ( thread_num = 1; thread_num < MAX_THREADS; thread_num++ )
      {
      SAP_arr[thread_num].ChipScalingConstantArr = NULL;
      SAP_arr[thread_num].ChipScalingConstantNotifiedArr = NULL;
      }


// ********************************************************************************
// ********************************************************************************
// LOOP
   first_time = 1;
   for ( iteration = 0; (iteration < num_iterations || num_iterations == -1); iteration++ )
      {

// Note: If NOT a new connection, 'client_IP' is NOT filled in by OpenMultipleSocketServer(). 'client_IP' is NOT filled in
// for repeated communications from a TTP since we do NOT close this socket. NOTE: On FIRST call, client_sockets array is
// initialized to ALL zeros.
      strcpy(client_IP, "");
      client_index = -1;
      Device_socket_desc = -1;
      SD = OpenMultipleSocketServer(MAX_STRING_LEN, &Bank_server_socket_desc, Bank_server_IP, port_number, client_IP,
         MAX_CLIENTS, client_sockets, &client_index, first_time);
      first_time = 0;

// Sanity check. 
      if ( client_index == -1 )
         { printf("ERROR: Failed to find an empty slot in client_sockets -- increase MAX_CLIENTS!\n"); exit(EXIT_FAILURE); }


struct timeval tv;
struct tm ptm, *ptm_ptr;
//char time_string[40];
char time_string[MAX_STRING_LEN];
long milliseconds;

// localtime_r is thread safe.
gettimeofday(&tv, NULL);
ptm_ptr = localtime_r(&tv.tv_sec, &ptm);
strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", ptm_ptr);
milliseconds = tv.tv_usec/1000;
//printf("Date: %s.%03ld\n", time_string, milliseconds);

//printf("ITERATION %d\tDate: %s.%06ld\n", iteration, time_string, milliseconds); fflush(stdout);
printf("ITERATION %d\tDate: %s.%03ld.%03ld\t%s\n", iteration, time_string, milliseconds, tv.tv_usec - milliseconds*1000, client_IP); fflush(stdout);
printf("\tSD and Client_IP returned by OpenMultipleSocketServer %d and %s\tClient index %d\tClient socket %d\tIterationCnt %d!\n", 
   SD, client_IP, client_index, client_sockets[client_index], iteration); fflush(stdout);
#ifdef DEBUG
#endif

// ========================================================
// Multiple TTP: Determine if request is from a TTP. Do NOT use the client_IP here for re-connect requests -- it doesn't work. 
// Use 'client_index' instead. NOTE: On the initial connection, the IP is correct.
      TTP_request = 0;
if (0)
   {
      for ( TTP_num = 0; TTP_num < num_TTPs; TTP_num++ )
         if ( strcmp(TTP_IPs[TTP_num], client_IP) == 0 || TTP_socket_indexes[TTP_num] == client_index )
            {
            TTP_request = 1;
            break;
            }

// Invalidate TTP_num if not found
      if ( TTP_num == num_TTPs )
         TTP_num = -1;
   }
else
   TTP_num = -1;

// ========================================================
// TTPs are always first, and they never close.
      if ( TTP_request == 1 )
         {

// Assign the new socket descriptor ONLY on the first iteration.
         if ( TTP_socket_descs[TTP_num] == -1 )
            {
            TTP_socket_descs[TTP_num] = SD;
            TTP_socket_indexes[TTP_num] = client_index;
            }

printf("\tTTP REQUEST: TTP %d with IP %s has connected with TTP_socket_desc %d at socket index %d\tIterationCnt %d!\n", TTP_num, TTP_IPs[TTP_num],
   TTP_socket_descs[TTP_num], TTP_socket_indexes[TTP_num], iteration); fflush(stdout);
#ifdef DEBUG
#endif
         }

// Client socket number is returned
      else if ( SD >= 0 )
         Device_socket_desc = SD;
      else
         { printf("ERROR: Socket descriptor returned %d is negative!\n", SD); exit(EXIT_FAILURE); }

// If NOT the first connection from the TTP, then 'client_IP' is null -- fill it in for printing purposes only.
// IP retrieval in OpenMultipleSocketServer does not seem to work -- returns '0.0.0.0'???
      if ( TTP_request == 1 && (strlen(client_IP) == 0 || strcmp(client_IP, "0.0.0.0") == 0 ))
         strcpy(client_IP, TTP_IPs[TTP_num]);

#ifdef DEBUG
printf("Client socket descriptor %d, client index %d from IP '%s'\n", SD, client_index, client_IP); fflush(stdout);
#endif

      int found_one = 0;
      while ( found_one == 0 )
         {
         for ( thread_num = 0; thread_num < MAX_THREADS; thread_num++ )
            {
            pthread_mutex_lock(&(ThreadDataArr[thread_num].Thread_mutex));
            if ( ThreadDataArr[thread_num].in_use == 0 )
               {
               ThreadDataArr[thread_num].TTP_request = TTP_request;
               if ( TTP_request == 1 )
                  ThreadDataArr[thread_num].Device_socket_desc = TTP_socket_descs[TTP_num];
               else
                  ThreadDataArr[thread_num].Device_socket_desc = Device_socket_desc;
               ThreadDataArr[thread_num].client_index = client_index;
               ThreadDataArr[thread_num].iteration_cnt = iteration;
               ThreadDataArr[thread_num].in_use = 1;
               ThreadDataArr[thread_num].TTP_num = TTP_num;
               found_one = 1;

// Make further activity on this socket descriptors ignored by OpenMultipleSocketServer() until the thread restores the 
// client_socket value when it completes communication with a TTP.
               client_sockets[client_index] = -1;

               pthread_cond_signal(&(ThreadDataArr[thread_num].Thread_cv));
               }
            pthread_mutex_unlock(&(ThreadDataArr[thread_num].Thread_mutex));
            if ( found_one == 1 )
               break;
            }
         }
       
printf("\tTasking Thread %d\n", thread_num); fflush(stdout);
#ifdef DEBUG
#endif

      }

// PERFORMANCE EVAL ONLY: If we read the database into memory, and updated it (by deleting elements because of 'max_chips'), then check to 
// see if we need to store it. This will only store the non-anonmous database. Since 5/20/2019, I've added a second database.
   if ( read_db_into_memory == 1 && max_chips != -1 )
      {
      char temp_str[MAX_STRING_LEN];

// If the user started the program using 'MasterWithRandom5095.db' and max_chips != -1, then save the existing 'trimmed' db to a file.
      if ( strlen(DB_name_NAT) - 23 > 0 )
         {
         strcpy(temp_str, &(DB_name_NAT[strlen(DB_name_NAT) - 23]));
      
         if ( strcmp(temp_str, "MasterWithRandom5095.db") == 0 )
            {
            strcpy(temp_str, DB_name_NAT);
            temp_str[strlen(DB_name_NAT) - 7] = '\0';
            sprintf(DB_name_NAT, "%s%d.db", temp_str, max_chips);
         
            printf("Saving 'in memory' database back to filesystem '%s'\n", DB_name_NAT); fflush(stdout);
            if ( LoadOrSaveDb(DB_NAT, DB_name_NAT, 1) != 0 )
               { printf("Failed to store 'in memory' database to %s: %s\n", DB_name_NAT, sqlite3_errmsg(DB_NAT)); sqlite3_close(DB_NAT); exit(EXIT_FAILURE); }
            }
         }
      }

// Close the databases.
   sqlite3_close(DB_NAT);
   sqlite3_close(DB_AT);
if (0)
   {
   sqlite3_close(DB_RunTime);
   sqlite3_close(DB_MAKE_PT_AT);
   sqlite3_close(DB_PUFCash_V3);
   }

// Free TTP_session_key in case of multithreading. 
   for ( TTP_num = 0; TTP_num < num_TTPs; TTP_num++ )
      {
      if ( TTP_session_keys[TTP_num] != NULL )
         free(TTP_session_keys[TTP_num]);
      TTP_session_keys[TTP_num] = NULL;
      }

// Close server sockets
   close(Bank_server_socket_desc);

if (0)
   {
   for ( TTP_num = 0; TTP_num < num_TTPs; TTP_num++ )
      close(TTP_socket_descs[TTP_num]);
   }

   return 0;
   }
