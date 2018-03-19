
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
#
# build 
# 
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

#INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../niki/opcua-sensor-interface)
#INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../niki/opcua-lwm2m-server)
#INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../niki/asneg-dbserver)

LIST(APPEND OpcUaNikiLoger_SRC
  ${PROJECT_SOURCE_DIR}/../../niki/logger/NikiLogger.cpp
  ${PROJECT_SOURCE_DIR}/../../niki/logger/NikiLogger.h
)

add_library(OpcUaNikiLoger SHARED ${OpcUaNikiLoger_SRC})

target_link_libraries(
    OpcUaNikiLoger
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



