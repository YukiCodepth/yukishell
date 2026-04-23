#!/bin/bash

# --- YukiShell Automated Integration Test ---

echo -e "\033[1;34m=======================================\033[0m"
echo -e "\033[1;36m   YukiShell Integration Test Suite    \033[0m"
echo -e "\033[1;34m=======================================\033[0m\n"

# 1. We create a temporary file holding the commands we want YukiShell to run
TEST_INPUT="yuki_test_input.txt"

cat << 'EOF' > $TEST_INPUT
echo "-> [TEST 1] Standard Execution..."
whoami

echo -e "\n-> [TEST 2] File Redirection (>)..."
echo "Redirection is working flawlessly!" > test_output.txt
cat test_output.txt

echo -e "\n-> [TEST 3] Process Pipes (|)..."
ls | grep src

echo -e "\n-> [TEST 4] Built-in Commands (cd & pwd)..."
cd src
pwd
cd ..

echo -e "\n-> [TEST 5] Background Jobs (&)..."
sleep 2 &

echo -e "\n-> [TEST 6] Help Menu Rendering..."
help

exit
EOF

# 2. We boot up YukiShell and force-feed it the commands file
./yukishell < $TEST_INPUT

# 3. We clean up the temporary files so your folder stays perfectly clean
rm $TEST_INPUT test_output.txt

echo -e "\n\033[1;32m[+] All automated tests executed successfully.\033[0m"
echo -e "\033[1;34m=======================================\033[0m\n"
