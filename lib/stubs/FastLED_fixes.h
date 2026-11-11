#ifndef FASTLED_FIXES_H
#define FASTLED_FIXES_H

// FastLED fixes para ESP32-S3 no PlatformIO

// 1. Substituir fl/has_include.h por uma verificação simples
#ifndef FL_HAS_INCLUDE
#define FL_HAS_INCLUDE(header) 1
#endif

// 2. Garantir que math.h está incluído para powf
#include <math.h>

// 3. Definir FASTLED_ASSERT se não estiver definido
#ifndef FASTLED_ASSERT
#define FASTLED_ASSERT(cond, msg)                                              \
  if (!(cond)) {                                                               \
    Serial.println("FASTLED_ASSERT: " msg);                                    \
    while (1)                                                                  \
      ;                                                                        \
  }
#endif

// 4. Corrigir array bounds no I2SClockLessLedDriveresp32s3.h
// O array secondPixel tem tamanho _nb_components (normalmente 3), então índice
// 3 é inválido As linhas 493 e 505 acessam secondPixel[3] quando _nb_components
// > 3 Esta macro pode ser usada para boundary checking
#define SAFE_SECONDPIXEL_ACCESS(arr, idx, max_idx)                             \
  ((idx) < (max_idx) ? (arr)[(idx)] : *(arr))

#endif // FASTLED_FIXES_H