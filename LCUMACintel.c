/********************************
 *      Homework 2 (part 1)        *
 ********************************
 *Description:      1. Read the message form "IntelMessage.txt" file
 *                  2. Read the seed from "IntelSeed.txt" file
 *                  3. Generate 2*n 64-bit random numbers based on the shared seed by ChaCha20 PRNG  (PRN length need to be 2*n*8)
 *                  4. You could hard-code the prime (q), and split the message and PRN into different unisgned char arrays or use pointers
 *                  5. Convert every 8 Bytes of q, message, and PRN into one __uint128_t
 *                  6. Compute the LC-UMAC based on these random numbers (messssage_i, a_i, b_i, q):
 *                     \sigma = \sum_{i}^{n} (a_i * m_i + b_i) mod q 
 *                  7. Write the LC-UMAC to a file "IntelLCUMAC.txt"
 * 
 *
 *Compile:          gcc LCUMACintel.c -ltomcrypt -o LCUMACintel
 * 
 *Run:              ./LCUMACintel IntelMessage1.txt IntelSeed.txt
 *
 *Documentation:    Use the provided functions for in-class exercise and Intel Demo guide
 *
 * Created By:      << Saleh Darzi >>
_______________________________________________________________________________*/

//Header Files
#include <stdlib.h>
#include <stdio.h>
#include <tomcrypt.h>

//Function prototypes
unsigned char* Read_File (char fileName[], int *fileLen);
unsigned char* PRNG(unsigned char *seed, unsigned long seedlen, unsigned long prnlen);
void Write_File(char fileName[], __uint128_t x);
__uint128_t Convert_to_128(unsigned char a[]);
void Show_in_Hex(char name[], unsigned char hex[], int hexlen);

/*************************************************************
						M A I N
**************************************************************/
int main (int argc, char* argv[])
{   
    unsigned char q[8] = "92233720";

    //Reading the message
    int messageLength;
    unsigned char* message;
    message = Read_File(argv[1], &messageLength);

    //Reading the seed
    int seedLength;
    unsigned char* seed;
    seed = Read_File(argv[2], &seedLength);

    //Generating PRN
    int n = messageLength/8;
    unsigned char* prn;
    prn = PRNG(seed, seedLength, 2*n*8);

    //Spliting all variables into 8 byte chunks and converting to __uint128_t
    __uint128_t intelMessage[n];
    for(int i = 0; i < n; i++){
        unsigned char tempMessage[8];
		memcpy(tempMessage, message+(8*i), 8);
        intelMessage[i] = Convert_to_128(tempMessage);
    }

    __uint128_t intelQ = Convert_to_128(q);

    __uint128_t a[n];
    __uint128_t b[n];
    for(int i = 0; i < 2*n; i++){
        unsigned char temp[8];
		memcpy(temp, prn+(8*i), 8);
        if(i % 2 == 0){
            a[i/2] = Convert_to_128(temp);
        } 
        else{
            b[i/2] = Convert_to_128(temp);
        }
    }

    //Computing the LC-UMAC
    __uint128_t temp[n];
    __uint128_t sigma = 0;
    for(int i = 0; i < n; i++){
        temp[i] = ((a[i] * intelMessage[i]) + b[i]) % intelQ;
        sigma += temp[i];
        sigma %= intelQ;
    }

    //Writing to file
    Write_File("IntelLCUMAC.txt", sigma);

	return 0;
}

/*************************************************************
					F u n c t i o n s
**************************************************************/

/*============================
        Read from File
==============================*/
unsigned char* Read_File (char fileName[], int *fileLen)
{
    FILE *pFile;
	pFile = fopen(fileName, "r");
	if (pFile == NULL)
	{
		printf("Error opening file.\n");
		exit(0);
	}
    fseek(pFile, 0L, SEEK_END);
    int temp_size = ftell(pFile)+1;
    fseek(pFile, 0L, SEEK_SET);
    unsigned char *output = (unsigned char*) malloc(temp_size);
	fgets(output, temp_size, pFile);
	fclose(pFile);

    *fileLen = temp_size-1;
	return output;
}

/*============================
        Write to File
==============================*/
void Write_File(char fileName[], __uint128_t x){
  FILE *pFile;
  pFile = fopen(fileName,"w");
  if (pFile == NULL){
    printf("Error opening file. \n");
    exit(0);
  }
  char temp[64];
  snprintf(temp, sizeof(temp), "%llu", (unsigned long long)x);
  fprintf(pFile, "%s\n", temp);
  fclose(pFile);
}

/*============================
        PRNG Fucntion 
==============================*/
unsigned char* PRNG(unsigned char *seed, unsigned long seedlen, unsigned long prnlen)
{
	int err;
    unsigned char *pseudoRandomNumber = (unsigned char*) malloc(prnlen);

	prng_state prng;                                                                     //LibTomCrypt structure for PRNG
    if ((err = chacha20_prng_start(&prng)) != CRYPT_OK){                                //Sets up the PRNG state without a seed
        printf("Start error: %s\n", error_to_string(err));
    }					                
	if ((err = chacha20_prng_add_entropy(seed, seedlen, &prng)) != CRYPT_OK) {           //Uses a seed to add entropy to the PRNG
        printf("Add_entropy error: %s\n", error_to_string(err));
    }	            
    if ((err = chacha20_prng_ready(&prng)) != CRYPT_OK) {                                   //Puts the entropy into action
        printf("Ready error: %s\n", error_to_string(err));
    }
    chacha20_prng_read(pseudoRandomNumber, prnlen, &prng);                                //Writes the result into pseudoRandomNumber[]

    if ((err = chacha20_prng_done(&prng)) != CRYPT_OK) {                                   //Finishes the PRNG state
        printf("Done error: %s\n", error_to_string(err));
    }

    return (unsigned char*)pseudoRandomNumber;
}

/*====================================
        Converting to __uint128_t
======================================*/
__uint128_t Convert_to_128(unsigned char a[])
{
    __uint128_t temp = a[0];
    for(int i=1; i<8; i++){
        temp = temp << (i*8) | a[i];
    }
    //printf("output __uint128_t is: %ld\n", temp);
    return temp;
}

void Show_in_Hex(char name[], unsigned char hex[], int hexlen)
{
	printf("%s: ", name);
	for (int i = 0 ; i < hexlen ; i++)
   		printf("%02x", hex[i]);
	printf("\n");
}

/*===================
        LC-UMAC 
=====================*/
//You could create a function to perfrom LC-UMAC
// Or just do the computations in Main()
// As you Wish
//__________________________________________________________________________________________________________________________
