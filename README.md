## Homormorphic Matrix Multiplication
Uses the [PALISADE](https://gitlab.com/palisade/palisade-release) library for FHE, and is loosely based on the [PALISADE Integer Examples]()https://gitlab.com/palisade/palisade-integer-examples). The example provided within `/src/enc_matric_mult.cpp` multiplies two *encrypted* integer matrices of different shapes (3x3 and 3x2). This program includes functions for matrix multiplication (and it's FHE analog), as well as the functions necessary to encrypt/decrypt entire matrices.
For more context on the goals/inspiration of this, see [MORE_INFO.md](MORE_INFO.md).

### Limitations
* Only integer matrices can be used. Should be capable of expanding to floats once PALISADE supports it. Probably need smart math people to help.
* Some data is leaked about the matrices (the number of rows and cols is not encrypted because we need to loop over each element).
* Each element of the matrices is encrypted seperately. This means that if 2 elements are the same value, they will also have the same encrypted value. This shouldn't be a problem (if it is, you have a much bigger cryptography problem), but should be noted.

### Building & Running
Follow the build instructions listed on the integer example repo [here](https://gitlab.com/palisade/palisade-integer-examples/-/blob/master/README.md#build-instructions-for-ubuntu)
To run, start in the build directory and do `bin/enc_matrix_mult`. First it'll run a cleartext matrix multiplication, which should run very quickly. After that, it will setup the crypto system, encrypt the matrices, run the encrypted matrix multiply operation, then decrypt the result. If I did my job right, both runs should produce the same results.
