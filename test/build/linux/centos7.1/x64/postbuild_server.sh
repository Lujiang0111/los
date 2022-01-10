#!/bin/bash
SHELL_FOLDER=$(cd "$(dirname "${0}")";pwd)/
PROJECT=$1
TARGET=$2

cd ${SHELL_FOLDER}
mkdir -p server/lib
\cp ${TARGET} server/
\cp run.sh server/
\cp debug.sh server/
\cp memcheck.sh server/

cd server
sed -i "2i PROJECT=${PROJECT}" run.sh
sed -i "2i PROJECT=${PROJECT}" debug.sh
sed -i "2i PROJECT=${PROJECT}" memcheck.sh
chmod +x *.sh