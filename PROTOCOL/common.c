// ========================================================================================================
// ========================================================================================================
// *********************************************** common.c ***********************************************
// ========================================================================================================
// ========================================================================================================
//
//--------------------------------------------------------------------------------
// Company: IC-Safety, LLC and University of New Mexico
// Engineer: Professor Jim Plusquellic
// Exclusive License: IC-Safety, LLC
// Copyright: Univ. of New Mexico
//--------------------------------------------------------------------------------


#include "utility.h"
#include "common.h"
#include <math.h>  

#include <errno.h>
#include <netinet/in.h>
  
#define TRUE   1
#define FALSE  0

// ===========================================================================================================
// ===========================================================================================================
// Create a new string at dest and copy from src.

void StringCreateAndCopy(char **dest, const char *src)
   {
   *dest = (char *)malloc((size_t)(strlen(src)+1));
   strcpy(*dest, src);
   return;
   }


// ===========================================================================================================
// ===========================================================================================================

void Allocate1DString(char **one_dim_arr_ptr, int size)
   {
   if ( (*one_dim_arr_ptr = (char *)calloc(size, sizeof(char))) == NULL )
      { printf("ERROR: Allocate1DString(): Failed to allocate string array!\n"); exit(EXIT_FAILURE); }

   return;
   }


// ===========================================================================================================
// ===========================================================================================================

void Free1DString(char **one_dim_arr_ptr)
   {
   if ( *one_dim_arr_ptr == NULL )
      { printf("ERROR: Free2DString(): 1-D array of strings is NULL!\n"); exit(EXIT_FAILURE); }

   free(*one_dim_arr_ptr);
   *one_dim_arr_ptr = NULL;

   return;
   }


// ===========================================================================================================
// ===========================================================================================================

unsigned char *Allocate1DUnsignedChar(int size)
   {
   unsigned char *one_dim_arr; 

   if ( (one_dim_arr = (unsigned char *)calloc(size, sizeof(unsigned char))) == NULL )
      { printf("ERROR: Allocate1DUnsignedChar(): Failed to allocate string array!\n"); exit(EXIT_FAILURE); }

   return one_dim_arr;
   }


// ===========================================================================================================
// ===========================================================================================================

void Allocate2DStrings(char ***two_dim_arr_ptr, int first_dim_size, int second_dim_size)
   {
   int i;

   if ( (*two_dim_arr_ptr = (char **)malloc(first_dim_size * sizeof(char *))) == NULL )
      { printf("ERROR: Allocate2DString(): Failed to allocate outer dimension of string array!\n"); exit(EXIT_FAILURE); }

   for ( i = 0; i < first_dim_size; i++ )
      if ( ((*two_dim_arr_ptr)[i] = (char *)calloc(second_dim_size, sizeof(char))) == NULL )
         { printf("ERROR: Allocate2DString(): Failed to allocate inner dimension of string array!\n"); exit(EXIT_FAILURE); }

   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// Adds one element to the outter array. ASSUMES *two_dim_arr_ptr is NUL initially

void ReAllocate2DStrings(char ***two_dim_arr_ptr, int *first_dim_size_ptr, int second_dim_size)
   {
   (*first_dim_size_ptr)++;
   if ( (*two_dim_arr_ptr = (char **)realloc(*two_dim_arr_ptr, *first_dim_size_ptr * sizeof(char *))) == NULL )
      { printf("ERROR: Allocate2DString(): Failed to re-allocate outer dimension of string array!\n"); exit(EXIT_FAILURE); }

   if ( ((*two_dim_arr_ptr)[*first_dim_size_ptr - 1] = (char *)calloc(second_dim_size, sizeof(char))) == NULL )
      { printf("ERROR: Allocate2DString(): Failed to allocate inner dimension of string array!\n"); exit(EXIT_FAILURE); }

   return;
   }


// ===========================================================================================================
// ===========================================================================================================

void Free2DStrings(char ***two_dim_arr_ptr, int first_dim_size)
   {
   int i;

   if ( *two_dim_arr_ptr == NULL )
      { printf("ERROR: Free2DString(): Root of 2-D array of strings is NULL!\n"); exit(EXIT_FAILURE); }

   for ( i = 0; i < first_dim_size; i++ )
      {
      if ( (*two_dim_arr_ptr)[i] == NULL )
         { printf("ERROR: Free2DString(): Child of 2-D array of strings is NULL!\n"); exit(EXIT_FAILURE); }

      free((*two_dim_arr_ptr)[i]);
      }

   free(*two_dim_arr_ptr);
   *two_dim_arr_ptr = NULL;

   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// Dynamically allocate 2-D array.

char **Allocate2DCharArray(int dim1_size, int dim2_size)
   {
   int dim1_num;
   char **TwoDimArray;

// Allocate space for chips.
   if ( (TwoDimArray = (char **)malloc(sizeof(char *)*dim1_size)) == NULL )
      { printf("ERROR: Allocate2DCharArray(): Initial malloc failed\n"); exit(EXIT_FAILURE); }

   for ( dim1_num = 0; dim1_num < dim1_size; dim1_num++ )
      if ( (TwoDimArray[dim1_num] = (char *)malloc(sizeof(char)*dim2_size)) == NULL )
         { printf("ERROR: Allocate2DCharArray(): Malloc failed for dim1_num %d\n", dim1_num); exit(EXIT_FAILURE); }

   return TwoDimArray;
   }


// ===========================================================================================================
// ===========================================================================================================
// Dynamically allocate 3-D array.

char ***Allocate3DCharArray(int dim1_size, int dim2_size, int dim3_size)
   {
   int dim1_num, dim2_num;
   char ***ThreeDimArray;

// Allocate space for chips.
   if ( (ThreeDimArray = (char ***)malloc(sizeof(char **)*dim1_size)) == NULL )
      { printf("ERROR: Allocate3DCharArray(): Initial malloc failed\n"); exit(EXIT_FAILURE); }

   for ( dim1_num = 0; dim1_num < dim1_size; dim1_num++ )
      {

// Allocate another set of max_TV pointers to char arrays.
      if ( (ThreeDimArray[dim1_num] = (char **)malloc(sizeof(char *)*dim2_size)) == NULL )
         { printf("ERROR: Allocate3DCharArray(): Malloc failed for chip %d\n", dim1_num); exit(EXIT_FAILURE); }

// Now allocate space for the PNs
      for ( dim2_num = 0; dim2_num < dim2_size; dim2_num++ )
         {
         if ( (ThreeDimArray[dim1_num][dim2_num] = (char *)malloc(sizeof(char)*dim3_size)) == NULL )
            { printf("ERROR: Allocate3DCharArray(): Malloc failed for chip %d and TV num %d\n", dim1_num, dim2_num); exit(EXIT_FAILURE); }
         }
      }

   return ThreeDimArray;
   }


// ===========================================================================================================
// ===========================================================================================================

int *Allocate1DIntArray(int dim1_size)
   {
   int *one_dim_arr;

   if ( (one_dim_arr = (int *)calloc(dim1_size, sizeof(int))) == NULL )
      { printf("ERROR: Allocate1DIntArray(): Failed to allocate int array!\n"); exit(EXIT_FAILURE); }

   return one_dim_arr;
   }


// ===========================================================================================================
// ===========================================================================================================

float *Allocate1DFloatArray(int dim1_size)
   {
   float *one_dim_arr;
   if ( (one_dim_arr = (float *)calloc(dim1_size, sizeof(float))) == NULL )
      { printf("ERROR: Allocate1DFloatArray(): Failed to allocate float array!\n"); exit(EXIT_FAILURE); }

   return one_dim_arr;
   }


// ===========================================================================================================
// ===========================================================================================================
// Dynamically allocate 2-D array.

float **Allocate2DFloatArray(int dim1_size, int dim2_size)
   {
   int dim1_num;
   float **TwoDimArray;

// Allocate space for chips.
   if ( (TwoDimArray = (float **)malloc(sizeof(float *)*dim1_size)) == NULL )
      { printf("ERROR: Allocate2DFloatArray(): Initial malloc failed\n"); exit(EXIT_FAILURE); }

   for ( dim1_num = 0; dim1_num < dim1_size; dim1_num++ )
      if ( (TwoDimArray[dim1_num] = (float *)malloc(sizeof(float)*dim2_size)) == NULL )
         { printf("ERROR: Allocate2DFloatArray(): Malloc failed for dim1_num %d\n", dim1_num); exit(EXIT_FAILURE); }

   return TwoDimArray;
   }


// ===========================================================================================================
// ===========================================================================================================

void Free2DFloatArray(float ***two_dim_arr_ptr, int dim1_size)
   {
   int i;

   if ( *two_dim_arr_ptr == NULL )
      { printf("ERROR: Free2DFloatArray(): Root of 2-D array of floats is NULL!\n"); exit(EXIT_FAILURE); }

   for ( i = 0; i < dim1_size; i++ )
      {
      if ( (*two_dim_arr_ptr)[i] == NULL )
         { printf("ERROR: Free2DFloatArray(): Child of 2-D array of floats is NULL!\n"); exit(EXIT_FAILURE); }

      free((*two_dim_arr_ptr)[i]);
      }

   free(*two_dim_arr_ptr);
   *two_dim_arr_ptr = NULL;

   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// Dynamically allocate 3-D array.

float ***Allocate3DFloatArray(int dim1_size, int dim2_size, int dim3_size)
   {
   int chip_num, TV_num;
   float ***ThreeDimArray;

// Allocate space for chips.
   if ( (ThreeDimArray = (float ***)malloc(sizeof(float **)*dim1_size)) == NULL )
      { printf("ERROR: Allocate3DFloatArray(): Initial malloc failed\n"); exit(EXIT_FAILURE); }

   for ( chip_num = 0; chip_num < dim1_size; chip_num++ )
      {

// Allocate another set of max_TV pointers to float arrays.
      if ( (ThreeDimArray[chip_num] = (float **)malloc(sizeof(float *)*dim2_size)) == NULL )
         { printf("ERROR: Allocate3DFloatArray(): Malloc failed for chip %d\n", chip_num); exit(EXIT_FAILURE); }

// Now allocate space for the PNs
      for ( TV_num = 0; TV_num < dim2_size; TV_num++ )
         {
         if ( (ThreeDimArray[chip_num][TV_num] = (float *)malloc(sizeof(float)*dim3_size)) == NULL )
            { printf("ERROR: Allocate3DFloatArray(): Malloc failed for chip %d and TV num %d\n", chip_num, TV_num); exit(EXIT_FAILURE); }
         }
      }

   return ThreeDimArray;
   }


// ===========================================================================================================
// ===========================================================================================================

void Free3DFloatArray(float ****three_dim_arr_ptr, int dim1_size, int dim2_size)
   {
   int i, j;

   if ( *three_dim_arr_ptr == NULL )
      { printf("ERROR: Free3DFloatArray(): Root of 3-D array of floats is NULL!\n"); exit(EXIT_FAILURE); }

   for ( i = 0; i < dim1_size; i++ )
      {
      if ( (*three_dim_arr_ptr)[i] == NULL )
         { printf("ERROR: Free3DFloatArray(): First child of 3-D array of floats is NULL!\n"); exit(EXIT_FAILURE); }

      for ( j = 0; j < dim2_size; j++ )
         {
         if ( (*three_dim_arr_ptr)[i][j] == NULL )
            { printf("ERROR: Free3DFloatArray(): Second child of 3-D array of floats is NULL!\n"); exit(EXIT_FAILURE); }
         free((*three_dim_arr_ptr)[i][j]);
         }
      free((*three_dim_arr_ptr)[i]);
      }

   free(*three_dim_arr_ptr);
   *three_dim_arr_ptr = NULL;

   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// Reads files with strings, one per line

int ReadXFile(int str_length, char *in_file, int max_vals, char **names) 
   {
   char line[str_length], *char_ptr;
   FILE *INFILE;
   int count = 0;

   if ( (INFILE = fopen(in_file, "r")) == NULL )
      { printf("ERROR: ReadXFile(): Data file '%s' open failed for reading!\n", in_file); exit(EXIT_FAILURE); }

// fgets reads upto 'str_length' characters and writes a terminating char after the last char read.
   while ( fgets(line, str_length - 1, INFILE) != NULL )
      {  

// Find the newline and eliminate it.
      if ((char_ptr = strrchr(line, '\n')) != NULL)
         *char_ptr = '\0';

// Skip blank lines 
      if ( strlen(line) == 0 )
         continue;

// Skip lines that are commented out. Returns a non-NULL pointer if any of characters in right string
// occur in left string.
      if ( strstr(line, "#") != NULL )
         continue;

// Check to make sure we don't overrun the array.
      if ( count == max_vals )
         { printf("ERROR: ReadXFile(): Number of vals read (%d) is larger than MAX (%d)!\n", count, max_vals); exit(EXIT_FAILURE); }

      strcpy(names[count], line);
      count++;
      }

   fclose(INFILE);
   return count;
   }


// ========================================================================================================
// ========================================================================================================
// Free up vectors and masks.

void FreeVectorsAndMasks(int *num_vecs_ptr, int *num_rise_vecs_ptr, unsigned char ***first_vecs_b_ptr, 
   unsigned char ***second_vecs_b_ptr, unsigned char ***masks_b_ptr)
   {
   int vec_mask_num;

   if ( first_vecs_b_ptr != NULL && *first_vecs_b_ptr != NULL )
      {
      for ( vec_mask_num = 0; vec_mask_num < *num_vecs_ptr; vec_mask_num++ )
         if ( (*first_vecs_b_ptr)[vec_mask_num] != NULL )
            free((*first_vecs_b_ptr)[vec_mask_num]);
         else
            { printf("ERROR: FreeVectorsAndMasks(): Actual first_vecs_b_ptr[%d] IS NULL!\n", vec_mask_num); exit(EXIT_FAILURE); }
      free(*first_vecs_b_ptr);
      }
   else
      {
#ifdef DEBUG
printf("WARNING: FreeVectorsAndMasks(): Called with NULL first_vecs_b_ptr!\n");
#endif
      }
   *first_vecs_b_ptr = NULL;

   if ( second_vecs_b_ptr != NULL && *second_vecs_b_ptr != NULL )
      {
      for ( vec_mask_num = 0; vec_mask_num < *num_vecs_ptr; vec_mask_num++ )
         if ( (*second_vecs_b_ptr)[vec_mask_num] != NULL )
            free((*second_vecs_b_ptr)[vec_mask_num]);
         else
            { printf("ERROR: FreeVectorsAndMasks(): Actual second_vecs_b_ptr[%d] IS NULL!\n", vec_mask_num); exit(EXIT_FAILURE); }
      free(*second_vecs_b_ptr);
      }
   else
      {
#ifdef DEBUG
printf("WARNING: FreeVectorsAndMasks(): Called with NULL second_vecs_b_ptr!\n");
#endif
      }
   *second_vecs_b_ptr = NULL;

   if ( masks_b_ptr != NULL && *masks_b_ptr != NULL )
      {
      for ( vec_mask_num = 0; vec_mask_num < *num_vecs_ptr; vec_mask_num++ )
         if ( (*masks_b_ptr)[vec_mask_num] != NULL )
            free((*masks_b_ptr)[vec_mask_num]);
         else
            { printf("ERROR: FreeVectorsAndMasks(): Actual masks_b_ptr[%d] IS NULL!\n", vec_mask_num); exit(EXIT_FAILURE); }
      free(*masks_b_ptr);
      }
   else
      {
#ifdef DEBUG
printf("WARNING: FreeVectorsAndMasks(): Called with NULL masks_b_ptr!\n");
#endif
      }
   *masks_b_ptr = NULL;

   *num_vecs_ptr = 0;
   *num_rise_vecs_ptr = 0;

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Get the IP address associated with the target interface.

void GetMyIPAddr(int max_string_len, const char *target_interface_name, char **IP_addr_ptr)
   {
   struct if_nameindex *if_nidxs, *intf;
   int interface_present; 

printf("GetMyIPAddr(): CALLED!\n"); fflush(stdout);
#ifdef DEBUG
#endif

// Check to make sure the target interface exists.
   if_nidxs = if_nameindex();
   interface_present = 0;
   if ( if_nidxs != NULL )
      {
      for (intf = if_nidxs; intf->if_index != 0 || intf->if_name != NULL; intf++)
         { 
         printf("%s\n", intf->if_name); fflush(stdout); 
         if ( strcmp(intf->if_name, target_interface_name) == 0 )
            interface_present = 1;
         }
      if_freenameindex(if_nidxs);
      }
   fflush(stdout);

   if ( interface_present == 0 )
      { printf("ERROR: GetMyIPAddr(): Target interface '%s' is NOT present!\n", target_interface_name); exit(EXIT_FAILURE); }

   int gethostname(char *name, size_t namelen);
   char hostbuffer[MAX_STRING_LEN];
//   struct hostent *host_entry;

   hostbuffer[max_string_len - 1] = '\0';
   if ( gethostname(hostbuffer, MAX_STRING_LEN - 1) == -1 )
      { printf("ERROR: GetMyIPAddr(): 'gethostname' failed!\n"); exit(EXIT_FAILURE); }
   printf("Hostname: '%s'\n", hostbuffer); fflush(stdout);


printf("GetMyIPAddr(): DONE!\n"); fflush(stdout);
#ifdef DEBUG
#endif

   return;
   }

// ========================================================================================================
// ========================================================================================================
// Open up multiple sockets and listen for connections from clients 

int OpenMultipleSocketServer(int max_string_len, int *master_socket_ptr, char *server_IP, int port_number, 
   char *client_IP, int max_clients, int *client_sockets, int *client_index_ptr, int initialize)
   {
   int new_socket, activity, i, sd;
   int opt = TRUE;
   int max_sd;

   struct sockaddr_in address;
   int addrlen;

// Set of socket descriptors
   fd_set readfds;
      
// First call, open master.
   if ( initialize == 1 )
      {

// Create a master socket
      if( (*master_socket_ptr = socket(AF_INET, SOCK_STREAM , 0)) == 0 )
         { perror("OpenMultipleSocketServer(): socket failed"); exit(EXIT_FAILURE); }
  
// Set master socket to allow multiple connections, this is just a good habit, it will work without this.
      if( setsockopt(*master_socket_ptr, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
         { perror("OpenMultipleSocketServer(): setsockopt"); exit(EXIT_FAILURE); }
  
// Prepare the sockaddr_in structure for the server. For the XWIN version when the devices are on different subnets, e.g.
// 192.168.1.10 and 192.168.2.10, etc, use INADDR_ANY to allow them to connect to the server. Can't do this if running
// multiple command line versions of the protocol.
      address.sin_family = AF_INET;

      address.sin_addr.s_addr = inet_addr(server_IP);
//      address.sin_addr.s_addr = ntohl(INADDR_ANY); // Added ntohl 4_27_2020 from reading http://alas.matf.bg.ac.rs/manuals/lspe/snode=27.html

      address.sin_port = htons(port_number);

// Bind the socket to localhost port 
      if ( bind(*master_socket_ptr, (struct sockaddr *)&address, sizeof(address)) < 0 )
         { perror("OpenMultipleSocketServer(): bind failed"); exit(EXIT_FAILURE); }

      printf("OpenMultipleSocketServer(): Listener on port %d\n", port_number);
     
// Try to specify maximum of n pending connections for the master socket
      if ( listen(*master_socket_ptr, max_clients) < 0 )
         { perror("OpenMultipleSocketServer(): listen"); exit(EXIT_FAILURE); }
      
// Accept the incoming connection
      puts("Waiting for connections ...");
      }
     
// ================================================
   while (1)
      {
      struct timeval to;

// Initially, set all bits in the file descriptor masks to 0 (clear the socket set)
      FD_ZERO(&readfds);
  
// 'master_socket_ptr' is just an integer. This sets the mask bit associated with the master socket descriptor.
      FD_SET(*master_socket_ptr, &readfds);
      max_sd = *master_socket_ptr;

// Time out value
      to.tv_sec = 0;
      to.tv_usec = 100;

// Do the same for all socket descriptors currently open -- set the corresponding mask bit. 
      for ( i = 0 ; i < max_clients ; i++ )
         {

// Socket descriptor
         sd = client_sockets[i];
             
// If valid socket descriptor then add to read list. Note, I now take out (temporarily at the Bank) sockets being serviced by Threads
// by setting their client_socket[i] to -1.
         if ( sd > 0 )
            FD_SET(sd, &readfds);
             
// Highest file descriptor number, need it for the select function
         if ( sd > max_sd )
            max_sd = sd;
         }
  
// Wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely. We are waiting on one class and ask "Is the socket
// ready for a read operation?"
      activity = select(max_sd + 1, &readfds, NULL, NULL, &to);
    
      if ( (activity < 0) && (errno != EINTR) ) 
         { 
         printf("OpenMultipleSocketServer(): select ERROR\t"); 
         if ( errno == EBADF )
            printf("Invalid file descriptor!\n");
         else if ( errno == EBADF )
            printf("nfds is negative or exceeds a resource limit!\n");
         else if ( errno == EINVAL )
            printf("Timeout value is invalid!\n");
         else if ( errno == ENOMEM )
            printf("Unable to allocate memory for internal tables!\n");
         else
            printf("UNKNOWN error!\n");
         }
      else if ( activity > 0 )
         break;

#ifdef DEBUG
printf("OpenMultipleSocketServer(): Waiting on select: Returned %d!\n", activity); fflush(stdout);
#endif
      }

#ifdef DEBUG
printf("OpenMultipleSocketServer(): Activity occurred on one of the sockets => Is master_socket %d FD_ISSET %d!\n", *master_socket_ptr,
   FD_ISSET(*master_socket_ptr, &readfds)); fflush(stdout);
#endif
          
// ---------------------------------------------------------------------------
// If something happened on the master socket, then its an incoming connection
   if ( FD_ISSET(*master_socket_ptr, &readfds) )
      {
      addrlen = sizeof(address);
      if ( (new_socket = accept(*master_socket_ptr, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0 )
         { perror("accept"); exit(EXIT_FAILURE); }
          
// Inform user of socket number - used in send and receive commands
#ifdef DEBUG
printf("OpenMultipleSocketServer(): New connection, socket fd is %d, IP is %s, port %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
fflush(stdout);
#endif

// Copy connecting client IP into return variable.
      strcpy(client_IP, inet_ntoa(address.sin_addr));

// Add new socket to array of sockets
      for ( i = 0; i < max_clients; i++ )
         {

// If position is empty, add the new socket descriptor to the array.
         if ( client_sockets[i] == 0 )
            {
            client_sockets[i] = new_socket;
#ifdef DEBUG
printf("OpenMultipleSocketServer(): Added to list of sockets as %d\n" , i); fflush(stdout);
#endif
            break;
            }
         }

// If none are found, return error code.
      if ( i < max_clients )
         *client_index_ptr = i;
      else
         {
         *client_index_ptr = -1;
         printf("ERROR: OpenMultipleSocketServer(): NO sockets are available! Increase size of client_sockets array!\n"); exit(EXIT_FAILURE);
         }

      return new_socket;
      }
          
// ---------------------------------------------------------------------------
// Else its an IO operation on some other socket that is already open
   *client_index_ptr = -1;
   for ( i = 0; i < max_clients; i++ )
      {
      sd = client_sockets[i];

// Do NOT check sockets that are open but in use by a thread.
      if ( sd == -1 )
         continue;

// Check if sd is still present in the readfds set of socket descriptors
      if ( FD_ISSET(sd, &readfds) )
         {

// IP retrieval doesn't seem to work... Returns 0.0.0.0.
         getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
         strcpy(client_IP, inet_ntoa(address.sin_addr));

#ifdef DEBUG
int count;
ioctl(sd, FIONREAD, &count);
printf("Connected host IP %s number of bytes %d (NOT ZERO %d)!\n" , client_IP, count, (int)(count != 0)); fflush(stdout);
#endif

#ifdef DEBUG
printf("Connected host IP %s has sent another message!\n" , client_IP); fflush(stdout);
#endif

         *client_index_ptr = i;

// Check if it was for closing, and also read the incoming message
//         if ( (valread = read(sd, buffer, 1024)) == 0 )
//            {

// Somebody disconnected, get his details and print
//            getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
//            printf("Host disconnected, IP %s, port %d\n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
                      
// Close the socket and mark as 0 in list for reuse
//            close(sd);
//            client_sockets[i] = 0;
//            }
                  
// Echo back the message that came in
//         else
//            {

// Set the string terminating NULL byte on the end of the data read
//            buffer[valread] = '\0';
//            send(sd, buffer, strlen(buffer), 0);
//            }
         }
      }
      
   return 0;
   }


// ========================================================================================================
// ========================================================================================================
// Open up a socket and listen for connections from clients 

int OpenSocketServer(int max_string_len, int *server_socket_desc_ptr, char *server_IP, int port_number, 
   int *client_socket_desc_ptr, struct sockaddr_in *client_addr_ptr, int accept_only, int check_and_return)
   {
   struct sockaddr_in server_addr;
   int sizeof_sock;
   int queue_size = 1;
   int opt = TRUE;

// Create a socket and prepare the socket_in structure.
   if ( accept_only == 0 )
      {
//      printf("OpenSocketServer(): Creating a socket\n"); fflush(stdout); 
      *server_socket_desc_ptr = socket(AF_INET, SOCK_STREAM, 0);
      if ( *server_socket_desc_ptr == -1 )
         { printf("ERROR: OpenSocketServer(): Could not create socket"); fflush(stdout); }

// Set master socket to allow multiple connections, this is just a good habit, it will work without this.
      if( setsockopt(*server_socket_desc_ptr, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
         { perror("OpenSocketServer(): setsockopt"); exit(EXIT_FAILURE); }

// Do non-block fctrl call
      if ( check_and_return == 1 )
         {
         int flags = fcntl(*server_socket_desc_ptr, F_GETFL, 0);
         if (flags < 0) return 0;
         flags = flags | O_NONBLOCK;
         if ( fcntl(*server_socket_desc_ptr, F_SETFL, flags) != 0 )
            { printf("ERROR: OpenSocketServer(): Failed to set 'fcntl' flags to non-blocking!\n"); exit(EXIT_FAILURE); }
         }
        
// Prepare the sockaddr_in structure for the server. For the XWIN version when the devices are on different subnets, e.g.
// 192.168.1.10 and 192.168.2.10, etc, use INADDR_ANY to allow them to connect to the server. Can't do this if running
// multiple command line versions of the protocol.
      server_addr.sin_family = AF_INET;

      server_addr.sin_addr.s_addr = inet_addr(server_IP);
//      server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

      server_addr.sin_port = htons(port_number);

      memset(server_addr.sin_zero, 0, 8); 

// Bind the server with the socket.
      if ( bind(*server_socket_desc_ptr, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 )
         { printf("ERROR: OpenSocketServer(): Failed to bind!\n"); exit(EXIT_FAILURE); }
//   printf("OpenSocketServer(): Bind done!\n"); fflush(stdout);

// Listen to the socket. 'queue_size' indicates how many requests can be pending before an error is
// returned to the remote computer requesting a connection.
      listen(*server_socket_desc_ptr, queue_size);
      }

//   printf("OpenSocketServer(): Waiting for incoming connections from clients ....\n"); fflush(stdout);

// Waiting and accept incoming connection from the client. Device address information is returned
// Note: you can also use 'select' here to determine if a connection request exists.
   sizeof_sock = sizeof(struct sockaddr_in);

   if ( check_and_return == 0 )
      {
      while ( (*client_socket_desc_ptr = accept(*server_socket_desc_ptr, (struct sockaddr *)client_addr_ptr, (socklen_t*)&sizeof_sock)) < 0 )
         ;
//   if (*client_socket_desc_ptr < 0)
//      { printf("ERROR: OpenSocketServer(): Failed accept\n"); exit(EXIT_FAILURE); }

//   printf("OpenSocketServer(): Connection accepted!\n"); fflush(stdout); 
      return 1;
      }
   else
      {
      if ( (*client_socket_desc_ptr = accept(*server_socket_desc_ptr, (struct sockaddr *)client_addr_ptr, (socklen_t*)&sizeof_sock)) < 0 )
         return 0;
      else
         return 1;
      }

   return 1;
   }


// ========================================================================================================
// ========================================================================================================
// Open up a client socket connection. Returns 0 on success and -1 if fails.

int OpenSocketClient(int max_string_len, char *server_IP, int port_number, int *server_socket_desc_ptr)
   {
   struct sockaddr_in server_addr;
   int result;

//   printf("OpenSocketClient(): Creating a socket\n"); fflush(stdout); 
   if ( (*server_socket_desc_ptr = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
      { printf("ERROR: OpenSocketClient(): Could not create socket"); exit(EXIT_FAILURE); }
        
   server_addr.sin_addr.s_addr = inet_addr(server_IP);
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(port_number);
   memset(server_addr.sin_zero, 0, 8); 

// Connect to it. 9/14/2018: Changing this from 'exit' to return error.
   if ( (result = connect(*server_socket_desc_ptr, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0 )
      { printf("INFO: OpenSocketClient: Connect failed -- no listener\n"); fflush(stdout); }
     
//   printf("OpenSocketClient(): Connected\n"); fflush(stdout);

// Close the socket if the connect fails.
   if ( result < 0 )
      close(*server_socket_desc_ptr);

   return result;
   }


// ========================================================================================================
// ========================================================================================================
// Open up a datagram socket for receiving broadcasts

int OpenSocketServerUDP(int max_string_len, int *bcast_server_socket_desc_ptr, char *server_IP, int port_number, 
   struct sockaddr_in *client_addr_ptr, int accept_only, int check_and_return, char *buff, char *bcast_subnet_IP)
   {
   struct sockaddr_in my_addr;
   unsigned int sizeof_sock;
   int opt = TRUE;
   int broadcastEnable = 1;

// Initialize the buffer to a NULL string.
   buff[0] = '\0';

// Create a socket and prepare the socket_in structure.
   if ( accept_only == 0 )
      {
      if ( (*bcast_server_socket_desc_ptr = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
         { perror("ERROR: OpenSocketServerUDP(): Cannnot create DATAGRAM socket!"); exit(EXIT_FAILURE); }

// Set master socket to allow multiple connections, this is just a good habit, it will work without this.
      if( setsockopt(*bcast_server_socket_desc_ptr, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
         { perror("ERROR: OpenSocketServerUDP(): setsockopt to allow multiple connections"); exit(EXIT_FAILURE); }

// Enable reception of broadcast messages.
      setsockopt(*bcast_server_socket_desc_ptr, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

// Do non-block fctrl call
      if ( check_and_return == 1 )
         {
         struct timeval read_timeout;
         read_timeout.tv_sec = 0;
         read_timeout.tv_usec = 1000;
         setsockopt(*bcast_server_socket_desc_ptr, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout));

//         int flags = fcntl(*bcast_server_socket_desc_ptr, F_GETFL, 0);
//         if (flags < 0) return 0;
//         flags = flags | O_NONBLOCK;
//         if ( fcntl(*bcast_server_socket_desc_ptr, F_SETFL, flags) != 0 )
//            { printf("ERROR: OpenSocketServerUDP(): Failed to set 'fcntl' flags to non-blocking!\n"); exit(EXIT_FAILURE); }
         }

// Prepare the sockaddr_in structure for the server. 
      memset(&my_addr, '\0', sizeof(struct sockaddr_in));
      my_addr.sin_family = AF_INET;
      my_addr.sin_port = (in_port_t)htons(port_number+1);
//      my_addr.sin_addr.s_addr = inet_addr(server_IP);
      my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//      my_addr.sin_addr.s_addr = inet_addr(bcast_subnet_IP);

      if ( bind(*bcast_server_socket_desc_ptr, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0 )
         { perror("ERROR: OpenSockeServerUDP(): Failed to bind"); }

// No 'listen' call for UDP.
      }

#ifdef DEBUG
printf("Checking\n"); fflush(stdout);
#endif

   sizeof_sock = sizeof(struct sockaddr_in);
   recvfrom(*bcast_server_socket_desc_ptr, buff, max_string_len, 0, (struct sockaddr *)client_addr_ptr, &sizeof_sock);

   return 1;
   }


// ========================================================================================================
// ========================================================================================================
// This function is designed to buffer data but in kernel space. It allows binary data to be transmitted. 
// To accomplish this, the first two bytes are interpreted as the length of the binary byte stream that follows.

int SockGetB(unsigned char *buffer, int buffer_size, int socket_desc)
   {
   int tot_bytes_received, target_num_bytes;
   unsigned char buffer_num_bytes[4];

// Call until two bytes are returned. The 2-byte buffer represents a number that is to be interpreted as the exact 
// number of binary bytes that will follow in the socket.
   target_num_bytes = 3;
   tot_bytes_received = 0;
   while ( tot_bytes_received < target_num_bytes )
      if ( (tot_bytes_received += recv(socket_desc, &buffer_num_bytes[tot_bytes_received], target_num_bytes - tot_bytes_received, 0)) < 0 )
         { printf("ERROR: SockGetB(): Error in receiving three byte cnt!\n"); fflush(stdout); return -1; }

// Translate the binary bytes into an integer.
   target_num_bytes = (int)(buffer_num_bytes[2] << 16) + (int)(buffer_num_bytes[1] << 8) + (int)buffer_num_bytes[0];

// DEBUG
//printf("SockGetB(): fetching %d bytes from socket\n", target_num_bytes); fflush(stdout);

// Sanity check.
   if ( target_num_bytes > buffer_size )
      { 
      printf("ERROR: SockGetB(): 'target_num_bytes' %d is larger than buffer input size %d\n", 
         target_num_bytes, buffer_size); fflush(stdout); return -1;
      }

// Now start reading binary bytes from the socket
   tot_bytes_received = 0;
   while ( tot_bytes_received < target_num_bytes )
      if ( (tot_bytes_received += recv(socket_desc, &buffer[tot_bytes_received], target_num_bytes - tot_bytes_received, 0)) < 0 )
         { printf("ERROR: SockGetB(): Error in receiving transmitted data!\n"); fflush(stdout); return -1; }

// Sanity check
   if ( tot_bytes_received != target_num_bytes )
      { 
      printf("ERROR: SockGetB(): Read more bytes then requested -- 'tot_bytes_received' %d is larger than target %d\n", 
         tot_bytes_received, target_num_bytes); fflush(stdout); return -1; 
      }

// DEBUG
//printf("SockGetB(): received %d bytes\n", tot_bytes_received); fflush(stdout);

   return tot_bytes_received;
   }


// ========================================================================================================
// ========================================================================================================
// This function sends binary or ASCII data of 'buffer_size' unsigned characters through the socket. It first 
// sends two binary bytes that represent the length of the binary or ASCII byte stream that follows.

int SockSendB(unsigned char *buffer, int buffer_size, int socket_desc)
   {
   unsigned char num_bytes[3];

// Sanity check. Don't yet support transfers larger than 16,777,215 bytes.
   if ( buffer_size > 16777215 )
      { printf("ERROR: SockSendB(): Size of buffer %d larger than max (16777215)!\n", buffer_size); fflush(stdout); return -1; }

   num_bytes[2] = (unsigned char)((buffer_size & 0x00FF0000) >> 16);
   num_bytes[1] = (unsigned char)((buffer_size & 0x0000FF00) >> 8);
   num_bytes[0] = (unsigned char)(buffer_size & 0x000000FF);
   if ( send(socket_desc, num_bytes, 3, 0) < 0 )
      { printf("ERROR: SockSendB(): Send 'num_bytes' %d failed\n", buffer_size); fflush(stdout); return -1; }
   if ( send(socket_desc, buffer, buffer_size, 0) < 0 )
      { printf("ERROR: SockSendB(): Send failed\n"); fflush(stdout); return -1; }

   return 0;
   }


// ========================================================================================================
// ========================================================================================================
// This function prints a header followed by a block of hex digits. Used in DEBUG mode.

void PrintHeaderAndHexVals(char *header_str, int num_bytes, unsigned char *vals, int max_vals_per_row)
   {
   int i;

   printf("%s", header_str);
   printf("\t\t");
   fflush(stdout);
   for ( i = 0; i < num_bytes; i++ )
      {
      printf("%02X ", vals[i]); 
      if ( (i+1) % max_vals_per_row == 0 )
         printf("\n\t\t");
      }
//   printf("\n\t\tThis last byte may have additional bits if non-zero (given left-to-right) %02X\n", vals[num_bytes]);
   printf("\n\n");
   fflush(stdout);

   return;
   }


// ========================================================================================================
// ========================================================================================================
// This function prints a header followed by a block of hex digits. Used in DEBUG mode.

void PrintHeaderAndBinVals(char *header_str, int num_bits, unsigned char *vals, int max_vals_per_row)
   {
   int i;

   printf("%s", header_str);
   printf("\t\t");
   fflush(stdout);
   for ( i = 0; i < num_bits; i++ )
      {
      if ( GetBitFromByte(vals[i/8], i % 8) == 0 )
         printf("0");
      else
         printf("1");
      if ( (i+1) % 8 == 0 )
         printf(" ");
      if ( (i+1) % max_vals_per_row == 0 )
         printf("\n\t\t");
      }
   printf("\n\n");
   fflush(stdout);

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Set the parameters to be used in the SiRF algorithm using the (n1 XOR n2) nonces.

void SelectParams(int nonce_len_bytes, unsigned char *nonce_bytes, int nonce_base_address, unsigned int *LFSR_seed_low_ptr, 
   unsigned int *LFSR_seed_high_ptr, unsigned int *RangeConstant_ptr, unsigned short *SpreadConstant_ptr, 
   unsigned short *Threshold_ptr, unsigned short *TrimCodeConstant_ptr)
   {

// Sanity check
   if ( nonce_base_address + 8 > nonce_len_bytes )
      { 
      printf("ERROR: SelectParams(): Number of nonce bytes in array %d not sufficient to support starting position at base address %d + 8 !\n", 
         nonce_len_bytes, nonce_base_address); exit(EXIT_FAILURE); 
      }

// BYTES 0, 1, 2 and 3: LFSR SEEDs 
   *LFSR_seed_low_ptr = (0x000007FF & ((nonce_bytes[1 + nonce_base_address] << 8) + nonce_bytes[0 + nonce_base_address]));
   *LFSR_seed_high_ptr = (0x000007FF & ((nonce_bytes[3 + nonce_base_address] << 8) + nonce_bytes[2 + nonce_base_address]));

#ifdef DEBUG
printf("\tCurrent function %d\tLFSR seeds choosen from byte[3] '%02X' byte[2] '%02X' byte[1] '%02X' byte[0] %02X => Low: %08X (%u)\tHigh %08X (%u)\n",
   current_function, nonce_bytes[3 + nonce_base_address], nonce_bytes[2 + nonce_base_address], nonce_bytes[1 + nonce_base_address], 
   nonce_bytes[0 + nonce_base_address], *LFSR_seed_low_ptr, *LFSR_seed_low_ptr, *LFSR_seed_high_ptr, *LFSR_seed_high_ptr);
#endif

// DEBUG
#ifdef DEBUG
printf("\tLFSR seed low %u\tLFSR seed high %u\tSpreadConstant %u\tThreshold %u\n", *LFSR_seed_low_ptr, *LFSR_seed_high_ptr, *SpreadConstant_ptr, *Threshold_ptr);
#endif

   return;
   }


// ========================================================================================================
// ========================================================================================================
// See /home/research/research/FPGAs/HELP/VHDL_HELP/contrib/lfsr_randgen/trunk/lfsr_pkg.vhd Other possibilities 
// from stdint: int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t 

void LFSR_11_A_bits_low(int load_seed, uint16_t seed, uint16_t *lfsr_ptr)
   {
   uint16_t bit, nor_bit;

/* Load the seed on the first iteration */
   if ( load_seed == 1 )
      *lfsr_ptr = seed;
   else
      {

/* Allow all zero state. See my BIST class notes in VLSI Testing. Note, we use low order bits
   here because bit is shifted onto the low side, not high side as in my lecture slides. */
      if ( !( (((*lfsr_ptr >> 9) & 1) == 1) || (((*lfsr_ptr >> 8) & 1) == 1) ||
              (((*lfsr_ptr >> 7) & 1) == 1) || (((*lfsr_ptr >> 6) & 1) == 1) || (((*lfsr_ptr >> 5) & 1) == 1) ||
              (((*lfsr_ptr >> 4) & 1) == 1) || (((*lfsr_ptr >> 3) & 1) == 1) || (((*lfsr_ptr >> 2) & 1) == 1) ||
              (((*lfsr_ptr >> 1) & 1) == 1) || (((*lfsr_ptr >> 0) & 1) == 1) ) )
         nor_bit = 1;
      else
         nor_bit = 0;

/* xor_out := rand(10) xor rand(8); */
      bit  = ((*lfsr_ptr >> 10) & 1) ^ ((*lfsr_ptr >> 8) & 1) ^ nor_bit;

/* printf("LFSR value %d\tNOR bit value %d\tlow order bit %d\n", *lfsr_ptr, nor_bit, bit); */

/* Change the shift of the bit to match the width of the data type. */
      *lfsr_ptr =  ((*lfsr_ptr << 1) | bit) & 2047;
      }

   return; 
   }


// ========================================================================================================
// ========================================================================================================
// Need to duplicate because of 'static' variable
void LFSR_11_A_bits_high(int load_seed, uint16_t seed, uint16_t *lfsr_ptr)
   {
   uint16_t bit, nor_bit;

/* Load the seed on the first iteration */
   if ( load_seed == 1 )
      *lfsr_ptr = seed;
   else
      {

/* Allow all zero state. See my BIST class notes in VLSI Testing. Note, we use low order bits
   here because bit is shifted onto the low side, not high side as in my lecture slides. */
      if ( !( (((*lfsr_ptr >> 9) & 1) == 1) || (((*lfsr_ptr >> 8) & 1) == 1) ||
              (((*lfsr_ptr >> 7) & 1) == 1) || (((*lfsr_ptr >> 6) & 1) == 1) || (((*lfsr_ptr >> 5) & 1) == 1) ||
              (((*lfsr_ptr >> 4) & 1) == 1) || (((*lfsr_ptr >> 3) & 1) == 1) || (((*lfsr_ptr >> 2) & 1) == 1) ||
              (((*lfsr_ptr >> 1) & 1) == 1) || (((*lfsr_ptr >> 0) & 1) == 1) ) )
         nor_bit = 1;
      else
         nor_bit = 0;

/* xor_out := rand(10) xor rand(8); */
      bit  = ((*lfsr_ptr >> 10) & 1) ^ ((*lfsr_ptr >> 8) & 1) ^ nor_bit;

/* printf("LFSR value %d\tNOR bit value %d\tlow order bit %d\n", *lfsr_ptr, nor_bit, bit); */

/* Change the shift of the bit to match the width of the data type. */
      *lfsr_ptr =  ((*lfsr_ptr << 1) | bit) & 2047;
      }

   return; 
   }


// ========================================================================================================
// ========================================================================================================
// Send num_vecs and vector pairs to the device for ID phase enrollment. 

void SendVectorsAndMasks(int max_string_len, int num_vecs, int device_socket_desc, int num_rise_vecs, int num_PIs, 
   unsigned char **first_vecs_b, unsigned char **second_vecs_b, int has_masks, int num_POs, unsigned char **masks)
   {
   char num_vecs_str[max_string_len];
   int i;

// Send num_vecs first. 
   sprintf(num_vecs_str, "%d %d %d", num_vecs, num_rise_vecs, has_masks);

#ifdef DEBUG
printf("Sending '%s'\n to device\n", num_vecs_str); fflush(stdout);
#endif

// Send num_bytes of string as two-binary bytes. When sending ASCII character strings, be sure to add one to include 
// the NULL termination character (+ 1) so the receiver can treat this as a string. 
   if ( SockSendB((unsigned char *)num_vecs_str, strlen(num_vecs_str) + 1, device_socket_desc) < 0 )
      { printf("ERROR: SendVectorsAndMasks(): Send '%s' failed\n", num_vecs_str); exit(EXIT_FAILURE); }

// Send first_vecs and second_vecs to remote server
   for ( i = 0; i < num_vecs; i++ )
      {
      if ( SockSendB(first_vecs_b[i], num_PIs/8, device_socket_desc) < 0 )
         { printf("ERROR: SendVectorsAndMasks(): Send 'first_vecs_b[%d]' failed!\n", i); exit(EXIT_FAILURE); }
      if ( SockSendB(second_vecs_b[i], num_PIs/8, device_socket_desc) < 0 )
         { printf("ERROR: SendVectorsAndMasks(): Send 'second_vecs_b[%d]' failed!\n", i); exit(EXIT_FAILURE); }
      if ( has_masks == 1 && SockSendB(masks[i], num_POs/8, device_socket_desc) < 0 )
         { printf("ERROR: SendVectorsAndMasks(): Send 'masks[%d]' failed!\n", i); exit(EXIT_FAILURE); }
      }

#ifdef DEBUG
printf("SendVectorsAndMasks(): Sent %d vector pairs!\n", num_vecs); fflush(stdout);
#endif

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Receive 'GO' and send vectors and masks. Called by verifier_regeneration.c, and in certain versions of
// the PUF-Cash protocol but the TTP?

void GoSendVectors(int max_string_len, int num_POs, int num_PIs, int device_socket_desc, int num_vecs, 
   int num_rise_vecs, int has_masks, unsigned char **first_vecs_b, unsigned char **second_vecs_b, 
   unsigned char **masks, int get_GO, int use_database_chlngs, int DB_ChallengeGen_seed, int DEBUG)
   {
   char request_str[max_string_len];

   struct timeval t0, t1;
   long elapsed; 

// ****************************************
// ***** Receive "GO signal" 
   if ( get_GO == 1 )
      {
      if ( DEBUG == 1 )
         {
         printf("GSV.1: Waiting 'GO'\n");
         gettimeofday(&t0, 0);
         }
      if ( SockGetB((unsigned char *)request_str, MAX_STRING_LEN, device_socket_desc) != 3 )
         { printf("ERROR: GoSendVectors(): Failed to get 'GO' from device!\n"); exit(EXIT_FAILURE); }
      if ( strcmp(request_str, "GO") != 0 )
         { printf("ERROR: GoSendVectors(): Did NOT receive 'GO' string from device!\n"); exit(EXIT_FAILURE); }
      if ( DEBUG == 1 )
         { gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed); }
      }

// ****************************************
// ***** Send the ID vectors to the device 
   if ( DEBUG == 1 )
      {
      printf("GSV.2: Sending vectors\n");
      gettimeofday(&t0, 0);
      }

// 11_1_2021: Added device-side Challenge generation (may have been in Oct), which requires only the LFSR seed used by the server.
// Always send the seed independent of whether we are generating and sending vectors from the server (here) or if the device will
// generate them using only the DB_ChallengeGen_seed.
   char DB_ChallengeGen_seed_str[max_string_len];

   sprintf(DB_ChallengeGen_seed_str, "%d", DB_ChallengeGen_seed);
   if ( SockSendB((unsigned char *)DB_ChallengeGen_seed_str, strlen(DB_ChallengeGen_seed_str) + 1, device_socket_desc) < 0 )
      { printf("ERROR: GoSendVectors(): Send '%s' failed\n", DB_ChallengeGen_seed_str); fflush(stdout); exit(EXIT_FAILURE); }

#ifdef DEBUG
printf("GoSendVectors(): Sending %u as ChallengeGen_seed to device!\n", DB_ChallengeGen_seed); fflush(stdout);
#endif

// NOTE: It is the responsibility of the verifier to provide enough rising vectors to supply at least 2048 rising and falling PNs but once the
// last rising is applied that gets us over the 2048, NO ADDITIONAL rising VECTORS should be applied and the first falling vector should be next.
   if ( use_database_chlngs == 0 )
      SendVectorsAndMasks(max_string_len, num_vecs, device_socket_desc, num_rise_vecs, num_PIs, first_vecs_b, second_vecs_b, has_masks, num_POs, masks);

   if ( DEBUG == 1 )
      { gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed); }
   fflush(stdout);

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Called by verifier_provision.c Send num_chlngs and vector pairs to the device for ID phase enrollment. 

void SendChlngsAndMasks(int max_string_len, int num_chlngs, int device_socket_desc, int num_rise_chlngs, int num_chlng_bits, 
   unsigned char **challenges_b, int has_masks, int num_POs, unsigned char **masks)
   {
   char num_chlngs_str[max_string_len];
   int i;

// Send num_chlnges first. 
   sprintf(num_chlngs_str, "%d %d %d", num_chlngs, num_rise_chlngs, has_masks);

#ifdef DEBUG
printf("Sending '%s'\n to device\n", num_chlngs_str); fflush(stdout);
#endif

// Send num_bytes of string as two-binary bytes. When sending ASCII character strings, be sure to add one to include 
// the NULL termination character (+ 1) so the receiver can treat this as a string. 
   if ( SockSendB((unsigned char *)num_chlngs_str, strlen(num_chlngs_str) + 1, device_socket_desc) < 0 )
      { printf("ERROR: SendChlngsAndMasks(): Send '%s' failed\n", num_chlngs_str); fflush(stdout); exit(EXIT_FAILURE); }

// Send first_vecs and second_vecs to remote server
   for ( i = 0; i < num_chlngs; i++ )
      {
      if ( SockSendB(challenges_b[i], num_chlng_bits/8, device_socket_desc) < 0 )
         { printf("ERROR: SendChlngsAndMasks(): Send 'challenges_b[%d]' failed\n", i); fflush(stdout); exit(EXIT_FAILURE); }
      if ( has_masks == 1 && SockSendB(masks[i], num_POs/8, device_socket_desc) < 0 )
         { printf("ERROR: SendChlngsAndMasks(): Send 'masks[%d]' failed\n", i); fflush(stdout); exit(EXIT_FAILURE); }
      }

// DEBUG
// printf("SendChlngsAndMasks(): Sent %d vector pairs!\n", num_chlngs); fflush(stdout);

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Receive 'GO' and send vectors and masks. NOT CURRENTLY USED. NOTE: IF YOU USE THIS, NEED TO ADD args
// used in GoSendVectors related to database seed.

void GoSendChlngs(int max_string_len, int num_POs, int num_chlng_bits, int device_socket_desc, int num_chlngs, 
   int num_rise_chlngs, int has_masks, unsigned char **challenges_b, unsigned char **masks, int get_GO, 
   int DEBUG)
   {
   char request_str[max_string_len];

   struct timeval t0, t1;
   long elapsed; 

// ****************************************
// ***** Receive "GO signal" 
   if ( get_GO == 1 )
      {
      if ( DEBUG == 1 )
         {
         printf("GSV.1: Waiting 'GO'\n");
         gettimeofday(&t0, 0);
         }
      if ( SockGetB((unsigned char *)request_str, MAX_STRING_LEN, device_socket_desc) != 2 )
         { printf("Receive 'GO' request failed"); fflush(stdout); exit(EXIT_FAILURE); }
      if ( DEBUG == 1 )
         { gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed); }
      }

// ****************************************
// ***** Send the ID vectors to the device 
   if ( DEBUG == 1 )
      {
      printf("GSV.2: Sending vectors\n");
      gettimeofday(&t0, 0);
      }

// NOTE: It is the responsibility of the verifier to provide enough rising vectors to supply at least 2048 rising and falling PNs but once the
// last rising is applied that gets us over the 2048, NO ADDITIONAL rising VECTORS should be applied and the first falling vector should be next.
   SendChlngsAndMasks(max_string_len, num_chlngs, device_socket_desc, num_rise_chlngs, num_chlng_bits, challenges_b, has_masks, num_POs, masks);
   if ( DEBUG == 1 )
      { gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed); }
   fflush(stdout);

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Reads in the vector file. Format is first vector/second vector on two separate lines, followed by a blank 
// line, then repeat. Space for first_vecs and second_vecs is allocated in the caller.
//
// USED FOR verifier_enrollment.c (NOTE: DATABASE commonDB.c defines another very similar version that reads
// ASCII version of the mask because we store the ASCII version in the database because of the 'u's and 'q's.
//
// Also using this in device_regeneration.c to restore KEKChlng information that is written to the SD card.

int ReadVectorAndMaskFilesBinary(int max_string_len, char *vec_pair_path, int num_PIs, int *num_rise_vecs_ptr, 
   unsigned char ***first_vecs_b_ptr, unsigned char ***second_vecs_b_ptr, int has_masks, char *mask_file_path, 
   int num_POs, unsigned char ***masks_b_ptr, int num_direction_chlng_bits)
   {
   char first_vec[num_PIs+1], second_vec[num_PIs+1];
   char line[max_string_len], *char_ptr;
   int first_second_vec, rise_or_fall;
   unsigned char *vec_ptr;
   int vec_pair_cnt;
   FILE *INFILE;

   if ( (INFILE = fopen(vec_pair_path, "r")) == NULL )
      { printf("ERROR: ReadVectorAndMaskFilesBinary(): Could not open VectorPair file %s\n", vec_pair_path); exit(EXIT_FAILURE); }

// Sanity check. Make sure 'num_PIs' is evenly divisible by 8 otherwise the binary conversion routine below will NOT work.
   if ( (num_PIs % 8) != 0 )
      { printf("ERROR: ReadVectorAndMaskFilesBinary(): 'num_PIs' %d MUST be divisible by 16!\n", num_PIs); exit(EXIT_FAILURE); }

   if ( has_masks == 1 && (num_POs % 8) != 0 )
      { printf("ERROR: ReadVectorAndMaskFilesBinary(): 'num_POs' %d MUST be divisible by 8!\n", num_POs); exit(EXIT_FAILURE); }

// Force realloc to behave like malloc. DO NOT DO 'masks_asc_ptr' here because we set it to NULL sometimes in the caller. Done below.
   *first_vecs_b_ptr = NULL;
   *second_vecs_b_ptr = NULL;
   *masks_b_ptr = NULL;

   vec_pair_cnt = 0;
   first_second_vec = 0;
   *num_rise_vecs_ptr = 0;
   rise_or_fall = -1;
   while ( fgets(line, max_string_len, INFILE) != NULL )
      {

// Find the newline and eliminate it.
      if ((char_ptr = strrchr(line, '\n')) != NULL)
         *char_ptr = '\0';

// Assume there is NO blank line as the first line in the file, and assume the last line is present but blank
      if ( strlen(line) == 0 )
         {
         vec_pair_cnt++;
         continue;
         }

// Sanity checks
      if ( strlen(line) != (unsigned int)num_PIs )
         { printf("ERROR: ReadVectorAndMaskFilesBinary(): Unexpected vector size %u -- expected %d!\n", (unsigned int)strlen(line), num_PIs); exit(EXIT_FAILURE); }

// Assume the challenges are constructed to be rising only or falling only. For the SRP, a rising vector includes a '1' in the LSB of the high order 16
// bits (dummy bits). Each bit refers to a row in the SRP with the LSB bit referring to the LAST row. Use that bit as the rise/fall indicator.
// Note the file ASCII version of the challenge has the high order bits of the challenge in the low order ASCII bit positions.

// Copy the first ASCII version of the vector into the stack allocated space + 1 (for NULL termination).
      if ( first_second_vec == 0 )
         strcpy(first_vec, line);
      else
         {
         strcpy(second_vec, line);

// LSB bit of high order 16 bit word that is referred to as the direction word. A value of '1' indicates a rising transition (THIS MAY CHANGE IN THE FUTURE).
         if ( second_vec[num_direction_chlng_bits - 1] == RISE_DIR )
            rise_or_fall = 0;
         else
            rise_or_fall = 1;

// Count up the rising vectors
         if ( rise_or_fall == 0 )
            (*num_rise_vecs_ptr)++;
         }

// =============================================
// Convert from ASCII to binary. Allocate space for binary unsigned char version of vectors. 
      if ( first_second_vec == 0 )
         {
         if ( (*first_vecs_b_ptr = (unsigned char **)realloc(*first_vecs_b_ptr, sizeof(unsigned char *)*(vec_pair_cnt + 1))) == NULL )
            { printf("ERROR: ReadVectorAndMaskFilesBinary(): Failed to reallocate storage for first_vecs_b_ptr array!\n"); exit(EXIT_FAILURE); }
         if ( ((*first_vecs_b_ptr)[vec_pair_cnt] = (unsigned char *)malloc(sizeof(unsigned char)*num_PIs/8)) == NULL )
            { printf("ERROR: ReadVectorAndMaskFilesBinary(): Failed to allocate storage for first_vecs_b_ptr string!\n"); exit(EXIT_FAILURE); }
         vec_ptr = (*first_vecs_b_ptr)[vec_pair_cnt];
         }
      else
         {
         if ( (*second_vecs_b_ptr = (unsigned char **)realloc(*second_vecs_b_ptr, sizeof(unsigned char *)*(vec_pair_cnt + 1))) == NULL )
            { printf("ERROR: ReadVectorAndMaskFilesBinary(): Failed to reallocate storage for second_vecs_b_ptr array!\n"); exit(EXIT_FAILURE); }
         if ( ((*second_vecs_b_ptr)[vec_pair_cnt] = (unsigned char *)malloc(sizeof(unsigned char)*num_PIs/8)) == NULL )
            { printf("ERROR: ReadVectorAndMaskFilesBinary(): Failed to allocate storage for second_vecs_b_ptr string!\n"); exit(EXIT_FAILURE); }
         vec_ptr = (*second_vecs_b_ptr)[vec_pair_cnt];
         }

#ifdef DEBUG
printf("ReadVectorAndMaskFilesBinary(): Binary vec for vec_pair_cnt %d\n\t", vec_pair_cnt); fflush(stdout);
#endif

// Convert ASCII version of vector to binary, reversing the order as appropriate for the hardware that uses these vectors. ASCII version of vector 
// processed from 0 to 'num_PIs' is high order to low order. Preserve this characteristics in the unsigned char binary byte array by starting the 
// load at the highest byte. NOTE: 'num_PIs' MUST BE DIVISIBLE BY 8 otherwise this code does not work properly, which is checked above.
// Added this call 7/29/2019.
      ConvertASCIIVecMaskToBinary(num_PIs, line, vec_ptr);

// Each non-zero line has a vector, invert first or second vector indicator and add 1 to vec_pair_cnt 
      first_second_vec = !first_second_vec;
      }
   fclose(INFILE);


// Read in the mask file if specified
   if ( has_masks == 1 )
      {
      int mask_vec_num;
      if ( (INFILE = fopen(mask_file_path, "r")) == NULL )
         { printf("ERROR: ReadVectorAndMaskFilesBinary(): Could not open Mask file %s\n", mask_file_path); exit(EXIT_FAILURE); }

// We MUST find a mask for every vector read above.
      for ( mask_vec_num = 0; mask_vec_num < vec_pair_cnt; mask_vec_num++ )
         {

// Sanity check
         if ( fgets(line, max_string_len, INFILE) == NULL )
            { printf("ERROR: ReadVectorAndMaskFilesBinary(): A mask file is requested but number of masks_b does NOT equal number of vectors!\n"); exit(EXIT_FAILURE); }

// Find the newline and eliminate it.
         if ((char_ptr = strrchr(line, '\n')) != NULL)
            *char_ptr = '\0';

// Allocate space for the mask (num_POs/8)
         if ( (*masks_b_ptr = (unsigned char **)realloc(*masks_b_ptr, sizeof(unsigned char *)*(mask_vec_num + 1))) == NULL )
            { printf("ERROR: ReadVectorAndMaskFilesBinary(): Failed to reallocate storage for masks_b_ptr array!\n"); exit(EXIT_FAILURE); }
         if ( ((*masks_b_ptr)[mask_vec_num] = (unsigned char *)malloc(sizeof(unsigned char)*num_POs/8)) == NULL )
            { printf("ERROR: ReadVectorAndMaskFilesBinary(): Failed to allocate storage for masks_b_ptr string!\n"); exit(EXIT_FAILURE); }

// ASCII version of mask processed from 0 to 'num_POs' is high order to low order. Preserve this characteristics in the unsigned char binary 
// byte array by starting the load at the highest byte. NOTE: 'num_POs' MUST BE DIVISIBLE BY 8 otherwise this code does not work properly,
// which is checked above. Added this call 7/29/2019.
         ConvertASCIIVecMaskToBinary(num_POs, line, (*masks_b_ptr)[mask_vec_num]);
         }
      fclose(INFILE);
      }

#ifdef DEBUG
int i;
printf("REGEN: Second Vec:\n\t");
for ( i = 0; i < num_PIs/8; i++ )
   printf("%2X ", (*first_vecs_b_ptr)[1][i]);
printf("\n\n");
#endif

   return vec_pair_cnt;
   }


// ========================================================================================================
// ========================================================================================================
// Called by verifier_provision.c Reads in the challenges file as challenges_b (NOT first_vecs_b/second_vecs_b).

int ReadChlngAndMaskFilesBinary(int max_string_len, char *chlng_path, int num_chlng_bits, int *num_rise_chlngs_ptr, 
   unsigned char ***challenges_b_ptr, int has_masks, char *mask_file_path, int num_POs, 
   unsigned char ***masks_b_ptr, int num_direction_chlng_bits)
   {
   char line[max_string_len], *char_ptr;
   char challenge[num_chlng_bits+1];
   unsigned char *vec_ptr;
   int rise_or_fall;
   int chlng_cnt;
   FILE *INFILE;

   if ( (INFILE = fopen(chlng_path, "r")) == NULL )
      { printf("ERROR: ReadChlngAndMaskFilesBinary(): Could not open VectorPair file %s\n", chlng_path); exit(EXIT_FAILURE); }

// Sanity check. Make sure 'num_chlng_bits' is evenly divisible by 8 otherwise the binary conversion routine below will NOT work.
   if ( (num_chlng_bits % 8) != 0 )
      { printf("ERROR: ReadChlngAndMaskFilesBinary(): 'num_chlng_bits' %d MUST be divisible by 8!\n", num_chlng_bits); exit(EXIT_FAILURE); }

   if ( has_masks == 1 && (num_POs % 8) != 0 )
      { printf("ERROR: ReadChlngAndMaskFilesBinary(): 'num_POs' %d MUST be divisible by 8!\n", num_POs); exit(EXIT_FAILURE); }

// Force realloc to behave like malloc. DO NOT DO 'masks_asc_ptr' here because we set it to NULL sometimes in the caller. Done below.
   *challenges_b_ptr = NULL;
   *masks_b_ptr = NULL;

   chlng_cnt = 0;
   *num_rise_chlngs_ptr = 0;
   while ( fgets(line, max_string_len, INFILE) != NULL )
      {

// Find the newline and eliminate it.
      if ((char_ptr = strrchr(line, '\n')) != NULL)
         *char_ptr = '\0';

// Sanity checks
      if ( strlen(line) != (unsigned int)num_chlng_bits )
         { printf("ERROR: ReadChlngAndMaskFilesBinary(): Unexpected vector size %u -- expected %d!\n", (unsigned int)strlen(line), num_chlng_bits); exit(EXIT_FAILURE); }

// Copy the ASCII version of the vector into the stack allocated space + 1 (for NULL termination).
      strcpy(challenge, line);

// Assume the challenges are constructed to be rising only or falling only. For the SRP, a rising vector includes a '1' in the LSB of the high order 16
// bits (dummy bits). Each bit refers to a row in the SRP with the LSB bit referring to the LAST row. Use that bit as the rise/fall indicator.
// Note the file ASCII version of the challenge has the high order bits of the challenge in the low order ASCII bit positions.

// LSB bit of high order 16 bit word that is referred to as the direction word. A value of '1' indicates a rising transition (THIS MAY CHANGE IN THE FUTURE).
      if ( challenge[num_direction_chlng_bits - 1] == RISE_DIR )
         rise_or_fall = 0;
      else
         rise_or_fall = 1;

// Count up the rising chlngs
      if ( rise_or_fall == 0 )
         (*num_rise_chlngs_ptr)++;

// =============================================
// Convert from ASCII to binary. Allocate space for binary unsigned char version of vectors. 
      if ( (*challenges_b_ptr = (unsigned char **)realloc(*challenges_b_ptr, sizeof(unsigned char *)*(chlng_cnt + 1))) == NULL )
         { printf("ERROR: ReadChlngAndMaskFilesBinary(): Failed to reallocate storage for challenges_b_ptr array!\n"); exit(EXIT_FAILURE); }
      if ( ((*challenges_b_ptr)[chlng_cnt] = (unsigned char *)malloc(sizeof(unsigned char)*num_chlng_bits/8)) == NULL )
         { printf("ERROR: ReadChlngAndMaskFilesBinary(): Failed to allocate storage for challenges_b_ptr string!\n"); exit(EXIT_FAILURE); }
      vec_ptr = (*challenges_b_ptr)[chlng_cnt];

// DEBUG
// printf("ReadChlngAndMaskFilesBinary(): Binary vec for chlng_cnt %d\n\t", chlng_cnt); fflush(stdout);

// Convert ASCII version of vector to binary, reversing the order as appropriate for the hardware that uses these vectors. ASCII version of vector 
// processed from 0 to 'num_chlng_bits' is high order to low order. Preserve this characteristics in the unsigned char binary byte array by starting the 
// load at the highest byte. NOTE: 'num_chlng_bits' MUST BE DIVISIBLE BY 8 otherwise this code does not work properly, which is checked above.
// Added this call 7/29/2019.
      ConvertASCIIVecMaskToBinary(num_chlng_bits, line, vec_ptr);

      chlng_cnt++;
      }
   fclose(INFILE);


// Read in the mask file if specified
   if ( has_masks == 1 )
      {
      int mask_vec_num;
      if ( (INFILE = fopen(mask_file_path, "r")) == NULL )
         { printf("ERROR: ReadChlngAndMaskFilesBinary(): Could not open Mask file %s\n", mask_file_path); exit(EXIT_FAILURE); }

// We MUST find a mask for every vector read above.
      for ( mask_vec_num = 0; mask_vec_num < chlng_cnt; mask_vec_num++ )
         {

// Sanity check
         if ( fgets(line, max_string_len, INFILE) == NULL )
            { printf("ERROR: ReadChlngAndMaskFilesBinary(): A mask file is requested but number of masks_b does NOT equal number of vectors!\n"); exit(EXIT_FAILURE); }

// Find the newline and eliminate it.
         if ((char_ptr = strrchr(line, '\n')) != NULL)
            *char_ptr = '\0';

// Allocate space for the mask (num_POs/8)
         if ( (*masks_b_ptr = (unsigned char **)realloc(*masks_b_ptr, sizeof(unsigned char *)*(mask_vec_num + 1))) == NULL )
            { printf("ERROR: ReadChlngAndMaskFilesBinary(): Failed to reallocate storage for masks_b_ptr array!\n"); exit(EXIT_FAILURE); }
         if ( ((*masks_b_ptr)[mask_vec_num] = (unsigned char *)malloc(sizeof(unsigned char)*num_POs/8)) == NULL )
            { printf("ERROR: ReadChlngAndMaskFilesBinary(): Failed to allocate storage for masks_b_ptr string!\n"); exit(EXIT_FAILURE); }

// ASCII version of mask processed from 0 to 'num_POs' is high order to low order. Preserve this characteristics in the unsigned char binary 
// byte array by starting the load at the highest byte. NOTE: 'num_POs' MUST BE DIVISIBLE BY 8 otherwise this code does not work properly,
// which is checked above. Added this call 7/29/2019.
         ConvertASCIIVecMaskToBinary(num_POs, line, (*masks_b_ptr)[mask_vec_num]);
         }
      fclose(INFILE);
      }

   return chlng_cnt;
   }


// ========================================================================================================
// ========================================================================================================
// Write vector and mask files. Format is first vector/second vector on two separate lines, followed by a blank 
// line.

void WriteVectorAndMaskFilesBinary(int max_string_len, char *vec_pair_path, int num_PIs, int num_vecs, 
   unsigned char **first_vecs_b_ptr, unsigned char **second_vecs_b_ptr, int has_masks, char *mask_file_path, 
   int num_POs, unsigned char **masks_b_ptr)
   {
   int byte_cnter, bit_cnter;
   unsigned char *vec_ptr;
   int vec_num, vec_pair;
   FILE *OUTFILE;

   if ( (OUTFILE = fopen(vec_pair_path, "w")) == NULL )
      { printf("ERROR: WriteVectorAndMaskFilesBinary(): Could not open '%s' for writing!\n", vec_pair_path); exit(EXIT_FAILURE); }

   for ( vec_num = 0; vec_num < num_vecs; vec_num++ )
      {
      for ( vec_pair = 0; vec_pair < 2; vec_pair++ )
         {

         if ( vec_pair == 0 )
            vec_ptr = first_vecs_b_ptr[vec_num];
         else
            vec_ptr = second_vecs_b_ptr[vec_num];

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
      if ( (OUTFILE = fopen(mask_file_path, "w")) == NULL )
         { printf("ERROR: WriteVectorAndMaskFilesBinary(): Could not open '%s' for writing!\n", mask_file_path); exit(EXIT_FAILURE); }

      for ( vec_num = 0; vec_num < num_vecs; vec_num++ )
         {

// Print ASCII version of bitstring in high order to low order. 
         for ( byte_cnter = num_POs/8 - 1; byte_cnter >= 0; byte_cnter-- )
            {
            for ( bit_cnter = 7; bit_cnter >= 0; bit_cnter-- )
               {

// Check binary bit for 0 or 1
               if ( (masks_b_ptr[vec_num][byte_cnter] & (unsigned char)(1 << bit_cnter)) == 0 )
                  fprintf(OUTFILE, "0");
               else
                  fprintf(OUTFILE, "1");
               }
            }
         fprintf(OUTFILE, "\n");
         }
      fclose(OUTFILE);
      }

#ifdef DEBUG
int i;
printf("ENROLL: Second Vec:\n\t");
for ( i = 0; i < num_PIs/8; i++ )
   printf("%2X ", first_vecs_b_ptr[1][i]);
printf("\n\n");
#endif

   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// Join to byte packed bitstrings. Extend bs1 bitstring, re-allocating space as needed, adding each of the bits
// from bs2 while packing the bytes as needed. It returns the new length of *bs1_ptr, which must be the
// sum of num_bits1 and num_bits2

int JoinBytePackedBitStrings(int num_bits1, unsigned char **bs1_ptr, int num_bits2, unsigned char *bs2)
   {
   int num_bytes1, new_num_bits, new_num_bytes, i, j;

// Sanity checks
   if ( bs2 == NULL )
      { printf("ERROR: JoinBytePackedBitStrings(): bs2 IS NULL!\n"); exit(EXIT_FAILURE); }

// Sanity checks
   if ( num_bits2 == 0 )
      { printf("ERROR: JoinBytePackedBitStrings(): num_bits2 is 0!\n"); exit(EXIT_FAILURE); }

// Total number of bits to be stored in the potentially larger byte packed bitstring *bs1_ptr
   new_num_bits = num_bits1 + num_bits2;

// Compute number of existing bytes in *bs1_ptr packed bitstring
   num_bytes1 = num_bits1/8;
   if ( (num_bits1 % 8) != 0 )
      num_bytes1++;

// Compute new number of bytes in *bs1_ptr packed bitstring
   new_num_bytes = new_num_bits/8;
   if ( (new_num_bits % 8) != 0 )
      new_num_bytes++;

// Sanity check
   if ( num_bytes1 > new_num_bytes )
      { printf("ERROR: JoinBytePackedBitStrings(): Original number of bytes %d is LARGER than new number of bytes %d\n", num_bytes1, new_num_bytes); exit(EXIT_FAILURE); }

// Increase the size of *bs1_ptr if needed.
   if ( num_bytes1 < new_num_bytes )
      if ( (*bs1_ptr = (unsigned char *)realloc(*bs1_ptr, sizeof(unsigned char)*new_num_bytes)) == NULL )
         { printf("ERROR: JoinBytePackedBitStrings(): Failed to realloc *bs1_ptr to new size!\n"); exit(EXIT_FAILURE); }

// Now transfer the bits from bs2 to *bs1_ptr
   for ( i = num_bits1, j = 0; i < new_num_bits; i++, j++ )
      SetBitInByte(&((*bs1_ptr)[i/8]), GetBitFromByte(bs2[j/8], j % 8), i % 8);

   return new_num_bits;
   }


// ===========================================================================================================
// ===========================================================================================================
// This routine eliminates 'bit_pos' bits from 'bs' with 'num_bits' bits total by overwriting bits starting at 
// position 0 with bits starting at position bit_pos.

int EliminatePackedBitsFromBS(int num_bits, unsigned char *bs, int bit_pos)
   {
   int i, j, new_bs_len;

// Sanity checks
   if ( bs == NULL )
      { printf("ERROR: EliminatePackedBitsFromBS(): generated bs IS NULL!\n"); exit(EXIT_FAILURE); }

// Sanity checks
   if ( num_bits == 0 )
      { printf("ERROR: EliminatePackedBitsFromBS(): num_bits is 0!\n"); exit(EXIT_FAILURE); }

// We are moving bits starting at bit_pos into positions starting at position 0 in bs. Therefore, bit_pos
// can NOT be position 0 (overwrite itself) and must be less than or equal to the last position bs
   if ( bit_pos <= 0 || bit_pos > num_bits )
      { printf("ERROR: EliminatePackedBitsFromBS(): bit_pos %d MUST be > 0 and <= %d!\n", bit_pos, num_bits); exit(EXIT_FAILURE); }

// Compute the new bitstring length. If num_bits is 10 and bit_pos is 4 (5th bit), then bits 4, 5, 6, 7, 8 and 9 will be moved
// into position 0, 1, 2, 3, 4 and 5. Then new length is 6. 
   new_bs_len = num_bits - bit_pos;

// Sanity check
   if ( new_bs_len < 0 )
      { printf("ERROR: EliminatePackedBitsFromBS(): New bitstring length is < 0!\n"); exit(EXIT_FAILURE); }

// Now move the bits. If bit_pos == num_bits, this moves NOTHING.
   for ( i = 0, j = bit_pos; j < num_bits; i++, j++ )
      SetBitInByte(&(bs[i/8]), GetBitFromByte(bs[j/8], j % 8), i % 8);

// Zero out bits above the new length of the bitstring, e.g, bits 6, 7, 8 and 9.
   for ( i = new_bs_len; i < num_bits; i++ )
      SetBitInByte(&(bs[i/8]), 0, i % 8);

   return new_bs_len; 
   }


// ===========================================================================================================
// ===========================================================================================================
// During enrollment, this routine computes XMR-based helper data (XMR_SHD) (OUTPUT) using enrollment SHD (SBG_SHD) 
// and enrollment SBS (SBG_SBS) that leverage the PUF to generate a random key via first strong bit (FSB_or_SKE == 0)
// OR encode a random key stored in Nonce (FSB_or_SKE == 1). The helper data XMR_SHD is the output of this function 
// during enrollment. 
//
// During regeneration we want to reproduce the random key, which is returned in Nonce (OUTPUT) using the rawbitstring 
// (SBG_SBS) and the XMR helper data, SBG_SHD, produced during enrollment.  Note that for regeneration, the raw 
// bitstring (SBG_SBS) is generated by setting the Threshold to 0 and calling SingleHelpBitGen() before this routine is 
// called. XMR MUST BE ODD, 1, 3, 5, etc otherwise we generate an error.
//
// Note that the hardware version overwrites SBG_SHD to create XMR_SHD (simply overwrites '1's with '0's) and SBG_SBS 
// to create the XMR_SBS (called Nonce_or_XMR_SBS here in this routine). The Nonce is stored in a separate storage location
// in hardware. It can do this because the XMR_SBS is ALWAYS smaller than the SBG_SBS so SBG_SBS bits processed can be overwritten 
// by XMR_SBS bits that occur at positions earlier in the same bitstring. 
//
// In the software version, we leave INPUTS SBG_SHD and SBG_SBS intact and optionally overwrite XMR_SHD and Nonce_or_XMR_SBS. 
// For FSB enrollment, the OUTPUT is XMR_SHD and Nonce_or_XMR_SBS. For FSB regeneration, the OUTPUT is ONLY Nonce_or_XMR_SBS.
//    Note for regeneration, the enrollment XMR_SHD is passed in as SBG_SHD and the SBG_SBS is the raw bitstring (no thresholding).
//
// For SKE enrollment, the OUTPUT is XMR_SHD, using an additional INPUT Nonce_or_XMR_SBS. For SKE regeneration, the OUTPUT 
// is ONLY Nonce_or_XMR_SBS.
//    Note for regeneration, the enrollment XMR_SHD is passed in as SBG_SHD and the SBG_SBS is the raw bitstring (no thresholding).
//    (EXACTLY THE SAME AS FSB regeneration).
//
// NOTE that XMR_SHD is NEEDED ONLY during enrollment, i.e., it is not referenced and is sometimes NULL during regeneration.
// in fact is the OUTPUT during FSB and SKE enrollment) and Nonce_or_XMR_SBS, which is also an OUTPUT during FSB enrollment
// and an INPUT during SKE enrollment, and is the OUTPUT during FSB or SKE regeneration.
//
// Note that num_minority_bit_flips and true_minority_bit_flips are computed during regen ONLY (of course). minority_bit_flips
// is ALWAYS computed but may be incorrect. It always adds to the minority_bit_flips the smaller of the number of disagreeing
// bits in the majority vote. true_minority_bit_flips are computed ONLY when KEK_enroll_key is NON-NULL and accounts for cases
// where the number of disagreeing bits in the majority vote is actually the majority. It does this by comparing the regenerated
// response bit with the actual bit in the KEK_enroll_key (if NON-NULL) and adds in the complement of the minority_bit_flips
// if the super-strong response bits disagree.

int KEK_FSB_SKE(int max_bits, int XMR, unsigned char *SBG_SHD, unsigned char *SBG_SBS, unsigned char *XMR_SHD, 
   int num_nonce_bits, unsigned char *Nonce_or_XMR_SBS, int enroll_or_regen, int do_minority_bit_flip_analysis, 
   int *num_minority_bit_flips_ptr, int start_actual_bit_pos, unsigned char *KEK_enroll_key, 
   int *true_minority_bit_flips_ptr, int FSB_or_SKE, int chip_num, int TV_num)
   {
   int num_zeros, num_ones, XMR_copy_cnt, tot_wasted_strong, tot_strong, SBS_tracker;
   int bit_num_of_last_fully_encoded_bit; 
   int strong_bit, bit_to_match;
   int bit_num, strong_bit_num;

#ifdef DEBUG
printf("Running KEK_FSB_SKE(): chip %d\tTV_num %d\n", chip_num, TV_num); fflush(stdout);
#endif

// Sanity check
   if ( (XMR % 2) == 0 )
      { printf("ERROR: KEK_FSB_SKE(): XMR MUST BE odd %d\n", XMR); exit(EXIT_FAILURE); }

// For statistics. Just count the number of times we find a strong bit of the 'wrong' value. This number should equal
// 2 times the size of the strong bitstring with 3MR, 4 times for 5MR, etc.
   tot_wasted_strong = 0;
   tot_strong = 0;

// Initialize regen and XMR variables.
   num_zeros = 0;
   num_ones = 0;
   XMR_copy_cnt = 0;
   bit_to_match = -1;
   strong_bit_num = 0;
   SBS_tracker = 0;
   bit_num_of_last_fully_encoded_bit = -1;
   for ( bit_num = 0; bit_num < max_bits; bit_num++ )
      {

// During enrollment, copy the Enrollment helper data (SBG_SHD) into the XMR_SHD. We will update this by writing potentially
// overwriting a '1' that may be stored here to the XMR_SHD below when it is necessary to eliminate strong bits that do NOT match 
// the bit that is searched for. 
      if ( enroll_or_regen == 0 )
         SetBitInByte(&(XMR_SHD[bit_num/8]), GetBitFromByte(SBG_SHD[bit_num/8], bit_num % 8), bit_num % 8);

// During enrollment, skip weak bits. During regeneration, skip all bits not deemed strong during the earlier call to this routine
// for enrollment.
      if ( GetBitFromByte(SBG_SHD[bit_num/8], bit_num % 8) == 0 )
         {

// For enrollment, we ONLY have the strong bits in the BS (it is the SBS). For regeneration, BS has ALL 2048 bits (weak and strong).
// Increment SBS_tracker after EVERY helper data bit processed during regeneration.
         if ( enroll_or_regen == 1 )
            SBS_tracker++; 
         continue;
         }

// Get the strong bit. NOTE: We INCREMENT SBS_tracker here (as we do in the VHDL state machine) since we do NOT reference SBG_SBS
// again below (we've stored the strong_bit here).
      strong_bit = GetBitFromByte(SBG_SBS[SBS_tracker/8], SBS_tracker % 8);
      SBS_tracker++; 
      tot_strong++; 

#ifdef DEBUG
printf("\t\tKEK_FSB_SKE(): Strong bit %d found at index %d\n", strong_bit, bit_num); fflush(stdout);
#endif

// If 'enrollment' then we are generating the XMR_SHD that will be used during regeneration, so update the XMR_SHD.
      if ( enroll_or_regen == 0 )
         {
         if ( XMR_copy_cnt == 0 )
            {
            if ( FSB_or_SKE == 0 )
               bit_to_match = strong_bit;
            else
               {
               bit_to_match = GetBitFromByte(Nonce_or_XMR_SBS[strong_bit_num/8], strong_bit_num % 8);
        
#ifdef DEBUG
printf("KEK_FSB_SKE(): NEW bit at pos %d to encode => %d!\n", strong_bit_num, bit_to_match); fflush(stdout);
#endif
               }
            }

// Compare the current 'strong' bit with the value bit_to_match. If it doesn't match, zero the helper data and skip this strong bit.
         if ( bit_to_match != strong_bit )
            { 
            SetBitInByte(&(XMR_SHD[bit_num/8]), 0, bit_num % 8);

#ifdef DEBUG
printf("\t\tKEK_FSB_SKE(): Eliminating strong bit at %d because of MISMATCH %d\n", bit_num, strong_bit); fflush(stdout);
#endif
            tot_wasted_strong++; 
            continue; 
            }
         }

// Keep track of how many of each bit value we have across the XMR of the bit. 
      if ( strong_bit == 0 )
         num_zeros++;
      else
         num_ones++;

// Force a valid bit to be stored when we are processing the last copy. Note this is also the first copy when XMR is 1
      if ( XMR_copy_cnt == XMR - 1 )
         {

// Assign the secret bit to be that of the majority. For enroll, they are all guaranteed to be the same. NOTE: equality is 
// NOT possible because XMR is ALWAYS odd. 
         if ( num_zeros > num_ones ) 
            { strong_bit = 0; }
         else
            { strong_bit = 1; }

#ifdef DEBUG
printf("\t\tKEK_FSB_SKE(): Enroll/Regen? %d\tFINAL: XMR_copy_cnt %d: Num zeros %d\tNum ones %d\tsecret bit %d\tbit index %d\n", 
   enroll_or_regen, XMR_copy_cnt, num_zeros, num_ones, strong_bit, bit_num); fflush(stdout);
#endif

// Store the strong_bit in the strong bit (FSB mode) or regenerated Nonce bit (SKE mode) during regeneration (we ALSO do this now for 
// SKE enrollment which SIMPLY re-writes the nonce bit with the SAME value). The SBS or Nonce bitstring IS the OUTPUT of KEK_FSB_SKE 
// during regeneration. 
         SetBitInByte(&(Nonce_or_XMR_SBS[strong_bit_num/8]), strong_bit, strong_bit_num % 8);

// If BOTH of num_zeros or num_ones are NON-zero, then there was disagreement and majority vote decided the output. Add the number of bits in the losing majority
// to the counter. This may be important in helping with selecting the correct chip in a database search by the server for this authentication function.
// If XMR is 5, and num_zeros is 2 and num_ones is 3, then num_ones is selected in the majority vote. We want to add 2 to the counter here in this case.
// Note: If num_zeros is 0 and num_ones is 5, then this adds nothing. Also note that num_zeros NEVER equals num_ones.
         if ( enroll_or_regen == 1 && do_minority_bit_flip_analysis == 1 )
            {
            int cur_num_minority_bit_flips;
            if ( num_zeros < num_ones )
               cur_num_minority_bit_flips = XMR - num_ones;
            else
               cur_num_minority_bit_flips = XMR - num_zeros; 
            *num_minority_bit_flips_ptr += cur_num_minority_bit_flips;

// Need to know how many mismatches are actually occurring for privacy-preserving authentication. The metric above on num_minority_bit_flips does not
// work. If I have a majority vote mismatch with the actual nonce bit, it may happen that all 5 bits in 5MR agree with the majority vote. In this case,
// num_minority_bit_flips will be 0 and should be 5!

// If the actual bit agrees with the super strong XMR bit just encoded, then add in current number of minority bit flips to the true_minority_bit_flips_ptr. NOTE: We
// may NOT have the enrollment key in some cases so don't do this if that parameter is NULL.
            if ( KEK_enroll_key != NULL )
               {
               if ( strong_bit == GetBitFromByte(KEK_enroll_key[(start_actual_bit_pos + strong_bit_num)/8], (start_actual_bit_pos + strong_bit_num) % 8) )
                  {
                  *true_minority_bit_flips_ptr += cur_num_minority_bit_flips;
#ifdef DEBUG
if ( cur_num_minority_bit_flips != 0 )
   printf("\t\tStrong bit num %d has minority bit flip error with %d bit flips!\n", start_actual_bit_pos + strong_bit_num, cur_num_minority_bit_flips); 
fflush(stdout);
#endif
                  }

// Otherwise, add in the complement of the current number of minority bit flips
               else 
                  {
                  *true_minority_bit_flips_ptr += (XMR - cur_num_minority_bit_flips);
#ifdef DEBUG
if ( XMR - cur_num_minority_bit_flips != 0 )
   printf("\t\tKEY BIT FLIP ERROR at position %d\tCorrect strong bit count is minority %d!\n", start_actual_bit_pos + strong_bit_num, XMR - cur_num_minority_bit_flips); 
fflush(stdout);
#endif
                  }
               }
            }

// Increment the strong_bit_num index here -- NOTE: THIS IS AN INDEX, NOT a count of the number of strong bits encoded. During enrollment, this will select the 
// next bit to encode (if there are any remaining). During regeneration, this allows us to exit once we've reproduced the number of required bits.
         strong_bit_num++;

// Store the bit_num index into the XMR_SHD that is associated with the last fully encoded 'bit_to_match' so we can eliminate '1' in XMR_SHD that partially
// encode the next bit but fail to fully encode it because we ran out -- note we have just completed a full encoding of a bit here and bit_num points to the last
// XMR_SHD bit that is set to 1 to enable this. So below, WE MUST INCREMENT ONE BEYOND THIS bit_num index and THEN start zeroing out. ONLY USED DURING ENROLLMENT.
         bit_num_of_last_fully_encoded_bit = bit_num;

// SKE mode ONLY: Increment to the next bit to encode if doing enrollment. 
         if ( FSB_or_SKE == 1 && strong_bit_num == num_nonce_bits )
            {
#ifdef DEBUG
printf("KEK_FSB_SKE(): BREAKING bit_num %d LOOP EARLY because we ran out of bits to encode!\n", bit_num); fflush(stdout);
#endif
            break;
            }

// Reset counters.
         num_zeros = 0;
         num_ones = 0;
         XMR_copy_cnt = 0;
         }

// Indicate a redundant bit has been generated and increment the number of redundant copies.
      else
         {

#ifdef DEBUG
printf("\t\tKEK_FSB_SKE(): REDUNDANT: XMR_copy_cnt %d: Num zeros %d\tNum ones %d\n", XMR_copy_cnt, num_zeros, num_ones); fflush(stdout);
#endif

         XMR_copy_cnt++;
         }
      }

// Sanity check: Assuming we ALWAYS succeed in encoding at least a couple bits, this should NEVER BE -1.
   if ( bit_num_of_last_fully_encoded_bit == -1 )
      { printf("WARNING: KEK_FSB_SKE(): 'bit_num_of_last_fully_encoded_bit' is -1!\n"); }

// SKE mode ONLY: If we exit the loop early, reset XMR_SHD bits that are 1 for the last partial encoding of the last strong bit. This will ensure 
// that the XMR_SHD has EXACTLY 'num_nonce_bits'*XMR_val helper data bits set to 1. This is NOT needed but I think if XMR helper data leaks anything, 
// then this will reduce the leakage even further. Remember, the XMR_SHD IS the 'product' of this routine during enrollment, while the 
// Nonce_or_XMR_SBS is the 'product' during regeneration -- in fact, XMR_SHD is NULL during regeneration. 
   if ( enroll_or_regen == 0 )
      for ( bit_num = bit_num_of_last_fully_encoded_bit + 1; bit_num < max_bits; bit_num++ )
         SetBitInByte(&(XMR_SHD[bit_num/8]), 0, bit_num % 8);

// Sanity check
   if ( enroll_or_regen == 1 && tot_wasted_strong != 0 )
      { printf("ERROR: KEK_FSB_SKE(): Regeneration requires tot_wasted_strong %d MUST BE 0!\n", tot_wasted_strong); exit(EXIT_FAILURE); }

// Sanity check
   if ( enroll_or_regen == 1 && tot_strong/XMR != strong_bit_num )
      { 
      printf("ERROR: KEK_FSB_SKE(): Regeneration requires total number of strong bits %d MUST BE equal to ALL strong bits div XMR %d!\n", 
         strong_bit_num, tot_strong/XMR); exit(EXIT_FAILURE); 
      }

#ifdef DEBUG
if ( XMR != 1 )
   printf("\tKEK_FSB_SKE(): Chip %d\tTV %d\tXMR %d\tTotal strong bits %d\tTot mismatches multiple of size ? %d\tMismatch normalized %.1f\tFinal size of strong bitstring %d\n", 
      chip_num, TV_num, XMR, tot_strong, tot_wasted_strong, tot_wasted_strong/(float)(XMR-1), strong_bit_num); 
printf("KEK_FSB_SKE(): returning!\n"); fflush(stdout);
#endif

// Return the number of SBS bits that were successfully encoded in the helper data. NOTE: strong_bit_num is a zero-based INDEX, NOT A COUNT. So if, for example,
// NO strong bits were encoded and we were working on the first bit_to_match bit during enrollment, this remains at zero, if one strong bit is encoded AND we are working 
// on the second bit_to_match, then this is 1 which means we successfully encoded the first one and am working on the second one but it was NOT FULLY ENCODED so
// we return 1 here. In the caller, this return value is used as the number of strong bits encoded. For the last bit, strong_bit_num is set to num_nonce_bits
// and we exit above, so the index is ONE BEYOND the end of the array.
   return strong_bit_num;
   }
