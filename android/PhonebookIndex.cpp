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

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include <unicode/alphaindex.h>
#include <unicode/ucol.h>
#include <unicode/uiter.h>
#include <unicode/ustring.h>
#include <unicode/utypes.h>
#include <unicode/uloc.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>

#include "PhonebookIndex.h"

#define MIN_OUTPUT_SIZE 6       // Minimum required size for the output buffer (in bytes)

namespace android {

// Wrapper class to enable using libutil SmartPointers with AlphabeticIndex.
class AlphabeticIndexRef : public RefBase {
public:
    AlphabeticIndexRef(const char *locale, UErrorCode &status) :
        m_index(locale, status), m_locale(NULL), m_isJapanese(false) {
        if (U_FAILURE(status)) {
            return;
        }
        m_locale = strdup(locale);
        if (m_locale == NULL) {
            status = U_MEMORY_ALLOCATION_ERROR;
            return;
        }
        char language[4];
        uloc_getLanguage(locale, language, sizeof(language), &status);
        if (U_FAILURE(status)) {
            return;
        }
        m_isJapanese = (strcmp(language, ULOC_JAPANESE) == 0);
    }
    virtual ~AlphabeticIndexRef() { free(m_locale); }

    AlphabeticIndex& operator*() { return m_index; }
    AlphabeticIndex* operator->() { return &m_index; }

    bool isLocale(const char *locale) const {
        return (locale != NULL && m_locale != NULL &&
                strcmp(m_locale, locale) == 0);
    }
    bool isJapanese() const { return m_isJapanese; }
    int32_t getLabel(int32_t bucketIndex, UChar *labelBuf, int32_t labelBufSize);

private:
    AlphabeticIndex m_index;
    char *m_locale;
    bool m_isJapanese;
};

int32_t AlphabeticIndexRef::getLabel(int32_t bucketIndex, UChar *labelBuf,
                                     int32_t labelBufSize) {
    UErrorCode status = U_ZERO_ERROR;
    m_index.resetBucketIterator(status);
    if (U_FAILURE(status)) {
        return -1;
    }
    for(int i = 0; i <= bucketIndex; ++i) {
        if (!m_index.nextBucket(status) || U_FAILURE(status)) {
            return -1;
        }
    }

    int32_t len;
    if (m_index.getBucketLabelType() == U_ALPHAINDEX_NORMAL) {
        len = m_index.getBucketLabel().extract(labelBuf, labelBufSize, status);
        if (U_FAILURE(status)) {
            return -1;
        }
    } else {
        // Use no label for underflow/inflow/overflow buckets
        labelBuf[0] = '\0';
        len = 0;
    }
    return len;
}

static Mutex gIndexMutex;
static sp<AlphabeticIndexRef> gIndex;

/**
 * Returns TRUE if the character belongs to a Hanzi unicode block
 */
static bool is_CJ(UChar32 c) {
    return (uscript_hasScript(c, USCRIPT_HAN) ||
            uscript_hasScript(c, USCRIPT_HIRAGANA) ||
            uscript_hasScript(c, USCRIPT_KATAKANA));
}

static bool initIndexForLocale(const char *locale) {
    if (locale == NULL) {
        return false;
    }

    if (gIndex != NULL && gIndex->isLocale(locale)) {
        return true;
    }

    UErrorCode status = U_ZERO_ERROR;
    sp<AlphabeticIndexRef> newIndex(new AlphabeticIndexRef(locale, status));
    if (newIndex == NULL || U_FAILURE(status)) {
        return false;
    }
    // Always create labels for Latin characters if not present in native set
    (*newIndex)->addLabels("en", status);
    if (U_FAILURE(status)) {
        return false;
    }
    if ((*newIndex)->getBucketCount(status) <= 0 || U_FAILURE(status)) {
        return false;
    }

    gIndex = newIndex;
    return true;
}

int32_t GetPhonebookIndex(UCharIterator *iter, const char *locale,
                          UChar *out, int32_t size, UBool *isError)
{
    if (size < MIN_OUTPUT_SIZE) {
        *isError = TRUE;
        return 0;
    }

    *isError = FALSE;
    out[0] = '\0';
    iter->move(iter, 0, UITER_ZERO);
    if (!iter->hasNext(iter)) {   // Empty input string
        return 0;
    }
    UnicodeString ustr;
    bool prefixIsNonNumeric = false;
    bool prefixIsNumeric = false;
    while (iter->hasNext(iter)) {
        UChar32 ch = uiter_next32(iter);
        // Ignore standard phone number separators and identify any string
        // that otherwise starts with a number.
        if (!prefixIsNumeric && !prefixIsNonNumeric) {
            if (u_isdigit(ch)) {
                prefixIsNumeric = true;
            } else if (!u_isspace(ch) && ch != '+' && ch != '(' &&
                       ch != ')' && ch != '.' && ch != '-' && ch != '#') {
                prefixIsNonNumeric = true;
            }
        }
        ustr.append(ch);
    }
    if (prefixIsNumeric) {
        out[0] = '#';
        return 1;
    }

    Mutex::Autolock autolock(gIndexMutex);
    if (!initIndexForLocale(locale)) {
        *isError = TRUE;
        return 0;
    }

    UErrorCode status = U_ZERO_ERROR;
    int32_t bucketIndex = (*gIndex)->getBucketIndex(ustr, status);
    if (U_FAILURE(status)) {
        *isError = TRUE;
        return 0;
    }

    int32_t len = gIndex->getLabel(bucketIndex, out, size);
    if (len < 0) {
        *isError = TRUE;
        return 0;
    }

    // For Japanese, label unclassified CJK ideographs with
    // Japanese word meaning "misc" or "other"
    if (gIndex->isJapanese() && len == 0 && is_CJ(ustr.char32At(0))) {
        out[0] = 0x4ED6;
        len = 1;
    }

    return len;
}

}  // namespace android
