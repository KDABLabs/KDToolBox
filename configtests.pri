# configure style tests
# QMAKE_CONFIG_TESTS_DIR defaults to dir of current .pro files + config.tests
# sets config_$test if successful, e.g. config_cpp11 for test config.tests/cpp11

load(configure)

log("Running configure tests...$$escape_expand(\\n)")
CONFIG_TESTS = $$files($$QMAKE_CONFIG_TESTS_DIR/*.pro, true)
for(test, CONFIG_TESTS) {
    test = $$basename(test)
    test ~= s/\\.pro$//
    qtCompileTest($$test)
}
