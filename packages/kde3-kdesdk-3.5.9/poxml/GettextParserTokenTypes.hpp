#ifndef INC_GettextParserTokenTypes_hpp_
#define INC_GettextParserTokenTypes_hpp_

/* $ANTLR 2.7.1: "gettext.g" -> "GettextParserTokenTypes.hpp"$ */
struct GettextParserTokenTypes {
	enum {
		EOF_ = 1,
		T_MSGID = 4,
		T_COMMENT = 5,
		T_STRING = 6,
		T_MSGID_PLURAL = 7,
		T_MSGSTR = 8,
		L_BRACKET = 9,
		T_INT = 10,
		R_BRACKET = 11,
		WS = 12,
		MSG_TAG = 13,
		ESC = 14,
		NULL_TREE_LOOKAHEAD = 3
	};
};
#endif /*INC_GettextParserTokenTypes_hpp_*/
