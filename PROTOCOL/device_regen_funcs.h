// ========================================================================================================
// ========================================================================================================
// ***************************************** device_regen_funcs.h *****************************************
// ========================================================================================================
// ========================================================================================================
//
//--------------------------------------------------------------------------------
// Company: IC-Safety, LLC and University of New Mexico
// Engineer: Professor Jim Plusquellic
// Exclusive License: IC-Safety, LLC
// Copyright: Univ. of New Mexico
//--------------------------------------------------------------------------------

void intHandler(int dummy);

void LoadUnloadBRAM(int max_string_len, int num_vals, volatile unsigned int *CtrlRegA, volatile unsigned int *DataRegA, 
   unsigned int ctrl_mask, unsigned char *ByteData, signed short *WordData, int load_or_unload, int byte_or_word_data, 
   int debug_flag);

int FetchTransSHD_SBS(int max_string_len, int target_bytes, volatile unsigned int *CtrlRegA, 
   volatile unsigned int *DataRegA, unsigned int ctrl_mask, int verifier_socket_desc, unsigned char *SHD_SBS, 
   int also_do_transfer, int SHD_or_SBS, int TA_or_KEK, int DUMP_BITSTRINGS, int DEBUG);

int CollectPNs(int max_string_len, int num_POs, int num_PIs, int vec_chunk_size, int max_generated_nonce_bytes, 
   volatile unsigned int *CtrlRegA, volatile unsigned int *DataRegA, unsigned int ctrl_mask, int num_vecs, 
   int num_rise_vecs, int has_masks, unsigned char **first_vecs_b, unsigned char **second_vecs_b, 
   unsigned char **masks_b, unsigned char *device_n1, int DUMP_BITSTRINGS, int DEBUG);

int KEK_DeviceAuthentication_SKE(int max_string_len, SRFHardwareParamsStruct *SHP_ptr, int verifier_socket_desc);

int KEK_ClientServerAuthen(int max_string_len, SRFHardwareParamsStruct *SHP_ptr, int verifier_socket_desc);

int TRNG(int max_string_len, SRFHardwareParamsStruct *SHP_ptr, int int_or_ext_mode, int load_seed,
   int store_nonce_num_bytes, unsigned char *nonce_arr);
