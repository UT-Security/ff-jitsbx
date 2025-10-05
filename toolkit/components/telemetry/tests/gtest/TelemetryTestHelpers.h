/* Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

#ifndef TelemetryTestHelpers_h_
#define TelemetryTestHelpers_h_

#include "monkeycage/TypeDecls.h"
#include "mozilla/TelemetryProcessEnums.h"
#include "nsITelemetry.h"

struct MCContext;

using mozilla::Telemetry::ProcessID;

namespace TelemetryTestHelpers {

void CheckUintScalar(const char* aName, MCContext* aCx,
                     JS::Handle<JS::Value> aSnapshot, uint32_t expectedValue);

void CheckBoolScalar(const char* aName, MCContext* aCx,
                     JS::Handle<JS::Value> aSnapshot, bool expectedValue);

void CheckStringScalar(const char* aName, MCContext* aCx,
                       JS::Handle<JS::Value> aSnapshot,
                       const char* expectedValue);

void CheckKeyedUintScalar(const char* aName, const char* aKey, MCContext* aCx,
                          JS::Handle<JS::Value> aSnapshot,
                          uint32_t expectedValue);

void CheckKeyedBoolScalar(const char* aName, const char* aKey, MCContext* aCx,
                          JS::Handle<JS::Value> aSnapshot, bool expectedValue);

void CheckNumberOfProperties(const char* aName, MCContext* aCx,
                             JS::Handle<JS::Value> aSnapshot,
                             uint32_t expectedNumProperties);

bool EventPresent(MCContext* aCx, const MC::RootedValue& aSnapshot,
                  const nsACString& aCategory, const nsACString& aMethod,
                  const nsACString& aObject);

nsTArray<nsString> EventValuesToArray(MCContext* aCx,
                                      const MC::RootedValue& aSnapshot,
                                      const nsAString& aCategory,
                                      const nsAString& aMethod,
                                      const nsAString& aObject);

void GetEventSnapshot(MCContext* aCx, JS::MutableHandle<JS::Value> aResult,
                      ProcessID aProcessType = ProcessID::Parent);

void GetScalarsSnapshot(bool aKeyed, MCContext* aCx,
                        JS::MutableHandle<JS::Value> aResult,
                        ProcessID aProcessType = ProcessID::Parent);

void GetAndClearHistogram(MCContext* cx, nsCOMPtr<nsITelemetry> mTelemetry,
                          const nsACString& name, bool is_keyed);

void GetProperty(MCContext* cx, const char* name, JS::Handle<JS::Value> valueIn,
                 JS::MutableHandle<JS::Value> valueOut);

void GetElement(MCContext* cx, uint32_t index, JS::Handle<JS::Value> valueIn,
                JS::MutableHandle<JS::Value> valueOut);

void GetSnapshots(MCContext* cx, nsCOMPtr<nsITelemetry> mTelemetry,
                  const char* name, JS::MutableHandle<JS::Value> valueOut,
                  bool is_keyed);
}  // namespace TelemetryTestHelpers

#endif
