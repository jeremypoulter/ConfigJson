#include <unity.h>
#include <EEPROM.h>

void runConfigOptDefinitionTests();
void runConfigOptSecretTests();
void runConfigOptVirtualBoolTests();
void runConfigOptVirtualMaskedBoolTests();
void runConfigJsonTests();

void setUp(void) {
  EEPROM.wipe();
}

void tearDown(void) {}

// PlatformIO's native test runner does `signal.Signals(abs(return_code))` on
// any nonzero exit without checking whether the process actually died from a
// signal, so a plain "N assertions failed" (UNITY_END()'s ordinary return
// value) gets misreported as e.g. "Program received signal SIGABRT". Collapse
// any nonzero result to a single sentinel outside the signal range instead of
// passing the raw failure count through.
static int toExitCode(int unityResult) {
  return 0 == unityResult ? 0 : 100;
}

int main(int argc, char **argv) {
  (void) argc;
  (void) argv;

  UNITY_BEGIN();
  runConfigOptDefinitionTests();
  runConfigOptSecretTests();
  runConfigOptVirtualBoolTests();
  runConfigOptVirtualMaskedBoolTests();
  runConfigJsonTests();
  return toExitCode(UNITY_END());
}
