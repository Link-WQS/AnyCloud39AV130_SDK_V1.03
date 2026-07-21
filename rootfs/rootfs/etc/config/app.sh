#!/bin/sh

if [ -f "/etc/config/DisableApp" ]; then
    echo "DisableApp eixst, so not start app"
else
    /app/two_wire_system &
fi
