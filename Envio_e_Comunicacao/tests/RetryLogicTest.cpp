#include "CppUTest/TestHarness.h"
#include "RetryLogic.hpp"
#include "mocks/millis.h"

TEST_GROUP(RetryLogicTest)
{
    const int maxRetries = 5;
    const unsigned long retryTimeoutMs = 3000;
    const unsigned long tryLaterTimeoutMs = 30000;

    RetryLogic retry{maxRetries, retryTimeoutMs, tryLaterTimeoutMs};

    void setup()
    {
        // O relógio do mock é global: sem isso um teste herda o tempo do anterior.
        millis_set(0);
    }

    // Avança o relógio e pergunta se pode tentar de novo.
    bool canRetryAfter(unsigned long ms)
    {
        millis_forward(ms);
        return retry.canRetry();
    }

    // Consome todo o orçamento de tentativas, respeitando o intervalo entre elas.
    void exhaustRetries()
    {
        for (int i = 0; i < maxRetries; i++) {
            CHECK_TRUE(canRetryAfter(retryTimeoutMs));
        }
    }

    // Esgota as tentativas e faz a chamada que empurra o RetryLogic para o tryLater.
    void enterTryLater()
    {
        exhaustRetries();
        CHECK_FALSE(canRetryAfter(retryTimeoutMs));
        CHECK_TRUE(retry.isInTryLater());
    }
};

TEST(RetryLogicTest, FirstAttemptWaitsForRetryTimeout)
{
    CHECK_FALSE(retry.canRetry());

    millis_forward(retryTimeoutMs - 1);
    CHECK_FALSE(retry.canRetry());

    millis_forward(1);
    CHECK_TRUE(retry.canRetry());
}

TEST(RetryLogicTest, NextAttemptWaitsForRetryTimeout)
{
    CHECK_TRUE(canRetryAfter(retryTimeoutMs));

    CHECK_FALSE(canRetryAfter(retryTimeoutMs - 1));
    CHECK_TRUE(canRetryAfter(1));
}

TEST(RetryLogicTest, AllowsExactlyMaxRetriesAttempts)
{
    exhaustRetries();

    CHECK_FALSE(retry.hasRetriesLeft());
    CHECK_FALSE(canRetryAfter(retryTimeoutMs));
}

TEST(RetryLogicTest, EntersTryLaterWhenRetriesAreExhausted)
{
    CHECK_FALSE(retry.isInTryLater());

    enterTryLater();
}

TEST(RetryLogicTest, StaysBlockedUntilTryLaterTimeoutElapses)
{
    enterTryLater();

    // Mesmo com o intervalo entre tentativas já vencido, o tryLater manda.
    CHECK_FALSE(canRetryAfter(tryLaterTimeoutMs - 1));
    CHECK_TRUE(retry.isInTryLater());
}

TEST(RetryLogicTest, ResumesWithFullBudgetAfterTryLaterExpires)
{
    enterTryLater();
    CHECK_FALSE(canRetryAfter(tryLaterTimeoutMs - 1));

    // Expirou: reseta o orçamento e já libera a primeira tentativa do novo ciclo.
    CHECK_TRUE(canRetryAfter(1));
    CHECK_FALSE(retry.isInTryLater());

    // Já gastou 1 do novo orçamento; devem sobrar exatamente maxRetries - 1.
    for (int i = 1; i < maxRetries; i++) {
        CHECK_TRUE(canRetryAfter(retryTimeoutMs));
    }
    CHECK_FALSE(retry.hasRetriesLeft());
}

TEST(RetryLogicTest, ResetRestoresRetryBudget)
{
    CHECK_TRUE(canRetryAfter(retryTimeoutMs));
    CHECK_TRUE(canRetryAfter(retryTimeoutMs));

    retry.reset();

    exhaustRetries();  // as maxRetries tentativas completas voltaram a existir
    CHECK_FALSE(retry.hasRetriesLeft());
}

TEST(RetryLogicTest, ResetCancelsTryLater)
{
    enterTryLater();

    retry.reset();

    CHECK_FALSE(retry.isInTryLater());
    CHECK_TRUE(retry.hasRetriesLeft());
}

TEST(RetryLogicTest, ZeroMaxRetriesGoesStraightToTryLater)
{
    RetryLogic noRetries(0, retryTimeoutMs, tryLaterTimeoutMs);

    CHECK_FALSE(noRetries.canRetry());
    CHECK_TRUE(noRetries.isInTryLater());
}
