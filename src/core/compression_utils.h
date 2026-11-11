#ifndef COMPRESSION_UTILS_H
#define COMPRESSION_UTILS_H

#include <cstddef>
#include <cstdint>


namespace Compression {
/**
 * @brief Compressão simples RLE (Run-Length Encoding) otimizada para logs
 * @param input Dados de entrada
 * @param inputSize Tamanho dos dados de entrada
 * @param output Buffer de saída
 * @param outputSize Tamanho dos dados comprimidos (retornado)
 * @return true se compressão foi bem-sucedida
 */
bool compressData(const uint8_t *input, size_t inputSize, uint8_t *output,
                  size_t &outputSize);

/**
 * @brief Descompressão RLE
 * @param input Dados comprimidos
 * @param inputSize Tamanho dos dados comprimidos
 * @param output Buffer de saída
 * @param outputSize Tamanho dos dados descomprimidos (retornado)
 * @return true se descompressão foi bem-sucedida
 */
bool decompressData(const uint8_t *input, size_t inputSize, uint8_t *output,
                    size_t &outputSize);
} // namespace Compression

#endif // COMPRESSION_UTILS_H