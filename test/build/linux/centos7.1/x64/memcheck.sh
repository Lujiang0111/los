#!/bin/bash
SHELL_FOLDER=$(cd "$(dirname "${0}")";pwd)/
LIBRARY=${SHELL_FOLDER}lib/

ldconfig -n ${LIBRARY}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${LIBRARY}
ulimit -n 65536

cd ${LIBRARY}
for file in *.so.*
do
	if [[ ${file}"x" != "x" ]]; then
		realname=`echo ${file} | rev |cut -d '/' -f 1 | rev`
		libname=`echo ${realname} |cut -d '.' -f 1`
		if [ ! -f ${libname}.so ]; then
			ln -sf ${realname} ${libname}.so
		fi
	fi
done

cd ${SHELL_FOLDER}
LOGSIZE=1000000
if [ -f runlog ]; then
	SIZE=`ls -l runlog | awk '{print $5}'`
	if [ $SIZE -gt $LOGSIZE ]; then
        	mv runlog runlog.old
        	echo -e "runlog too big, restart at `date`" >> runlog
	fi
fi

echo -e "${PROJECT}-valgrind start at `date`" >> runlog
chmod +x ${PROJECT}
valgrind --time-stamp=yes --log-file=${PROJECT}-memcheck.log --leak-check=full --show-leak-kinds=all --tool=memcheck ./${PROJECT}
echo -e "${PROJECT}-valgrind stop at `date`" >> runlog