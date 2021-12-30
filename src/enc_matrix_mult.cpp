#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "palisade.h"
#include "debug.h"
using namespace std;
using namespace lbcrypto;

using CT = Ciphertext<DCRTPoly>; // ciphertext
using PT = Plaintext;			 // plaintext
using vecInt = vector<int64_t>;	 // vector of ints
using vecCT = vector<CT>;		 // vector of ciphertexts

void get_input_from_file(string fname, int64_t matrix[][10], int64_t &row_count, int64_t &col_count);
void multiplyMatrices(int64_t firstMatrix[][10], int64_t secondMatrix[][10], int64_t multResult[][10], int64_t rowFirst, int64_t columnFirst, int64_t rowSecond, int64_t columnSecond);
void display(int64_t mult[][10], int64_t rowFirst, int64_t columnSecond);

void encryptMatrix(CryptoContext<DCRTPoly> &cc, LPPublicKey<DCRTPoly> &pk, int64_t clearMatrix[][10], CT encMatrix[][10], int64_t num_rows, int64_t num_cols);
void decryptMatrix(CryptoContext<DCRTPoly> &cc, LPPrivateKey<DCRTPoly> &privatekey, int64_t clearMatrix[][10], CT encMatrix[][10], int64_t num_rows, int64_t num_cols);
void encMultiplyMatrices(CryptoContext<DCRTPoly> &cc, LPPublicKey<DCRTPoly> &pk, CT encFirstMatrix[][10], CT encSecondMatrix[][10], CT encResult[][10], int64_t rowFirst, int64_t columnFirst, int64_t rowSecond, int64_t columnSecond);

int main()
{
	int64_t firstMatrix[10][10], secondMatrix[10][10], mult[10][10], rowFirst, columnFirst, rowSecond, columnSecond;
	string file1 = "data/matrix1.txt";
	string file2 = "data/matrix2.txt";
	get_input_from_file(file1, firstMatrix, rowFirst, columnFirst);
	cout << "Matrix 1:" << endl;
	display(firstMatrix, rowFirst, columnFirst);

	get_input_from_file(file2, secondMatrix, rowSecond, columnSecond);
	cout << "Matrix 2:" << endl;
	display(secondMatrix, rowSecond, columnSecond);

	if (columnFirst != rowSecond)
	{
		cout << "Error! column of first matrix not equal to row of second." << endl;
		exit(0);
	}

	TIC(auto t1);
	multiplyMatrices(firstMatrix, secondMatrix, mult, rowFirst, columnFirst, rowSecond, columnSecond);
	auto plain_time_ms = TOC_MS(t1);
	cout << "Plaintext execution time " << plain_time_ms << " mSec." << endl;

	cout << "Regular Result:" << endl;
	display(mult, rowFirst, columnSecond);

	uint32_t plaintextModulus = 786433; // plaintext prime modulus

	// now lets do that shit but use FHE
	cout << "Step 1- setting up BFV RNS crypto system" << endl;
	uint32_t multDepth = 32;

	double sigma = 3.2;
	SecurityLevel securityLevel = HEStd_128_classic;

	// Instantiate the crypto context
	CryptoContext<DCRTPoly> cc =
		CryptoContextFactory<DCRTPoly>::genCryptoContextBFVrns(
			plaintextModulus, securityLevel, sigma, 0, multDepth, 0, OPTIMIZED);

	// Enable features that you wish to use
	// not sure what SHE is, but copypasting it from integer example
	cc->Enable(ENCRYPTION);
	cc->Enable(SHE);

	cout << "Step 2 - Key Generation" << endl;
	// Initialize Public Key Containers
	// Generate a public/private key pair
	auto keyPair = cc->KeyGen();

	// Generate the relinearization key
	cc->EvalMultKeyGen(keyPair.secretKey);
	cout << "This is your public key " << keyPair.publicKey << endl;

	cout << "Step 3 - Encrypting the matrices" << endl;
	cout << "First matrix..." << endl;
	CT encFirstMatrix[10][10], encSecondMatrix[10][10];
	encryptMatrix(cc, keyPair.publicKey, firstMatrix, encFirstMatrix, rowFirst, columnFirst);
	cout << "Second matrix..." << endl;
	encryptMatrix(cc, keyPair.publicKey, secondMatrix, encSecondMatrix, rowSecond, columnSecond);

	cout << "Step 4 - Multiply the encrypted matrices" << endl;
	CT encResultMatrix[10][10];
	TIC(auto t2);
	encMultiplyMatrices(cc, keyPair.publicKey, encFirstMatrix, encSecondMatrix, encResultMatrix, rowFirst, columnFirst, rowSecond, columnSecond);
	auto cipher_ms = TOC_MS(t2);
	cout << "Encrpyted execution time " << cipher_ms << " mSec." << endl;

	cout << "Step 5 - Decrypt" << endl;
	int64_t decodedResult[10][10];
	decryptMatrix(cc, keyPair.secretKey, decodedResult, encResultMatrix, rowFirst, columnSecond);
	display(decodedResult, rowFirst, columnSecond);

	return 0;
}

void get_input_from_file(string fname, int64_t matrix[][10], int64_t &row_count, int64_t &col_count)
{
	ifstream in_file;
	in_file.open(fname);
	if (!in_file)
	{
		cerr << "Can't open file for input: " << fname;
		exit(-1); // error exit
	}
	string line;

	// first line is rowcount
	// then col count
	// then each element

	int line_number = 0;
	while (getline(in_file, line))
	{
		stringstream iss(line);
		if (line_number == 0)
		{
			iss >> row_count;
		}
		if (line_number == 1)
		{
			iss >> col_count;
		}
		if ((line_number > 1) && (line_number <= row_count + 1))
		{
			for (int i = 0; i < col_count; i++)
			{
				iss >> matrix[line_number - 2][i];
			}
		}
		if (line_number > row_count + 1)
		{
			cout << "too many lines" << endl;
		}
		line_number++;
	}

	in_file.close();
	return;
}

void encryptMatrix(CryptoContext<DCRTPoly> &cc, LPPublicKey<DCRTPoly> &pk, int64_t clearMatrix[][10], CT encMatrix[][10], int64_t num_rows, int64_t num_cols)
{
	for (int64_t i = 0; i < num_rows; i++)
	{
		for (int64_t j = 0; j < num_cols; j++)
		{
			// just make each element of matrix into its own cipher text
			// maybe theres a smarter way to do this, but im not very smart
			vector<int64_t> val = {clearMatrix[i][j]};
			Plaintext pt = cc->MakePackedPlaintext(val);
			CT ct = cc->Encrypt(pk, pt);

			encMatrix[i][j] = ct;
		}
	}
}

void decryptMatrix(CryptoContext<DCRTPoly> &cc, LPPrivateKey<DCRTPoly> &privatekey, int64_t clearMatrix[][10], CT encMatrix[][10], int64_t num_rows, int64_t num_cols)
{
	for (int64_t i = 0; i < num_rows; i++)
	{
		for (int64_t j = 0; j < num_cols; j++)
		{
			Plaintext pt;
			cc->Decrypt(privatekey, encMatrix[i][j], &pt);
			clearMatrix[i][j] = pt->GetPackedValue()[0];
		}
	}
}

void multiplyMatrices(int64_t firstMatrix[][10], int64_t secondMatrix[][10], int64_t mult[][10], int64_t rowFirst, int64_t columnFirst, int64_t rowSecond, int64_t columnSecond)
{
	int64_t i, j, k;

	// Initializing elements of matrix mult to 0.
	for (i = 0; i < rowFirst; ++i)
	{
		for (j = 0; j < columnSecond; ++j)
		{
			mult[i][j] = 0;
		}
	}

	// Multiplying matrix firstMatrix and secondMatrix and storing in array mult.
	for (i = 0; i < rowFirst; ++i)
	{
		for (j = 0; j < columnSecond; ++j)
		{
			for (k = 0; k < columnFirst; ++k)
			{
				mult[i][j] += firstMatrix[i][k] * secondMatrix[k][j];
			}
		}
	}
}

void encMultiplyMatrices(CryptoContext<DCRTPoly> &cc, LPPublicKey<DCRTPoly> &pk, CT encFirstMatrix[][10], CT encSecondMatrix[][10], CT encResult[][10], int64_t rowFirst, int64_t columnFirst, int64_t rowSecond, int64_t columnSecond)
{
	int64_t i, j, k;

	// Initializing elements of matrix mult to encrypt(0)
	// this requires knowledge of public key
	for (i = 0; i < rowFirst; ++i)
	{
		for (j = 0; j < columnSecond; ++j)
		{
			vector<int64_t> val = {0};
			Plaintext pt = cc->MakePackedPlaintext(val);
			CT ct = cc->Encrypt(pk, pt);
			encResult[i][j] = ct;
		}
	}

	for (i = 0; i < rowFirst; ++i)
	{
		for (j = 0; j < columnSecond; ++j)
		{
			for (k = 0; k < columnFirst; ++k)
			{
				encResult[i][j] = cc->EvalAdd(encResult[i][j],
											  cc->EvalMult(encFirstMatrix[i][k], encSecondMatrix[k][j]));
			}
		}
	}
}

void display(int64_t mult[][10], int64_t rowFirst, int64_t columnSecond)
{
	int64_t i, j;

	for (i = 0; i < rowFirst; ++i)
	{
		for (j = 0; j < columnSecond; ++j)
		{
			cout << mult[i][j] << " ";
			if (j == columnSecond - 1)
				cout << endl
					 << endl;
		}
	}
}
