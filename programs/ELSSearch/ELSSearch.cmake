##*****************************************************************************
##
## Copyright (C) 2024-2025 Donna Whisnant, a.k.a. Dewtronics.
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

set(elssearch_QT_COMPONENTS
	Core Gui Xml Concurrent
)

set(elssearch_path ${CMAKE_CURRENT_LIST_DIR})

# =============================================================================

# Source Files:
# -------------

set(elssearch_SOURCES
	"${elssearch_path}/LetterMatrix.cpp"
	"${elssearch_path}/ELSResult.cpp"
	"${elssearch_path}/FindELS.cpp"
	# ----
	"${elssearch_path}/ELSSearchMainWindow.cpp"
	"${elssearch_path}/ELSBibleDatabaseSelectDlg.cpp"
	"${elssearch_path}/LetterMatrixTableModel.cpp"
)

# -------------------------------------

# Headers Files:
# --------------

set(elssearch_SHARED_HEADERS
	"${elssearch_path}/ELSSearchMainWindow.h"
	"${elssearch_path}/ELSBibleDatabaseSelectDlg.h"
)

set(elssearch_PRIVATE_HEADERS
	"${elssearch_path}/LetterMatrix.h"
	"${elssearch_path}/ELSResult.h"
	"${elssearch_path}/FindELS.h"
	# ----
	"${elssearch_path}/LetterMatrixTableModel.h"
)

# -------------------------------------

# Forms Files:
# ------------

set(elssearch_FORMS
	"${elssearch_path}/ELSSearchMainWindow.ui"
	"${elssearch_path}/ELSBibleDatabaseSelectDlg.ui"
)

# -------------------------------------

# Resources Files:
# ----------------

set(elssearch_RESOURCES
#	"${elssearch_path}/ELSSearch.qrc"
)

# =============================================================================

set(elssearch_link_libraries)

set(elssearch_compile_definitions
	USING_ELSSEARCH
	USE_EXTENDED_INDEXES
)

# =============================================================================

function(setup_elssearch_subtargets Target)
	target_include_directories(${Target} PRIVATE ${elssearch_path})

	target_compile_definitions(${Target} PRIVATE
		${elssearch_compile_definitions}
	)

	target_link_libraries(${Target} PRIVATE
		${elssearch_link_libraries}
	)
endfunction()

# =============================================================================


