
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
#
# build 
# 
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

include_directories(
   ${PROJECT_SOURCE_DIR}/OpcUaEddl
)

#file(GLOB OpcUaEddl_SRC ${PROJECT_SOURCE_DIR}/OpcUaEddl/*.cpp)

LIST(APPEND OpcUaEddl_SRC
  ${PROJECT_SOURCE_DIR}/OpcUaEddl/OpcUaEddlLib.cpp
  ${PROJECT_SOURCE_DIR}/OpcUaEddl/OpcUaEddlLib.h
  ${PROJECT_SOURCE_DIR}/OpcUaEddl/EddlParser.cpp
  ${PROJECT_SOURCE_DIR}/OpcUaEddl/EddlParser.h
  ${PROJECT_SOURCE_DIR}/OpcUaEddl/EddlParserStruct.h
  ${PROJECT_SOURCE_DIR}/OpcUaEddl/EddlParserUtils.h
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



