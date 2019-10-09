# README - how to use this file
# In order to build the docker images for development (i.e. build stage) or
# the light-weight run-time image (i.e. run stage) you must map a local copy
# of the swmr-testapp sources into the image /src/swmr-testapp 
# This is achieved by adding a copy of the host . dir into the image in /src/swmr-testapp/
# When running the development image (build stage) you must map the host swmr-testapp dir
# into /src/swmr-testapp/ using the -v flag to docker run.
#
# To build a development/build (large) image:
#   docker build --target build -t build-swmr-testapp .
# Run the development/build image to rebuilt/test/tweak:
#   docker run --rm -it build-swmr-testapp
#
# To build the runtime image:
#   docker build --target run -t swmr-testapp .
# The runtime image comes with a default CMD to run the swmr writer. The output destination
# directory is specified with the environment variable SWMR_WRITE_DEST and the number of (4MB)
# image frames to write is set with the variable SWMR_WRITE_NFRAMES.
# Run like this for example, note the mapping fo /data:
#   docker run --rm -it -v `pwd`/data:/data --env SWMR_WRITE_DEST=/data swmr-testapp
#

FROM centos:7 as build

# Set the working directory to /app
WORKDIR /src

RUN yum -y update && yum -y clean all &&\
    yum groupinstall -y 'Development Tools' &&\
    yum install -y boost boost-devel cmake log4cxx-devel apr-util-devel zlib-devel epel-release &&\
    yum -y clean all && rm -rf /var/cache/yum &&\
    cd /src/ &&\
        curl -L -O https://www.hdfgroup.org/ftp/HDF5/releases/hdf5-1.10/hdf5-1.10.5/src/hdf5-1.10.5.tar.bz2 &&\
        tar -jxf hdf5-1.10.5.tar.bz2 &&\
        mkdir -p /src/build-hdf5-1.10 && cd /src/build-hdf5-1.10 &&\
        /src/hdf5-1.10.5/configure --prefix=/usr/local && make >> /src/hdf5build.log 2>&1 && make install &&\
    cd / && rm -rf /src/build* /src/*.tar*


WORKDIR /src/

# Copy the host . dir into src/swmr-testapp
ADD . /src/swmr-testapp

RUN git --git-dir=/src/swmr-testapp/.git --work-tree=/src/swmr-testapp/ branch &&\
    git --git-dir=/src/swmr-testapp/.git --work-tree=/src/swmr-testapp/ remote -v &&\
    mkdir -p /src/build-swmr-testapp && cd /src/build-swmr-testapp &&\
    cmake -DLOG4CXX_ROOT_DIR=/usr/ -DHDF5_ROOT=/usr/local/ /src/swmr-testapp/ &&\
    make VERBOSE=1 && make install


FROM centos:7 as run

WORKDIR /root
COPY --from=build /usr/local/ /usr/local/
RUN yum -y update &&\
    yum -y install boost log4cxx apr-util zlib epel-release &&\
    yum -y clean all && rm -rf /var/cache/yum
ENV SWMR_WRITE_DEST /
ENV SWMR_WRITE_NFRAMES 20
CMD echo ${SWMR_WRITE_DEST}/$HOSTNAME.log && swmr write -f /usr/local/data/img_4M.h5 -n${SWMR_WRITE_NFRAMES} --direct ${SWMR_WRITE_DEST}/$HOSTNAME.h5 >> ${SWMR_WRITE_DEST}/$HOSTNAME.log
