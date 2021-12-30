# FHE Integer Matrix Multiplication
The goal is to make a very simple matrix multiplication function that multiplies integer matrices, where the matrices involved are fully homomorphic encrypted (FHE).


## Inspiration
I want to be able to use cloud compute without AWS/Azure/GCloud having my data. Being able to encrypt my data (pictures, videos, tables, text), then sending them to a cloud provider to provide arbitrary operations on the encrypted data, then sending the result back for me to decrypt is desired.
This would mean I could use cheamp compute without having to trust them with potentially sensitive/private data.
What if we could swap `import numpy as np` with `import FHEpy as np` and do all our regular operations as FHE operations?

## State of the Ecosystem
Right now it seems like [PALISADE](https://gitlab.com/palisade/palisade-development) is probably the most developed library for FHE. This project was founded by some of the founders at [Duality](https://dualitytech.com). Basically they want to use FHE for data privacy in large scale Financial, Healthcare, Insurance and Govt applications. That's probably a use for FHE, but don't these industrices already have data centers they can trust? I guess there's something gained by having one server for encrypting/decrypting the raw data then sending that cipherdata around, because they no longer have to trust every place the cipherdata goes. Still, seems like a cool demo and not a real necessity for FHE computation. FHE seems like the perfect thing for some of the decentraziled computating like @HOME or maybe a p2p cloud service where you don't have the luxury of trusting anyone.
So we've got PALISADE and a few other (TODO maybe write these down) FHE tools.


## Deliverables
* Simple website where users can drop in 2 encrypted integers (following certain encryption specifications) and perform integer operations.
* Exanding that to work as a matrix multiplier. Shouldn't require any additional low-level operations, albeit there's probably ways to optimize. Throughput doesn't matter for a V0.


## More Scoping
* Can we make a dropin replacement for some of the base operators in numpy or PyTorch. If yes, there's huge utility in not having to trust AWS to train your ML models, or for inference.



### References
1. [Somewhat Practical Fully Homomorphic Encryption](https://eprint.iacr.org/2012/144.pdf)
