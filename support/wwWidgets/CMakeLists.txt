##*****************************************************************************
##
## Copyright (C) 2012-2022 Donna Whisnant, a.k.a. Dewtronics.
## Contact: http://www.dewtronics.com/
##
## This file is part of the KJVCanOpener Application as originally written
## and developed for Bethel Church, Festus, MO.
##
## GNU General Public License Usage
## This file may be used under the terms of the GNU General Public License
## version 3.0 as published by the Free Software Foundation and appearing
## in the file gpl-3.0.txt included in the packaging of this file. Please
## review the following information to ensure the GNU General Public License
## version 3.0 requirements will be met:
## http://www.gnu.org/copyleft/gpl.html.
##
## Other Usage
## Alternatively, this file may be used in accordance with the terms and
## conditions contained in a signed written agreement between you and
## Dewtronics.
##
##*****************************************************************************

cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# =============================================================================

option(wwWidgets_QwwBreadCrumb "Enable QwwBreadCrumb" OFF)
option(wwWidgets_QwwButtonLineEdit "Enable QwwButtonLineEdit" ON)
option(wwWidgets_QwwClearLineEdit "Enable QwwClearLineEdit" ON)
option(wwWidgets_QwwColorButton "Enable QwwColorButton" ON)
option(wwWidgets_QwwColorComboBox "Enable QwwColorComboBox" ON)
option(wwWidgets_QwwConfigWidget "Enable QwwConfigWidget" ON)
option(wwWidgets_QwwFileChooser "Enable QwwFileChooser" ON)
option(wwWidgets_QwwHueSatPicker "Enable QwwHueSatPicker" ON)
option(wwWidgets_QwwHueSatRadialPicker "Enable QwwHueSatRadialPicker" ON)
option(wwWidgets_QwwLed "Enable QwwLed" ON)
option(wwWidgets_QwwListNavigator "Enable QwwListNavigator" ON)
option(wwWidgets_QwwListWidget "Enable QwwListWidget" ON)
option(wwWidgets_QwwLoginBox "Enable QwwLoginBox" ON)
option(wwWidgets_QwwLongSpinBox "Enable QwwLongSpinBox" ON)
option(wwWidgets_QwwNavigationBar "Enable QwwNavigationBar" ON)
option(wwWidgets_QwwNumPad "Enable QwwNumPad" ON)
option(wwWidgets_QwwResetLineEdit "Enable QwwResetLineEdit" ON)
option(wwWidgets_QwwRichTextButton "Enable QwwRichTextButton" ON)
option(wwWidgets_QwwRichTextEdit "Enable QwwRichTextEdit" ON)
option(wwWidgets_QwwTaskPanel "Enable QwwTaskPanel" ON)
option(wwWidgets_QwwTextSpinBox "Enable QwwTextSpinBox" ON)
option(wwWidgets_QwwTipWidget "Enable QwwTipWidget" ON)
option(wwWidgets_QwwTwoColorIndicator "Enable QwwTwoColorIndicator" ON)

# =============================================================================

set(wwWidgets_SHARED_HEADERS
	widgets/wwglobal.h
)
set(wwWidgets_PRIVATE_HEADERS
	widgets/colormodel.h
	widgets/wwglobal_p.h
)
set(wwWidgets_FORMS)
set(wwWidgets_SOURCES
	widgets/colormodel.cpp
	widgets/wwglobal_p.cpp
)

# =============================================================================

if(wwWidgets_QwwBreadCrumb)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwbreadcrumb/QwwBreadCrumb
		widgets/qwwbreadcrumb/qwwbreadcrumb.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwbreadcrumb/qwwbreadcrumb.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwButtonLineEdit)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwbuttonlineedit/QwwButtonLineEdit
		widgets/qwwbuttonlineedit/qwwbuttonlineedit.h
	)
	list(APPEND wwWidgets_PRIVATE_HEADERS
		widgets/qwwbuttonlineedit/qwwbuttonlineedit_p.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwbuttonlineedit/qwwbuttonlineedit.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwClearLineEdit)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwclearlineedit/QwwClearLineEdit
		widgets/qwwclearlineedit/qwwclearlineedit.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwclearlineedit/qwwclearlineedit.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwColorButton)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwcolorbutton/QwwColorButton
		widgets/qwwcolorbutton/qwwcolorbutton.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwcolorbutton/qwwcolorbutton.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwColorComboBox)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwcolorcombobox/QwwColorComboBox
		widgets/qwwcolorcombobox/qwwcolorcombobox.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwcolorcombobox/qwwcolorcombobox.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwConfigWidget)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwconfigwidget/QwwConfigWidget
		widgets/qwwconfigwidget/qwwconfigwidget.h
	)
	list(APPEND wwWidgets_PRIVATE_HEADERS
		widgets/qwwconfigwidget/qwwconfigwidget_p.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwconfigwidget/qwwconfigwidget.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwFileChooser)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwfilechooser/QwwFileChooser
		widgets/qwwfilechooser/qwwfilechooser.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwfilechooser/qwwfilechooser.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwHueSatPicker)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwhuesatpicker/QwwHueSatPicker
		widgets/qwwhuesatpicker/qwwhuesatpicker.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwhuesatpicker/qwwhuesatpicker.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwHueSatRadialPicker)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwhuesatradialpicker/QwwHueSatRadialPicker
		widgets/qwwhuesatradialpicker/qwwhuesatradialpicker.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwhuesatradialpicker/qwwhuesatradialpicker.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwLed)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwled/QwwLed
		widgets/qwwled/qwwled.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwled/qwwled.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwListNavigator)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwlistnavigator/QwwListNavigator
		widgets/qwwlistnavigator/qwwlistnavigator.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwlistnavigator/qwwlistnavigator.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwListWidget)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwlistwidget/QwwListWidget
		widgets/qwwlistwidget/qwwlistwidget.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwlistwidget/qwwlistwidget.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwLoginBox)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwloginbox/QwwLoginBox
		widgets/qwwloginbox/qwwloginbox.h
	)
	list(APPEND wwWidgets_FORMS
		widgets/qwwloginbox/loginbox.ui
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwloginbox/qwwloginbox.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwLongSpinBox)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwlongspinbox/QwwLongSpinBox
		widgets/qwwlongspinbox/qwwlongspinbox.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwlongspinbox/qwwlongspinbox.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwNavigationBar)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwnavigationbar/QwwNavigationBar
		widgets/qwwnavigationbar/qwwnavigationbar.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwnavigationbar/qwwnavigationbar.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwNumPad)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwnumpad/QwwNumPad
		widgets/qwwnumpad/qwwnumpad.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwnumpad/qwwnumpad.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwResetLineEdit)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwresetlineedit/QwwResetLineEdit
		widgets/qwwresetlineedit/qwwresetlineedit.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwresetlineedit/qwwresetlineedit.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwRichTextButton)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwrichtextbutton/QwwRichTextButton
		widgets/qwwrichtextbutton/qwwrichtextbutton.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwrichtextbutton/qwwrichtextbutton.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwRichTextEdit)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwrichtextedit/QwwRichTextEdit
		widgets/qwwrichtextedit/qwwrichtextedit.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwrichtextedit/qwwrichtextedit.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwTaskPanel)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwtaskpanel/QwwTaskPanel
		widgets/qwwtaskpanel/qwwtaskpanel.h
	)
	list(APPEND wwWidgets_PRIVATE_HEADERS
		widgets/qwwtaskpanel/qwwtaskpanel_p.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwtaskpanel/qwwtaskpanel.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwTextSpinBox)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwtextspinbox/QwwTextSpinBox
		widgets/qwwtextspinbox/qwwtextspinbox.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwtextspinbox/qwwtextspinbox.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwTipWidget)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwtipwidget/QwwTipWidget
		widgets/qwwtipwidget/qwwtipwidget.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwtipwidget/qwwtipwidget.cpp
	)
endif()

# -----------------------------------------------------------------------------

if(wwWidgets_QwwTwoColorIndicator)
	list(APPEND wwWidgets_SHARED_HEADERS
		widgets/qwwtwocolorindicator/QwwTwoColorIndicator
		widgets/qwwtwocolorindicator/qwwtwocolorindicator.h
	)
	list(APPEND wwWidgets_SOURCES
		widgets/qwwtwocolorindicator/qwwtwocolorindicator.cpp
	)
endif()

# =============================================================================

set(TRANSLATIONS_WWWIDGETS
	"translations/wwwidgets_en.ts"
	"translations/wwwidgets_fr.ts"
	"translations/wwwidgets_es.ts"
	"translations/wwwidgets_de.ts"
)

# =============================================================================

if(WIN32)
	set(MY_COMPILE_OPTIONS "/W3")
else()
	set(MY_COMPILE_OPTIONS "-Wall" "-Wextra")
endif()

# -----------------------------------------------------------------------------


add_library(wwWidgets STATIC ${wwWidgets_SOURCES} ${wwWidgets_FORMS} ${wwWidgets_PRIVATE_HEADERS} ${wwWidgets_SHARED_HEADERS})
set_target_properties(wwWidgets PROPERTIES PUBLIC_HEADER "${wwWidgets_SHARED_HEADERS}")
target_compile_options(wwWidgets PUBLIC ${MY_COMPILE_OPTIONS})
target_link_libraries(wwWidgets
	Qt${QT_VERSION_MAJOR}::Core
	Qt${QT_VERSION_MAJOR}::Widgets
)
target_include_directories(wwWidgets PRIVATE
	widgets
	widgets/qwwbreadcrumb
	widgets/qwwbuttonlineedit
	widgets/qwwclearlineedit
	widgets/qwwcolorbutton
	widgets/qwwcolorcombobox
	widgets/qwwconfigwidget
	widgets/qwwfilechooser
	widgets/qwwhuesatpicker
	widgets/qwwhuesatradialpicker
	widgets/qwwled
	widgets/qwwlistnavigator
	widgets/qwwlistwidget
	widgets/qwwloginbox
	widgets/qwwlongspinbox
	widgets/qwwnavigationbar
	widgets/qwwnumpad
	widgets/qwwresetlineedit
	widgets/qwwrichtextbutton
	widgets/qwwrichtextedit
	widgets/qwwtaskpanel
	widgets/qwwtextspinbox
	widgets/qwwtipwidget
	widgets/qwwtwocolorindicator
)

target_include_directories(wwWidgets SYSTEM INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwbreadcrumb>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwbuttonlineedit>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwclearlineedit>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwcolorbutton>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwcolorcombobox>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwconfigwidget>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwfilechooser>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwhuesatpicker>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwhuesatradialpicker>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwled>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwlistnavigator>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwlistwidget>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwloginbox>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwlongspinbox>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwnavigationbar>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwnumpad>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwresetlineedit>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwrichtextbutton>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwrichtextedit>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwtaskpanel>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwtextspinbox>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwtipwidget>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwwtwocolorindicator>
	$<INSTALL_INTERFACE:wwWidgets/include>  # <prefix>/wwWidgets/include
)

if(DEPRECATION_ENABLE_WARNINGS)
	target_compile_definitions(wwWidgets PRIVATE
		QT_DEPRECATED_WARNINGS
	)
endif()

if(DEPRECATION_DISABLE_APIS)
	target_compile_definitions(wwWidgets PRIVATE
		QT_DISABLE_DEPRECATED_BEFORE=0x060000			# disables all the APIs deprecated before Qt 6.0.0
	)
endif()

set_source_files_properties(${TRANSLATIONS_WWWIDGETS}
   	PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")

if(QT_VERSION_MAJOR EQUAL 5)
	qt5_add_translation(TRANSLATION_QM_FILES ${TRANSLATIONS_WWWIDGETS})
	target_sources(wwWidgets PRIVATE ${TRANSLATION_QM_FILES})
else()
	qt_add_lrelease(wwWidgets TS_FILES ${TRANSLATIONS_WWWIDGETS}
			QM_FILES_OUTPUT_VARIABLE TRANSLATION_QM_FILES)
endif()

install(TARGETS wwWidgets
		EXPORT wwWidgets
		ARCHIVE DESTINATION "wwWidgets/lib"
		PUBLIC_HEADER DESTINATION "wwWidgets/include"
)
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/translations" DESTINATION wwWidgets/)
install(EXPORT wwWidgets DESTINATION wwWidgets/cmake)


