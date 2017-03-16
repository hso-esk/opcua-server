
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
#
# build 
# 
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../niki/opcua-sensor-interface)

LIST(APPEND OpcUaIPSO_SRC
  ${PROJECT_SOURCE_DIR}/../../niki/ipso/OpcUaIPSOLib.cpp
  ${PROJECT_SOURCE_DIR}/../../niki/ipso/OpcUaIPSOLib.h
  ${PROJECT_SOURCE_DIR}/../../niki/ipso/IPSOParser.cpp
  ${PROJECT_SOURCE_DIR}/../../niki/ipso/IPSOParser.h
  ${PROJECT_SOURCE_DIR}/../../niki/ipso/OpcUaLWM2MObserver.cpp
  ${PROJECT_SOURCE_DIR}/../../niki/ipso/OpcUaLWM2MObserver.h
)

add_library(OpcUaIPSO SHARED ${OpcUaIPSO_SRC})

target_link_libraries(
    OpcUaIPSO
    OpcUaStackCore
    OpcUaStackServer
    OpcUaStackClient
    OpcUaSensorInterface
    OpcUalwm2m
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



