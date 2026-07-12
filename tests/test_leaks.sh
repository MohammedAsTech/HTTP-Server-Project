#!/bin/bash

SERVER=./cmake-build-debug/my_server
PORT=8081
BASE=http://localhost:$PORT
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m'

echo "══════════════════════════════════════════════════"
echo " Memory Leak Check — valgrind"
echo "══════════════════════════════════════════════════"

# check valgrind is installed
if ! command -v valgrind &>/dev/null; then
    echo -e "${RED}ERROR: valgrind not found. Install with: sudo apt install valgrind${NC}"
    exit 1
fi

# check nc is installed
if ! command -v nc &>/dev/null; then
    echo -e "${RED}ERROR: nc (netcat) not found. Install with: sudo apt install netcat${NC}"
    exit 1
fi

echo "Building server..."
make clean && make
if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed${NC}"
    exit 1
fi

mkdir -p www
echo "<html><body>Hello</body></html>" > www/index.html
echo "body { color: red; }" > www/style.css
echo "console.log('hi');" > www/app.js
echo '{"key":"value"}' > www/data.json
echo "plain text file" > www/readme.txt

echo ""
echo "Starting server under valgrind on port $PORT..."
valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --error-exitcode=1 \
    --log-file=valgrind_output.txt \
    $SERVER $PORT &
VALGRIND_PID=$!
sleep 2

if ! kill -0 $VALGRIND_PID 2>/dev/null; then
    echo -e "${RED}Server failed to start under valgrind${NC}"
    cat valgrind_output.txt
    exit 1
fi

echo -e "${GREEN}Server running under valgrind (PID $VALGRIND_PID)${NC}"
echo ""
echo "Firing test requests..."

# normal requests
curl -s $BASE/ > /dev/null
curl -s $BASE/index.html > /dev/null
curl -s $BASE/style.css > /dev/null
curl -s $BASE/app.js > /dev/null
curl -s $BASE/data.json > /dev/null
curl -s $BASE/readme.txt > /dev/null

# 404 cases
curl -s $BASE/missing > /dev/null
curl -s $BASE/doesnotexist > /dev/null
curl -s $BASE/admin > /dev/null

# POST echo
curl -s -X POST -d "hello" $BASE/echo > /dev/null
curl -s -X POST -d "" $BASE/echo > /dev/null
curl -s -X POST -d "x" $BASE/echo > /dev/null
curl -s -X POST -H 'Content-Type: application/json' -d '{"a":1}' $BASE/echo > /dev/null
curl -s -X POST -H 'Content-Type: text/plain' -d 'plain' $BASE/echo > /dev/null

# wrong methods
curl -s -X POST $BASE/ > /dev/null
curl -s -X DELETE $BASE/ > /dev/null
curl -s -X PUT $BASE/index.html > /dev/null

# path traversal
curl -s --path-as-is $BASE/../etc/passwd > /dev/null
curl -s --path-as-is $BASE/../../etc > /dev/null

# oversized body
curl -s -X POST -d "$(python3 -c "print('A'*9000)")" $BASE/echo > /dev/null

# malformed via netcat
echo 'GARBAGE' | nc -q1 localhost $PORT > /dev/null 2>&1
printf 'GET / HTTP/1.1\r\n' | nc -q1 localhost $PORT > /dev/null 2>&1
printf '\x00\x01\x02' | nc -q1 localhost $PORT > /dev/null 2>&1
echo '' | nc -q1 localhost $PORT > /dev/null 2>&1

# concurrent requests
for i in {1..4}; do
    curl -s $BASE/ > /dev/null &
done
wait

for i in {1..4}; do
    curl -s -X POST -d "concurrent$i" $BASE/echo > /dev/null &
done
wait

echo -e "${GREEN}All test requests fired.${NC}"
echo ""
echo "Sending SIGINT to trigger graceful shutdown..."
kill -SIGINT $VALGRIND_PID
wait $VALGRIND_PID
EXIT_CODE=$?

echo ""
echo "══════════════════════════════════════════════════"
echo " Valgrind Report"
echo "══════════════════════════════════════════════════"
cat valgrind_output.txt

echo ""
echo "══════════════════════════════════════════════════"
echo " Leak Summary"
echo "══════════════════════════════════════════════════"

DEFINITELY=$(grep "definitely lost:" valgrind_output.txt | grep -v "0 bytes")
POSSIBLY=$(grep "possibly lost:" valgrind_output.txt | grep -v "0 bytes")
ERRORS=$(grep "ERROR SUMMARY:" valgrind_output.txt | grep -v "0 errors")

LEAK_FOUND=0

if [ -n "$DEFINITELY" ]; then
    echo -e "${RED}FAIL: Definite leaks detected:${NC}"
    echo -e "${YELLOW}$DEFINITELY${NC}"
    LEAK_FOUND=1
else
    echo -e "${GREEN}PASS: No definite leaks${NC}"
fi

if [ -n "$POSSIBLY" ]; then
    echo -e "${YELLOW}WARN: Possible leaks detected:${NC}"
    echo -e "${YELLOW}$POSSIBLY${NC}"
else
    echo -e "${GREEN}PASS: No possible leaks${NC}"
fi

if [ -n "$ERRORS" ]; then
    echo -e "${RED}FAIL: Valgrind errors detected:${NC}"
    echo -e "${YELLOW}$ERRORS${NC}"
    LEAK_FOUND=1
else
    echo -e "${GREEN}PASS: No valgrind errors${NC}"
fi

echo ""
echo "Full report saved to: valgrind_output.txt"
echo ""

if [ $LEAK_FOUND -eq 1 ]; then
    echo -e "${RED}Leak check FAILED — see valgrind_output.txt for details${NC}"
    exit 1
else
    echo -e "${GREEN}Leak check PASSED${NC}"
fi
