#!/usr/bin/env bats
# File: student_tests.sh
# 
# Create your unit tests suite in this file

# --- Z: Zero ---
@test "Z: Zero Input - Blank line shows warning" {
  run ./dsh <<EOF

EOF
  [[ "$output" =~ "warning: no commands provided" ]]
  [ "$status" -eq 0 ]
}

# --- O: One ---
@test "O: One Argument - 'cd' with no directory argument does not change directory" {
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

@test "O: One Command - 'pwd' returns current directory" {
  current=$(pwd)
  run ./dsh <<EOF
pwd
exit
EOF
  stripped_output=$(echo "$output" | tr -d '[:space:]')
  expected=$(echo "$current" | tr -d '[:space:]')
  [[ "$stripped_output" == *"$expected"* ]]
  [ "$status" -eq 0 ]
}

# --- M: Multiple ---
@test "M: Multiple Commands - Sequential execution" {
  run ./dsh <<EOF
echo "first command"
echo "second command"
exit
EOF
  [[ "$output" =~ "first command" ]]
  [[ "$output" =~ "second command" ]]
  [ "$status" -eq 0 ]
}

@test "M: Pipeline - ls and grep" {
    run ./dsh <<EOF
ls | grep dshlib.c
exit
EOF
    [[ "$output" =~ "dshlib.c" ]]
    [ "$status" -eq 0 ]
}

# --- B: Boundaries ---
@test "B: Boundary - External command with excessive spacing and quoted argument" {
  run ./dsh <<EOF
echo    " hello,     world "
EOF
  [[ "$output" =~ " hello,     world " ]]
  [ "$status" -eq 0 ]
}

@test "B: Boundary - Command near maximum length" {
  # Create a command string near the maximum length (SH_CMD_MAX).
  # Here we use 'echo' plus a long string of characters.
  long_str=$(head -c 200 </dev/zero | tr '\0' 'A')
  run ./dsh <<EOF
echo "$long_str"
exit
EOF
  [[ "$output" =~ "$long_str" ]]
  [ "$status" -eq 0 ]
}

# --- I: Interfaces ---
@test "I: Interfaces - Output redirection >" {
  run ./dsh <<EOF
echo "hello, class" > out.txt
cat out.txt
exit
EOF
  stripped_output=$(echo "$output" | tr -d '[:space:]')
  [[ "$stripped_output" =~ "hello,class" ]]
  [ "$status" -eq 0 ]
  rm -f out.txt
}

@test "I: Interfaces - Input redirection <" {
  tmpfile=$(mktemp)
  echo "input redirection test" > "$tmpfile"
  run ./dsh <<EOF
cat < "$tmpfile"
exit
EOF
  [[ "$output" =~ "input redirection test" ]]
  [ "$status" -eq 0 ]
  rm "$tmpfile"
}

# --- E: Exceptions ---
@test "E: Exceptions - Invalid Input: Non-existent command returns error message" {
  run ./dsh <<EOF
nonexistentcommand
EOF
  [[ "$output" =~ "execvp: error:" ]]
  [ "$status" -eq 0 ]
}

@test "E: Exceptions - 'cd' with too many arguments prints error" {
  run ./dsh <<EOF
cd dir1 dir2
EOF
  [[ "$output" =~ "cd: too many arguments" ]]
  [ "$status" -eq 0 ]
}

# --- S: Simple Scenarios ---
@test "S: Simple - Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF
    [ "$status" -eq 0 ]
}

@test "S: Simple - 'exit' command terminates the shell loop" {
  run ./dsh <<EOF
exit
EOF
  [ "$status" -eq 0 ]
}

@test "S: Simple - Pipeline: echo and tr" {
    run ./dsh <<EOF
echo hello world | tr a-z A-Z
exit
EOF
    [[ "$output" =~ "HELLO WORLD" ]]
    [ "$status" -eq 0 ]
}

# --- Additional Test: Zombie Process Cleanup (for general process cleanup) ---
@test "Zombie Cleanup: No zombie processes remain after command execution" {
    run ./dsh <<EOF
ls
exit
EOF
    sleep 1
    zombies=$(ps -o stat= -u "$USER" | grep "Z" || true)
    [ -z "$zombies" ]
    [ "$status" -eq 0 ]
}
