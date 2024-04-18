/********************************
 *      Homework 2 (part 2)        *
 ********************************
 *Description:      1. Read the message form "GMPMessage.txt" file
 *                  2. Read the seed from "GMPSeed.txt" file
 *                  3. Generate 2*n 256-bit random numbers based on the shared seed by ChaCha20 PRNG  (PRN length need to be 2*n*32)
 *                  4. You could hard-code the prime (q), and split the message and PRN into different unisgned char arrays or use pointers
 *                  5. Declare and Initialize your mpz_t variables 
 *                  6. Convert every 32 Bytes of your unsigned char data to Hex via "Convert_to_Hex()" function
 *                  7. Use the "mpz_set_str()" with base 16 to cnvert the Hex array into mpz_t
 *                  8. Compute the LC-UMAC based on these random numbers (messssage_i, a_i, b_i, q):
 *                       \sigma = \sum_{i}^{n} (a_i * m_i + b_i) mod q 
 *                  9. Convert the LC-UMAC to Hex and write it in a file "GMPLCUMAC.txt"
 * 
 *
 *Compile:          gcc LCUMACgmp.c -lgmp -ltomcrypt -o LCUMACgmp
 * 
 *Run:              ./LCUMACgmp GMPMessage1.txt GMPSeed.txt
 *
 *Documentation:    Use the provided functions for in-class exercise and GMP Demo guide
 *
 * Created By:      << Saleh Darzi >>
_______________________________________________________________________________*/

//Header Files
#include <stdlib.h>
#include <stdio.h>
#include <tomcrypt.h>
#include <gmp.h>


//Function prototypes
unsigned char* Read_File (char fileName[], int *fileLen);
unsigned char* PRNG(unsigned char *seed, unsigned long seedlen, unsigned long prnlen);
void Write_File(char fileName[], char input[]);
void Convert_to_Hex (char output[], unsigned char input[], int inputlength);
void Show_in_Hex (char name[], unsigned char hex[], int hexlen);

/*************************************************************
						M A I N
**************************************************************/
int main (int argc, char* argv[])
{   
    mpz_t q;
    mpz_init_set_str(q, "115792089237316195423570985008687907853269984665640564039457584007913129639747", 10);

    //Reading the message
    int messageLength;
    unsigned char* message = Read_File(argv[1], &messageLength);

    //Reading the seed
    int seedLength;
    unsigned char* seed = Read_File(argv[2], &seedLength);

    //Generating prn
    int n = messageLength/32;
    unsigned char* prn = PRNG(seed, seedLength, 2*n*32);

    //Declaring and initializing mpz_t variables
    mpz_t gmpM;
    mpz_init(gmpM);
    mpz_t gmpA;
    mpz_init(gmpA);
    mpz_t gmpB;
    mpz_init(gmpB);
    mpz_t sigma;
    mpz_init(sigma);
    mpz_t product;
    mpz_init(product);
    mpz_t sum;
    mpz_init(sum);
    mpz_t mod;
    mpz_init(mod);

    //Computing LC-UMAC
    for(int i = 0, j = 0, k = 1; i < n; i++, j+=2, k+=2){
        //Converting variables to hex
        char* hexM = malloc(64);
        char* hexA = malloc(64);
        char* hexB = malloc(64);
        unsigned char temp[32];

        memcpy(temp, message+(32*i), 32);
        Convert_to_Hex(hexM, temp, 32);
        mpz_set_str(gmpM, hexM, 16);

        memcpy(temp, prn+(32*j), 32);
        Convert_to_Hex(hexA, temp, 32);
        mpz_set_str(gmpA, hexA, 16);

        memcpy(temp, prn+(32*k), 32);
        Convert_to_Hex(hexB, temp, 32);
        mpz_set_str(gmpB, hexB, 16);
        
        //Performing gmp arithmetic
        mpz_mul(product, gmpA, gmpM);
        mpz_add(sum, product, gmpB);
        mpz_mod(mod, sum, q);

        mpz_add(sigma, sigma, mod);
        mpz_mod(sigma, sigma, q);

        free(hexM);
        free(hexA);
        free(hexB);
    }

    //Writing to file
    char* LCUMAC = malloc(79);
    mpz_get_str(LCUMAC, 16, sigma);
    Write_File("GMPLCUMAC.txt", LCUMAC);
    free(LCUMAC);

    mpz_clear(q);
    mpz_clear(gmpM);
    mpz_clear(gmpA);
    mpz_clear(gmpB);
    mpz_clear(sigma);
    mpz_clear(product);
    mpz_clear(sum);
    mpz_clear(mod);
    return 0;
}

/*************************************************************
					F u n c t i o n s
**************************************************************/

//Function: int mpz_set_str (mpz_t rop, const char *str, int base)

    //Set the value of rop from str, a null-terminated C string in base base. White space is allowed in the string, and is simply ignored.

    //The base may vary from 2 to 62, or if base is 0, then the leading characters are used: 0x and 0X for hexadecimal, 0b and 0B for binary, 0 for octal, or decimal otherwise.

    //For bases up to 36, case is ignored; upper-case and lower-case letters have the same value. For bases 37 to 62, upper-case letter represent the usual 10..35 while lower-case letter represent 36..61.

    //This function returns 0 if the entire string is a valid number in base base. Otherwise it returns -1. 



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
void Write_File(char fileName[], char input[]){
  FILE *pFile;
  pFile = fopen(fileName,"w");
  if (pFile == NULL){
    printf("Error opening file. \n");
    exit(0);
  }
  fputs(input, pFile);
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

/*============================
        Convert to Hex 
==============================*/
void Convert_to_Hex (char output[], unsigned char input[], int inputlength)
{
    for (int i = 0 ; i < inputlength ; i++) {
        sprintf(&output[2*i], "%02x", input[i]);
    }
    //printf("Hex format: %s\n", output);
}

/*============================
        Showing in Hex 
==============================*/
void Show_in_Hex (char name[], unsigned char hex[], int hexlen)
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
