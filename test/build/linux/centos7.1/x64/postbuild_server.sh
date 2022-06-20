#!/bin/bash
shell_dir=$(cd "$(dirname "${0}")";pwd)/
project=$1
target=$2

cd ${shell_dir}
mkdir -p server/lib
\cp ${target} server/
\cp run.sh server/
\cp debug.sh server/
\cp memcheck.sh server/

if [ -d "../../../../conf" ]; then
    \cp ../../../../conf/* server/ -r
fi

cd ${shell_dir}server/
sed -i "2i project=${project}" run.sh
sed -i "2i project=${project}" debug.sh
sed -i "2i project=${project}" memcheck.sh
chmod +x *.sh