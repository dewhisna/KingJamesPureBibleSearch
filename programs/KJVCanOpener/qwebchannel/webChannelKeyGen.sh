#!/bin/bash

openssl rand -out webChannelAdmin.key -hex 256

echo -n 'const char *g_pWebChannelAdminKey = "' > webChannelKeys.cpp
cat webChannelAdmin.key | tr -d '\n\r' >> webChannelKeys.cpp
echo '";' >> webChannelKeys.cpp

mkdir -p html/admin
echo -n 'var webChannelKeys = { "webChannelAdminKey": "' > html/admin/webChannelKeys.js
cat webChannelAdmin.key | tr -d '\n\r' >> html/admin/webChannelKeys.js
echo '" };' >> html/admin/webChannelKeys.js

