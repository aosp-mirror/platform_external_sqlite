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

#include "PhoneticStringUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <utils/String8.h>

using namespace android;

class TestExecutor {
 public:
  TestExecutor() : m_total_count(0), m_success_count(0), m_success(true) {}
  bool DoAllTests();
 private:
  void DoOneTest(void (TestExecutor::*test)());

  void testUtf32At();
  void testGetPhoneticallySortableCodePointAscii();
  void testGetPhoneticallySortableCodePointKana();
  void testGetPhoneticallySortableCodePointWhitespaceOnly();
  void testGetPhoneticallySortableCodePointSimpleCompare();
  void testGetUtf8FromUtf32();
  void testGetPhoneticallySortableString();
  void testGetNormalizedString();
  void testLongString();

  // Note: When adding a test, do not forget to add it to DoOneTest().

  int m_total_count;
  int m_success_count;

  bool m_success;
};

#define ASSERT_EQ_VALUE(input, expected)                                \
  ({                                                                    \
    if ((expected) != (input)) {                                        \
      printf("0x%X(result) != 0x%X(expected)\n", input, expected);      \
      m_success = false;                                                \
      return;                                                           \
    }                                                                   \
  })

#define EXPECT_EQ_VALUE(input, expected)                                \
  ({                                                                    \
    if ((expected) != (input)) {                                        \
      printf("0x%X(result) != 0x%X(expected)\n", input, expected);      \
      m_success = false;                                                \
    }                                                                   \
  })


bool TestExecutor::DoAllTests() {
  DoOneTest(&TestExecutor::testUtf32At);
  DoOneTest(&TestExecutor::testGetPhoneticallySortableCodePointAscii);
  DoOneTest(&TestExecutor::testGetPhoneticallySortableCodePointKana);
  DoOneTest(&TestExecutor::testGetPhoneticallySortableCodePointWhitespaceOnly);
  DoOneTest(&TestExecutor::testGetPhoneticallySortableCodePointSimpleCompare);
  DoOneTest(&TestExecutor::testGetUtf8FromUtf32);
  DoOneTest(&TestExecutor::testGetPhoneticallySortableString);
  DoOneTest(&TestExecutor::testGetNormalizedString);
  DoOneTest(&TestExecutor::testLongString);

  printf("Test total: %d\nSuccess: %d\nFailure: %d\n",
         m_total_count, m_success_count, m_total_count - m_success_count);

  bool success = m_total_count == m_success_count;
  printf("\n%s\n", success ? "Success" : "Failure");

  return success;
}

void TestExecutor::DoOneTest(void (TestExecutor::*test)()) {
  m_success = true;

  (this->*test)();

  ++m_total_count;
  m_success_count += m_success ? 1 : 0;
}

#define TEST_GET_UTF32AT(src, index, expected_next, expected_value)     \
  ({                                                                    \
    size_t next;                                                        \
    int32_t ret = utf32_at(src, strlen(src), index, &next);   \
    if (ret < 0) {                                                      \
      printf("getUtf32At() returned negative value (src: %s, index: %d)\n", \
             (src), (index));                                           \
      m_success = false;                                                \
    } else if (next != (expected_next)) {                               \
      printf("next is unexpected value (src: %s, actual: %u, expected: %u)\n", \
             (src), next, (expected_next));                             \
    } else {                                                            \
      EXPECT_EQ_VALUE(ret, (expected_value));                           \
    }                                                                   \
   })

void TestExecutor::testUtf32At() {
  printf("testUtf32At()\n");

  TEST_GET_UTF32AT("a", 0, 1, 97);
  // Japanese hiragana "a"
  TEST_GET_UTF32AT("\xE3\x81\x82", 0, 3, 0x3042);
  // Japanese fullwidth katakana "a" with ascii a
  TEST_GET_UTF32AT("a\xE3\x82\xA2", 1, 4, 0x30A2);

  // 2 PUA
  TEST_GET_UTF32AT("\xF3\xBE\x80\x80\xF3\xBE\x80\x88", 0, 4, 0xFE000);
  TEST_GET_UTF32AT("\xF3\xBE\x80\x80\xF3\xBE\x80\x88", 4, 8, 0xFE008);
}

void TestExecutor::testGetPhoneticallySortableCodePointAscii() {
  printf("testGetPhoneticallySortableCodePoint()\n");
  int halfwidth[94];
  int fullwidth[94];
  int i;
  char32_t codepoint;
  bool next_is_consumed;
  for (i = 0, codepoint = 0x0021; codepoint <= 0x007E; ++i, ++codepoint) {
    halfwidth[i] = GetPhoneticallySortableCodePoint(codepoint, 0,
                                                    &next_is_consumed);
    if (halfwidth[i] < 0) {
      printf("returned value become negative at 0x%04X", codepoint);
      m_success = false;
      return;
    }
    if (next_is_consumed) {
      printf("next_is_consumed become true at 0x%04X", codepoint);
      m_success = false;
      return;
    }
  }
  for (i = 0, codepoint = 0xFF01; codepoint <= 0xFF5E; ++i, ++codepoint) {
    fullwidth[i] = GetPhoneticallySortableCodePoint(codepoint, 0,
                                                    &next_is_consumed);
    if (fullwidth[i] < 0) {
      printf("returned value become negative at 0x%04X", codepoint);
      m_success = false;
      return;
    }
    if (next_is_consumed) {
      printf("next_is_consumed become true at 0x%04X", codepoint);
      m_success = false;
      return;
    }
  }

  for (i = 0; i < 94; i++) {
    EXPECT_EQ_VALUE(halfwidth[i], fullwidth[i]);
  }
}

void TestExecutor::testGetPhoneticallySortableCodePointKana() {
  printf("testGetPhoneticallySortableCodePointKana()\n");
  int hiragana[86];
  int fullwidth_katakana[86];
  int i;
  char32_t codepoint;
  bool next_is_consumed;

  for (i = 0, codepoint = 0x3041; codepoint <= 0x3096; ++i, ++codepoint) {
    hiragana[i] = GetPhoneticallySortableCodePoint(codepoint, 0,
                                                   &next_is_consumed);
    if (hiragana[i] < 0) {
      printf("returned value become negative at 0x%04X", codepoint);
      m_success = false;
      return;
    }
    if (next_is_consumed) {
      printf("next_is_consumed become true at 0x%04X", codepoint);
      m_success = false;
      return;
    }
  }

  for (i = 0, codepoint = 0x30A1; codepoint <= 0x30F6; ++i, ++codepoint) {
    fullwidth_katakana[i] = GetPhoneticallySortableCodePoint(codepoint, 0,
                                                   &next_is_consumed);
    if (fullwidth_katakana[i] < 0) {
      printf("returned value become negative at 0x%04X", codepoint);
      m_success = false;
      return;
    }
    if (next_is_consumed) {
      printf("next_is_consumed become true at 0x%04X", codepoint);
      m_success = false;
      return;
    }
  }

  // hankaku-katakana space do not have some characters corresponding to
  // zenkaku-hiragana (e.g. xwa, xka, xku). To make test easier, insert
  // zenkaku-katakana version of them into this array (See the value 0x30??).
  char32_t halfwidth_katakana[] = {
    0xFF67, 0xFF71, 0xFF68, 0xFF72, 0xFF69, 0xFF73, 0xFF6A, 0xFF74, 0xFF6B,
    0xFF75, 0xFF76, 0xFF76, 0xFF9E, 0xFF77, 0xFF77, 0xFF9E, 0xFF78, 0xFF78,
    0xFF9E, 0xFF79, 0xFF79, 0xFF9E, 0xFF7A, 0xFF7A, 0xFF9E, 0xFF7B, 0xFF7B,
    0xFF9E, 0xFF7C, 0xFF7C, 0xFF9E, 0xFF7D, 0xFF7D, 0xFF9E, 0xFF7E, 0xFF7E,
    0xFF9E, 0xFF7F, 0xFF7F, 0xFF9E, 0xFF80, 0xFF80, 0xFF9E, 0xFF81, 0xFF81,
    0xFF9E, 0xFF6F, 0xFF82, 0xFF82, 0xFF9E, 0xFF83, 0xFF83, 0xFF9E, 0xFF84,
    0xFF84, 0xFF9E, 0xFF85, 0xFF86, 0xFF87, 0xFF88, 0xFF89, 0xFF8A, 0xFF8A,
    0xFF9E, 0xFF8A, 0xFF9F, 0xFF8B, 0xFF8B, 0xFF9E, 0xFF8B, 0xFF9F, 0xFF8C,
    0xFF8C, 0xFF9E, 0xFF8C, 0xFF9F, 0xFF8D, 0xFF8D, 0xFF9E, 0xFF8D, 0xFF9F,
    0xFF8E, 0xFF8E, 0xFF9E, 0xFF8E, 0xFF9F, 0xFF8F, 0xFF90, 0xFF91, 0xFF92,
    0xFF93, 0xFF6C, 0xFF94, 0xFF6D, 0xFF95, 0xFF6E, 0xFF96, 0xFF97, 0xFF98,
    0xFF99, 0xFF9A, 0xFF9B, 0x30EE, 0xFF9C, 0x30F0, 0x30F1, 0xFF66, 0xFF9D,
    0xFF73, 0xFF9E, 0x30F5, 0x30F6};
  int len = sizeof(halfwidth_katakana)/sizeof(int);

  int halfwidth_katakana_result[86];

  int j;
  for (i = 0, j = 0; i < len && j < 86; ++i, ++j) {
    char32_t codepoint = halfwidth_katakana[i];
    char32_t next_codepoint = i + 1 < len ? halfwidth_katakana[i + 1] : 0;
    halfwidth_katakana_result[j] =
        GetPhoneticallySortableCodePoint(codepoint, next_codepoint,
                                         &next_is_consumed);
    // Consume voiced mark/half-voiced mark.
    if (next_is_consumed) {
      ++i;
    }
  }
  ASSERT_EQ_VALUE(i, len);
  ASSERT_EQ_VALUE(j, 86);

  for (i = 0; i < 86; ++i) {
    EXPECT_EQ_VALUE(fullwidth_katakana[i], hiragana[i]);
    EXPECT_EQ_VALUE(halfwidth_katakana_result[i], hiragana[i]);
  }
}

void TestExecutor::testGetPhoneticallySortableCodePointWhitespaceOnly() {
  printf("testGetPhoneticallySortableCodePointWhitespaceOnly()\n");
  // Halfwidth space
  int result = GetPhoneticallySortableCodePoint(0x0020, 0x0061, NULL);
  ASSERT_EQ_VALUE(result, -1);
  // Fullwidth space
  result = GetPhoneticallySortableCodePoint(0x3000, 0x0062, NULL);
  ASSERT_EQ_VALUE(result, -1);
  // tab
  result = GetPhoneticallySortableCodePoint(0x0009, 0x0062, NULL);
  ASSERT_EQ_VALUE(result, -1);
}

void TestExecutor::testGetPhoneticallySortableCodePointSimpleCompare() {
  printf("testGetPhoneticallySortableCodePointSimpleCompare()\n");

  char32_t codepoints[] = {
    0x3042, 0x30AB, 0xFF7B, 0x305F, 0x30CA, 0xFF8A, 0x30D0, 0x3071,
    0x307E, 0x30E4, 0xFF97, 0x308F, 0x3093, 0x3094, 'A', 'Z',
    '0', '9', '!', '/', ':', '?', '[', '`', '{', '~'};
  size_t len = sizeof(codepoints)/sizeof(int);
  bool next_is_consumed;
  for (size_t i = 0; i < len - 1; ++i) {
    int codepoint_a =
        GetPhoneticallySortableCodePoint(codepoints[i], 0,
                                         &next_is_consumed);
    if (next_is_consumed) {
      printf("next_is_consumed become true at 0x%04X", codepoint_a);
      m_success = false;
      return;
    }
    int codepoint_b =
        GetPhoneticallySortableCodePoint(codepoints[i + 1], 0,
                                         &next_is_consumed);
    if (next_is_consumed) {
      printf("next_is_consumed become true at 0x%04X", codepoint_b);
      m_success = false;
      return;
    }

    if (codepoint_a >= codepoint_b) {
      printf("0x%04X (from 0x%04X) >= 0x%04X (from 0x%04X)\n",
             codepoint_a, codepoints[i], codepoint_b, codepoints[i + 1]);
      m_success = false;
      return;
    }
  }
}

#define EXPECT_EQ_CODEPOINT_UTF8(codepoint, expected)                   \
  ({                                                                    \
    char32_t codepoints[1] = {codepoint};                                \
    status_t ret = string8.setTo(codepoints, 1);                        \
    if (ret != NO_ERROR) {                                              \
      printf("GetUtf8FromCodePoint() returned false at 0x%04X\n", codepoint); \
      m_success = false;                                                \
    } else {                                                            \
      const char* string = string8.string();                            \
      if (strcmp(string, expected) != 0) {                              \
        printf("Failed at codepoint 0x%04X\n", codepoint);              \
        for (const char *ch = string; *ch != '\0'; ++ch) {              \
          printf("0x%X ", *ch);                                         \
        }                                                               \
        printf("!= ");                                                  \
        for (const char *ch = expected; *ch != '\0'; ++ch) {            \
          printf("0x%X ", *ch);                                         \
        }                                                               \
        printf("\n");                                                   \
        m_success = false;                                              \
      }                                                                 \
    }                                                                   \
  })

void TestExecutor::testGetUtf8FromUtf32() {
  printf("testGetUtf8FromUtf32()\n");
  String8 string8;

  EXPECT_EQ_CODEPOINT_UTF8('a', "\x61");
  // Armenian capital letter AYB (2 bytes in UTF8)
  EXPECT_EQ_CODEPOINT_UTF8(0x0530, "\xD4\xB0");
  // Japanese 'a' (3 bytes in UTF8)
  EXPECT_EQ_CODEPOINT_UTF8(0x3042, "\xE3\x81\x82");
  // Kanji
  EXPECT_EQ_CODEPOINT_UTF8(0x65E5, "\xE6\x97\xA5");
  // PUA (4 byets in UTF8)
  EXPECT_EQ_CODEPOINT_UTF8(0xFE016, "\xF3\xBE\x80\x96");
  EXPECT_EQ_CODEPOINT_UTF8(0xFE972, "\xF3\xBE\xA5\xB2");
}

#define EXPECT_EQ_UTF8_UTF8(src, expected)                              \
  ({                                                                    \
    if (!GetPhoneticallySortableString(src, &dst, &len)) {              \
      printf("GetPhoneticallySortableString() returned false.\n");      \
      m_success = false;                                                \
    } else {                                                            \
      if (strcmp(dst, expected) != 0) {                                 \
        for (const char *ch = dst; *ch != '\0'; ++ch) {                 \
          printf("0x%X ", *ch);                                         \
        }                                                               \
        printf("!= ");                                                  \
        for (const char *ch = expected; *ch != '\0'; ++ch) {            \
          printf("0x%X ", *ch);                                         \
        }                                                               \
        printf("\n");                                                   \
        m_success = false;                                              \
      }                                                                 \
      free(dst);                                                        \
    }                                                                   \
   })

void TestExecutor::testGetPhoneticallySortableString() {
  printf("testGetPhoneticallySortableString()\n");
  char *dst;
  size_t len;

  // halfwidth alphabets -> fullwidth alphabets.
  EXPECT_EQ_UTF8_UTF8("ABCD",
                      "\xEF\xBC\xA1\xEF\xBC\xA2\xEF\xBC\xA3\xEF\xBC\xA4");
  // halfwidth/fullwidth-katakana -> hiragana
  EXPECT_EQ_UTF8_UTF8(
      "\xE3\x81\x82\xE3\x82\xA4\xE3\x81\x86\xEF\xBD\xB4\xE3\x82\xAA",
      "\xE3\x81\x82\xE3\x81\x84\xE3\x81\x86\xE3\x81\x88\xE3\x81\x8A");

  // whitespace -> string which should be placed at last
  EXPECT_EQ_UTF8_UTF8("    \t", "\xF0\x9F\xBF\xBD");
}

#undef EXPECT_EQ_UTF8_UTF8

#define EXPECT_EQ_UTF8_UTF8(src, expected)                              \
  ({                                                                    \
    if (!GetNormalizedString(src, &dst, &len)) {                        \
      printf("GetPhoneticallySortableString() returned false.\n");      \
      m_success = false;                                                \
    } else {                                                            \
      if (strcmp(dst, expected) != 0) {                                 \
        for (const char *ch = dst; *ch != '\0'; ++ch) {                 \
          printf("0x%X ", *ch);                                         \
        }                                                               \
        printf("!= ");                                                  \
        for (const char *ch = expected; *ch != '\0'; ++ch) {            \
          printf("0x%X ", *ch);                                         \
        }                                                               \
        printf("\n");                                                   \
        m_success = false;                                              \
      }                                                                 \
      free(dst);                                                        \
    }                                                                   \
   })

void TestExecutor::testGetNormalizedString() {
  printf("testGetNormalizedString()\n");
  char *dst;
  size_t len;

  // halfwidth alphabets/symbols -> keep it as is.
  EXPECT_EQ_UTF8_UTF8("ABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%^&'()",
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%^&'()");
  EXPECT_EQ_UTF8_UTF8("abcdefghijklmnopqrstuvwxyz[]{}\\@/",
                      "abcdefghijklmnopqrstuvwxyz[]{}\\@/");

  // halfwidth/fullwidth-katakana -> hiragana
  EXPECT_EQ_UTF8_UTF8(
      "\xE3\x81\x82\xE3\x82\xA4\xE3\x81\x86\xEF\xBD\xB4\xE3\x82\xAA",
      "\xE3\x81\x82\xE3\x81\x84\xE3\x81\x86\xE3\x81\x88\xE3\x81\x8A");

  // whitespace -> keep it as is.
  EXPECT_EQ_UTF8_UTF8("    \t", "    \t");
}

void TestExecutor::testLongString() {
  printf("testLongString()\n");
  char * dst;
  size_t len;
  EXPECT_EQ_UTF8_UTF8("Qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqtttttttttttttttttttttttttttttttttttttttttttttttttgggggggggggggggggggggggggggggggggggggggbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
      "Qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqtttttttttttttttttttttttttttttttttttttttttttttttttggggggggggggggggggggggggggggggggggg");
}


int main() {
  TestExecutor executor;
  if(executor.DoAllTests()) {
    return 0;
  } else {
    return 1;
  }
}
