ADD_SUBDIRECTORY( actions )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )

INCLUDE(KDE3Macros)

########### install files ###############


KDE3_INSTALL_ICONS( hicolor )

