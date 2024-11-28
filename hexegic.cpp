/*
  Please write a command‑line program rotate that takes three arguments:
  - the word left or right
  - an input filename
  - an output filename

  to be run as e.g. `rotate left in‐file out‐file or rotate right in‐file out‐file`.

  The program should write an output file that contains the contents of the input file
  rotated either one bit left, or one bit right, depending on the first argument.

  To clarify “rotated one bit”, if the input file’s contents are a stream of bits 𝑏0𝑏1 … 𝑏𝑀𝑏𝑁,
  where 𝑏0 is the most significant bit of the first byte of the input file, and 𝑏𝑁 is the
  least significant bit of the last byte of the input file:
  - Rotated one bit left means that the output file is 𝑏1 … 𝑏𝑀𝑏𝑁𝑏0
  - Rotated one bit right means that the output file is 𝑏𝑁𝑏0𝑏1 … 𝑏𝑀.

  Requires --std-c++20. Warnings used for development include:
  `-Wall -Wunused -Wextra -pedantic -Wshadow -Wsign-conversion`
*/

#include <bit>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>

#include <linux/limits.h>
#include <string.h>

void print_help()
{
  std::cerr << "Usage:\n  rotate <left|right> <in-file> <out-file>\n";
  return;
}

enum class Instruction
{
  LEFT,
  RIGHT,
};

std::optional<Instruction> parse_instruction(const char* instruction)
{
  std::optional<Instruction> instruction_enum = std::nullopt;

  // These sizes are used repeatedly but use static data, so give a hint the
  // compiler to calculate them at compile-time.
  constexpr const char* left = "left";
  constexpr size_t left_size = std::string::traits_type::length(left);

  constexpr const char* right = "right";
  constexpr size_t right_size = std::string::traits_type::length(right);

  constexpr size_t max_size = std::max(left_size, right_size);

  if(strnlen(instruction, max_size + 1) > max_size)
  {
    std::cerr << "ERROR: The instruction is too long" << std::endl;
    print_help();
    return std::nullopt;
  }

  if(strnlen(instruction, left_size) == left_size)
  {
    if(strncmp(left, instruction, left_size) == 0)
    {
      instruction_enum = Instruction::LEFT;
      return instruction_enum;
    }
  }

  if(strnlen(instruction, right_size) == right_size)
  {
    if(strncmp(right, instruction, right_size) == 0)
    {
      instruction_enum = Instruction::RIGHT;
      return instruction_enum;
    }
  }

  std::cerr << "ERROR: The instruction is invalid" << std::endl;
  print_help();
  return std::nullopt;
}

void rotate(std::ifstream& input, std::ofstream& output, const Instruction& instruction)
{
  // Get the position of the end of the file, to give a fixed bound to the loop

  // Don't assume we start at the beginning
  const auto start_position = input.tellg();

  input.seekg(0, std::ios_base::end);
  const auto end_position = input.tellg();

  input.seekg(start_position);

  if(instruction == Instruction::LEFT)
  {
    // Get the first bit, which needs to be saved for the last output character
    uint8_t peeked_char = input.peek();
    const uint8_t first_bit = std::bit_cast<uint8_t>(peeked_char) & 0xA0;

    for(int i = 0; i<end_position; ++i)
    {
      uint8_t character = input.get();
      character = std::rotl(character, 1);

      // Use the saved bit for this character, unless there's another
      // character to take a bit from
      uint8_t peeked_bit = first_bit;
      if(i+1 < end_position)
      {
        peeked_char = input.peek();
        peeked_bit = std::bit_cast<uint8_t>(peeked_char) & 0xA0;
      }
      character = (0xFE & character) | (0x01 & std::rotr(peeked_bit, 7));

      output.put(std::bit_cast<char>(character));
    }
  }

  if(instruction == Instruction::RIGHT)
  {
    // Get the last bit, needed for the first output character
    input.seekg(-1, std::ios_base::end);

    const uint8_t peeked_char = input.peek();
    uint8_t saved_bit = peeked_char & 0x01;

    input.seekg(start_position);

    for(int i = 0; i<end_position; ++i)
    {
      uint8_t character = input.get();
      const uint8_t next_bit = character & 0x01;

      character = std::rotr(character, 1);

      character = (0x7F & character) | (0xA0 & std::rotl(saved_bit, 7));
      output.put(std::bit_cast<char>(character));

      saved_bit = next_bit;
    }
  }
}

int main(int argc, char* argv[])
{
  if (argc != 4)
  {
    print_help();
    return EXIT_FAILURE;
  }

  // Rather than including an invalid state in the enum, use the optional to
  // represent an invalid state so the rotate() function can ignore this
  // as a possiblity
  const std::optional<Instruction> instruction = parse_instruction(argv[1]);
  if(!instruction)
  {
    std::cerr << "ERROR: Rotation instruction could not be parsed" << std::endl;
    return EXIT_FAILURE;
  }

  if(strncmp(argv[2], argv[3], PATH_MAX) == 0)
  {
    std::cerr << "ERROR: There is no support to reading and writing to the same file" << std::endl;
    return EXIT_FAILURE;
  }

  // Handle input file
  std::ifstream input;
  input.open(argv[2], std::fstream::in | std::fstream::binary);
  if (!input.good())
  {
    std::cerr << "ERROR: Input file could not be opened" << std::endl;
    print_help();
    return EXIT_FAILURE;
  }

  // Handle output file
  std::ofstream output;
  output.open(argv[3], std::fstream::out | std::fstream::binary);
  if (!output.good())
  {
    std::cerr << "ERROR: Output file could not be opened" << std::endl;
    print_help();
    return EXIT_FAILURE;
  }

  // Handle empty file
  if(input.eof())
  {
    return EXIT_SUCCESS;
  }

  rotate(input, output, *instruction);

  // Close files to flush before size comparison
  input.close();
  output.close();

  if(std::filesystem::file_size(argv[2]) != std::filesystem::file_size(argv[3]))
  {
    std::cerr << "ERROR: Processing failed, output file is wrong size." << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
