#!/usr/bin/env bats
# student_tests.sh
# Combined test suite: ZOMBIES tests + bonus tests from assignment_tests.sh
# extra tests for the extra credit.

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

# -----------------------
# ZOMBIES TESTS
# -----------------------

@test "Zero Input: Blank line shows warning" {
  run ./dsh <<EOF

EOF
  [[ "$output" =~ "warning: no commands provided" ]]
  [ "$status" -eq 0 ]
}

@test "One Argument: 'cd' with no directory argument does not change directory" {
  current=$(pwd)
  run ./dsh <<EOF
cd
pwd
EOF
  stripped_output=$(echo "$output" | tr -d '[:space:]')
  expected=$(echo "$current" | tr -d '[:space:]')
  [[ "$stripped_output" == *"$expected"* ]]
  [ "$status" -eq 0 ]
}

@test "Many Arguments: 'cd' with too many arguments prints error" {
  run ./dsh <<EOF
cd dir1 dir2
EOF
  [[ "$output" =~ "cd: too many arguments" ]]
  [ "$status" -eq 0 ]
}

@test "Boundary: External command with excessive spacing and quoted argument" {
  run ./dsh <<EOF
echo    " hello,     world "
EOF
  [[ "$output" =~ " hello,     world " ]]
  [ "$status" -eq 0 ]
}

@test "Invalid Input: Non-existent command returns error message" {
  run ./dsh <<EOF
nonexistentcommand
EOF
  [[ "$output" =~ "execvp: error:" ]]
  [ "$status" -eq 0 ]
}

@test "Exceptional: External command 'which echo' returns a valid path" {
  run ./dsh <<EOF
which echo
EOF
  [[ "$output" =~ "/" ]]
  [ "$status" -eq 0 ]
}

@test "Special: 'exit' command terminates the shell loop" {
  run ./dsh <<EOF
exit
EOF
  [ "$status" -eq 0 ]
}

# -----------------------------
# ASSIGNMENT_TESTS.SH TESTS
# -----------------------------

@test "Change directory" {
    current=$(pwd)
    
    test_dir="$HOME/tmp/dsh-test"
    mkdir -p "$HOME/tmp"          # $HOME/tmp bc ~/tmp doesn't work on wsl
    cd "$HOME/tmp"
    mkdir -p dsh-test 
    
    run "${current}/dsh" <<EOF                
cd dsh-test
pwd
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="${test_dir}dsh2>dsh2>dsh2>cmdloopreturned0"
    
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"
    
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Change directory - no args" {
    current=$(pwd)
    
    test_dir="$HOME/tmp"
    mkdir -p "$HOME/tmp"
    cd "$HOME/tmp"
    mkdir -p dsh-test
    
    run "${current}/dsh" <<EOF                
cd
pwd
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="${test_dir}dsh2>dsh2>dsh2>cmdloopreturned0"
    
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"
    
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

# ------------------------------------------
# EXTRA CREDIT TESTS: Exit Code Handling
# ------------------------------------------

@test "Exit code from valid external command (non-zero)" {
    run ./dsh <<EOF
sh -c "exit 7"
rc
exit
EOF

    [[ "$output" =~ (^|[[:space:]])7($|[[:space:]]) ]]
    [ "$status" -eq 0 ]
}

@test "Exit code from valid external command (zero)" {
    run ./dsh <<EOF
sh -c "exit 0"
rc
exit
EOF

    [[ "$output" =~ (^|[[:space:]])0($|[[:space:]]) ]]
    [ "$status" -eq 0 ]
}

@test "Exit code from invalid external command" {
    run ./dsh <<EOF
nonexistentcommand
rc
exit
EOF
    # For an invalid command, the child exits with ERR_EXEC_CMD (-6).
    # 8-bit unsigned exit code -6 becomes 250.
    [[ "$output" =~ (^|[[:space:]])250($|[[:space:]]) ]]
    [ "$status" -eq 0 ]
}

@test "Exit code from permission denied command" {
    tmpfile=$(mktemp)
    echo "#!/bin/sh" > "$tmpfile"
    echo "exit 5" >> "$tmpfile"
    chmod -x "$tmpfile"
    run ./dsh <<EOF
"$tmpfile"
rc
exit
EOF
    
    [[ "$output" =~ "Permission denied" ]]
    [[ "$output" =~ (^|[[:space:]])250($|[[:space:]]) ]]
    [ "$status" -eq 0 ]
    rm "$tmpfile"
}

