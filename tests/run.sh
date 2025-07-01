#!/bin/bash

# Expect everything to be run in ./project
# All paths specified relative to this assumption

OUTPUT_FOLDER="../output"
ANSI_FORMAT_BOLD="\e[1m"
ANSI_COLOR_GREEN="\x1b[32m"
ANSI_COLOR_RED="\x1b[31m"
ANSI_RESET="\x1b[0m"
declare -i TESTS=0
declare -i TESTS_PASSED=0
declare -i SUITES=0
DID_SUITE_PASS=true
declare -i SUITES_PASSED=0

# Conditionally print a title with
# a green check or red cross
title() {
  TESTS+=1
  [[ -n $1 ]] && icon="✗" || icon="✓"
  [[ -n $1 ]] && color="$ANSI_COLOR_RED" || color="$ANSI_COLOR_GREEN"
  echo -e "    ${color}${icon}${ANSI_RESET} ${2}"
}

# Format and output bold text
bold() {
  echo -ne "${ANSI_FORMAT_BOLD}$1${ANSI_RESET}"
}

# Account after each test
process_result() {
  [[ -n $1 ]] || TESTS_PASSED+=1
  [[ -n $1 ]] && DID_SUITE_PASS=false
  [[ -n $1 ]] && echo "$1"
}

# Account after each suite
process_suite() {
  [[ $1 == true ]] && SUITES_PASSED+=1
  # Reset for the next one
  DID_SUITE_PASS=true
}

# Test suite calling stuff without a
# subcommand and different global flags
suite_stuff() {
  SUITES+=1
  echo "  stuff with no subcommand"

  command="stuff"
  test_stuff=$(diff <($command) "${OUTPUT_FOLDER}/test_stuff")
  title "$test_stuff" "should show help when stuff called by itself"
  process_result "$test_stuff"

  command="stuff --help"
  test_stuff_help=$(diff <($command) "${OUTPUT_FOLDER}/test_stuff")
  title "$test_stuff_help" "should show help when stuff called with --help"
  process_result "$test_stuff_help"

  command="stuff --version"
  test_stuff_version=$(diff <($command) "${OUTPUT_FOLDER}/test_stuff_version")
  title "$test_stuff_version" "should show version when stuff called with --version"
  process_result "$test_stuff_version"

  process_suite "$DID_SUITE_PASS"

  echo ""
}

# Test suite calling stuff list with
# different list specific flags
suite_stuff_list() {
  SUITES+=1
  echo "  stuff list subcommand"

  command="stuff list --root ../root"
  test_stuff_list=$(diff <($command) "${OUTPUT_FOLDER}/test_stuff_list")
  title "$test_stuff_list" "should list local project contents when no links exist"
  process_result "$test_stuff_list"

  command="stuff list --help"
  test_stuff_list_help=$(diff <($command) "${OUTPUT_FOLDER}/test_stuff_list_help")
  title "$test_stuff_list_help" "should show help when stuff list called with --help"
  process_result "$test_stuff_list_help"

  command="stuff list --root ../root --linked"
  test_stuff_list_linked_empty=$(diff <($command) "${OUTPUT_FOLDER}/test_stuff_list_linked_empty")
  title "$test_stuff_list_linked_empty" "should list nothing when no files are linked"
  process_result "$test_stuff_list_linked_empty"

  # Don't hardcode the root directory
  ln --symbolic "${PWD}/.one" ../root/.one
  command="stuff list --root ../root --linked"
  test_stuff_list_linked_one=$(diff <($command) "${OUTPUT_FOLDER}/test_stuff_list_linked_one")
  title "$test_stuff_list_linked_one" "should list linked files when files are linked"
  process_result "$test_stuff_list_linked_one"
  rm ../root/.one

  # Don't hardcode the root directory
  ln --symbolic "${PWD}/.one" ../root/.one
  ln --symbolic "${PWD}/folder" ../root/folder
  command="stuff list --root ../root --linked"
  test_stuff_list_linked_two=$(diff <($command) "${OUTPUT_FOLDER}/test_stuff_list_linked_two")
  title "$test_stuff_list_linked_two" "should list linked folders when folders are linked"
  process_result "$test_stuff_list_linked_two"
  rm -rf ../root/folder
  rm ../root/.one

  process_suite "$DID_SUITE_PASS"

  echo ""
}

# Test suite calling stuff link with
# different link specific flags
suite_stuff_link() {
  SUITES+=1
  echo "  stuff link subcommand"

  command="stuff link"
  test_stuff_link=$(diff <($command) "${OUTPUT_FOLDER}/test_stuff_link")
  title "$test_stuff_link" "should show help when stuff link called by itself"
  process_result "$test_stuff_link"

  command="stuff link --help"
  test_stuff_link_help=$(diff <($command) "${OUTPUT_FOLDER}/test_stuff_link")
  title "$test_stuff_link_help" "should show help when stuff link called with --help"
  process_result "$test_stuff_link_help"

  process_suite "$DID_SUITE_PASS"

  echo ""
}

# Test suite calling stuff unlink with
# different unlink specific flags
suite_stuff_unlink() {
  SUITES+=1
  echo "  stuff unlink subcommand"

  command="stuff unlink"
  test_stuff_unlink=$(diff <($command) "${OUTPUT_FOLDER}/test_stuff_unlink")
  title "$test_stuff_unlink" "should show help when stuff unlink called by itself"
  process_result "$test_stuff_unlink"

  command="stuff unlink --help"
  test_stuff_unlink_help=$(diff <($command) "${OUTPUT_FOLDER}/test_stuff_unlink")
  title "$test_stuff_unlink_help" "should show help when stuff unlink called with --help"
  process_result "$test_stuff_unlink_help"

  process_suite "$DID_SUITE_PASS"

  echo ""
}

# Run all test suites and output general
# test results based on observations
run() {
  start_time=$EPOCHREALTIME
  echo -e "${ANSI_FORMAT_BOLD}stuff ./run.sh${ANSI_RESET} harness\n"
  suite_stuff
  suite_stuff_list
  suite_stuff_link
  suite_stuff_unlink
  bold "Test suites: ${SUITES_PASSED} passed" && echo ", ${SUITES} total"
  bold "Tests:       ${TESTS_PASSED} passed" && echo ", ${TESTS} total"
  end_time=$EPOCHREALTIME
  elapsed=$(echo "$end_time - $start_time" | bc)
  bold "Time:        " && echo "${elapsed} seconds"
}

run
