/*
 * Copyright 2010, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ctype.h>
#include <string.h>

#include <unicode/ucol.h>
#include <unicode/uiter.h>
#include <unicode/ustring.h>
#include <unicode/utypes.h>

#include "PhonebookIndex.h"
#include "PhoneticStringUtils.h"

#define MIN_OUTPUT_SIZE 6       // Minimum required size for the output buffer (in bytes)

namespace android {

#define CP_JAPANESE_A  0x3042
#define CP_JAPANESE_KA 0x304B
#define CP_JAPANESE_SA 0x3055
#define CP_JAPANESE_TA 0x305F
#define CP_JAPANESE_NA 0x306A
#define CP_JAPANESE_HA 0x306F
#define CP_JAPANESE_MA 0x307E
#define CP_JAPANESE_YA 0x3084
#define CP_JAPANESE_RA 0x3089
#define CP_JAPANESE_WA 0x308F

// IMPORTANT!  Keep the codes below SORTED. We are doing a binary search on the array
static UChar DEFAULT_CHAR_MAP[] = {
    0x00C6,    'A',       // AE
    0x00DF,    'S',       // Etzett
    0x1100, 0x3131,       // HANGUL LETTER KIYEOK
    0x1101, 0x3132,       // HANGUL LETTER SSANGKIYEOK
    0x1102, 0x3134,       // HANGUL LETTER NIEUN
    0x1103, 0x3137,       // HANGUL LETTER TIKEUT
    0x1104, 0x3138,       // HANGUL LETTER SSANGTIKEUT
    0x1105, 0x3139,       // HANGUL LETTER RIEUL
    0x1106, 0x3141,       // HANGUL LETTER MIEUM
    0x1107, 0x3142,       // HANGUL LETTER PIEUP
    0x1108, 0x3143,       // HANGUL LETTER SSANGPIEUP
    0x1109, 0x3145,       // HANGUL LETTER SIOS
    0x110A, 0x3146,       // HANGUL LETTER SSANGSIOS
    0x110B, 0x3147,       // HANGUL LETTER IEUNG
    0x110C, 0x3148,       // HANGUL LETTER CIEUC
    0x110D, 0x3149,       // HANGUL LETTER SSANGCIEUC
    0x110E, 0x314A,       // HANGUL LETTER CHIEUCH
    0x110F, 0x314B,       // HANGUL LETTER KHIEUKH
    0x1110, 0x314C,       // HANGUL LETTER THIEUTH
    0x1111, 0x314D,       // HANGUL LETTER PHIEUPH
    0x1112, 0x314E,       // HANGUL LETTER HIEUH
    0x111A, 0x3140,       // HANGUL LETTER RIEUL-HIEUH
    0x1121, 0x3144,       // HANGUL LETTER PIEUP-SIOS
    0x1161, 0x314F,       // HANGUL LETTER A
    0x1162, 0x3150,       // HANGUL LETTER AE
    0x1163, 0x3151,       // HANGUL LETTER YA
    0x1164, 0x3152,       // HANGUL LETTER YAE
    0x1165, 0x3153,       // HANGUL LETTER EO
    0x1166, 0x3154,       // HANGUL LETTER E
    0x1167, 0x3155,       // HANGUL LETTER YEO
    0x1168, 0x3156,       // HANGUL LETTER YE
    0x1169, 0x3157,       // HANGUL LETTER O
    0x116A, 0x3158,       // HANGUL LETTER WA
    0x116B, 0x3159,       // HANGUL LETTER WAE
    0x116C, 0x315A,       // HANGUL LETTER OE
    0x116D, 0x315B,       // HANGUL LETTER YO
    0x116E, 0x315C,       // HANGUL LETTER U
    0x116F, 0x315D,       // HANGUL LETTER WEO
    0x1170, 0x315E,       // HANGUL LETTER WE
    0x1171, 0x315F,       // HANGUL LETTER WI
    0x1172, 0x3160,       // HANGUL LETTER YU
    0x1173, 0x3161,       // HANGUL LETTER EU
    0x1174, 0x3162,       // HANGUL LETTER YI
    0x1175, 0x3163,       // HANGUL LETTER I
    0x11AA, 0x3133,       // HANGUL LETTER KIYEOK-SIOS
    0x11AC, 0x3135,       // HANGUL LETTER NIEUN-CIEUC
    0x11AD, 0x3136,       // HANGUL LETTER NIEUN-HIEUH
    0x11B0, 0x313A,       // HANGUL LETTER RIEUL-KIYEOK
    0x11B1, 0x313B,       // HANGUL LETTER RIEUL-MIEUM
    0x11B3, 0x313D,       // HANGUL LETTER RIEUL-SIOS
    0x11B4, 0x313E,       // HANGUL LETTER RIEUL-THIEUTH
    0x11B5, 0x313F,       // HANGUL LETTER RIEUL-PHIEUPH
};

/**
 * Binary search to map an individual character to the corresponding phone book index.
 */
static UChar map_character(UChar c, UChar * char_map, int32_t length) {
  int from = 0, to = length;
  while (from < to) {
    int m = ((to + from) >> 1) & ~0x1;    // Only consider even positions
    UChar cm = char_map[m];
    if (cm == c) {
      return char_map[m + 1];
    } else if (cm < c) {
      from = m + 2;
    } else {
      to = m;
    }
  }
  return 0;
}

/**
 * Returns TRUE if the character belongs to a Hanzi unicode block
 */
static bool is_CJK(UChar c) {
  return
       (0x4e00 <= c && c <= 0x9fff)     // CJK_UNIFIED_IDEOGRAPHS
    || (0x3400 <= c && c <= 0x4dbf)     // CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A
    || (0x3000 <= c && c <= 0x303f)     // CJK_SYMBOLS_AND_PUNCTUATION
    || (0x2e80 <= c && c <= 0x2eff)     // CJK_RADICALS_SUPPLEMENT
    || (0x3300 <= c && c <= 0x33ff)     // CJK_COMPATIBILITY
    || (0xfe30 <= c && c <= 0xfe4f)     // CJK_COMPATIBILITY_FORMS
    || (0xf900 <= c && c <= 0xfaff);    // CJK_COMPATIBILITY_IDEOGRAPHS
}

/**
 * Returns TRUE if the character is collated as a letter.
 */
static bool is_letterlike_symbol(UChar c) {
  return
       (0x3208 <= c && c <= 0x32FE)     // Katakana, Circled
    || (0x3300 <= c && c <= 0x3357);    // Squared Japanese Katakana Words
}

int32_t GetPhonebookIndex(UCharIterator * iter, const char * locale, UChar * out, int32_t size,
        UBool * isError)
{
  if (size < MIN_OUTPUT_SIZE) {
    *isError = TRUE;
    return 0;
  }

  *isError = FALSE;

  // Normalize the first character to remove accents using the NFD normalization
  UErrorCode errorCode = U_ZERO_ERROR;
  int32_t len = unorm_next(iter, out, size, UNORM_NFD,
          0 /* options */, TRUE /* normalize */, NULL, &errorCode);
  if (U_FAILURE(errorCode)) {
    *isError = TRUE;
    return 0;
  }

  if (len == 0) {   // Empty input string
    return 0;
  }

  UChar c = out[0];

  if (!u_isalpha(c)) {
    // Digits go into a # section. Everything else goes into the empty section
    // The unicode function u_isdigit would also identify other characters as digits (arabic),
    // but if we caught them here we'd risk having the same section before and after alpha-letters
    // which might break the assumption that each section exists only once
    if (c >= '0' && c <= '9') {
      out[0] = '#';
      return 1;
    } else if (is_letterlike_symbol(c)) {
      if (0x3208 <= c && c <= 0x32FE) {
        // Katakana, Circled
        if (c < 0x32D5)      c = CP_JAPANESE_A;
        else if (c < 0x32DA) c = CP_JAPANESE_KA;
        else if (c < 0x32DF) c = CP_JAPANESE_SA;
        else if (c < 0x32E4) c = CP_JAPANESE_TA;
        else if (c < 0x32E9) c = CP_JAPANESE_NA;
        else if (c < 0x32EE) c = CP_JAPANESE_HA;
        else if (c < 0x32F3) c = CP_JAPANESE_MA;
        else if (c < 0x32F6) c = CP_JAPANESE_YA;
        else if (c < 0x32FB) c = CP_JAPANESE_RA;
        else                 c = CP_JAPANESE_WA;
        out[0] = c;
        return 1;
      } else if (0x3300 <= c && c <= 0x3357) {
        // Squared Japanese Katakana Words
        if (c < 0x330B)      c = CP_JAPANESE_A;
        else if (c < 0x331F) c = CP_JAPANESE_KA;
        else if (c < 0x3324) c = CP_JAPANESE_SA;
        else if (c < 0x3328) c = CP_JAPANESE_TA;
        else if (c < 0x332A) c = CP_JAPANESE_NA;
        else if (c < 0x3343) c = CP_JAPANESE_HA;
        else if (c < 0x334E) c = CP_JAPANESE_MA;
        else if (c < 0x3351) c = CP_JAPANESE_YA;
        else if (c < 0x3357) c = CP_JAPANESE_RA;
        else                 c = CP_JAPANESE_WA;
        out[0] = c;
        return 1;
      }
      // Expect unprocessed chars are caught at DEFAULT_CHAR_MAP...
    } else {
      return 0;
    }
  }

  c = u_toupper(c);

  // Check for explicitly mapped characters
  UChar c_mapped = map_character(c, DEFAULT_CHAR_MAP, sizeof(DEFAULT_CHAR_MAP) / sizeof(UChar));
  if (c_mapped != 0) {
    out[0] = c_mapped;
    return 1;
  }

  // Convert Kanas to Hiragana
  UChar next = len > 2 ? out[1] : 0;
  c = android::GetNormalizedCodePoint(c, next, NULL);

  // Traditional grouping of Hiragana characters
  if (0x3041 <= c && c <= 0x309F) {
    if (c < 0x304B)      c = CP_JAPANESE_A;
    else if (c < 0x3055) c = CP_JAPANESE_KA;
    else if (c < 0x305F) c = CP_JAPANESE_SA;
    else if (c < 0x306A) c = CP_JAPANESE_TA;
    else if (c < 0x306F) c = CP_JAPANESE_NA;
    else if (c < 0x307E) c = CP_JAPANESE_HA;
    else if (c < 0x3083) c = CP_JAPANESE_MA;
    else if (c < 0x3089) c = CP_JAPANESE_YA;
    else if (c < 0x308E) c = CP_JAPANESE_RA;
    else if (c < 0x3094) c = CP_JAPANESE_WA;
    else return 0;       // Others are not readable
    out[0] = c;
    return 1;
  } else if (0x30A0 <= c && c <= 0x30FF) {
    // Dot, onbiki, iteration marks are not readable
    return 0;
  }

  if (is_CJK(c)) {
    if (strncmp(locale, "ja", 2) == 0) {
      // Japanese word meaning "misc" or "other"
      out[0] = 0x4ED6;
      return 1;
    } else {
      return 0;
    }
  }

  out[0] = c;
  return 1;
}

}  // namespace android
