
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
#
# prebuild
#
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
configure_file(
    "${PROJECT_SOURCE_DIR}/../../asneg-dbserver/OpcUaDB/src/OpcUaDB/Build/BuildConfig.h.in"
    "${PROJECT_BINARY_DIR}/BuildConfig_temp.h"
)

configure_file(
    "${PROJECT_SOURCE_DIR}/../../asneg-dbserver/OpcUaDB/src/OpcUaDB/linux/etc/init/OpcUaDB.conf.in"
    "${PROJECT_BINARY_DIR}/OpcUaDB.conf"
)

configure_file(
    "${PROJECT_SOURCE_DIR}/../../asneg-dbserver/OpcUaDB/src/OpcUaDB/linux/etc/init.d/OpcUaDB.in"
    "${PROJECT_BINARY_DIR}/OpcUaDB"
)

configure_file(
    "${PROJECT_SOURCE_DIR}/../../asneg-dbserver/OpcUaDB/src/OpcUaDB/linux/usr/lib/systemd/system/OpcUaDB.service.in"
    "${PROJECT_BINARY_DIR}/OpcUaDB.service"
)

FILE(READ "${PROJECT_BINARY_DIR}/BuildConfig_temp.h" tempfile)
FILE(REMOVE "${PROJECT_BINARY_DIR}/BuildConfig_temp.h")
FILE(APPEND "${PROJECT_BINARY_DIR}/BuildConfig.h" "${tempfile}")

# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
#
# build
#
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../asneg-dbserver/OpcUaDB/src)


LIST(APPEND OpcUaDB_SRC
  ${PROJECT_SOURCE_DIR}/../../asneg-dbserver/OpcUaDB/src/OpcUaDB/DBServer/DatabaseConfig.cpp
  ${PROJECT_SOURCE_DIR}/../../asneg-dbserver/OpcUaDB/src/OpcUaDB/DBServer/DatabaseConfig.h
  ${PROJECT_SOURCE_DIR}/../../asneg-dbserver/OpcUaDB/src/OpcUaDB/DBServer/DBModelConfig.cpp
  ${PROJECT_SOURCE_DIR}/../../asneg-dbserver/OpcUaDB/src/OpcUaDB/DBServer/DBModelConfig.h
  ${PROJECT_SOURCE_DIR}/../../asneg-dbserver/OpcUaDB/src/OpcUaDB/DBServer/DBServer.cpp
  ${PROJECT_SOURCE_DIR}/../../asneg-dbserver/OpcUaDB/src/OpcUaDB/DBServer/DBServer.h
  ${PROJECT_SOURCE_DIR}/../../asneg-dbserver/OpcUaDB/src/OpcUaDB/DBServer/OpcUaAccessConfig.cpp
  ${PROJECT_SOURCE_DIR}/../../asneg-dbserver/OpcUaDB/src/OpcUaDB/DBServer/OpcUaAccessConfig.h
  ${PROJECT_SOURCE_DIR}/../../asneg-dbserver/OpcUaDB/src/OpcUaDB/odbc/Connection.cpp
  ${PROJECT_SOURCE_DIR}/../../asneg-dbserver/OpcUaDB/src/OpcUaDB/odbc/Connection.h
  ${PROJECT_SOURCE_DIR}/../../niki/asneg-dbserver/NikiDatabaseServer.cpp
  ${PROJECT_SOURCE_DIR}/../../niki/asneg-dbserver/NikiDatabaseServer.h
  ${PROJECT_SOURCE_DIR}/../../niki/asneg-dbserver/NikiDatabaseConfig.cpp
  ${PROJECT_SOURCE_DIR}/../../niki/asneg-dbserver/NikiDatabaseConfig.h
  ${PROJECT_SOURCE_DIR}/../../niki/asneg-dbserver/NikiDbModelConfig.cpp
  ${PROJECT_SOURCE_DIR}/../../niki/asneg-dbserver/NikiDbModelConfig.h
)

add_library(OpcUaDB SHARED ${OpcUaDB_SRC})

target_link_libraries(
    OpcUaLogger
    OpcUaDB
    ${pthread}
    OpcUaStackCore
    OpcUaStackServer
    OpcUaStackClient
    ${Boost_LIBRARIES}
    libodbc.so
)

