FROM pwncollege-challenge-2304

USER root
RUN echo hi

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get update && xargs apt-get install -yqq <<EOF
    zip
EOF

COPY sbin/none              /usr/local/sbin/none
COPY sbin/address           /usr/local/sbin/address
COPY sbin/memory            /usr/local/sbin/memory
COPY sbin/thread            /usr/local/sbin/thread
COPY sbin/undefined         /usr/local/sbin/undefined
COPY sbin/cover             /usr/local/sbin/cover
COPY sbin/inspector         /usr/local/sbin/inspector
COPY sbin/find-drv          /usr/local/sbin/find-drv
COPY sbin/e                 /usr/local/sbin/e
COPY sbin/loc-calc          /usr/local/sbin/loc-calc
COPY sbin/reports           /usr/local/sbin/reports
COPY sbin/fi                /usr/local/sbin/fi
COPY sbin/loc_count         /usr/local/sbin/loc_count
COPY sbin/loc_count2        /usr/local/sbin/loc_count2
COPY sbin/loc_count120      /usr/local/sbin/loc_count120
COPY sbin/test-crashes      /usr/local/sbin/test-crashes
COPY sbin/announce          /usr/local/sbin/announce
COPY sbin/compile.patch     /usr/local/sbin/compile.patch

USER hacker