/* vim:set ts=2 sw=2 sts=0 et: */
/* Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

#include "gtest/gtest.h"
#include "monkeycage/Conversions.h"
#include "monkeycage/SandboxStack.h"
#include "mozilla/Telemetry.h"
#include "TelemetryFixture.h"
#include "TelemetryTestHelpers.h"

using namespace mozilla;
using namespace TelemetryTestHelpers;

TEST_F(TelemetryTestFixture, AccumulateCountHistogram) {
  const uint32_t kExpectedValue = 200;
  AutoJSContextWithGlobal cx(mCleanGlobal);

  const char* telemetryTestCountName =
      Telemetry::GetHistogramName(Telemetry::TELEMETRY_TEST_COUNT);
  ASSERT_STREQ(telemetryTestCountName, "TELEMETRY_TEST_COUNT")
      << "The histogram name is wrong";

  GetAndClearHistogram(cx.GetJSContext(), mTelemetry, "TELEMETRY_TEST_COUNT"_ns,
                       false);

  // Accumulate in the histogram
  Telemetry::Accumulate(Telemetry::TELEMETRY_TEST_COUNT, kExpectedValue / 2);
  Telemetry::Accumulate("TELEMETRY_TEST_COUNT", kExpectedValue / 2);

  // Get a snapshot for all the histograms
  MC::Rooted<JS::Value> snapshot(cx.GetJSContext());
  GetSnapshots(cx.GetJSContext(), mTelemetry, telemetryTestCountName, &snapshot,
               false);

  // Get the histogram from the snapshot
  MC::Rooted<JS::Value> histogram(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), telemetryTestCountName, snapshot, &histogram);

  // Get "sum" property from histogram
  MC::Rooted<JS::Value> sum(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "sum", histogram, &sum);

  // Check that the "sum" stored in the histogram matches with |kExpectedValue|
  MC::SandboxStack<uint32_t> uSum = 0;
  JS::ToUint32(cx.GetJSContext(), sum, uSum);
  ASSERT_EQ(*uSum.UNSAFE_unverified(), kExpectedValue)
      << "The histogram is not returning expected value";
}

TEST_F(TelemetryTestFixture, AccumulateKeyedCountHistogram) {
  const uint32_t kExpectedValue = 100;
  AutoJSContextWithGlobal cx(mCleanGlobal);

  GetAndClearHistogram(cx.GetJSContext(), mTelemetry,
                       "TELEMETRY_TEST_KEYED_COUNT"_ns, true);

  // Accumulate data in the provided key within the histogram
  Telemetry::Accumulate(Telemetry::TELEMETRY_TEST_KEYED_COUNT, "sample"_ns,
                        kExpectedValue);

  // Get a snapshot for all the histograms
  MC::Rooted<JS::Value> snapshot(cx.GetJSContext());
  GetSnapshots(cx.GetJSContext(), mTelemetry, "TELEMETRY_TEST_KEYED_COUNT",
               &snapshot, true);

  // Get the histogram from the snapshot
  MC::Rooted<JS::Value> histogram(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "TELEMETRY_TEST_KEYED_COUNT", snapshot,
              &histogram);

  // Get "sample" property from histogram
  MC::Rooted<JS::Value> expectedKeyData(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "sample", histogram, &expectedKeyData);

  // Get "sum" property from keyed data
  MC::Rooted<JS::Value> sum(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "sum", expectedKeyData, &sum);

  // Check that the sum stored in the histogram matches with |kExpectedValue|
  MC::SandboxStack<uint32_t> uSum = 0;
  JS::ToUint32(cx.GetJSContext(), sum, uSum);
  ASSERT_EQ(*uSum.UNSAFE_unverified(), kExpectedValue)
      << "The histogram is not returning expected sum";
}

TEST_F(TelemetryTestFixture, TestKeyedKeysHistogram) {
  AutoJSContextWithGlobal cx(mCleanGlobal);

  MC::Rooted<JS::Value> testHistogram(cx.GetJSContext());
  MC::Rooted<JS::Value> rval(cx.GetJSContext());

  GetAndClearHistogram(cx.GetJSContext(), mTelemetry,
                       "TELEMETRY_TEST_KEYED_KEYS"_ns, true);

  // Test the accumulation on both the allowed and unallowed keys, using
  // the API that accepts histogram IDs.
  Telemetry::Accumulate(Telemetry::TELEMETRY_TEST_KEYED_KEYS, "not-allowed"_ns,
                        1);
  Telemetry::Accumulate(Telemetry::TELEMETRY_TEST_KEYED_KEYS, "testkey"_ns, 0);
  // Do the same, using the API that accepts the histogram name as a string.
  Telemetry::Accumulate("TELEMETRY_TEST_KEYED_KEYS", "not-allowed"_ns, 1);
  Telemetry::Accumulate("TELEMETRY_TEST_KEYED_KEYS", "CommonKey"_ns, 1);

  // Get a snapshot for all the histograms
  MC::Rooted<JS::Value> snapshot(cx.GetJSContext());
  GetSnapshots(cx.GetJSContext(), mTelemetry, "TELEMETRY_TEST_KEYED_KEYS",
               &snapshot, true);

  // Get the histogram from the snapshot
  MC::Rooted<JS::Value> histogram(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "TELEMETRY_TEST_KEYED_KEYS", snapshot,
              &histogram);

  // Get "testkey" property from histogram and check that it stores the correct
  // data.
  MC::Rooted<JS::Value> expectedKeyData(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "testkey", histogram, &expectedKeyData);
  ASSERT_TRUE(!expectedKeyData.isUndefined())
  << "Cannot find the expected key in the histogram data";
  MC::Rooted<JS::Value> sum(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "sum", expectedKeyData, &sum);
  MC::SandboxStack<uint32_t> uSum = 0;
  JS::ToUint32(cx.GetJSContext(), sum, uSum);
  ASSERT_EQ(*uSum.UNSAFE_unverified(), 0U)
      << "The histogram is not returning expected sum for 'testkey'";

  // Do the same for the "CommonKey" property.
  GetProperty(cx.GetJSContext(), "CommonKey", histogram, &expectedKeyData);
  ASSERT_TRUE(!expectedKeyData.isUndefined())
  << "Cannot find the expected key in the histogram data";
  GetProperty(cx.GetJSContext(), "sum", expectedKeyData, &sum);
  JS::ToUint32(cx.GetJSContext(), sum, uSum);
  ASSERT_EQ(*uSum.UNSAFE_unverified(), 1U)
      << "The histogram is not returning expected sum for 'CommonKey'";

  GetProperty(cx.GetJSContext(), "not-allowed", histogram, &expectedKeyData);
  ASSERT_TRUE(expectedKeyData.isUndefined())
  << "Unallowed keys must not be recorded in the histogram data";

  // The 'not-allowed' key accumulation for 'TELEMETRY_TESTED_KEYED_KEYS' was
  // attemtped twice, so we expect the count of
  // 'telemetry.accumulate_unknown_histogram_keys' to be 2
  const uint32_t expectedAccumulateUnknownCount = 2;
  MC::Rooted<JS::Value> scalarsSnapshot(cx.GetJSContext());
  GetScalarsSnapshot(true, cx.GetJSContext(), &scalarsSnapshot);
  CheckKeyedUintScalar("telemetry.accumulate_unknown_histogram_keys",
                       "TELEMETRY_TEST_KEYED_KEYS", cx.GetJSContext(),
                       scalarsSnapshot, expectedAccumulateUnknownCount);
}

TEST_F(TelemetryTestFixture, AccumulateCategoricalHistogram) {
  const uint32_t kExpectedValue = 2;

  AutoJSContextWithGlobal cx(mCleanGlobal);

  GetAndClearHistogram(cx.GetJSContext(), mTelemetry,
                       "TELEMETRY_TEST_CATEGORICAL"_ns, false);

  // Accumulate one unit into the categorical histogram with label
  // Telemetry::LABELS_TELEMETRY_TEST_CATEGORICAL::CommonLabel
  Telemetry::AccumulateCategorical(
      Telemetry::LABELS_TELEMETRY_TEST_CATEGORICAL::CommonLabel);

  // Accumulate another unit into the same categorical histogram using a string
  // label
  Telemetry::AccumulateCategorical(Telemetry::TELEMETRY_TEST_CATEGORICAL,
                                   "CommonLabel"_ns);

  // Get a snapshot for all the histograms
  MC::Rooted<JS::Value> snapshot(cx.GetJSContext());
  GetSnapshots(cx.GetJSContext(), mTelemetry, "TELEMETRY_TEST_CATEGORICAL",
               &snapshot, false);

  // Get our histogram from the snapshot
  MC::Rooted<JS::Value> histogram(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "TELEMETRY_TEST_CATEGORICAL", snapshot,
              &histogram);

  // Get values object from histogram. Each entry in the object maps to a label
  // in the histogram.
  MC::Rooted<JS::Value> values(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "values", histogram, &values);

  // Get the value for the label we care about
  MC::Rooted<JS::Value> value(cx.GetJSContext());
  GetElement(cx.GetJSContext(),
             static_cast<uint32_t>(
                 Telemetry::LABELS_TELEMETRY_TEST_CATEGORICAL::CommonLabel),
             values, &value);

  // Check that the value stored in the histogram matches with |kExpectedValue|
  MC::SandboxStack<uint32_t> uValue = 0;
  JS::ToUint32(cx.GetJSContext(), value, uValue);
  ASSERT_EQ(*uValue.UNSAFE_unverified(), kExpectedValue)
      << "The histogram is not returning expected value";
}

TEST_F(TelemetryTestFixture, AccumulateKeyedCategoricalHistogram) {
  const uint32_t kSampleExpectedValue = 2;
  const uint32_t kOtherSampleExpectedValue = 1;

  AutoJSContextWithGlobal cx(mCleanGlobal);

  GetAndClearHistogram(cx.GetJSContext(), mTelemetry,
                       "TELEMETRY_TEST_KEYED_CATEGORICAL"_ns, true);

  // Accumulate one unit into the categorical histogram with label
  // Telemetry::LABELS_TELEMETRY_TEST_KEYED_CATEGORICAL::CommonLabel
  Telemetry::AccumulateCategoricalKeyed(
      "sample"_ns,
      Telemetry::LABELS_TELEMETRY_TEST_KEYED_CATEGORICAL::CommonLabel);
  // Accumulate another unit into the same categorical histogram
  Telemetry::AccumulateCategoricalKeyed(
      "sample"_ns,
      Telemetry::LABELS_TELEMETRY_TEST_KEYED_CATEGORICAL::CommonLabel);
  // Accumulate another unit into a different categorical histogram
  Telemetry::AccumulateCategoricalKeyed(
      "other-sample"_ns,
      Telemetry::LABELS_TELEMETRY_TEST_KEYED_CATEGORICAL::CommonLabel);

  // Get a snapshot for all the histograms
  MC::Rooted<JS::Value> snapshot(cx.GetJSContext());
  GetSnapshots(cx.GetJSContext(), mTelemetry,
               "TELEMETRY_TEST_KEYED_CATEGORICAL", &snapshot, true);
  // Get the histogram from the snapshot
  MC::Rooted<JS::Value> histogram(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "TELEMETRY_TEST_KEYED_CATEGORICAL", snapshot,
              &histogram);

  // Check that the sample histogram contains the values we expect
  MC::Rooted<JS::Value> sample(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "sample", histogram, &sample);
  // Get values object from sample. Each entry in the object maps to a label in
  // the histogram.
  MC::Rooted<JS::Value> sampleValues(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "values", sample, &sampleValues);
  // Get the value for the label we care about
  MC::Rooted<JS::Value> sampleValue(cx.GetJSContext());
  GetElement(
      cx.GetJSContext(),
      static_cast<uint32_t>(
          Telemetry::LABELS_TELEMETRY_TEST_KEYED_CATEGORICAL::CommonLabel),
      sampleValues, &sampleValue);
  // Check that the value stored in the histogram matches with
  // |kSampleExpectedValue|
  MC::SandboxStack<uint32_t> uSampleValue = 0;
  JS::ToUint32(cx.GetJSContext(), sampleValue, uSampleValue);
  ASSERT_EQ(*uSampleValue.UNSAFE_unverified(), kSampleExpectedValue)
      << "The sample histogram is not returning expected value";

  // Check that the other-sample histogram contains the values we expect
  MC::Rooted<JS::Value> otherSample(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "other-sample", histogram, &otherSample);
  // Get values object from the other-sample. Each entry in the object maps to a
  // label in the histogram.
  MC::Rooted<JS::Value> otherValues(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "values", otherSample, &otherValues);
  // Get the value for the label we care about
  MC::Rooted<JS::Value> otherValue(cx.GetJSContext());
  GetElement(
      cx.GetJSContext(),
      static_cast<uint32_t>(
          Telemetry::LABELS_TELEMETRY_TEST_KEYED_CATEGORICAL::CommonLabel),
      otherValues, &otherValue);
  // Check that the value stored in the histogram matches with
  // |kOtherSampleExpectedValue|
  MC::SandboxStack<uint32_t> uOtherValue = 0;
  JS::ToUint32(cx.GetJSContext(), otherValue, uOtherValue);
  ASSERT_EQ(*uOtherValue.UNSAFE_unverified(), kOtherSampleExpectedValue)
      << "The other-sample histogram is not returning expected value";
}

TEST_F(TelemetryTestFixture, AccumulateCountHistogram_MultipleSamples) {
  nsTArray<uint32_t> samples({4, 4, 4});
  const uint32_t kExpectedSum = 12;

  AutoJSContextWithGlobal cx(mCleanGlobal);

  GetAndClearHistogram(cx.GetJSContext(), mTelemetry, "TELEMETRY_TEST_COUNT"_ns,
                       false);

  // Accumulate in histogram
  Telemetry::Accumulate(Telemetry::TELEMETRY_TEST_COUNT, samples);

  // Get a snapshot of all the histograms
  MC::Rooted<JS::Value> snapshot(cx.GetJSContext());
  GetSnapshots(cx.GetJSContext(), mTelemetry, "TELEMETRY_TEST_COUNT", &snapshot,
               false);

  // Get histogram from snapshot
  MC::Rooted<JS::Value> histogram(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "TELEMETRY_TEST_COUNT", snapshot, &histogram);

  // Get "sum" from histogram
  MC::Rooted<JS::Value> sum(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "sum", histogram, &sum);

  // Check that sum matches with aValue
  MC::SandboxStack<uint32_t> uSum = 0;
  JS::ToUint32(cx.GetJSContext(), sum, uSum);
  ASSERT_EQ(*uSum.UNSAFE_unverified(), kExpectedSum)
      << "This histogram is not returning expected value";
}

TEST_F(TelemetryTestFixture, AccumulateLinearHistogram_MultipleSamples) {
  nsTArray<uint32_t> samples({4, 4, 4});
  const uint32_t kExpectedCount = 3;

  AutoJSContextWithGlobal cx(mCleanGlobal);

  GetAndClearHistogram(cx.GetJSContext(), mTelemetry,
                       "TELEMETRY_TEST_LINEAR"_ns, false);

  // Accumulate in the histogram
  Telemetry::Accumulate(Telemetry::TELEMETRY_TEST_LINEAR, samples);

  // Get a snapshot of all the histograms
  MC::Rooted<JS::Value> snapshot(cx.GetJSContext());
  GetSnapshots(cx.GetJSContext(), mTelemetry, "TELEMETRY_TEST_LINEAR",
               &snapshot, false);

  // Get histogram from snapshot
  MC::Rooted<JS::Value> histogram(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "TELEMETRY_TEST_LINEAR", snapshot, &histogram);

  // Get "values" object from histogram
  MC::Rooted<JS::Value> values(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "values", histogram, &values);

  // Index 0 is only for values less than 'low'. Values within range start at
  // index 1
  MC::Rooted<JS::Value> count(cx.GetJSContext());
  const uint32_t index = 1;
  GetElement(cx.GetJSContext(), index, values, &count);

  // Check that this count matches with nSamples
  MC::SandboxStack<uint32_t> uCount = 0;
  JS::ToUint32(cx.GetJSContext(), count, uCount);
  ASSERT_EQ(*uCount.UNSAFE_unverified(), kExpectedCount)
      << "The histogram did not accumulate the correct number of values";
}

TEST_F(TelemetryTestFixture, AccumulateLinearHistogram_DifferentSamples) {
  nsTArray<uint32_t> samples(
      {4, 8, 2147483646, uint32_t(INT_MAX) + 1, UINT32_MAX});

  AutoJSContextWithGlobal cx(mCleanGlobal);

  mTelemetry->ClearScalars();
  GetAndClearHistogram(cx.GetJSContext(), mTelemetry,
                       "TELEMETRY_TEST_LINEAR"_ns, false);

  // Accumulate in histogram
  Telemetry::Accumulate(Telemetry::TELEMETRY_TEST_LINEAR, samples);

  // Get a snapshot of all histograms
  MC::Rooted<JS::Value> snapshot(cx.GetJSContext());
  GetSnapshots(cx.GetJSContext(), mTelemetry, "TELEMETRY_TEST_LINEAR",
               &snapshot, false);

  // Get histogram from snapshot
  MC::Rooted<JS::Value> histogram(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "TELEMETRY_TEST_LINEAR", snapshot, &histogram);

  // Get values object from histogram
  MC::Rooted<JS::Value> values(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "values", histogram, &values);

  // Get values in first and last buckets
  MC::Rooted<JS::Value> countFirst(cx.GetJSContext());
  MC::Rooted<JS::Value> countLast(cx.GetJSContext());
  const uint32_t firstIndex = 1;
  // Buckets are indexed by their start value
  const uint32_t lastIndex = INT32_MAX - 1;
  GetElement(cx.GetJSContext(), firstIndex, values, &countFirst);
  GetElement(cx.GetJSContext(), lastIndex, values, &countLast);

  // Check that the values match
  MC::SandboxStack<uint32_t> uCountFirst = 0;
  MC::SandboxStack<uint32_t> uCountLast = 0;
  JS::ToUint32(cx.GetJSContext(), countFirst, uCountFirst);
  JS::ToUint32(cx.GetJSContext(), countLast, uCountLast);

  const uint32_t kExpectedCountFirst = 2;
  // We expect 2147483646 to be in the last bucket, as well the two samples
  // above 2^31 (prior to bug 1438335, values between INT_MAX and UINT32_MAX
  // would end up as 0s)
  const uint32_t kExpectedCountLast = 3;
  ASSERT_EQ(*uCountFirst.UNSAFE_unverified(), kExpectedCountFirst)
      << "The first bucket did not accumulate the correct number of values";
  ASSERT_EQ(*uCountLast.UNSAFE_unverified(), kExpectedCountLast)
      << "The last bucket did not accumulate the correct number of values";

  // We accumulated two values that had to be clamped. We expect the count in
  // 'telemetry.accumulate_clamped_values' to be 2 (only one storage).
  const uint32_t expectedAccumulateClampedCount = 2;
  MC::Rooted<JS::Value> scalarsSnapshot(cx.GetJSContext());
  GetScalarsSnapshot(true, cx.GetJSContext(), &scalarsSnapshot);
  CheckKeyedUintScalar("telemetry.accumulate_clamped_values",
                       "TELEMETRY_TEST_LINEAR", cx.GetJSContext(),
                       scalarsSnapshot, expectedAccumulateClampedCount);
}

TEST_F(TelemetryTestFixture, AccumulateKeyedCountHistogram_MultipleSamples) {
  const nsTArray<uint32_t> samples({5, 10, 15});
  const uint32_t kExpectedSum = 5 + 10 + 15;

  AutoJSContextWithGlobal cx(mCleanGlobal);

  GetAndClearHistogram(cx.GetJSContext(), mTelemetry,
                       "TELEMETRY_TEST_KEYED_COUNT"_ns, true);

  // Accumulate data in the provided key within the histogram
  Telemetry::Accumulate(Telemetry::TELEMETRY_TEST_KEYED_COUNT, "sample"_ns,
                        samples);

  // Get a snapshot for all the histograms
  MC::Rooted<JS::Value> snapshot(cx.GetJSContext());
  GetSnapshots(cx.GetJSContext(), mTelemetry, "TELEMETRY_TEST_KEYED_COUNT",
               &snapshot, true);

  // Get the histogram from the snapshot
  MC::Rooted<JS::Value> histogram(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "TELEMETRY_TEST_KEYED_COUNT", snapshot,
              &histogram);

  // Get "sample" property from histogram
  MC::Rooted<JS::Value> expectedKeyData(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "sample", histogram, &expectedKeyData);

  // Get "sum" property from keyed data
  MC::Rooted<JS::Value> sum(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "sum", expectedKeyData, &sum);

  // Check that the sum stored in the histogram matches with |kExpectedSum|
  MC::SandboxStack<uint32_t> uSum = 0;
  JS::ToUint32(cx.GetJSContext(), sum, uSum);
  ASSERT_EQ(*uSum.UNSAFE_unverified(), kExpectedSum)
      << "The histogram is not returning expected sum";
}

TEST_F(TelemetryTestFixture, TestKeyedLinearHistogram_MultipleSamples) {
  AutoJSContextWithGlobal cx(mCleanGlobal);

  mTelemetry->ClearScalars();
  GetAndClearHistogram(cx.GetJSContext(), mTelemetry,
                       "TELEMETRY_TEST_KEYED_LINEAR"_ns, true);

  const nsTArray<uint32_t> samples({1, 5, 250000, UINT_MAX});
  // Test the accumulation on the key 'testkey', using
  // the API that accepts histogram IDs.
  Telemetry::Accumulate(Telemetry::TELEMETRY_TEST_KEYED_LINEAR, "testkey"_ns,
                        samples);

  // Get a snapshot for all the histograms
  MC::Rooted<JS::Value> snapshot(cx.GetJSContext());
  GetSnapshots(cx.GetJSContext(), mTelemetry, "TELEMETRY_TEST_KEYED_LINEAR",
               &snapshot, true);

  // Get the histogram from the snapshot
  MC::Rooted<JS::Value> histogram(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "TELEMETRY_TEST_KEYED_LINEAR", snapshot,
              &histogram);

  // Get "testkey" property from histogram.
  MC::Rooted<JS::Value> expectedKeyData(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "testkey", histogram, &expectedKeyData);
  ASSERT_TRUE(!expectedKeyData.isUndefined())
  << "Cannot find the expected key in the histogram data";

  // Get values object from 'testkey' histogram.
  MC::Rooted<JS::Value> values(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "values", expectedKeyData, &values);

  // Get values in first and last buckets.
  MC::Rooted<JS::Value> countFirst(cx.GetJSContext());
  MC::Rooted<JS::Value> countLast(cx.GetJSContext());
  const uint32_t firstIndex = 1;
  // Buckets are indexed by their start value
  const uint32_t lastIndex = 250000;
  GetElement(cx.GetJSContext(), firstIndex, values, &countFirst);
  GetElement(cx.GetJSContext(), lastIndex, values, &countLast);

  // Check that the values match.
  MC::SandboxStack<uint32_t> uCountFirst = 0;
  MC::SandboxStack<uint32_t> uCountLast = 0;
  JS::ToUint32(cx.GetJSContext(), countFirst, uCountFirst);
  JS::ToUint32(cx.GetJSContext(), countLast, uCountLast);

  const uint32_t kExpectedCountFirst = 2;
  const uint32_t kExpectedCountLast = 2;
  ASSERT_EQ(*uCountFirst.UNSAFE_unverified(), kExpectedCountFirst)
      << "The first bucket did not accumulate the correct number of values for "
         "key 'testkey'";
  ASSERT_EQ(*uCountLast.UNSAFE_unverified(), kExpectedCountLast)
      << "The last bucket did not accumulate the correct number of values for "
         "key 'testkey'";

  // We accumulated one keyed values that had to be clamped. We expect the
  // count in 'telemetry.accumulate_clamped_values' to be 1
  const uint32_t expectedAccumulateClampedCount = 1;
  MC::Rooted<JS::Value> scalarsSnapshot(cx.GetJSContext());
  GetScalarsSnapshot(true, cx.GetJSContext(), &scalarsSnapshot);
  CheckKeyedUintScalar("telemetry.accumulate_clamped_values",
                       "TELEMETRY_TEST_KEYED_LINEAR", cx.GetJSContext(),
                       scalarsSnapshot, expectedAccumulateClampedCount);
}

TEST_F(TelemetryTestFixture, TestKeyedKeysHistogram_MultipleSamples) {
  AutoJSContextWithGlobal cx(mCleanGlobal);
  mTelemetry->ClearScalars();
  const nsTArray<uint32_t> samples({false, false, true, 32, true});

  GetAndClearHistogram(cx.GetJSContext(), mTelemetry,
                       "TELEMETRY_TEST_KEYED_KEYS"_ns, true);

  // Test the accumulation on both the allowed and unallowed keys, using
  // the API that accepts histogram IDs.
  Telemetry::Accumulate(Telemetry::TELEMETRY_TEST_KEYED_KEYS, "not-allowed"_ns,
                        samples);
  Telemetry::Accumulate(Telemetry::TELEMETRY_TEST_KEYED_KEYS, "testkey"_ns,
                        samples);

  // Get a snapshot for all the histograms
  MC::Rooted<JS::Value> snapshot(cx.GetJSContext());
  GetSnapshots(cx.GetJSContext(), mTelemetry, "TELEMETRY_TEST_KEYED_KEYS",
               &snapshot, true);

  // Get the histogram from the snapshot
  MC::Rooted<JS::Value> histogram(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "TELEMETRY_TEST_KEYED_KEYS", snapshot,
              &histogram);

  // Get "testkey" property from histogram and check that it stores the correct
  // data.
  MC::Rooted<JS::Value> testKeyData(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "testkey", histogram, &testKeyData);
  ASSERT_TRUE(!testKeyData.isUndefined())
  << "Cannot find the key 'testkey' in the histogram data";

  MC::Rooted<JS::Value> values(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "values", testKeyData, &values);

  // Get values in buckets 0,1,2
  const uint32_t falseIndex = 0;
  const uint32_t trueIndex = 1;
  const uint32_t otherIndex = 2;

  MC::Rooted<JS::Value> countFalse(cx.GetJSContext());
  MC::Rooted<JS::Value> countTrue(cx.GetJSContext());
  MC::Rooted<JS::Value> countOther(cx.GetJSContext());

  GetElement(cx.GetJSContext(), falseIndex, values, &countFalse);
  GetElement(cx.GetJSContext(), trueIndex, values, &countTrue);
  GetElement(cx.GetJSContext(), otherIndex, values, &countOther);

  MC::SandboxStack<uint32_t> uCountFalse = 0;
  MC::SandboxStack<uint32_t> uCountTrue = 0;
  MC::SandboxStack<uint32_t> uCountOther = 0;
  JS::ToUint32(cx.GetJSContext(), countFalse, uCountFalse);
  JS::ToUint32(cx.GetJSContext(), countTrue, uCountTrue);
  JS::ToUint32(cx.GetJSContext(), countOther, uCountOther);

  const uint32_t kExpectedCountFalse = 2;
  const uint32_t kExpectedCountTrue = 3;
  const uint32_t kExpectedCountOther = 0;

  ASSERT_EQ(*uCountFalse.UNSAFE_unverified(), kExpectedCountFalse)
      << "The histogram did not accumulate the correct number of 'false' "
         "booleans for key 'testkey'";
  ASSERT_EQ(*uCountTrue.UNSAFE_unverified(), kExpectedCountTrue)
      << "The histogram did not accumulate the correct number of 'true' "
         "booleans for key 'testkey'";
  ASSERT_EQ(*uCountOther.UNSAFE_unverified(), kExpectedCountOther)
      << "The histogram did not accumulate the correct number of undefined "
         "values for key 'testkey'";

  // Here we check that we are not accumulating to a different (but still
  // 'allowed') key. Get "CommonKey" property from histogram and check that it
  // has no data. Since we accumulated no data to it, commonKeyData should be
  // undefined.
  MC::Rooted<JS::Value> commonKeyData(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "CommonKey", histogram, &commonKeyData);
  ASSERT_TRUE(commonKeyData.isUndefined())
  << "Found data in key 'CommonKey' even though we accumulated no data to "
     "it";

  // Here we check that our function does not allow accumulation into unallowed
  // keys. Get 'not-allowed' property from histogram and check that this also
  // has no data. This should contain no data because this key is not allowed.
  MC::Rooted<JS::Value> notAllowedKeyData(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "not-allowed", histogram, &notAllowedKeyData);
  ASSERT_TRUE(notAllowedKeyData.isUndefined())
  << "Found data in key 'not-allowed' even though accumuling data to it is "
     "not allowed";

  // The 'not-allowed' key accumulation for 'TELEMETRY_TESTED_KEYED_KEYS' was
  // attemtped once, so we expect the count of
  // 'telemetry.accumulate_unknown_histogram_keys' to be 1
  const uint32_t expectedAccumulateUnknownCount = 1;
  MC::Rooted<JS::Value> scalarsSnapshot(cx.GetJSContext());
  GetScalarsSnapshot(true, cx.GetJSContext(), &scalarsSnapshot);
  CheckKeyedUintScalar("telemetry.accumulate_unknown_histogram_keys",
                       "TELEMETRY_TEST_KEYED_KEYS", cx.GetJSContext(),
                       scalarsSnapshot, expectedAccumulateUnknownCount);
}

TEST_F(TelemetryTestFixture,
       AccumulateCategoricalHistogram_MultipleStringLabels) {
  const uint32_t kExpectedValue = 2;
  const nsTArray<nsCString> labels({"CommonLabel"_ns, "CommonLabel"_ns});
  AutoJSContextWithGlobal cx(mCleanGlobal);

  GetAndClearHistogram(cx.GetJSContext(), mTelemetry,
                       "TELEMETRY_TEST_CATEGORICAL"_ns, false);

  // Accumulate the units into a categorical histogram using a string label
  Telemetry::AccumulateCategorical(Telemetry::TELEMETRY_TEST_CATEGORICAL,
                                   labels);

  // Get a snapshot for all the histograms
  MC::Rooted<JS::Value> snapshot(cx.GetJSContext());
  GetSnapshots(cx.GetJSContext(), mTelemetry, "TELEMETRY_TEST_CATEGORICAL",
               &snapshot, false);

  // Get our histogram from the snapshot
  MC::Rooted<JS::Value> histogram(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "TELEMETRY_TEST_CATEGORICAL", snapshot,
              &histogram);

  // Get values object from histogram. Each entry in the object maps to a label
  // in the histogram.
  MC::Rooted<JS::Value> values(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "values", histogram, &values);

  // Get the value for the label we care about
  MC::Rooted<JS::Value> value(cx.GetJSContext());
  GetElement(cx.GetJSContext(),
             static_cast<uint32_t>(
                 Telemetry::LABELS_TELEMETRY_TEST_CATEGORICAL::CommonLabel),
             values, &value);

  // Check that the value stored in the histogram matches with |kExpectedValue|
  MC::SandboxStack<uint32_t> uValue = 0;
  JS::ToUint32(cx.GetJSContext(), value, uValue);
  ASSERT_EQ(*uValue.UNSAFE_unverified(), kExpectedValue)
      << "The histogram is not returning expected value";

  // Now we check for no accumulation when a bad label is present in the array.
  //
  // The 'values' property is not initialized unless data is accumulated so
  // keeping another test to check for this case alone is wasteful as we will
  // have to accumulate some data anyway.

  const nsTArray<nsCString> badLabelArray({"CommonLabel"_ns, "BadLabel"_ns});

  // Try to accumulate the array into the histogram.
  Telemetry::AccumulateCategorical(Telemetry::TELEMETRY_TEST_CATEGORICAL,
                                   badLabelArray);

  // Get snapshot of all the histograms
  GetSnapshots(cx.GetJSContext(), mTelemetry, "TELEMETRY_TEST_CATEGORICAL",
               &snapshot, false);

  // Get our histogram from the snapshot
  GetProperty(cx.GetJSContext(), "TELEMETRY_TEST_CATEGORICAL", snapshot,
              &histogram);

  // Get values array from histogram
  GetProperty(cx.GetJSContext(), "values", histogram, &values);

  // Get the value for the label we care about
  GetElement(cx.GetJSContext(),
             static_cast<uint32_t>(
                 Telemetry::LABELS_TELEMETRY_TEST_CATEGORICAL::CommonLabel),
             values, &value);

  // Check that the value stored in the histogram matches with |kExpectedValue|
  *uValue = 0;
  JS::ToUint32(cx.GetJSContext(), value, uValue);
  ASSERT_EQ(*uValue.UNSAFE_unverified(), kExpectedValue)
      << "The histogram accumulated data when it should not have";
}

TEST_F(TelemetryTestFixture,
       AccumulateCategoricalHistogram_MultipleEnumValues) {
  const uint32_t kExpectedValue = 2;
  const nsTArray<Telemetry::LABELS_TELEMETRY_TEST_CATEGORICAL> enumLabels(
      {Telemetry::LABELS_TELEMETRY_TEST_CATEGORICAL::CommonLabel,
       Telemetry::LABELS_TELEMETRY_TEST_CATEGORICAL::CommonLabel});

  AutoJSContextWithGlobal cx(mCleanGlobal);

  GetAndClearHistogram(cx.GetJSContext(), mTelemetry,
                       "TELEMETRY_TEST_CATEGORICAL"_ns, false);

  // Accumulate the units into a categorical histogram using the enumLabels
  // array
  Telemetry::AccumulateCategorical<
      Telemetry::LABELS_TELEMETRY_TEST_CATEGORICAL>(enumLabels);

  // Get a snapshot for all the histograms
  MC::Rooted<JS::Value> snapshot(cx.GetJSContext());
  GetSnapshots(cx.GetJSContext(), mTelemetry, "TELEMETRY_TEST_CATEGORICAL",
               &snapshot, false);

  // Get our histogram from the snapshot
  MC::Rooted<JS::Value> histogram(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "TELEMETRY_TEST_CATEGORICAL", snapshot,
              &histogram);

  // Get values object from histogram. Each entry in the object maps to a label
  // in the histogram.
  MC::Rooted<JS::Value> values(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "values", histogram, &values);

  // Get the value for the label we care about
  MC::Rooted<JS::Value> value(cx.GetJSContext());
  GetElement(cx.GetJSContext(),
             static_cast<uint32_t>(
                 Telemetry::LABELS_TELEMETRY_TEST_CATEGORICAL::CommonLabel),
             values, &value);

  // Check that the value stored in the histogram matches with |kExpectedValue|
  MC::SandboxStack<uint32_t> uValue = 0;
  JS::ToUint32(cx.GetJSContext(), value, uValue);
  ASSERT_EQ(*uValue.UNSAFE_unverified(), kExpectedValue)
      << "The histogram is not returning expected value";
}

TEST_F(TelemetryTestFixture,
       AccumulateKeyedCategoricalHistogram_MultipleEnumValues) {
  const uint32_t kExpectedCommonLabel = 2;
  const uint32_t kExpectedLabel2 = 1;
  const nsTArray<Telemetry::LABELS_TELEMETRY_TEST_KEYED_CATEGORICAL> enumLabels(
      {Telemetry::LABELS_TELEMETRY_TEST_KEYED_CATEGORICAL::CommonLabel,
       Telemetry::LABELS_TELEMETRY_TEST_KEYED_CATEGORICAL::CommonLabel,
       Telemetry::LABELS_TELEMETRY_TEST_KEYED_CATEGORICAL::Label2});

  AutoJSContextWithGlobal cx(mCleanGlobal);

  GetAndClearHistogram(cx.GetJSContext(), mTelemetry,
                       "TELEMETRY_TEST_KEYED_CATEGORICAL"_ns, true);

  // Accumulate the array into the categorical keyed histogram
  Telemetry::AccumulateCategoricalKeyed("sampleKey"_ns, enumLabels);

  // Get a snapshot for all the histograms
  MC::Rooted<JS::Value> snapshot(cx.GetJSContext());
  GetSnapshots(cx.GetJSContext(), mTelemetry,
               "TELEMETRY_TEST_KEYED_CATEGORICAL", &snapshot, true);

  // Get the histogram from the snapshot
  MC::Rooted<JS::Value> histogram(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "TELEMETRY_TEST_KEYED_CATEGORICAL", snapshot,
              &histogram);

  // Check that the sampleKey histogram contains correct number of CommonLabel
  // samples
  MC::Rooted<JS::Value> sample(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "sampleKey", histogram, &sample);

  // Get values object from the sample. Each entry in the object maps to a label
  // in the histogram.
  MC::Rooted<JS::Value> sampleKeyValues(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "values", sample, &sampleKeyValues);

  // Get the count of CommonLabel
  MC::Rooted<JS::Value> commonLabelValue(cx.GetJSContext());
  GetElement(
      cx.GetJSContext(),
      static_cast<uint32_t>(
          Telemetry::LABELS_TELEMETRY_TEST_KEYED_CATEGORICAL::CommonLabel),
      sampleKeyValues, &commonLabelValue);

  // Check that the value stored in the histogram matches with
  // |kExpectedCommonLabel|
  MC::SandboxStack<uint32_t> uCommonLabelValue = 0;
  JS::ToUint32(cx.GetJSContext(), commonLabelValue, uCommonLabelValue);
  ASSERT_EQ(*uCommonLabelValue.UNSAFE_unverified(), kExpectedCommonLabel)
      << "The sampleKey histogram did not accumulate the correct number of "
         "CommonLabel samples";

  // Check that the sampleKey histogram contains the correct number of Label2
  // values Get the count of Label2
  MC::Rooted<JS::Value> label2Value(cx.GetJSContext());
  GetElement(cx.GetJSContext(),
             static_cast<uint32_t>(
                 Telemetry::LABELS_TELEMETRY_TEST_KEYED_CATEGORICAL::Label2),
             sampleKeyValues, &label2Value);

  // Check that the value stored in the histogram matches with |kExpectedLabel2|
  MC::SandboxStack<uint32_t> uLabel2Value = 0;
  JS::ToUint32(cx.GetJSContext(), label2Value, uLabel2Value);
  ASSERT_EQ(*uLabel2Value.UNSAFE_unverified(), kExpectedLabel2)
      << "The sampleKey histogram did not accumulate the correct number of "
         "Label2 samples";
}

TEST_F(TelemetryTestFixture, AccumulateTimeDelta) {
  const uint32_t kExpectedValue = 100;
  const TimeStamp start = TimeStamp::Now();
  const TimeDuration delta = TimeDuration::FromMilliseconds(50);

  AutoJSContextWithGlobal cx(mCleanGlobal);

  GetAndClearHistogram(cx.GetJSContext(), mTelemetry, "TELEMETRY_TEST_COUNT"_ns,
                       false);

  // Accumulate in the histogram
  Telemetry::AccumulateTimeDelta(Telemetry::TELEMETRY_TEST_COUNT, start - delta,
                                 start);

  Telemetry::AccumulateTimeDelta(Telemetry::TELEMETRY_TEST_COUNT, start - delta,
                                 start);

  Telemetry::AccumulateTimeDelta(Telemetry::TELEMETRY_TEST_COUNT, start, start);

  // end > start timestamp gives zero contribution
  Telemetry::AccumulateTimeDelta(Telemetry::TELEMETRY_TEST_COUNT, start + delta,
                                 start);

  // Get a snapshot for all the histograms
  MC::Rooted<JS::Value> snapshot(cx.GetJSContext());
  GetSnapshots(cx.GetJSContext(), mTelemetry, "TELEMETRY_TEST_COUNT", &snapshot,
               false);

  // Get the histogram from the snapshot
  MC::Rooted<JS::Value> histogram(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "TELEMETRY_TEST_COUNT", snapshot, &histogram);

  // Get "sum" property from histogram
  MC::Rooted<JS::Value> sum(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "sum", histogram, &sum);

  // Check that the "sum" stored in the histogram matches with |kExpectedValue|
  MC::SandboxStack<uint32_t> uSum = 0;
  JS::ToUint32(cx.GetJSContext(), sum, uSum);
  ASSERT_EQ(*uSum.UNSAFE_unverified(), kExpectedValue)
      << "The histogram is not returning expected value";
}

TEST_F(TelemetryTestFixture, AccumulateKeyedTimeDelta) {
  const uint32_t kExpectedValue = 100;
  const TimeStamp start = TimeStamp::Now();
  const TimeDuration delta = TimeDuration::FromMilliseconds(50);

  AutoJSContextWithGlobal cx(mCleanGlobal);

  GetAndClearHistogram(cx.GetJSContext(), mTelemetry,
                       "TELEMETRY_TEST_KEYED_COUNT"_ns, true);

  // Accumulate time delta in the provided key within the histogram
  Telemetry::AccumulateTimeDelta(Telemetry::TELEMETRY_TEST_KEYED_COUNT,
                                 "sample"_ns, start - delta, start);

  Telemetry::AccumulateTimeDelta(Telemetry::TELEMETRY_TEST_KEYED_COUNT,
                                 "sample"_ns, start - delta, start);

  // end > start timestamp gives zero contribution
  Telemetry::AccumulateTimeDelta(Telemetry::TELEMETRY_TEST_KEYED_COUNT,
                                 "sample"_ns, start + delta, start);

  Telemetry::AccumulateTimeDelta(Telemetry::TELEMETRY_TEST_KEYED_COUNT,
                                 "sample"_ns, start, start);

  // Get a snapshot for all the histograms
  MC::Rooted<JS::Value> snapshot(cx.GetJSContext());
  GetSnapshots(cx.GetJSContext(), mTelemetry, "TELEMETRY_TEST_KEYED_COUNT",
               &snapshot, true);

  // Get the histogram from the snapshot
  MC::Rooted<JS::Value> histogram(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "TELEMETRY_TEST_KEYED_COUNT", snapshot,
              &histogram);

  // Get "sample" property from histogram
  MC::Rooted<JS::Value> expectedKeyData(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "sample", histogram, &expectedKeyData);

  // Get "sum" property from keyed data
  MC::Rooted<JS::Value> sum(cx.GetJSContext());
  GetProperty(cx.GetJSContext(), "sum", expectedKeyData, &sum);

  // Check that the sum stored in the histogram matches with |kExpectedValue|
  MC::SandboxStack<uint32_t> uSum = 0;
  JS::ToUint32(cx.GetJSContext(), sum, uSum);
  ASSERT_EQ(*uSum.UNSAFE_unverified(), kExpectedValue)
      << "The histogram is not returning expected sum";
}
