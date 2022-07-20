#!/bin/bash
shell_path=$(cd "$(dirname "$0")";pwd)/
library_path=${shell_path}lib/

ldconfig -n ${library_path}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${library_path}
ulimit -n 65536

cd ${library_path}
for file in *.so.*
do
    if [[ ${file}"x" != "x" ]]; then
        realname=$(echo ${file} | rev |cut -d '/' -f 1 | rev)
        libname=$(echo ${realname} |cut -d '.' -f 1)
        if [ ! -f ${libname}.so ]; then
            ln -sf ${realname} ${libname}.so
        fi
    fi
done

cd ${shell_path}
if [ -f runlog ]; then
    runlog_size=$(ls -l runlog | awk '{print $5}')
    if [ ${runlog_size} -gt 10000000 ]; then
            echo -e "runlog too big, restart at $(date)" > runlog
    fi
fi

function TrapSigint()
{
    :
}
trap TrapSigint 2

cd ${shell_path}
echo -e "${project} start at $(date)" >> runlog
chmod +x ${project}
./${project}
echo -e "${project} stop at $(date)" >> runlog