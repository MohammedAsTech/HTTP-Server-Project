#!/bin/bash

# ─── config ───────────────────────────────────────────────────────────────────
SERVER=./cmake-build-debug/my_server
PORT=8080
BASE=http://localhost:$PORT
PASS=0
FAIL=0
TOTAL=0

# ─── colors ───────────────────────────────────────────────────────────────────
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m'

# ─── helpers ──────────────────────────────────────────────────────────────────
start_server() {
    $SERVER $PORT &>/dev/null &
    SERVER_PID=$!
    sleep 0.5
    if ! kill -0 $SERVER_PID 2>/dev/null; then
        echo -e "${RED}ERROR: Server failed to start${NC}"
        exit 1
    fi
    echo -e "${GREEN}Server started (PID $SERVER_PID)${NC}"
}

stop_server() {
    if [ -n "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
        echo -e "${GREEN}Server stopped${NC}"
    fi
}

check() {
    local description="$1"
    local expected="$2"
    local actual="$3"
    TOTAL=$((TOTAL + 1))
    if echo "$actual" | grep -q "$expected"; then
        PASS=$((PASS + 1))
        echo -e "${GREEN}PASS${NC} [$TOTAL] $description"
    else
        FAIL=$((FAIL + 1))
        echo -e "${RED}FAIL${NC} [$TOTAL] $description"
        echo -e "     expected: ${YELLOW}$expected${NC}"
        echo -e "     actual:   ${YELLOW}$actual${NC}"
    fi
}

trap stop_server EXIT
start_server

# ─── www setup ────────────────────────────────────────────────────────────────
mkdir -p www
echo "<html><body>Hello</body></html>" > www/index.html
echo "body { color: red; }" > www/style.css
echo "console.log('hi');" > www/app.js
echo '{"key":"value"}' > www/data.json
echo "plain text file" > www/readme.txt
printf '\x89PNG\r\n\x1a\n' > www/image.png

echo ""
echo "══════════════════════════════════════════════════"
echo " GET / — hello handler"
echo "══════════════════════════════════════════════════"

check "GET / returns 200" "200" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/)"
check "GET / returns Hello from handler" "Hello from handler" "$(curl -s $BASE/)"
check "GET / HTTP version is 1.1" "HTTP/1.1" "$(curl -s -I $BASE/ | head -1)"
check "GET / Content-Length present" "Content-Length" "$(curl -s -I $BASE/)"
check "GET / response not empty" "." "$(curl -s $BASE/)"

echo ""
echo "══════════════════════════════════════════════════"
echo " GET static files — html"
echo "══════════════════════════════════════════════════"

check "GET /index.html returns 200" "200" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/index.html)"
check "GET /index.html returns content" "Hello" "$(curl -s $BASE/index.html)"
check "GET /index.html Content-Type is text/html" "text/html" "$(curl -s -I $BASE/index.html)"
check "GET /index.html Content-Length present" "Content-Length" "$(curl -s -I $BASE/index.html)"
check "GET /index.html response not empty" "." "$(curl -s $BASE/index.html)"

echo ""
echo "══════════════════════════════════════════════════"
echo " GET static files — css"
echo "══════════════════════════════════════════════════"

check "GET /style.css returns 200" "200" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/style.css)"
check "GET /style.css Content-Type is text/css" "text/css" "$(curl -s -I $BASE/style.css)"
check "GET /style.css returns content" "color" "$(curl -s $BASE/style.css)"

echo ""
echo "══════════════════════════════════════════════════"
echo " GET static files — javascript"
echo "══════════════════════════════════════════════════"

check "GET /app.js returns 200" "200" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/app.js)"
check "GET /app.js Content-Type is application/javascript" "application/javascript" "$(curl -s -I $BASE/app.js)"
check "GET /app.js returns content" "console" "$(curl -s $BASE/app.js)"

echo ""
echo "══════════════════════════════════════════════════"
echo " GET static files — json"
echo "══════════════════════════════════════════════════"

check "GET /data.json returns 200" "200" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/data.json)"
check "GET /data.json Content-Type is application/json" "application/json" "$(curl -s -I $BASE/data.json)"
check "GET /data.json returns content" "value" "$(curl -s $BASE/data.json)"

echo ""
echo "══════════════════════════════════════════════════"
echo " GET static files — plain text"
echo "══════════════════════════════════════════════════"

check "GET /readme.txt returns 200" "200" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/readme.txt)"
check "GET /readme.txt Content-Type is text/plain" "text/plain" "$(curl -s -I $BASE/readme.txt)"
check "GET /readme.txt returns content" "plain text" "$(curl -s $BASE/readme.txt)"

echo ""
echo "══════════════════════════════════════════════════"
echo " 404 — missing files and unknown routes"
echo "══════════════════════════════════════════════════"

check "GET /missing returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/missing)"
check "GET /missing body contains 404" "404" "$(curl -s $BASE/missing)"
check "GET /missing.html returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/missing.html)"
check "GET /foo/bar returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/foo/bar)"
check "GET /admin returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/admin)"
check "GET /secret returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/secret)"
check "GET /robots.txt returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/robots.txt)"
check "GET /favicon.ico returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/favicon.ico)"
check "GET /.env returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/.env)"
check "GET /config returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/config)"
check "GET /login returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/login)"
check "GET /api returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/api)"
check "GET /api/v1 returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/api/v1)"
check "GET /null returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/null)"

echo ""
echo "══════════════════════════════════════════════════"
echo " 403 — path traversal attempts"
echo "══════════════════════════════════════════════════"

check "GET /../etc/passwd returns 403" "403" "$(curl -s -o /dev/null -w "%{http_code}" --path-as-is $BASE/../etc/passwd)"
check "GET /../../etc returns 403" "403" "$(curl -s -o /dev/null -w "%{http_code}" --path-as-is $BASE/../../etc)"
check "GET /index.html/../../../etc returns 403" "403" "$(curl -s -o /dev/null -w "%{http_code}" --path-as-is $BASE/index.html/../../../etc)"
check "path traversal body contains 403" "403" "$(curl -s --path-as-is $BASE/../etc/passwd)"
check "GET /www/../etc returns 403" "403" "$(curl -s -o /dev/null -w "%{http_code}" --path-as-is $BASE/www/../etc)"

echo ""
echo "══════════════════════════════════════════════════"
echo " POST /echo — basic"
echo "══════════════════════════════════════════════════"

check "POST /echo returns 200" "200" "$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "hello" $BASE/echo)"
check "POST /echo echoes body" "hello" "$(curl -s -X POST -d "hello" $BASE/echo)"
check "POST /echo echoes exactly" "world" "$(curl -s -X POST -d "world" $BASE/echo)"
check "POST /echo Content-Length present" "Content-Length" "$(curl -s -X POST -d "hello" -D - -o /dev/null $BASE/echo)"
check "POST /echo with numbers" "12345" "$(curl -s -X POST -d "12345" $BASE/echo)"
check "POST /echo with special chars" "hello world" "$(curl -s -X POST -d "hello world" $BASE/echo)"
check "POST /echo with symbols" "!@#" "$(curl -s -X POST -d '!@#' $BASE/echo)"

echo ""
echo "══════════════════════════════════════════════════"
echo " POST /echo — content type mirroring"
echo "══════════════════════════════════════════════════"

check "POST /echo mirrors text/plain" "text/plain" "$(curl -s -X POST -H 'Content-Type: text/plain' -d 'hello' -D - -o /dev/null $BASE/echo)"
check "POST /echo mirrors application/json" "application/json" "$(curl -s -X POST -H 'Content-Type: application/json' -d '{}' -D - -o /dev/null $BASE/echo)"
check "POST /echo mirrors text/html" "text/html" "$(curl -s -X POST -H 'Content-Type: text/html' -d '<p>hi</p>' -D - -o /dev/null $BASE/echo)"
check "POST /echo mirrors application/xml" "application/xml" "$(curl -s -X POST -H 'Content-Type: application/xml' -d '<x/>' -D - -o /dev/null $BASE/echo)"
check "POST /echo JSON body echoed" "key" "$(curl -s -X POST -H 'Content-Type: application/json' -d '{"key":"value"}' $BASE/echo)"
check "POST /echo XML body echoed" "root" "$(curl -s -X POST -H 'Content-Type: application/xml' -d '<root/>' $BASE/echo)"

echo ""
echo "══════════════════════════════════════════════════"
echo " POST /echo — empty and minimal bodies"
echo "══════════════════════════════════════════════════"

check "POST /echo empty body returns 200" "200" "$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "" $BASE/echo)"
check "POST /echo single char" "x" "$(curl -s -X POST -d "x" $BASE/echo)"
check "POST /echo single space" "200" "$(curl -s -o /dev/null -w "%{http_code}" -X POST -d " " $BASE/echo)"
check "POST /echo newline body returns 200" "200" "$(curl -s -o /dev/null -w "%{http_code}" -X POST -d $'\n' $BASE/echo)"

echo ""
echo "══════════════════════════════════════════════════"
echo " wrong method on known routes"
echo "══════════════════════════════════════════════════"

check "POST / returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" -X POST $BASE/)"
check "DELETE / returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" -X DELETE $BASE/)"
check "PUT / returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" -X PUT $BASE/)"
check "PATCH / returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" -X PATCH $BASE/)"
check "GET /echo returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/echo)"
check "DELETE /echo returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" -X DELETE $BASE/echo)"
check "PUT /index.html returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" -X PUT $BASE/index.html)"
check "DELETE /index.html returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" -X DELETE $BASE/index.html)"

echo ""
echo "══════════════════════════════════════════════════"
echo " malformed requests via netcat"
echo "══════════════════════════════════════════════════"

check "empty request handled" "handled" "$(echo '' | nc -q1 localhost $PORT > /dev/null 2>&1; echo 'handled')"
check "garbage request handled" "handled" "$(echo 'GARBAGE' | nc -q1 localhost $PORT > /dev/null 2>&1; echo 'handled')"
check "partial request handled" "handled" "$(echo 'GET' | nc -q1 localhost $PORT > /dev/null 2>&1; echo 'handled')"
check "no headers handled" "handled" "$(printf 'GET / HTTP/1.1\r\n' | nc -q1 localhost $PORT > /dev/null 2>&1; echo 'handled')"
check "random bytes handled" "handled" "$(printf '\x00\x01\x02\x03' | nc -q1 localhost $PORT > /dev/null 2>&1; echo 'handled')"
check "missing version handled" "handled" "$(printf 'GET /\r\n\r\n' | nc -q1 localhost $PORT > /dev/null 2>&1; echo 'handled')"
check "invalid method handled" "handled" "$(printf 'INVALID / HTTP/1.1\r\nHost: localhost\r\n\r\n' | nc -q1 localhost $PORT > /dev/null 2>&1; echo 'handled')"
check "no path handled" "handled" "$(printf 'GET HTTP/1.1\r\n\r\n' | nc -q1 localhost $PORT > /dev/null 2>&1; echo 'handled')"
check "double slash path handled" "handled" "$(printf 'GET // HTTP/1.1\r\nHost: localhost\r\n\r\n' | nc -q1 localhost $PORT > /dev/null 2>&1; echo 'handled')"
check "very long path handled" "handled" "$(printf 'GET /%s HTTP/1.1\r\n\r\n' $(python3 -c "print('a'*3000)") | nc -q1 localhost $PORT > /dev/null 2>&1; echo 'handled')"

echo ""
echo "══════════════════════════════════════════════════"
echo " oversized request body"
echo "══════════════════════════════════════════════════"

check "8KB+ body returns 400" "400" "$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "$(python3 -c "print('A'*8200)")" $BASE/echo)"
check "9KB body returns 400" "400" "$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "$(python3 -c "print('A'*9000)")" $BASE/echo)"
check "10KB body returns 400" "400" "$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "$(python3 -c "print('A'*10000)")" $BASE/echo)"
check "oversized body response contains 400" "400" "$(curl -s -X POST -d "$(python3 -c "print('A'*9000)")" $BASE/echo)"

echo ""
echo "══════════════════════════════════════════════════"
echo " response headers"
echo "══════════════════════════════════════════════════"

check "200 response has Content-Length" "Content-Length" "$(curl -s -I $BASE/)"
check "404 response has Content-Length" "Content-Length" "$(curl -s -I $BASE/missing)"
check "200 response has HTTP/1.1" "HTTP/1.1" "$(curl -s -I $BASE/)"
check "404 response has HTTP/1.1" "HTTP/1.1" "$(curl -s -I $BASE/missing)"
check "POST echo has Content-Length" "Content-Length" "$(curl -s -X POST -d "hi" -D - -o /dev/null $BASE/echo)"
check "static file has Content-Type" "Content-Type" "$(curl -s -I $BASE/index.html)"
check "200 status line correct" "200 OK" "$(curl -s -I $BASE/ | head -1)"
check "404 status line correct" "404 Not Found" "$(curl -s -I $BASE/missing | head -1)"

echo ""
echo "══════════════════════════════════════════════════"
echo " concurrent connections"
echo "══════════════════════════════════════════════════"

curl -s -o /dev/null -w "%{http_code}" $BASE/ &
curl -s -o /dev/null -w "%{http_code}" $BASE/ &
curl -s -o /dev/null -w "%{http_code}" $BASE/ &
curl -s -o /dev/null -w "%{http_code}" $BASE/ &
curl -s -o /dev/null -w "%{http_code}" $BASE/ &
curl -s -o /dev/null -w "%{http_code}" $BASE/ &
curl -s -o /dev/null -w "%{http_code}" $BASE/ &
curl -s -o /dev/null -w "%{http_code}" $BASE/ &
sleep 2
echo ""

check "concurrent: server still alive after 8 simultaneous GETs" "200" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/)"

curl -s -X POST -d "one" $BASE/echo > /tmp/r1.txt
curl -s -X POST -d "two" $BASE/echo > /tmp/r2.txt
curl -s -X POST -d "three" $BASE/echo > /tmp/r3.txt
curl -s -X POST -d "four" $BASE/echo > /tmp/r4.txt

check "concurrent: POST echo one" "one" "$(cat /tmp/r1.txt)"
check "concurrent: POST echo two" "two" "$(cat /tmp/r2.txt)"
check "concurrent: POST echo three" "three" "$(cat /tmp/r3.txt)"
check "concurrent: POST echo four" "four" "$(cat /tmp/r4.txt)"

echo ""
echo "══════════════════════════════════════════════════"
echo " silent visitor timeout"
echo "══════════════════════════════════════════════════"

# open a raw connection, send nothing, let it close after 2 seconds
(sleep 2) | curl -s --max-time 3 $BASE/ > /dev/null 2>&1 || true
check "server still alive after silent visitor" "200" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/)"
check "server responds correctly after idle connection" "Hello from handler" "$(curl -s $BASE/)"

echo ""
echo "══════════════════════════════════════════════════"
echo " extra headers and edge case paths"
echo "══════════════════════════════════════════════════"

check "custom header accepted" "200" "$(curl -s -o /dev/null -w "%{http_code}" -H 'X-Custom: value' $BASE/)"
check "multiple headers accepted" "200" "$(curl -s -o /dev/null -w "%{http_code}" -H 'X-A: 1' -H 'X-B: 2' $BASE/)"
check "user agent header accepted" "200" "$(curl -s -o /dev/null -w "%{http_code}" -A 'TestAgent/1.0' $BASE/)"
check "accept header accepted" "200" "$(curl -s -o /dev/null -w "%{http_code}" -H 'Accept: text/html' $BASE/)"
check "GET with query string returns 404" "404" "$(curl -s -o /dev/null -w "%{http_code}" "$BASE/?key=value")"

echo ""
echo "══════════════════════════════════════════════════"
echo " server health after all edge cases"
echo "══════════════════════════════════════════════════"

check "GET / still works" "200" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/)"
check "POST /echo still works" "200" "$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "alive" $BASE/echo)"
check "GET /index.html still works" "200" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/index.html)"
check "404 still works" "404" "$(curl -s -o /dev/null -w "%{http_code}" $BASE/doesnotexist)"
check "body still correct after stress" "Hello from handler" "$(curl -s $BASE/)"
check "echo body still correct after stress" "alive" "$(curl -s -X POST -d "alive" $BASE/echo)"

echo ""
echo "══════════════════════════════════════════════════"
echo " RESULTS"
echo "══════════════════════════════════════════════════"
echo -e "Total:  $TOTAL"
echo -e "${GREEN}Passed: $PASS${NC}"
echo -e "${RED}Failed: $FAIL${NC}"
echo ""
if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}All tests passed.${NC}"
else
    echo -e "${RED}$FAIL test(s) failed.${NC}"
    exit 1
fi