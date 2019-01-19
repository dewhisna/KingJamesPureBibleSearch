//
// Bible Text Database Interface
//

#ifndef BIBLE_TEXT_H_
#define BIBLE_TEXT_H_

#include <dbstruct.h>

#include <QString>

// ============================================================================

//
// MasoreticPentateuch
// https://www.crosswire.org/sword/modules/ModInfo.jsp?modName=SPMT
//	Module Identifier		SPMT
//	Book Name				Masoretic Text parallel to the Samaritan Pentateuch
//	Module Type				Bible
//	Language				he
//	Module Version			1.0 (2012-06-27)
//	Minimum SWORD Version	1.6.3
//	Download Size			1.09 MB
//	Install Size			1.29 MB
//	About					Masoretic Text parallel to the Samaritan Pentateuch
//							with Morphology, Transliteration, Strongs Concordance
//							Numbers and Idiomatic Translation
//							Digitized and prepared by Aleksandr Sigalov.
//							Text was compiled by manually and electronically
//							comparing several "kosher" Torah Scrolls that are
//							currently in use in the Synagogues. Resulting text is
//							the most precise that you can currently find on-line
//							and should match 99% of Scrolls and Tikkun Stams
//							(Scroll manuscripts). Verse numbering and morphology
//							is based on the Leningrad Codex. Text and morphology
//							was also electronically compared with WLC 4.14
//							[http://www.tanach.us/Tanach.xml#Home].
//	Distribution License	Copyrighted; Free non-commercial distribution
//
class MasoreticPentateuch : public CBibleDatabasePtr
{
private:
	MasoreticPentateuch();		// Only creatable through instance

public:
	static const CBibleDatabase *instance();
	static QString databasePath();
};

// ============================================================================

#endif	// BIBLE_TEXT_H_
