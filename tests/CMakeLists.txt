macro(rangemap_add_test TESTNAME TEST_SOURCE)
  add_executable(${TESTNAME} ${TEST_SOURCE})
  target_link_libraries(${TESTNAME} PUBLIC gtest gmock gtest_main)
  target_link_libraries(${TESTNAME} PUBLIC rangemap)
  add_test(NAME ${TESTNAME}
           COMMAND ${TESTNAME})
  set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

rangemap_add_test(test_basic test_basic.cc)
