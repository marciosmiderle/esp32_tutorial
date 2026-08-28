#pragma once

#include <Arduino.h>
#include <MD5Builder.h>
#include <mbedtls/sha256.h>

class CheckSum {
  String expectedHash;   // hex lowercase
  bool useSha256 = false;
  MD5Builder md5;
  mbedtls_sha256_context shaCtx;

public:
  void begin();
  void add(const uint8_t* buf, const size_t n);
  String getDigest();
  bool parseHash(String& body);
  bool getUseSha256();
  const String getExpectedHash();
  bool parseHashBody(const String& body);
  String toHex(const uint8_t* data, size_t len);
  static String deriveHashUrl(const String& binUrl, bool sha256);
};
