#include "compression_utils.h"
#include <algorithm>
#include <cstring>


namespace Compression {

bool compressData(const uint8_t *input, size_t inputSize, uint8_t *output,
                  size_t &outputSize) {
  if (!input || !output || inputSize == 0) {
    outputSize = 0;
    return false;
  }

  // Compressão RLE simples otimizada para texto/logs
  // Formato: [count][byte] para runs > 3, ou byte direto
  size_t outPos = 0;
  size_t maxOutput = inputSize; // Pior caso: sem compressão

  for (size_t i = 0; i < inputSize;) {
    uint8_t current = input[i];
    uint8_t count = 1;

    // Contar repetições
    while (i + count < inputSize && input[i + count] == current &&
           count < 255) {
      count++;
    }

    if (count >= 4 && outPos + 2 <= maxOutput) {
      // Usar RLE: [count][byte]
      output[outPos++] = count;
      output[outPos++] = current;
      i += count;
    } else {
      // Bytes únicos: escrever diretamente
      uint8_t uniqueCount = 0;
      while (uniqueCount < count && i + uniqueCount < inputSize &&
             (uniqueCount < 3 ||
              input[i + uniqueCount] != input[i + uniqueCount - 1])) {
        uniqueCount++;
      }

      if (outPos + uniqueCount <= maxOutput) {
        memcpy(&output[outPos], &input[i], uniqueCount);
        outPos += uniqueCount;
        i += uniqueCount;
      } else {
        // Buffer insuficiente
        outputSize = 0;
        return false;
      }
    }
  }

  outputSize = outPos;
  return outPos < inputSize; // Só retorna true se houve compressão
}

bool decompressData(const uint8_t *input, size_t inputSize, uint8_t *output,
                    size_t &outputSize) {
  if (!input || !output || inputSize == 0) {
    outputSize = 0;
    return false;
  }

  size_t outPos = 0;
  size_t i = 0;

  while (i < inputSize) {
    if (input[i] >= 4) {
      // RLE: [count][byte]
      uint8_t count = input[i++];
      if (i >= inputSize)
        break;

      uint8_t byte = input[i++];
      if (outPos + count > outputSize) {
        return false; // Buffer insuficiente
      }

      memset(&output[outPos], byte, count);
      outPos += count;
    } else {
      // Byte único
      if (outPos >= outputSize) {
        return false; // Buffer insuficiente
      }
      output[outPos++] = input[i++];
    }
  }

  outputSize = outPos;
  return true;
}

} // namespace Compression