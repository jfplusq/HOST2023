// ========================================================================================================
// ========================================================================================================
// *********************************************** common.h ***********************************************
// ========================================================================================================
// ========================================================================================================
//
//--------------------------------------------------------------------------------
// Company: IC-Safety, LLC and University of New Mexico
// Engineer: Professor Jim Plusquellic
// Exclusive License: IC-Safety, LLC
// Copyright: Univ. of New Mexico
//--------------------------------------------------------------------------------

#define _DEFAULT_SOURCE 1

#ifndef COMMON_INCLUDED 
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>  
#include <sys/mman.h>

#include <netdb.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <net/if.h> 
#include <sys/ioctl.h>

#include <time.h>
#include <sys/time.h>

#include <pthread.h>

#include "utility.h"

// 10_10_2022: THIS MUST BE LARGER THAN THE NUMBER OF CHIPS IN THE DB. Used in verifier_regen_funcs.c in Cobra authentication function
// when we are saving SHD to a file for analysis. It is checked against 'SAP_ptr->num_chips' and we exit if not true.
#define MAX_CHIPS 5000

#define FUNC_DA 0
#define FUNC_VA 1
#define FUNC_SE 2
#define FUNC_PROVISION 3
#define FUNC_LL_REGEN 4
#define FUNC_INT_TRNG 5
#define FUNC_EXT_TRNG 6
#define FUNC_LL_ENROLL 7
#define FUNC_RB 8

#define SF_MODE_PCR 0
#define SF_MODE_PBD 1
#define SF_MODE_POPONLY 2

// =====================================================================================================================
// =====================================================================================================================
// These 'FIXED_' parameters are ONLY used when the flag 'FIX_PARAMS' is set to 1.
// Constraint: Values assigned for the LFSR seeds must be >= 0 and <= 2047
#define FIXED_LFSR_SEED_LOW 0
#define FIXED_LFSR_SEED_HIGH 0

// RangeConstant cannot be too small otherwise entropy is reduced. 128 is minimum.
#define FIXED_RANGE_CONSTANT 160

#define FIXED_SPREAD_CONSTANT 20
#define FIXED_THRESHOLD 3

#define FIXED_XMR 5

// MAX IS 64 here, but any even value from 20 to 64 should work okay.
#define FIXED_TRIMCODE_CONSTANT 32

// 10_29_2022: THIS MUST MATCH WHAT IS BEING USED IN THE VERILOG CODE.
#define MAX_SCALING_VALUE (5)
#define SCALING_PRECISION_NB (11)

// The default value is 1.0 -- NO SCALING. BUT WE MUST PASS IN the equivalent of 1.0, which is 1 << SCALING_PRECISION_NB
#define FIXED_SCALING_CONSTANT (1 << SCALING_PRECISION_NB);

// Entropy source datapath characteristics for processing vectors on the device side. These constants evaluate to
// 24*32+16 = 768 + 16 = 784.
#define NUM_MODULES 24
#define NUM_CHALLENGE_BITS_PER_MODULE 32
#define NUM_DIRECTION_CHLNG_BITS 16
#define NUM_CHLNG_BITS (NUM_MODULES * NUM_CHALLENGE_BITS_PER_MODULE + NUM_DIRECTION_CHLNG_BITS)

#define RISE_DIR '1'

// Storing challenges in database as two vectors instead of one configuration challenge (right now). NUM_PIs is
// set to 784/2 = 392.
// *** VEC_CHLNG ***
#define NUM_PIS NUM_CHLNG_BITS/2;
//#define NUM_PIS 782
#define NUM_POS 32

// =====================================================================================================================
// =====================================================================================================================
// COMMON PARAMETERS for verifier_regeneration.c, device_regeneration.c and ttp_DB.c. 
// Set to 1 once we reduce SF from 16-bits to 8-bits. 1_x_2022: DONE
#define SF_WORDS_TO_BYTES_MULT 1


// A SPREADCONSTANT of 20 is split in half and this results in super-strong regions at 5 and -5. Randomization spreads 
// these across the region -9 to -1 and 1 to 9 because 20/2 - 2 = 8 so random values are between 0 and 8 which are then
// shifted by subtracting 20/4-1 = 4. With a THREADHOLD_CONSTANT of 3, any PNDco_xx within the threshold region of -3 to 3 
// is classified as a weak bit. 
//
// A SPREADCONSTANT of 16 is split in half and this results in super-strong regions at 4 and -4. Randomization spreads 
// these across the region -7 to -1 and 1 to 7 because 16/2 - 2 = 6 so random values are between 0 and 6 which are then
// shifted by subtracting 16/4-1 = 3. With a THREADHOLD_CONSTANT of 3, any PNDco_xx within the threshold region of -3 to 3 
// is classified as a weak bit. 

//#define RANGE_CONSTANT 160
#define RANGE_CONSTANT 180

#define SPREAD_CONSTANT 20
#define THRESHOLD_CONSTANT 3

// THIS MUST be larger than the Entropy (larger than the largest range among the PNDc for all chips). It can be ANY
// EVEN NUMBER, 32, 36, 42, upto 254. Smaller values introduce more flips when processing PNDc to PNDco. 32 seems to
// be a good value to pass all NIST tests, although I suspect you can safely to larger, up to 64. We flip according to
// the number of times this constant is applied to get the PNDc into the range of -16 to +16 (when it is 32). PNDc
// can be large, in the 100's because they are the difference of two raw path delay values.
#define TRIMCODE_CONSTANT 32

// The current maximum number of nonce bytes that the TRNG will output per timing operation (with random vectors).
#define MAX_EXT_TRNG_ITERATIONS 20
#define MAX_EXT_TRNG_BYTES_PER_ITERATION 256
#define MAX_EXT_TRNG_NUM_BYTES (MAX_EXT_TRNG_ITERATIONS * MAX_EXT_TRNG_BYTES_PER_ITERATION)

// Tune these as needed to the desired key size for SessionEncryption (SE). Actually, using software routine to test 
// session key that is 128 bits so need to stick with that unless you change the software routine. THESE MUST BE DIVISIBLE
// BY 8 SINCE WE SAVE 'byte' REPRESENTATIONS OF THEM IN data structures. 
#define SE_TARGET_NUM_KEY_BITS 256
#define KEK_TARGET_NUM_KEY_BITS 256
#define KEK_AUTHEN_NUM_NONCE_BITS 256

// Default value for XMR
#define XMR_VAL 5 

// Specify minimum number of bits that must be generated for device and verifier authentication to succeed. Currently,
// all bits in the generated bitstrings must match in order for authenication to be classified as successful. This restriction
// can be relaxed as needed with appropriate changes in the DeviceAuthenication and VerifierAuthenication functions.
#define AUTHEN_MIN_BITSTRING_SIZE 80

// -----------------------------------
// Set to 1 to enable Cobra to be used for DeviceAuthentication. 
#define DO_COBRA 0

// NOTE: When XMR_VAL is set to 1 KEK_FSB/NMBF will ALWAYS be zero (b/c there are NO minority bits in this case) AND NMM and
// NTBF will be equal. This applies ONLY to DeviceAuthentication
#define DEVICE_SKE_AUTHEN_XMR_VAL 5

// NOTE: THIS MUST be set to XMR_VAL 1 otherwise the shifting that occurs in SHD bitstrings because of XMR make it difficult
// to correlate SHD. 10_10_2022: Also, we use POPULATION-ONLY SF WITH NO FLIPs by the device (so no need to send SF back to server).
#define DEVICE_COBRA_AUTHEN_XMR_VAL 1

// 11_6_2022: Wondering if I split the strong and weak regions in half that the COBRA PCCs get closer to 50%, instead of 30%.
// Started on this -- looks like it is working. Needed to change ComputePersonalizedScalingConstants() too. But doesn't look
// like the effect I want. Out of time -- this is an optimization for later.
//#define DEVICE_COBRA_AUTHEN_THRESHOLD_CONSTANT 5

// For SKE mode authentication, the CC is the ratio of true_minority_bit_flips/num_strong_bits*100.0, i.e., it is a percentage 
// between 0 and 100. Ideally, the authentic chip has 0 for CC. PCC is percentage change difference between the lowest CC and
// second lowest CC, as (second-first)/second*100. ZED data analysis across TV corners show this never gets lower than 70% for 
// authentic chips.
#define CC_SKE_AUTHEN_THRESHOLD 5.0
#define PCC_SKE_AUTHEN_THRESHOLD 25.0

// 11_5_2022: Trying more iterations for Cobra
#define NUM_COBRA_ITERATIONS 2
#define PCC_COBRA_AUTHEN_THRESHOLD 15.0

// This applies ONLY for VerifierAuthentication. With XMR 5, we may be a bit-flip or two. This is the max tolerable.
#define AUTHENTICATION_ALLOWED_NUM_MISMATCH_BITS 5
// -----------------------------------

// Number of devices that can simultaneously connect
#define MAX_CLIENTS 100

// Number of DA attempts that are allowed.
#define MAX_DA_RETRIES 5

// Absolute minimum size of any response bitstring generated by Alice, delivered by Bob and used by the Bank to transfer funds.
#define MIN_RESPONSE_BSTRING_LEN 64

// Length of V4 IP address, 192.168.100.150 + NULL char
#define IP_LENGTH 16

// =====================================================================================================================
// =====================================================================================================================
// MAKE protocol constants

#define AES_INPUT_NUM_BITS 128
#define AES_INPUT_NUM_BYTES (AES_INPUT_NUM_BITS/8)

// We are using AES encrypt_256/decrypt_256 in CBC mode now, which requires key size to be 256.
#define AES_KEY_NUM_BITS_256 256
#define AES_KEY_NUM_BYTES_256 (AES_KEY_NUM_BITS/8)

#define AES_IV_NUM_BYTES 16

#define HASH_IN_LEN_BITS 256
#define HASH_IN_LEN_BYTES (HASH_IN_LEN_BITS/8)

#define HASH_OUT_LEN_BITS 256
#define HASH_OUT_LEN_BYTES (HASH_OUT_LEN_BITS/8)

// =====================================================================================================================
// =====================================================================================================================
// GPIO transfer size
#define CHLNG_CHUNK_SIZE 16

// Number required for XOR nonce to enable HELP to SelectParams
#define NUM_XOR_NONCE_BYTES 8

// NOTE: This is the range I'm using in the hardware. I find the largest negative value in the distribution, subtract
// that from all values (shifting the distribution left for negative largest values and right for positive). The
// binning of values therefore starts at bin 0 and goes up through the largest positive value (minus the negative value).
// Distribution ranges are typically between 250 to 300. In the hardware, I also preserve 1 digit of precision, which is
// equivalent to multiplying the range by 2. I also do this on the server to get extra precision out of the range.
#define DIST_RANGE 1024

// Represents +6.25% and -93.75% since NUMBER of PNDIFFS is 2048. Given w.r.t. NUM_PNDIFFS defined above.
#define RANGE_LOW_LIMIT 128
#define RANGE_HIGH_LIMIT 1920

// For checks that the hardware version is not overflowing.
#define LARGEST_POS_VAL ((16384/16) - 1)
#define LARGEST_NEG_VAL -LARGEST_POS_VAL

// =====================================================================================================================
// =====================================================================================================================
void StringCreateAndCopy(char **dest, const char *src);

void Allocate1DString(char **one_dim_arr_ptr, int size);
void Free1DString(char **one_dim_arr_ptr);

unsigned char *Allocate1DUnsignedChar(int size);

void Allocate2DStrings(char ***two_dim_arr_ptr, int first_dim_size, int second_dim_size);
void ReAllocate2DStrings(char ***two_dim_arr_ptr, int *first_dim_size_ptr, int second_dim_size);
void Free2DStrings(char ***two_dim_arr_ptr, int first_dim_size);

char **Allocate2DCharArray(int dim1_size, int dim2_size);
char ***Allocate3DCharArray(int dim1_size, int dim2_size, int dim3_size);

int *Allocate1DIntArray(int dim1_size);

float *Allocate1DFloatArray(int dim1_size);
float **Allocate2DFloatArray(int dim1_size, int dim2_size);
void Free2DFloatArray(float ***two_dim_arr_ptr, int dim1_size);
float ***Allocate3DFloatArray(int dim1_size, int dim2_size, int dim3_size);
void Free3DFloatArray(float ****three_dim_arr_ptr, int dim1_size, int dim2_size);


int ReadXFile(int str_length, char *in_file, int max_vals, char **names);

void FreeVectorsAndMasks(int *num_vecs_ptr, int *num_rise_vecs_ptr, unsigned char ***first_vecs_b_ptr, 
   unsigned char ***second_vecs_b_ptr, unsigned char ***masks_b_ptr);

void GetMyIPAddr(int max_string_len, const char *target_interface_name, char **IP_addr_ptr);

int OpenMultipleSocketServer(int max_string_len, int *master_socket_ptr, char *server_IP, int port_number, char *client_IP, 
   int max_clients, int *client_sockets, int *client_index_ptr, int initialize);

int OpenSocketServer(int max_string_len, int *server_socket_desc_ptr, char *server_IP, int port_number, int *client_socket_desc_ptr, 
   struct sockaddr_in *client_addr_ptr, int accept_only, int check_and_return);

int OpenSocketClient(int max_string_len, char *server_IP, int port_number, int *server_socket_desc_ptr);

int OpenSocketServerUDP(int max_string_len, int *bcast_server_socket_desc_ptr, char *server_IP, int port_number, 
   struct sockaddr_in *client_addr_ptr, int accept_only, int check_and_return, char *buff, char *bcast_subnet_IP);

int SockGetB(unsigned char *buffer, int buffer_size, int socket_desc);

int SockSendB(unsigned char *buffer, int buffer_size, int socket_desc);

void PrintHeaderAndHexVals(char *header_str, int num_vals, unsigned char *vals, int max_vals_per_row);

void PrintHeaderAndBinVals(char *header_str, int num_vals, unsigned char *vals, int max_vals_per_row);

void SelectParams(int nonce_len_bytes, unsigned char *nonce_bytes, int nonce_base_address, unsigned int *LFSR_seed_low_ptr, 
   unsigned int *LFSR_seed_high_ptr, unsigned int *RangeConstant_ptr, unsigned short *SpreadConstant_ptr, 
   unsigned short *Threshold_ptr, unsigned short *TrimCodeConstant_ptr);

void LFSR_11_A_bits_low(int load_seed, uint16_t seed, uint16_t *lfsr);
void LFSR_11_A_bits_high(int load_seed, uint16_t seed, uint16_t *lfsr);


void SendVectorsAndMasks(int max_string_len, int num_vecs, int device_socket_desc, int num_rise_vecs, int num_PIs, 
   unsigned char **first_vecs_b, unsigned char **second_vecs_b, int has_masks, int num_POs, unsigned char **masks);

void GoSendVectors(int max_string_len, int num_POs, int num_PIs, int device_socket_desc, int num_vecs, 
   int num_rise_vecs, int has_masks, unsigned char **first_vecs_b, unsigned char **second_vecs_b, 
   unsigned char **masks, int get_GO, int use_database_chlngs, int DB_ChallengeGen_seed, int DEBUG);

void SendChlngsAndMasks(int max_string_len, int num_chlngs, int device_socket_desc, int num_rise_chlngs, int num_chlng_bits, 
   unsigned char **challenges_b, int has_masks, int num_POs, unsigned char **masks);

void GoSendChlngs(int max_string_len, int num_POs, int num_chlng_bits, int device_socket_desc, int num_chlngs, 
   int num_rise_chlngs, int has_masks, unsigned char **challenges_b, unsigned char **masks, int get_GO, 
   int DEBUG);


int ReadVectorAndMaskFilesBinary(int max_string_len, char *vec_pair_path, int num_PIs, int *num_rise_vecs_ptr, 
   unsigned char ***first_vecs_b_ptr, unsigned char ***second_vecs_b_ptr, int has_masks, char *mask_file_path, 
   int num_POs, unsigned char ***masks_b_ptr, int num_direction_chlng_bits);

int ReadChlngAndMaskFilesBinary(int max_string_len, char *chlng_path, int num_chlng_bits, int *num_rise_chlngs_ptr, 
   unsigned char ***challenges_b_ptr, int has_masks, char *mask_file_path, int num_POs, 
   unsigned char ***masks_b_ptr, int num_direction_chlng_bits);

void WriteVectorAndMaskFilesBinary(int max_string_len, char *vec_pair_path, int num_PIs, int num_vecs, 
   unsigned char **first_vecs_b_ptr, unsigned char **second_vecs_b_ptr, int has_masks, char *mask_file_path, 
   int num_POs, unsigned char **masks_b_ptr);

int JoinBytePackedBitStrings(int num_bits1, unsigned char **bs1_ptr, int num_bits2, unsigned char *bs2);

int EliminatePackedBitsFromBS(int num_bits1, unsigned char *bs, int bit_pos);

int KEK_FSB_SKE(int max_bits, int XMR, unsigned char *SBG_SHD, unsigned char *SBG_SBS, unsigned char *XMR_SHD, 
   int num_nonce_bits, unsigned char *Nonce_or_XMR_SBS, int enroll_or_regen, int do_minority_bit_flip_analysis, 
   int *num_minority_bit_flips_ptr, int start_actual_bit_pos, unsigned char *KEK_enroll_key, 
   int *true_minority_bit_flips_ptr, int FSB_or_SKE, int chip_num, int TV_num);

// =========================
#define ECT_NUM_BYTES 16

struct transfer 
   {
   int id_from;
   int id_to;
   int amount;
   time_t timestamp;
   int type;
   };

#define COMMON_INCLUDED 
#endif
