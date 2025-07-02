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
make_title() {
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

assert_empty_directory() {
  directory_contents="$(find "$1" -type l)"
  [[ -n $directory_contents ]] && echo "$directory_contents"
}

assert_root_contents() {
  diff <(find "../root" -type l) <(echo "$1")
}

# Test suite calling stuff without a
# subcommand and different global flags
suite_stuff() {
  SUITES+=1
  echo "  stuff with no subcommand"

  command="stuff"
  file="test_stuff"
  output_stuff=$(diff <($command) "${OUTPUT_FOLDER}/${file}")
  title="should show help when called with nothing"
  make_title "$output_stuff" "$title"
  process_result "$output_stuff"

  command="stuff --help"
  file="test_stuff"
  output_help=$(diff <($command) "${OUTPUT_FOLDER}/${file}")
  title="should show help when called with help"
  make_title "$output_help" "$title"
  process_result "$output_help"

  command="stuff --version"
  file="test_stuff_version"
  output_version=$(diff <($command) "${OUTPUT_FOLDER}/${file}")
  title="should show version when called with version"
  make_title "$output_version" "$title"
  process_result "$output_version"

  process_suite "$DID_SUITE_PASS"

  echo ""
}

# Test suite calling stuff link with
# different link specific flags
suite_stuff_link() {
  SUITES+=1
  echo "  stuff link subcommand"

  command="stuff link"
  file="test_stuff_link"
  output_link=$(diff <($command) "${OUTPUT_FOLDER}/${file}")
  title="should show help when called with nothing"
  make_title "$output_link" "$title"
  process_result "$output_link"

  command="stuff link --help"
  file="test_stuff_link"
  output_link_help=$(diff <($command) "${OUTPUT_FOLDER}/${file}")
  title="should show help when called with help"
  make_title "$output_link_help" "$title"
  process_result "$output_link_help"

  command="stuff link --root ../root ./.one"
  file="test_stuff_link_file"
  output_link_file=$(diff <($command) "${OUTPUT_FOLDER}/${file}")
  output_link_file+=$(assert_root_contents "../root/.one")
  title="should link a file when given a file path"
  make_title "$output_link_file" "$title"
  process_result "$output_link_file"
  rm ../root/.one

  command="stuff link --root ../root ./folder"
  file="test_stuff_link_folder"
  output_link_folder=$(diff <($command) "${OUTPUT_FOLDER}/${file}")
  output_link_folder+=$(assert_root_contents "../root/folder")
  title="should link a folder when given a folder path"
  make_title "$output_link_folder" "$title"
  process_result "$output_link_folder"
  rm -rf ../root/folder

  ln --symbolic "${PWD}/.one" ../root/.one
  command="stuff link --root ../root --force ./.one"
  file="test_stuff_link_file"
  output_link_force=$(assert_root_contents "../root/.one")
  output_link_force+=$(diff <($command) "${OUTPUT_FOLDER}/$file")
  output_link_force+=$(assert_root_contents "../root/.one")
  title="should link an already linked file when forced"
  make_title "$output_link_force" "$title"
  process_result "$output_link_force"
  rm ../root/.one

  process_suite "$DID_SUITE_PASS"

  echo ""
}

# Test suite calling stuff list with
# different list specific flags
suite_stuff_list() {
  SUITES+=1
  echo "  stuff list subcommand"

  command="stuff list --help"
  file="test_stuff_list_help"
  output_list=$(diff <($command) "${OUTPUT_FOLDER}/${file}")
  title="should show help when called with help"
  make_title "$output_list" "$title"
  process_result "$output_list"

  command="stuff list --root ../root"
  file="test_stuff_list"
  output_list_help=$(diff <($command) "${OUTPUT_FOLDER}/${file}")
  title="should list project contents without links"
  make_title "$output_list_help" "$title"
  process_result "$output_list_help"

  command="stuff list --root ../root --linked"
  file="test_stuff_list_linked_empty"
  output_list_empty=$(diff <($command) "${OUTPUT_FOLDER}/${file}")
  title="should filter nothing when nothing is linked"
  make_title "$output_list_empty" "$title"
  process_result "$output_list_empty"

  ln --symbolic "${PWD}/.one" ../root/.one
  command="stuff list --root ../root --linked"
  file="test_stuff_list_linked_file"
  output_list_file=$(diff <($command) "${OUTPUT_FOLDER}/${file}")
  title="should filter linked files when files are linked"
  make_title "$output_list_file" "$title"
  process_result "$output_list_file"
  rm ../root/.one

  ln --symbolic "${PWD}/folder" ../root/folder
  command="stuff list --root ../root --linked"
  file="test_stuff_list_linked_folder"
  output_list_folder=$(diff <($command) "${OUTPUT_FOLDER}/${file}")
  title="should filter linked folders when folders are linked"
  make_title "$output_list_folder" "$title"
  process_result "$output_list_folder"
  rm -rf ../root/folder

  process_suite "$DID_SUITE_PASS"

  echo ""
}

# Test suite calling stuff unlink with
# different unlink specific flags
suite_stuff_unlink() {
  SUITES+=1
  echo "  stuff unlink subcommand"

  command="stuff unlink"
  file="test_stuff_unlink"
  output_unlink=$(diff <($command) "${OUTPUT_FOLDER}/${file}")
  title="should show help when called with nothing"
  make_title "$output_unlink" "$title"
  process_result "$output_unlink"

  command="stuff unlink --help"
  file="test_stuff_unlink"
  output_unlink_help=$(diff <($command) "${OUTPUT_FOLDER}/${file}")
  title="should show help when called with help"
  make_title "$output_unlink_help" "$title"
  process_result "$output_unlink_help"

  ln --symbolic "${PWD}/.one" ../root/.one
  command="stuff unlink --root ../root ./.one"
  file="test_stuff_unlink_file"
  output_unlink_file=$(diff <($command) "${OUTPUT_FOLDER}/${file}")
  output_unlink_file+=$(assert_empty_directory "../root")
  title="should unlink a file when given a linked file path"
  make_title "$output_unlink_file" "$title"
  process_result "$output_unlink_file"

  ln --symbolic "${PWD}/folder" ../root/folder
  command="stuff unlink --root ../root ./folder"
  file="test_stuff_unlink_folder"
  output_unlink_folder=$(diff <($command) "${OUTPUT_FOLDER}/${file}")
  output_unlink_folder+=$(assert_empty_directory "../root")
  title="should unlink a folder when given a linked folder path"
  make_title "$output_unlink_folder" "$title"
  process_result "$output_unlink_folder"

  process_suite "$DID_SUITE_PASS"

  echo ""
}

# Run all test suites and output general
# test results based on observations
run() {
  start_time=$EPOCHREALTIME
  echo -e "${ANSI_FORMAT_BOLD}stuff ./run.sh${ANSI_RESET} harness\n"
  suite_stuff
  suite_stuff_link
  suite_stuff_list
  suite_stuff_unlink
  bold "Test suites: ${SUITES_PASSED} passed" && echo ", ${SUITES} total"
  bold "Tests:       ${TESTS_PASSED} passed" && echo ", ${TESTS} total"
  end_time=$EPOCHREALTIME
  elapsed=$(echo "$end_time - $start_time" | bc)
  bold "Time:        " && echo "${elapsed} seconds"
}

run
