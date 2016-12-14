
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
#
# build 
# 
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../niki/opcua-sensor-interface)

LIST(APPEND OpcUaEddl_SRC
  ${PROJECT_SOURCE_DIR}/../../niki/eddl/OpcUaEddlLib.cpp
  ${PROJECT_SOURCE_DIR}/../../niki/eddl/OpcUaEddlLib.h
  ${PROJECT_SOURCE_DIR}/../../niki/eddl/EddlParser.cpp
  ${PROJECT_SOURCE_DIR}/../../niki/eddl/EddlParser.h
  ${PROJECT_SOURCE_DIR}/../../niki/eddl/EddlParserStruct.h
  ${PROJECT_SOURCE_DIR}/../../niki/eddl/EddlParserUtils.h
)

add_library(OpcUaEddl SHARED ${OpcUaEddl_SRC})

target_link_libraries(
    OpcUaEddl
    OpcUaStackCore
    OpcUaStackServer
    OpcUaStackClient
    OpcUaSensorInterface
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
#file(GLOB INC_OpcUaEddl ${PROJECT_SOURCE_DIR}/OpcUaEddl/*.h)


install(TARGETS OpcUaEddl DESTINATION ${LIB_DIR}/OpcUaStack)
install(FILES ${PROJECT_SOURCE_DIR}/OpcUaEddl/Config/OpcUaServer.xml DESTINATION ${CONF_DIR}/OpcUaStack) 
install(FILES ${PROJECT_SOURCE_DIR}/OpcUaEddl/Config/Opc.Ua.NodeSet.xml DESTINATION ${CONF_DIR}/OpcUaStack) 
install(FILES ${PROJECT_SOURCE_DIR}/OpcUaEddl/Config/Opc.Ua.NodeSetRaspberry.xml DESTINATION ${CONF_DIR}/OpcUaStack) 
install(FILES ${PROJECT_SOURCE_DIR}/OpcUaEddl/Config/Opc.Ua.NodeSetTest.xml DESTINATION ${CONF_DIR}/OpcUaStack) 
install(FILES ${INC_OpcUaEddl} DESTINATION ${INC_DIR}/OpcUaStack/OpcUaEddl)



