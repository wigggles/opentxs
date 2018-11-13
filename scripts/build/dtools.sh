#!/bin/bash -x
GITREPO=`git rev-parse --show-toplevel`
if [ `pwd` != ${GITREPO} ]; then
   echo "must run from the top level of the git repo"
   exit 1
fi
DOCKERFILE=Dockerfiles/Dockerfile.tools
DFILEHASH=`git log --format=%h ${DOCKERFILE} | head -1`
NAME="opentxs_build:${DFILEHASH}"
if [ `docker images ${NAME} | wc -l` -lt 2 ] ; then
    docker build -f ${DOCKERFILE} -t "${NAME}" `dirname ${DOCKERFILE}`
fi
set -e
#docker run -t -v `pwd`:/src --user $(id -u):$(id -g) "${NAME}" /bin/sh -c "$@"
# run as root
#DMOUNT=${DMOUNT:="-v `pwd`:"}
#docker run --rm ${DMOUNT}/src "${NAME}" /bin/sh -c "$@"
docker run --rm -v `pwd`:/src "${NAME}" /bin/sh -c "$@"
OWNER=$(id -u):$(id -g)
# ..but chown everything back to the user
docker run --rm -v `pwd`:/src "${NAME}" /bin/sh -c "chown -R ${OWNER} ."
