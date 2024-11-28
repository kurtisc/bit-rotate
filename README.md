- Requires --std-c++20. Warnings used for development include:
  - `-Wall -Wunused -Wextra -pedantic -Wshadow -Wsign-conversion`

Please write a command‑line program rotate that takes three arguments:
- the word left or right
- an input filename
- an output filename
to be run as e.g. rotate left in‐file out‐file or rotate right in‐file out‐file.

The program should write an output file that contains the contents of the input file rotated either one bit left, or one bit right, depending on the first argument. To clarify “rotated one bit”, if the input file’s contents are a stream of bits 𝑏0𝑏1...𝑏𝑀𝑏𝑁, where 𝑏0 is the most significant bit of the first byte of the input file, and 𝑏𝑁 is the least significant bit of the last byte of the input file:

- Rotated one bit left means that the output file is 𝑏1...𝑏𝑀𝑏𝑁𝑏0
- Rotated one bit right means that the output file is 𝑏𝑁𝑏0𝑏1...𝑏𝑀.

