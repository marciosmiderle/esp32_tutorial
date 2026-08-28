#include "CheckSum.hpp"

void CheckSum::begin() {
  if (useSha256) {
    mbedtls_sha256_init(&shaCtx);
    mbedtls_sha256_starts(&shaCtx, 0);
  } else {
    md5.begin();
  }
}

void CheckSum::add(const uint8_t* buf, const size_t n) {
  if (useSha256) {
    mbedtls_sha256_update(&shaCtx, buf, n);
  } else {
    md5.add(buf, n);
  }
}

String CheckSum::getDigest() {
  String actual;
  if (useSha256) {
    uint8_t digest[32];
    mbedtls_sha256_finish(&shaCtx, digest);
    mbedtls_sha256_free(&shaCtx);
    actual = toHex(digest, 32);
  } else {
    md5.calculate();
    actual = md5.toString();
    actual.toLowerCase();
  }
  return actual;
}

bool CheckSum::parseHash(String& body) {
  if (!parseHashBody(body)) {
    return false;
  }
  Serial.printf("[OTA] hash (%s): %s\n", useSha256 ? "sha256" : "md5",
		expectedHash.c_str());
  return true;
}

bool CheckSum::getUseSha256() {
  return useSha256;
}

const String CheckSum::getExpectedHash() {
  return expectedHash;
}

bool CheckSum::parseHashBody(const String& body) {
  String t = body;
  t.trim();
  t.toLowerCase();

  String hex;
  for (size_t i = 0; i < t.length(); ++i) {
    char c = t.charAt(i);
    const bool isHex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
    if (isHex) {
      hex += c;
    } else if (hex.length() == 32 || hex.length() == 64) {
      break;
    } else {
      hex = "";
    }
  }

  expectedHash = hex;
  if (hex.length() == 32) {
    useSha256 = false;
    return true;
  }
  if (hex.length() == 64) {
    useSha256 = true;
    return true;
  }
  return false;
}

String CheckSum::toHex(const uint8_t* data, size_t len) {
  static const char* kDigits = "0123456789abcdef";
  String s;
  s.reserve(len * 2);
  for (size_t i = 0; i < len; ++i) {
    s += kDigits[(data[i] >> 4) & 0x0F];
    s += kDigits[data[i] & 0x0F];
  }
  return s;
}

String CheckSum::deriveHashUrl(const String& binUrl, bool sha256) {
  String base = binUrl;
  base += sha256 ? ".sha256" : ".md5";
  return base;
}
