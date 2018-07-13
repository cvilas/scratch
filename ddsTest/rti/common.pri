NDDSHOME=/opt/rti_connext_dds-5.2.3

NDDSARCH = x64Linux3gcc4.8.2

NDDSLIBS = -L$${NDDSHOME}/lib/$${NDDSARCH}/ \
        -lnddscpp2 -lnddsc -lnddscore \
        -ldl -lnsl -lm -lpthread -lrt

NDDSINCLUDE = \
                $${NDDSHOME}/include/ \
                $${NDDSHOME}/include/ndds/ \
                $${NDDSHOME}/include/ndds/hpp

NDDS_DEFINES = RTI_UNIX RTI_LINUX RTI_64BIT

COMMON_CXX = g++-4.8
COMMON_LINKER = g++-4.8
COMMON_CXXFLAGS = -m64 -Wall -std=c++11
COMMON_LFLAGS = -m64 -Wl,--no-as-needed -pthread -std=c++11

