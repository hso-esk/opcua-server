
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
#
# build 
# 
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

#INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../opcua-plugin/opcua-sensor-interface)
#INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../opcua-plugin/opcua-lwm2m-server)
#INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../opcua-plugin/asneg-dbserver)

LIST(APPEND OpcUaLogger_SRC
  ${PROJECT_SOURCE_DIR}/../../opcua-plugin/logger/Logger.cpp
  ${PROJECT_SOURCE_DIR}/../../opcua-plugin/logger/Logger.h
)

add_library(OpcUaLogger SHARED ${OpcUaLogger_SRC})

target_link_libraries(
    OpcUaLogger
    OpcUaStackCore
    ${pthread}
    ${Boost_LIBRARIES}
)


# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
#
# install
# 
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
#file(GLOB INC_OpcUaEddl ${PROJECT_SOURCE_DIR}/OpcUaIPSO/*.h)


#install(TARGETS OpcUaEddl DESTINATION ${LIB_DIR}/OpcUaStack)
#install(FILES ${PROJECT_SOURCE_DIR}/OpcUaIPSO/Config/OpcUaServer.xml DESTINATION ${CONF_DIR}/OpcUaStack) 
#install(FILES ${PROJECT_SOURCE_DIR}/OpcUaIPSO/Config/Opc.Ua.NodeSet.xml DESTINATION ${CONF_DIR}/OpcUaStack) 
#install(FILES ${PROJECT_SOURCE_DIR}/OpcUaIPSO/Config/Opc.Ua.NodeSetRaspberry.xml DESTINATION ${CONF_DIR}/OpcUaStack) 
#install(FILES ${PROJECT_SOURCE_DIR}/OpcUaIPSO/Config/Opc.Ua.NodeSetTest.xml DESTINATION ${CONF_DIR}/OpcUaStack) 
#install(FILES ${INC_OpcUaEddl} DESTINATION ${INC_DIR}/OpcUaStack/OpcUaIPSO)



