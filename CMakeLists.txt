project(qaptupdator)
cmake_minimum_required(VERSION 2.6.0)

set(QT_MIN_VERSION "5.11.0")
set(KF5_MIN_VERSION "5.54.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# QT += qml quick

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Qml Widgets DBus Core Quick Test Gui QuickControls2)
find_package(KF5 REQUIRED COMPONENTS Config Auth I18n CoreAddons XmlGui DBusAddons Declarative Plasma Notifications KIO)
# DBusAddons
include(FindPkgConfig REQUIRED)
find_package(QApt REQUIRED)
find_package(DebconfKDE REQUIRED)

set(CMAKE_CXX_FLAGS "-o0")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(qaptupdator_SRCS main.cpp
	#qapttest.cpp
	#cacheupdatewidget.cpp
	#commitwidget.cpp
	UpgradeModel.cpp
	resource.qrc
 )

include( FindPkgConfig )

include_directories(.)
add_executable(qaptupdator ${qaptupdator_SRCS})

target_link_libraries(qaptupdator
	DebconfKDE::Main
	KF5::KIOCore
	KF5::KIOWidgets
	KF5::XmlGui
	KF5::I18n
	KF5::Notifications
	KF5::CoreAddons
	KF5::DBusAddons
	QApt::Main
	Qt5::Qml
)

install(TARGETS qaptupdator ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(FILES org.jingos.qaptupdator.desktop DESTINATION ${KDE_INSTALL_APPDIR})
ki18n_install(po)
