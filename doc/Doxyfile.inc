# This file provides overrides to the Doxyfile configuration

PROJECT_NAME = Avona Reference Design
PROJECT_BRIEF = "Avona Reference Design User Guide"

PREDEFINED = __DOXYGEN__=1 
PREDEFINED += DWORD_ALIGNED=
PREDEFINED += __attribute__((weak))=
PREDEFINED += C_API= MA_C_API= C_API_START= C_API_END= EXTERN_C=

#INPUT += 

ALIASES += "beginrst=^^\verbatim embed:rst^^"
ALIASES += "endrst=\endverbatim"
