/*
 * Copyright (C) 2017 The Android Open Source Project
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

//
// Note that similar (or almost same) tests exist in Java side (See
// DatabaseGeneralTest.java in AndroidTests). The differences are:
// - this test is quite easy to do (You can do it in your Unix PC)
// - this test is not automatically executed by build servers
//
// You should also execute the test before submitting this.
//

#include "PhoneNumberUtils.h"

#include <stdio.h>
#include <string.h>

#include <gtest/gtest.h>

using namespace android;


TEST(PhoneNumberUtils, compareLooseNullOrEmpty) {
    EXPECT_FALSE(phone_number_compare_loose(NULL, NULL));
    EXPECT_FALSE(phone_number_compare_loose("", NULL));
    EXPECT_FALSE(phone_number_compare_loose(NULL, ""));
    EXPECT_FALSE(phone_number_compare_loose("", ""));
}

TEST(PhoneNumberUtils, compareLooseDigitsSame) {
    EXPECT_TRUE(phone_number_compare_loose("999", "999"));
    EXPECT_TRUE(phone_number_compare_loose("119", "119"));
}

TEST(PhoneNumberUtils, compareLooseDigitsDifferent) {
    EXPECT_FALSE(phone_number_compare_loose("123456789", "923456789"));
    EXPECT_FALSE(phone_number_compare_loose("123456789", "123456781"));
    EXPECT_FALSE(phone_number_compare_loose("123456789", "1234567890"));
    EXPECT_TRUE(phone_number_compare_loose("123456789", "0123456789"));
}

TEST(PhoneNumberUtils, compareLooseGoogle) {
    EXPECT_TRUE(phone_number_compare_loose("650-253-0000", "6502530000"));
    EXPECT_TRUE(phone_number_compare_loose("650-253-0000", "650 253 0000"));
    EXPECT_TRUE(phone_number_compare_loose("650 253 0000", "6502530000"));
}

TEST(PhoneNumberUtils, compareLooseTrunkPrefixUs) {
    // trunk (NDD) prefix must be properly handled in US
    EXPECT_TRUE(phone_number_compare_loose("650-253-0000", "1-650-253-0000"));
    EXPECT_TRUE(phone_number_compare_loose("650-253-0000", "   1-650-253-0000"));

    EXPECT_TRUE(phone_number_compare_loose("650-253-0000", "11-650-253-0000"));
    EXPECT_TRUE(phone_number_compare_loose("650-253-0000", "0-650-253-0000"));
    EXPECT_TRUE(phone_number_compare_loose("555-4141", "+1-700-555-4141"));

    EXPECT_TRUE(phone_number_compare_loose("+1 650-253-0000", "6502530000"));
    EXPECT_TRUE(phone_number_compare_loose("001 650-253-0000", "6502530000"));
    EXPECT_TRUE(phone_number_compare_loose("0111 650-253-0000", "6502530000"));
}

TEST(PhoneNumberUtils, compareLooseDifferentCountryCode) {
    EXPECT_FALSE(phone_number_compare_loose("+19012345678", "+819012345678"));
}

TEST(PhoneNumberUtils, compareLooseTrunkJapan) {
    EXPECT_TRUE(phone_number_compare_loose("+31771234567", "0771234567"));
    EXPECT_TRUE(phone_number_compare_loose("090-1234-5678", "+819012345678"));
    EXPECT_TRUE(phone_number_compare_loose("090(1234)5678", "+819012345678"));
    EXPECT_TRUE(phone_number_compare_loose("090-1234-5678", "+81-90-1234-5678"));

    EXPECT_TRUE(phone_number_compare_loose("+819012345678", "090-1234-5678"));
    EXPECT_TRUE(phone_number_compare_loose("+819012345678", "090(1234)5678"));
    EXPECT_TRUE(phone_number_compare_loose("+81-90-1234-5678", "090-1234-5678"));
}

TEST(PhoneNumberUtils, compareLooseTrunkRussia) {
    EXPECT_TRUE(phone_number_compare_loose("+79161234567", "89161234567"));

}

TEST(PhoneNumberUtils, compareLooseTrunkFrance) {
    EXPECT_TRUE(phone_number_compare_loose("+33123456789", "0123456789"));
}

TEST(PhoneNumberUtils, compareLooseTrunkHungary) {
    EXPECT_TRUE(phone_number_compare_loose("+36 1 234 5678", "06 1234-5678"));
}

TEST(PhoneNumberUtils, compareLooseTrunkMexico) {
    EXPECT_TRUE(phone_number_compare_loose("+52 55 1234 5678", "01 55 1234 5678"));
}

TEST(PhoneNumberUtils, compareLooseTrunkMongolia) {
    EXPECT_TRUE(phone_number_compare_loose("+976 1 123 4567", "01 1 23 4567"));
    EXPECT_TRUE(phone_number_compare_loose("+976 2 234 5678", "02 2 34 5678"));
}

TEST(PhoneNumberUtils, compareLooseTrunkNetherlandsCities) {
    EXPECT_TRUE(phone_number_compare_loose("+31771234567", "0771234567"));
}

TEST(PhoneNumberUtils, compareLooseInternationalJapan) {
    EXPECT_FALSE(phone_number_compare_loose("+818012345678", "+819012345678"));
    EXPECT_TRUE(phone_number_compare_loose("+819012345678", "+819012345678"));
}

TEST(PhoneNumberUtils, compareLooseTrunkIgnoreJapan) {
    // Trunk prefix must not be ignored in Japan
    EXPECT_TRUE(phone_number_compare_loose("090-1234-5678", "90-1234-5678"));
    EXPECT_FALSE(phone_number_compare_loose("090-1234-5678", "080-1234-5678"));
    EXPECT_FALSE(phone_number_compare_loose("090-1234-5678", "190-1234-5678"));
    EXPECT_FALSE(phone_number_compare_loose("090-1234-5678", "890-1234-5678"));
    EXPECT_FALSE(phone_number_compare_loose("080-1234-5678", "+819012345678"));
    EXPECT_FALSE(phone_number_compare_loose("+81-90-1234-5678", "+81-090-1234-5678"));
}

TEST(PhoneNumberUtils, compareLooseInternationalNational) {
    EXPECT_TRUE(phone_number_compare_loose("+593(800)123-1234", "8001231234"));
}

TEST(PhoneNumberUtils, compareLooseTwoContinuousZeros) {
    // Two continuous 0 at the begining of the phone string should be
    // treated as trunk prefix for caller id purposes.
    EXPECT_TRUE(phone_number_compare_loose("008001231234", "8001231234"));
}

TEST(PhoneNumberUtils, compareLooseCallerIdThailandUs) {
    // Test broken caller ID seen on call from Thailand to the US
    EXPECT_TRUE(phone_number_compare_loose("+66811234567", "166811234567"));
    // Confirm that the bug found before does not re-appear.
    EXPECT_TRUE(phone_number_compare_loose("650-000-3456", "16500003456"));
    EXPECT_TRUE(phone_number_compare_loose("011 1 7005554141", "+17005554141"));
    EXPECT_FALSE(phone_number_compare_loose("011 11 7005554141", "+17005554141"));
    EXPECT_TRUE(phone_number_compare_loose("+44 207 792 3490", "00 207 792 3490"));
}

TEST(PhoneNumberUtils, compareLooseNamp1661) {
    // This is not related to Thailand case. NAMP "1" + region code "661".
    EXPECT_TRUE(phone_number_compare_loose("16610001234", "6610001234"));
}

TEST(PhoneNumberUtils, compareLooseAlphaDifferent) {
    // We also need to compare two alpha addresses to make sure two different strings
    // aren't treated as the same addresses. This is relevant to SMS as SMS sender may
    // contain all alpha chars.
    EXPECT_TRUE(phone_number_compare_loose("abcd", "bcde"));
}

TEST(PhoneNumberUtils, compareLooseAlphaNumericDifferent) {
    EXPECT_FALSE(phone_number_compare_loose("1-800-flowers", "800-flowers"));
    // TODO: "flowers" and "adcdefg" should not match
    //EXPECT_FALSE(phone_number_compare_loose("1-800-flowers", "1-800-abcdefg"));
}

// TODO: we currently do not support this comparison. It maybe nice to support this
// TODO: in the future.
//TEST(PhoneNumberUtils, compareLooseLettersToDigits) {
//  EXPECT_TRUE(phone_number_compare_loose("1-800-flowers", "1-800-356-9377"));
//}

TEST(PhoneNumberUtils, compareLooseWrongPrefix) {
    // Japan
    EXPECT_FALSE(phone_number_compare_loose("290-1234-5678", "+819012345678"));
    EXPECT_FALSE(phone_number_compare_loose("+819012345678", "290-1234-5678"));
    // USA
    EXPECT_TRUE(phone_number_compare_loose("0550-450-3605", "+15504503605"));
    EXPECT_FALSE(phone_number_compare_loose("550-450-3605", "+14504503605"));
    EXPECT_FALSE(phone_number_compare_loose("550-450-3605", "+15404503605"));
    EXPECT_FALSE(phone_number_compare_loose("550-450-3605", "+15514503605"));
    EXPECT_FALSE(phone_number_compare_loose("5504503605", "+14504503605"));

    EXPECT_FALSE(phone_number_compare_loose("+14504503605", "550-450-3605"));
    EXPECT_FALSE(phone_number_compare_loose("+15404503605", "550-450-3605"));
    EXPECT_FALSE(phone_number_compare_loose("+15514503605", "550-450-3605"));
    EXPECT_FALSE(phone_number_compare_loose("+14504503605", "5504503605"));
}


