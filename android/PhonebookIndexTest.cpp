/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include "PhonebookIndex.h"

#include <unicode/unistr.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace android;

class TestExecutor {
public:
    TestExecutor() : m_total_count(0), m_success_count(0), m_success(true) {}
    bool DoAllTests();
private:
    void DoOneTest(void (TestExecutor::*test)());

    void testGetIndex(const char *src, const char *locale,
                      int32_t expected_len, UChar *expected_value);
    void testEnglish();

    // Note: When adding a test, do not forget to add it to DoOneTest().

    int m_total_count;
    int m_success_count;

    bool m_success;
};


bool TestExecutor::DoAllTests() {
    DoOneTest(&TestExecutor::testEnglish);

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

#define BUFFER_SIZE 10

static void printUTF8Str(const char *utf8_str) {
    printf("%s (", utf8_str);
    for(; *utf8_str != '\0'; ++utf8_str) {
        printf("\\x%02hhX", *utf8_str);
    }
    printf(")");
}

static void printUChars(const UChar *uc_str, int32_t len) {
    std::string utf8_str;
    UnicodeString(uc_str, len).toUTF8String(utf8_str);
    printf("%s (", utf8_str.c_str());
    for(int i=0; i<len; ++i) {
        printf("0x%02hx%s", uc_str[i], i < (len - 1) ? " " : "");
    }
    printf(")");
}

void TestExecutor::testGetIndex(
    const char *src, const char *locale,
    int32_t expected_len, UChar *expected_value) {
    UBool isError;

    UCharIterator iter;
    uiter_setUTF8(&iter, src, -1);

    UChar outBuf[BUFFER_SIZE];

    int32_t len = GetPhonebookIndex(&iter, locale, outBuf, sizeof(outBuf), &isError);
    if (isError) {
        printf("GetPhonebookIndex returned error (%s:%s)\n", locale, src);
        m_success = false;
    } else if (len != expected_len) {
        printf("len is unexpected value (src: [%s] %s, ", locale, src);
        printf("actual: %u (", len);
        printUChars(outBuf, len);
        printf("), expected: %u (", expected_len);
        printUChars(expected_value, expected_len);
        printf("))\n");
        m_success = false;
    } else {
        printf("[%s] %s: ", locale, src);
        printUChars(outBuf, len);

        if (memcmp(outBuf, expected_value, sizeof(UChar)*expected_len) != 0) {
            printf(", expected ");
            printUChars(expected_value, expected_len);
            m_success = false;
        }
        printf("\n");
    }
}

#define TEST_GET_UTF8STR_INDEX(src, locale, ...)                \
    ({                                                          \
        UChar uc_expected[] = {__VA_ARGS__};                    \
        int32_t len = sizeof(uc_expected)/sizeof(UChar);        \
        testGetIndex((src), (locale), len, uc_expected);        \
    })

#define TEST_GET_UCHAR_INDEX(src, locale, ...)                           \
    ({                                                                   \
        std::string utf8_str;                                            \
        UnicodeString((UChar) (src)).toUTF8String(utf8_str);             \
        TEST_GET_UTF8STR_INDEX(utf8_str.c_str(), (locale), __VA_ARGS__); \
    })

void TestExecutor::testEnglish() {
    printf("testEnglish()\n");

    // English [A-Z]
    TEST_GET_UTF8STR_INDEX("Allen", "en", 'A');
    TEST_GET_UTF8STR_INDEX("allen", "en", 'A');
    TEST_GET_UTF8STR_INDEX("123456", "en", '#');
    TEST_GET_UTF8STR_INDEX("+1 (123) 456-7890", "en", '#');
    TEST_GET_UTF8STR_INDEX("(33) 44.55.66.08", "en", '#');
    TEST_GET_UTF8STR_INDEX("123 Jump", "en", '#');
    // Arabic numbers
    TEST_GET_UTF8STR_INDEX("\u0662\u0663\u0664\u0665\u0666", "en", '#');

    // Japanese
    //   sorts hiragana/katakana, Kanji/Chinese, English, other
    // …, あ, か, さ, た, な, は, ま, や, ら, わ, …
    // hiragana "a"
    TEST_GET_UCHAR_INDEX(0x3041, "ja", 0x3042);
    // katakana "a"
    TEST_GET_UCHAR_INDEX(0x30A1, "ja", 0x3042);

    // Kanji (sorts to inflow section)
    TEST_GET_UCHAR_INDEX(0x65E5, "ja", 0x4ed6);
    // English
    TEST_GET_UTF8STR_INDEX("Smith", "ja", 'S');
    TEST_GET_UTF8STR_INDEX("234567", "ja", '#');
    // Chinese (sorts to inflow section)
    TEST_GET_UCHAR_INDEX(0x6c88 /* Shen/Chen */, "ja", 0x4ed6);
    // Korean Hangul (sorts to overflow section)
    TEST_GET_UCHAR_INDEX(0x1100, "ja", /* null */ );

    // Korean (sorts Korean, then English)
    // …, ᄀ, ᄂ, ᄃ, ᄅ, ᄆ, ᄇ, ᄉ, ᄋ, ᄌ, ᄎ, ᄏ, ᄐ, ᄑ, ᄒ, …
    TEST_GET_UCHAR_INDEX(0x1100, "ko", 0x1100);
    TEST_GET_UCHAR_INDEX(0x3131, "ko", 0x1100);
    TEST_GET_UCHAR_INDEX(0x1101, "ko", 0x1100);
    TEST_GET_UCHAR_INDEX(0x1161, "ko", 0x1112);

    // Czech
    // …, [A-C], Č,[D-H], CH, [I-R], Ř, S, Š, [T-Z], Ž, …
    TEST_GET_UTF8STR_INDEX("Cena", "cs", 'C');
    TEST_GET_UTF8STR_INDEX("Čáp", "cs", 0x010c);
    TEST_GET_UTF8STR_INDEX("Ruda", "cs", 'R');
    TEST_GET_UTF8STR_INDEX("Řada", "cs", 0x0158);
    TEST_GET_UTF8STR_INDEX("Selka", "cs", 'S');
    TEST_GET_UTF8STR_INDEX("Šála", "cs", 0x0160);
    TEST_GET_UTF8STR_INDEX("Zebra", "cs", 'Z');
    TEST_GET_UTF8STR_INDEX("Žába", "cs", 0x017d);
    TEST_GET_UTF8STR_INDEX("Chata", "cs", 'C', 'H');

    // French: [A-Z] (no accented chars)
    TEST_GET_UTF8STR_INDEX("Øfer", "fr", 'O');
    TEST_GET_UTF8STR_INDEX("Œster", "fr", 'O');

    // Danish: [A-Z], Æ, Ø, Å
    TEST_GET_UTF8STR_INDEX("Ænes", "da", 0xc6);
    TEST_GET_UTF8STR_INDEX("Øfer", "da", 0xd8);
    TEST_GET_UTF8STR_INDEX("Œster", "da", 0xd8);
    TEST_GET_UTF8STR_INDEX("Ågård", "da", 0xc5);

    // German: [A-Z] (no ß or umlauted characters in standard alphabet)
    TEST_GET_UTF8STR_INDEX("ßind", "de", 'S');

    // Simplified Chinese (default collator Pinyin): [A-Z]
    // Shen/Chen (simplified): should be, usually, 'S' for name collator and 'C' for apps/other
    TEST_GET_UCHAR_INDEX(0x6c88 /* Shen/Chen */, "zh_CN", 'C');
    // Shen/Chen (traditional)
    TEST_GET_UCHAR_INDEX(0x700b, "zh_CN", 'S');
    // Jia/Gu: should be, usually, 'J' for name collator and 'G' for apps/other
    TEST_GET_UCHAR_INDEX(0x8d3e /* Jia/Gu */, "zh_CN", 'J');

    // Traditional Chinese
    // …, 一, 丁, 丈, 不, 且, 丞, 串, 並, 亭, 乘, 乾, 傀, 亂, 僎, 僵, 儐, 償, 叢, 儳, 嚴, 儷, 儻, 囌, 囑, 廳, …
    TEST_GET_UCHAR_INDEX(0x6c88 /* Shen/Chen */, "zh_TW", 0x5080);
    TEST_GET_UCHAR_INDEX(0x700b /* Shen/Chen */, "zh_TW", 0x53e2);
    TEST_GET_UCHAR_INDEX(0x8d3e /* Jia/Gu */, "zh_TW", 0x5080);

    // Thai (sorts English then Thai)
    // …, ก, ข, ฃ, ค, ฅ, ฆ, ง, จ, ฉ, ช, ซ, ฌ, ญ, ฎ, ฏ, ฐ, ฑ, ฒ, ณ, ด, ต, ถ, ท, ธ, น, บ, ป, ผ, ฝ, พ, ฟ, ภ, ม, ย, ร, ฤ, ล, ฦ, ว, ศ, ษ, ส, ห, ฬ, อ, ฮ, …,

    TEST_GET_UTF8STR_INDEX("\u0e2d\u0e07\u0e04\u0e4c\u0e40\u0e25\u0e47\u0e01",
                           "th", 0xe2d);
    TEST_GET_UTF8STR_INDEX("\u0e2a\u0e34\u0e07\u0e2b\u0e40\u0e2a\u0e19\u0e35",
                           "th", 0xe2a);
    // Thai numbers ((02) 432-0281)
    TEST_GET_UTF8STR_INDEX("(\u0e50\u0e52) \u0e54\u0e53\u0e52-"
                           "\u0e50\u0e52\u0e58\u0e51", "th", '#');

    // Arabic (sorts English then Arabic)
    // …, ا, ب, ت, ث, ج, ح, خ, د, ذ, ر, ز, س, ش, ص, ض, ط, ظ, ع, غ, ف, ق, ك, ل, م, ن, ه, و, ي, …
    TEST_GET_UTF8STR_INDEX("\u0646\u0648\u0631" /* Noor */, "ar", 0x646);
    // Arabic numbers (34567)
    TEST_GET_UTF8STR_INDEX("\u0662\u0663\u0664\u0665\u0666", "ar", '#');

    // Hebrew (sorts English then Hebrew)
    // …, א, ב, ג, ד, ה, ו, ז, ח, ט, י, כ, ל, מ, נ, ס, ע, פ, צ, ק, ר, ש, ת, …
    TEST_GET_UTF8STR_INDEX("\u05e4\u05e8\u05d9\u05d3\u05de\u05df",  "he", 0x5e4);
}

int main() {
    TestExecutor executor;
    if(executor.DoAllTests()) {
        return 0;
    } else {
        return 1;
    }
}
