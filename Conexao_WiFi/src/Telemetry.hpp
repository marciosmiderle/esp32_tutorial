#pragma once

class Telemetry {
 unsigned int goodReads = 0;
 unsigned int badReads = 0;

public:
  typedef enum {
    GOOD = 0,
    ATENTION,
    BAD,
    FAIL
  } HealthStatus;

  void addGoodRead() { goodReads++; }

  void addBadRead() { badReads++; }

  unsigned int getGoodReads() const { return goodReads; }

  unsigned int getBadReads() const { return badReads; }

  const char* health() { return toString(healthStatus()); }

  HealthStatus healthStatus() {
    const float ratio = healthRatio();

    if (ratio >= 0.9) {
      return GOOD;
    } else if (ratio >= 0.5) {
      return ATENTION;
    } else if (ratio >= 0.2) {
      return BAD;
    } else {
      return FAIL;
    }
  }

  float healthRatio() { return badReads == 0 ? 1 : goodReads / badReads; }

  static const char* toString(HealthStatus h) {
    switch (h) {
    case GOOD:
      return "Good";
      break;

    case ATENTION: {
      return "Atention";
      break;
    }
    case BAD: {
      return "Bad";
      break;
    }
    case FAIL:
    default:
      return "Fail";
      break;
    }
  }
};
