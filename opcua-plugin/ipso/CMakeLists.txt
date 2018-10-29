
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
#
# build 
# 
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../opcua-plugin/opcua-sensor-interface)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../opcua-plugin/opcua-lwm2m-server)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../opcua-plugin/database)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../opcua-plugin/logger)

LIST(APPEND OpcUaIPSO_SRC
  ${PROJECT_SOURCE_DIR}/../../opcua-plugin/ipso/OpcUaLWM2MLib.cpp
  ${PROJECT_SOURCE_DIR}/../../opcua-plugin/ipso/OpcUaLWM2MLib.h
  ${PROJECT_SOURCE_DIR}/../../opcua-plugin/ipso/IPSOParser.cpp
  ${PROJECT_SOURCE_DIR}/../../opcua-plugin/ipso/IPSOParser.h
)

add_library(OpcUaIPSO SHARED ${OpcUaIPSO_SRC})

target_link_libraries(
    OpcUaIPSO
    OpcUaStackCore
    OpcUaStackServer
    OpcUaStackClient
    OpcUaSensorInterface
    OpcUalwm2m
    OpcUaDB
    OpcUaLogger
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
)
