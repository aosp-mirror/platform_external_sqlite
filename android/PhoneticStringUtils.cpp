/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>

#include "PhoneticStringUtils.h"

// We'd like 0 length string last of sorted list. So when input string is NULL
// or 0 length string, we use these instead.
#define CODEPOINT_FOR_NULL_STR 0xFFFD
#define STR_FOR_NULL_STR "\xEF\xBF\xBD"

// We assume that users will not notice strings not sorted properly when the
// first 128 characters are the same.
#define MAX_CODEPOINTS 128

namespace android {

int GetCodePointFromUtf8(const char *src, size_t len, size_t index, int *next) {
    if (src == NULL || len <= index) {
        return -1;
    }

    if ((src[index] >> 7) == 0) {
        if (next != NULL) {
            *next = index + 1;
        }
        return src[index];
    }
    if ((src[index] & 64) == 0) {
        return -1;
    }
    int mask;
    size_t num_to_read;
    for (num_to_read = 1, mask = 64;  // 01000000
         num_to_read < 7 && (src[index] & mask) == mask;
         num_to_read++, mask >>= 1) {
    }
    if (num_to_read == 7) {
        return -1;
    }

    if (num_to_read + index > len) {
        return -1;
    }

    {
        size_t i;
        for (i = 0, mask = 0; i < (7 - num_to_read); i++) {
            mask = (mask << 1) + 1;
        }
    }

    int codepoint = mask & src[index];

    for (size_t i = 1; i < num_to_read; i++) {
        if ((src[i + index] & 192) != 128) {  // must be 10xxxxxx
            return -1;
        }
        codepoint = (codepoint << 6) + (src[i + index] & 63);
    }

    if (next != NULL) {
        *next = index + num_to_read;
    }

    return codepoint;
}

int GetPhoneticallySortableCodePoint(int codepoint,
                                     int next_codepoint,
                                     bool *next_is_consumed) {
    if (next_is_consumed != NULL) {
        *next_is_consumed = false;
    }

    if (codepoint <= 0x0020 || codepoint == 0x3000) {
        // Whitespace should be ignored.
        // Note: Formally, more "whitespace" exist. This block only
        // handles part of them
        return -1;
    } else if ((0x0021 <= codepoint && codepoint <= 0x007E) ||
               (0xFF01 <= codepoint && codepoint <= 0xFF5E)) {
        // Ascii and fullwidth ascii

        if (0x0021 <= codepoint && codepoint <= 0x007E) {
            // Convert ascii to fullwidth ascii so that they become
            // behind hiragana.
            // 65248 = 0xFF01 - 0x0021
            codepoint += 65248;
        }

        // Now, there is only fullwidth ascii.
        if (0xFF10 <= codepoint && codepoint <= 0xFF19) {
            // Numbers should be after alphabets but before symbols.
            // 86 = 0xFF66
            // (the beginning of halfwidth-katakankana space) - 0xFF10
            return codepoint + 86;
        } else if (0xFF41 <= codepoint && codepoint <= 0xFF5A) {
            // Make lower alphabets same as capital alphabets.
            // 32 = 0xFF41 - 0xFF21
            return codepoint - 32;
        } else if (0xFF01 <= codepoint && codepoint <= 0xFF0F) {
            // Symbols (Ascii except alphabet nor number)
            // These should be at the end of sorting, just after numebers
            // (see below)
            //
            // We use halfwidth-katakana space for storing those symbols.
            // 111 = 0xFF70 (0xFF19 + 86 + 1) - 0xFF01
            return codepoint + 111;
        } else if (0xFF1A <= codepoint && codepoint <= 0xFF20) {
            // Symbols (cont.)
            // 101 = 0xFF7F (0xFF0F + 111 + 1) - 0xFF1A
            return codepoint + 101;
        } else if (0xFF3B <= codepoint && codepoint <= 0xFF40) {
            // Symbols (cont.)
            // 75 = 0xFF86 (0xFF20 + 101 + 1) - 0xFF3B (= 101 - 26)
            return codepoint + 75;
        } else if (0xFF5B <= codepoint && codepoint <= 0xFF5E) {
            // Symbols (cont.)
            // 49 = 0xFF8C (0xFF40 + 75 + 1) - 0xFF5B (= 75 - 26)
            return codepoint + 49;
        } else {
            return codepoint;
        }
    } else if (codepoint == 0x02DC || codepoint == 0x223C) {
        // tilde
        return 0xFF5E;
    } else if (codepoint <= 0x3040 ||
               (0x3100 <= codepoint && codepoint < 0xFF00) ||
               codepoint == CODEPOINT_FOR_NULL_STR) {
        // Move Kanji and other non-Japanese characters behind symbols.
        return codepoint + 0x10000;
    }

    // Below is Kana-related handling.

    // First, convert fullwidth katakana and halfwidth katakana to hiragana
    if (0x30A1 <= codepoint && codepoint <= 0x30F6) {
        // Make fullwidth katakana same as hiragana.
        // 96 == 0x30A1 - 0x3041c
        codepoint = codepoint - 96;
    } else if (0xFF66 <= codepoint && codepoint <= 0xFF9F) {
        // Make halfwidth katakana same as hiragana
        switch (codepoint) {
            case 0xFF66: // wo
                codepoint = 0x3092;
                break;
            case 0xFF67: // xa
                codepoint = 0x3041;
                break;
            case 0xFF68: // xi
                codepoint = 0x3043;
                break;
            case 0xFF69: // xu
                codepoint = 0x3045;
                break;
            case 0xFF6A: // xe
                codepoint = 0x3047;
                break;
            case 0xFF6B: // xo
                codepoint = 0x3049;
                break;
            case 0xFF6C: // xya
                codepoint = 0x3083;
                break;
            case 0xFF6D: // xyu
                codepoint = 0x3085;
                break;
            case 0xFF6E: // xyo
                codepoint = 0x3087;
                break;
            case 0xFF6F: // xtsu
                codepoint = 0x3063;
                break;
            case 0xFF70: // -
                codepoint = 0x30FC;
                break;
            case 0xFF9C: // wa
                codepoint = 0x308F;
                break;
            case 0xFF9D: // n
                codepoint = 0x3093;
                break;
            default:
                {
                    if (0xFF71 <= codepoint && codepoint <= 0xFF75) {
                        // a, i, u, e, o
                        if (codepoint == 0xFF73 && next_codepoint == 0xFF9E) {
                            if (next_is_consumed != NULL) {
                                *next_is_consumed = true;
                            }
                            codepoint = 0x3094; // vu
                        } else {
                            codepoint = 0x3042 + (codepoint - 0xFF71) * 2;
                        }
                    } else if (0xFF76 <= codepoint && codepoint <= 0xFF81) {
                        // ka - chi
                        if (next_codepoint == 0xFF9E) {
                            // "dakuten" (voiced mark)
                            if (next_is_consumed != NULL) {
                                *next_is_consumed = true;
                            }
                            codepoint = 0x304B + (codepoint - 0xFF76) * 2 + 1;
                        } else {
                            codepoint = 0x304B + (codepoint - 0xFF76) * 2;
                        }
                    } else if (0xFF82 <= codepoint && codepoint <= 0xFF84) {
                        // tsu, te, to (skip xtsu)
                        if (next_codepoint == 0xFF9E) {
                            // "dakuten" (voiced mark)
                            if (next_is_consumed != NULL) {
                                *next_is_consumed = true;
                            }
                            codepoint = 0x3064 + (codepoint - 0xFF82) * 2 + 1;
                        } else {
                            codepoint = 0x3064 + (codepoint - 0xFF82) * 2;
                        }
                    } else if (0xFF85 <= codepoint && codepoint <= 0xFF89) {
                        // na, ni, nu, ne, no
                        codepoint = 0x306A + (codepoint - 0xFF85);
                    } else if (0xFF8A <= codepoint && codepoint <= 0xFF8E) {
                        // ha, hi, hu, he, ho
                        if (next_codepoint == 0xFF9E) {
                            // "dakuten" (voiced mark)
                            if (next_is_consumed != NULL) {
                                *next_is_consumed = true;
                            }
                            codepoint = 0x306F + (codepoint - 0xFF8A) * 3 + 1;
                        } else if (next_codepoint == 0xFF9F) {
                            // "han-dakuten" (half voiced mark)
                            if (next_is_consumed != NULL) {
                                *next_is_consumed = true;
                            }
                            codepoint = 0x306F + (codepoint - 0xFF8A) * 3 + 2;
                        } else {
                            codepoint = 0x306F + (codepoint - 0xFF8A) * 3;
                        }
                    } else if (0xFF8F <= codepoint && codepoint <= 0xFF93) {
                        // ma, mi, mu, me, mo
                        codepoint = 0x307E + (codepoint - 0xFF8F);
                    } else if (0xFF94 <= codepoint && codepoint <= 0xFF96) {
                        // ya, yu, yo
                        codepoint = 0x3084 + (codepoint - 0xFF94) * 2;
                    } else if (0xFF97 <= codepoint && codepoint <= 0xFF9B) {
                        // ra, ri, ru, re, ro
                        codepoint = 0x3089 + (codepoint - 0xFF97);
                    }
                    // Note: 0xFF9C, 0xFF9D are handled above
                } // end of default
        } // end of case
    }

    // Trivial kana conversions.
    // e.g. xa => a
    switch (codepoint) {
        case 0x3041:
        case 0x3043:
        case 0x3045:
        case 0x3047:
        case 0x3049:
        case 0x308E: // xwa
            codepoint++;
            break;
        case 0x3095: // xka
            codepoint = 0x304B;
            break;
        case 0x3096: // xku
            codepoint = 0x304F;
            break;
    }

    return codepoint;
}

bool GetUtf8FromCodePoint(int codepoint, char *dst, size_t len, size_t *index) {
    if (codepoint < 128) {  // 1 << 7
        if (*index >= len) {
            return false;
        }
        // 0xxxxxxx
        dst[*index] = static_cast<char>(codepoint);
        (*index)++;
    } else if (codepoint < 2048) {  // 1 << (6 + 5)
        if (*index + 1 >= len) {
            return false;
        }
        // 110xxxxx
        dst[(*index)++] = static_cast<char>(192 | (codepoint >> 6));
        // 10xxxxxx
        dst[(*index)++] = static_cast<char>(128 | (codepoint & 63));
    } else if (codepoint < 65536) {  // 1 << (6 * 2 + 4)
        if (*index + 2 >= len) {
            return false;
        }
        // 1110xxxx
        dst[(*index)++] = static_cast<char>(224 | (codepoint >> 12));
        // 10xxxxxx
        dst[(*index)++] = static_cast<char>(128 | ((codepoint >> 6) & 63));
        dst[(*index)++] = static_cast<char>(128 | (codepoint & 63));
    } else if (codepoint < 2097152) {  // 1 << (6 * 3 + 3)
        if (*index + 3 >= len) {
            return false;
        }
        // 11110xxx
        dst[(*index)++] = static_cast<char>(240 | (codepoint >> 18));
        // 10xxxxxx
        dst[(*index)++] = static_cast<char>(128 | ((codepoint >> 12) & 63));
        dst[(*index)++] = static_cast<char>(128 | ((codepoint >> 6) & 63));
        dst[(*index)++] = static_cast<char>(128 | (codepoint & 63));
    } else if (codepoint < 67108864) {  // 1 << (6 * 2 + 2)
        if (*index + 4 >= len) {
            return false;
        }
        // 111110xx
        dst[(*index)++] = static_cast<char>(248 | (codepoint >> 24));
        // 10xxxxxx
        dst[(*index)++] = static_cast<char>(128 | ((codepoint >> 18) & 63));
        dst[(*index)++] = static_cast<char>(128 | ((codepoint >> 12) & 63));
        dst[(*index)++] = static_cast<char>(128 | ((codepoint >> 6) & 63));
        dst[(*index)++] = static_cast<char>(128 | (codepoint & 63));
    } else {
        if (*index + 5 >= len) {
            return false;
        }
        // 1111110x
        dst[(*index)++] = static_cast<char>(252 | (codepoint >> 30));
        // 10xxxxxx
        dst[(*index)++] = static_cast<char>(128 | ((codepoint >> 24) & 63));
        dst[(*index)++] = static_cast<char>(128 | ((codepoint >> 18) & 63));
        dst[(*index)++] = static_cast<char>(128 | ((codepoint >> 12) & 63));
        dst[(*index)++] = static_cast<char>(128 | ((codepoint >> 6) & 63));
        dst[(*index)++] = static_cast<char>(128 | (codepoint & 63));
    }
    return true;
}

bool GetPhoneticallySortableString(const char *src, char **dst, size_t *len){
    if (dst == NULL || len == NULL) {
        return false;
    }

    if (src == NULL || *src == '\0') {
        src = STR_FOR_NULL_STR;
    }

    size_t src_len = strlen(src);
    int codepoints[MAX_CODEPOINTS];
    size_t new_len = 0;

    size_t codepoint_index;
    {
        int i, next;
        for (codepoint_index = 0, i = 0, next = 0;
             static_cast<size_t>(i) < src_len &&
                     codepoint_index < MAX_CODEPOINTS;
             i = next) {
            int codepoint = GetCodePointFromUtf8(src, src_len, i, &next);
            if (codepoint <= 0) {
                return false;
            }
            int tmp_next;
            int next_codepoint = GetCodePointFromUtf8(src, src_len,
                                                      next, &tmp_next);
            bool next_is_consumed = false;

            // It is ok even if next_codepoint is negative.
            codepoints[codepoint_index] =
                    GetPhoneticallySortableCodePoint(codepoint,
                                                     next_codepoint,
                                                     &next_is_consumed);
            // dakuten (voiced mark) or han-dakuten (half-voiced mark) existed.
            if (next_is_consumed) {
                next = tmp_next;
            }

            if (codepoints[codepoint_index] < 0) {
              // Do not increment codepoint_index.
              continue;
            }

            if (codepoints[codepoint_index] < 128) {  // 1 << 7
                new_len++;
            } else if (codepoints[codepoint_index] < 2048) {
                // 1 << (6 + 5)
                new_len += 2;
            } else if (codepoints[codepoint_index] < 65536) {
                // 1 << (6 * 2 + 4)
                new_len += 3;
            } else if (codepoints[codepoint_index] < 2097152) {
                // 1 << (6 * 3 + 3)
                new_len += 4;
            } else if (codepoints[codepoint_index] < 67108864) {
                // 1 << (6 * 2 + 2)
                new_len += 5;
            } else {
                new_len += 6;
            }

            codepoint_index++;
        }
    }

    if (codepoint_index == 0) {
        // If all of codepoints are invalid, we place the string at the end of
        // the list.
        codepoints[0] = 0x10000 + CODEPOINT_FOR_NULL_STR;
        codepoint_index = 1;
        new_len = 4;
    }

    new_len += 1;  // For '\0'.

    *dst = static_cast<char *>(malloc(sizeof(char) * new_len));
    if (*dst == NULL) {
        return false;
    }

    size_t ch_index;
    {
        size_t i;
        for (i = 0, ch_index = 0; i < codepoint_index; i++) {
            if (!GetUtf8FromCodePoint(codepoints[i], *dst,
                                      new_len, &ch_index)) {
                free(*dst);
                *dst = NULL;
                return false;
            }
        }
    }

    if (ch_index != new_len - 1) {
        free(*dst);
        *dst = NULL;
        return false;
    }

    (*dst)[new_len - 1] = '\0';
    *len = new_len;
    return true;
}

}  // namespace android
